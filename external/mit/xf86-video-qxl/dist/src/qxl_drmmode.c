/*
 * Copyright © 2007 Red Hat, Inc.
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

#include <errno.h>
#ifdef XF86DRM_MODE

#include <sys/ioctl.h>
#include "qxl_drmmode.h"
#include "X11/Xatom.h"
#include "xf86DDC.h"
#include <X11/extensions/dpmsconst.h>
#include <cursorstr.h>

#include "qxl.h"
#include "qxl_surface.h"

static void drmmode_show_cursor (xf86CrtcPtr crtc);

static void
drmmode_ConvertFromKMode(ScrnInfoPtr	scrn,
		     drmModeModeInfo *kmode,
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
drmmode_ConvertToKMode(ScrnInfoPtr	scrn,
		     drmModeModeInfo *kmode,
		     DisplayModePtr	mode)
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
//	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmmode_crtc->dpms_mode = mode;

#if 0
	/* bonghits in the randr 1.2 - uses dpms to disable crtc - bad buzz */
	if (mode == DPMSModeOff) {
//		drmModeSetCrtc(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
//			       0, 0, 0, NULL, 0, NULL);
	}
#endif
}

static Bool
drmmode_set_mode_major(xf86CrtcPtr crtc, DisplayModePtr mode,
		     Rotation rotation, int x, int y)
{
	ScrnInfoPtr pScrn = crtc->scrn;
	CursorPtr cursor;
	xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	int saved_x, saved_y;
	Rotation saved_rotation;
	DisplayModeRec saved_mode;
	uint32_t *output_ids;
	int output_count = 0;
	Bool ret = TRUE;
	int i;
	int fb_id;
	drmModeModeInfo kmode;
	int pitch;
	int height;
	qxl_screen_t *qxl = crtc->scrn->driverPrivate;

	pitch = pScrn->displayWidth * ((pScrn->bitsPerPixel + 7) >> 3);
	height = pScrn->virtualY;
	if (drmmode->fb_id == 0) {
		ret = drmModeAddFB(drmmode->fd,
				   pScrn->virtualX, height,
                                   pScrn->depth, pScrn->bitsPerPixel,
				   pitch,
				   qxl_kms_bo_get_handle(qxl->primary->bo),
                                   &drmmode->fb_id);
                if (ret < 0) {
                        ErrorF("failed to add fb\n");
                        return FALSE;
                }
        }

	saved_mode = crtc->mode;
	saved_x = crtc->x;
	saved_y = crtc->y;
	saved_rotation = crtc->rotation;

	if (mode) {
		crtc->mode = *mode;
		crtc->x = x;
		crtc->y = y;
		crtc->rotation = rotation;
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,5,99,0,0)
		crtc->transformPresent = FALSE;
#endif
	}

	output_ids = calloc(sizeof(uint32_t), xf86_config->num_output);
	if (!output_ids) {
		ret = FALSE;
		goto done;
	}

	if (mode) {
		for (i = 0; i < xf86_config->num_output; i++) {
			xf86OutputPtr output = xf86_config->output[i];
			drmmode_output_private_ptr drmmode_output;

			if (output->crtc != crtc)
				continue;

			drmmode_output = output->driver_private;
			output_ids[output_count] = drmmode_output->mode_output->connector_id;
			output_count++;
		}

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)
		crtc->funcs->gamma_set(crtc, crtc->gamma_red, crtc->gamma_green,
				       crtc->gamma_blue, crtc->gamma_size);
#endif
		
		drmmode_ConvertToKMode(crtc->scrn, &kmode, mode);

		fb_id = drmmode->fb_id;
		if (drmmode_crtc->rotate_fb_id) {
			fb_id = drmmode_crtc->rotate_fb_id;
			x = y = 0;
		}
		ret = drmModeSetCrtc(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
				     fb_id, x, y, output_ids, output_count, &kmode);
		if (ret) {
			xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
				   "failed to set mode: %s", strerror(-ret));
			return FALSE;
		} else {
			ret = TRUE;
		}

		if (crtc->scrn->pScreen)
			xf86CrtcSetScreenSubpixelOrder(crtc->scrn->pScreen);
		/* go through all the outputs and force DPMS them back on? */
		for (i = 0; i < xf86_config->num_output; i++) {
			xf86OutputPtr output = xf86_config->output[i];

			if (output->crtc != crtc)
				continue;

			output->funcs->dpms(output, DPMSModeOn);
		}
	}

#if 0
	if (pScrn->pScreen &&
		!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE))
		xf86_reload_cursors(pScrn->pScreen);
#endif

done:
	if (!ret) {
		crtc->x = saved_x;
		crtc->y = saved_y;
		crtc->rotation = saved_rotation;
		crtc->mode = saved_mode;
	}
#if defined(XF86_CRTC_VERSION) && XF86_CRTC_VERSION >= 3
	else
		crtc->active = TRUE;
#endif

        cursor = xf86_config->cursor;
        if (cursor)
            drmmode_show_cursor(crtc);

	return ret;
}

static void
drmmode_set_cursor_colors (xf86CrtcPtr crtc, int bg, int fg)
{

}

static void
drmmode_set_cursor_position (xf86CrtcPtr crtc, int x, int y)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmModeMoveCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id, x, y);
}

static void
drmmode_show_cursor (xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	uint32_t handle = qxl_kms_bo_get_handle(drmmode_crtc->cursor_bo);
	static Bool use_set_cursor2 = TRUE;

	if (use_set_cursor2) {
		xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
		CursorPtr cursor = xf86_config->cursor;
		int ret;
		ret = drmModeSetCursor2(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id, handle, 64, 64, cursor->bits->xhot, cursor->bits->yhot);
		if (ret == -EINVAL)
			use_set_cursor2 = FALSE;
		else
			return;
	}
	drmModeSetCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id, handle, 64, 64);
}

static void
drmmode_load_cursor_argb (xf86CrtcPtr crtc, CARD32 *image)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	int i;
	uint32_t *ptr;

	/* cursor should be mapped already */
	ptr = (uint32_t *)(drmmode_crtc->cursor_ptr);

	for (i = 0; i < 64 * 64; i++)
		ptr[i] = image[i];

	drmmode_show_cursor(crtc);
}

static void
drmmode_hide_cursor (xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	
	drmModeSetCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id, 0, 64, 64);
}

static void
drmmode_crtc_gamma_set(xf86CrtcPtr crtc, uint16_t *red, uint16_t *green,
                      uint16_t *blue, int size)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmModeCrtcSetGamma(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
			    size, red, green, blue);
}
static const xf86CrtcFuncsRec drmmode_crtc_funcs = {
    .dpms = drmmode_crtc_dpms,
    .set_mode_major = drmmode_set_mode_major,
    .set_cursor_colors = drmmode_set_cursor_colors,
    .set_cursor_position = drmmode_set_cursor_position,
    .show_cursor = drmmode_show_cursor,
    .hide_cursor = drmmode_hide_cursor,
    .load_cursor_argb = drmmode_load_cursor_argb,

    .gamma_set = drmmode_crtc_gamma_set,
    .destroy = NULL, /* XXX */
};

static void
drmmode_crtc_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int num)
{
	qxl_screen_t *qxl = pScrn->driverPrivate;
	xf86CrtcPtr crtc;
	drmmode_crtc_private_ptr drmmode_crtc;

	crtc = xf86CrtcCreate(pScrn, &drmmode_crtc_funcs);
	if (crtc == NULL)
		return;
	crtc->driverIsPerformingTransform = FALSE;

	drmmode_crtc = xnfcalloc(sizeof(drmmode_crtc_private_rec), 1);
	drmmode_crtc->mode_crtc = drmModeGetCrtc(drmmode->fd, drmmode->mode_res->crtcs[num]);
	drmmode_crtc->drmmode = drmmode;
	crtc->driver_private = drmmode_crtc;

	{
		int cursor_size = 64*4*64;

		drmmode_crtc->cursor_bo = qxl->bo_funcs->bo_alloc(qxl, cursor_size, "cursor");
		if (!drmmode_crtc->cursor_bo) {
			ErrorF("failed to allocate cursor buffer\n");
			return;
		}

		drmmode_crtc->cursor_ptr = qxl->bo_funcs->bo_map(drmmode_crtc->cursor_bo);
	}
	
	return;
}


static xf86OutputStatus
drmmode_output_detect(xf86OutputPtr output)
{
	/* go to the hw and retrieve a new output struct */
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	xf86OutputStatus status;
	drmModeFreeConnector(drmmode_output->mode_output);

	drmmode_output->mode_output = drmModeGetConnector(drmmode->fd, drmmode_output->output_id);

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
drmmode_output_mode_valid(xf86OutputPtr output, DisplayModePtr pModes)
{
	return MODE_OK;
}

static DisplayModePtr
drmmode_output_get_modes(xf86OutputPtr output)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr koutput = drmmode_output->mode_output;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	int i;
	DisplayModePtr Modes = NULL, Mode;
	drmModePropertyPtr props;
	xf86MonPtr mon = NULL;

	/* look for an EDID property */
	for (i = 0; i < koutput->count_props; i++) {
		props = drmModeGetProperty(drmmode->fd, koutput->props[i]);
		if (props && (props->flags & DRM_MODE_PROP_BLOB)) {
			if (!strcmp(props->name, "EDID")) {
				if (drmmode_output->edid_blob)
					drmModeFreePropertyBlob(drmmode_output->edid_blob);
				drmmode_output->edid_blob = drmModeGetPropertyBlob(drmmode->fd, koutput->prop_values[i]);
			}
			drmModeFreeProperty(props);
		}
	}

	if (drmmode_output->edid_blob) {
		mon = xf86InterpretEDID(output->scrn->scrnIndex,
					drmmode_output->edid_blob->data);
		if (mon && drmmode_output->edid_blob->length > 128)
			mon->flags |= MONITOR_EDID_COMPLETE_RAWDATA;
	}
	xf86OutputSetEDID(output, mon);

	/* modes should already be available */
	for (i = 0; i < koutput->count_modes; i++) {
		Mode = xnfalloc(sizeof(DisplayModeRec));

		drmmode_ConvertFromKMode(output->scrn, &koutput->modes[i], Mode);
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
	for (i = 0; i < drmmode_output->num_props; i++) {
		drmModeFreeProperty(drmmode_output->props[i].mode_prop);
		free(drmmode_output->props[i].atoms);
	}
	for (i = 0; i < drmmode_output->mode_output->count_encoders; i++) {
		drmModeFreeEncoder(drmmode_output->mode_encoders[i]);
		free(drmmode_output->mode_encoders);
	}
	free(drmmode_output->props);
	drmModeFreeConnector(drmmode_output->mode_output);
	free(drmmode_output);
	output->driver_private = NULL;
}

static void
drmmode_output_dpms(xf86OutputPtr output, int mode)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr koutput = drmmode_output->mode_output;
	drmmode_ptr drmmode = drmmode_output->drmmode;

	drmModeConnectorSetProperty(drmmode->fd, koutput->connector_id,
				    drmmode_output->dpms_enum_id, mode);
	return;
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
	drmmode_output->props[j].index = i;
	drmmode_output->props[j].mode_prop = drmmode_prop;
	drmmode_output->props[j].value = mode_output->prop_values[i];
	drmmode_output->num_props++;
	j++;
    }

    for (i = 0; i < drmmode_output->num_props; i++) {
	drmmode_prop_ptr p = &drmmode_output->props[i];
	drmmode_prop = p->mode_prop;

	if (drmmode_prop->flags & DRM_MODE_PROP_RANGE) {
	    INT32 qrange[2];
	    INT32 value = p->value;

	    p->num_atoms = 1;
	    p->atoms = calloc(p->num_atoms, sizeof(Atom));
	    if (!p->atoms)
		continue;
	    p->atoms[0] = MakeAtom(drmmode_prop->name, strlen(drmmode_prop->name), TRUE);
	    qrange[0] = drmmode_prop->values[0];
	    qrange[1] = drmmode_prop->values[1];
	    err = RRConfigureOutputProperty(output->randr_output, p->atoms[0],
		    FALSE, TRUE,
		    drmmode_prop->flags & DRM_MODE_PROP_IMMUTABLE ? TRUE : FALSE,
		    2, qrange);
	    if (err != 0) {
		xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
			"RRConfigureOutputProperty error, %d\n", err);
	    }
	    err = RRChangeOutputProperty(output->randr_output, p->atoms[0],
		    XA_INTEGER, 32, PropModeReplace, 1, &value, FALSE, TRUE);
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
		if (drmmode_prop->enums[j].value == p->value)
		    break;
	    /* there's always a matching value */
	    err = RRChangeOutputProperty(output->randr_output, p->atoms[0],
		    XA_ATOM, 32, PropModeReplace, 1, &p->atoms[j+1], FALSE, TRUE);
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
    int i;

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

	    drmModeConnectorSetProperty(drmmode->fd, drmmode_output->output_id,
		    p->mode_prop->prop_id, (uint64_t)val);
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
		    drmModeConnectorSetProperty(drmmode->fd, drmmode_output->output_id,
			    p->mode_prop->prop_id, p->mode_prop->enums[j].value);
		    return TRUE;
		}
	    }
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
	    int j;

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
    .dpms = drmmode_output_dpms,
    .create_resources = drmmode_output_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property = drmmode_output_set_property,
    .get_property = drmmode_output_get_property,
#endif
#if 0

    .save = drmmode_crt_save,
    .restore = drmmode_crt_restore,
    .mode_fixup = drmmode_crt_mode_fixup,
    .prepare = drmmode_output_prepare,
    .mode_set = drmmode_crt_mode_set,
    .commit = drmmode_output_commit,
#endif
    .detect = drmmode_output_detect,
    .mode_valid = drmmode_output_mode_valid,

    .get_modes = drmmode_output_get_modes,
    .destroy = drmmode_output_destroy
};

static int subpixel_conv_table[7] = { 0, SubPixelUnknown,
				      SubPixelHorizontalRGB,
				      SubPixelHorizontalBGR,
				      SubPixelVerticalRGB,
				      SubPixelVerticalBGR,
				      SubPixelNone };

const char *output_names[] = {
    "None",
    "VGA",
    "DVI-I",
    "DVI-D",
    "DVI-A",
    "Composite",
    "SVIDEO",
    "LVDS",
    "Component",
    "DIN",
    "DP",
    "HDMI",
    "HDMI-B",
    "TV",
    "eDP",
    "Virtual",
    "DSI",
    "DPI",
};

static void
drmmode_output_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int num)
{
	xf86OutputPtr output;
	drmModeConnectorPtr koutput;
	drmModeEncoderPtr *kencoders = NULL;
	drmmode_output_private_ptr drmmode_output;
	drmModePropertyPtr props;
	char name[32];
	int i;

	koutput = drmModeGetConnector(drmmode->fd, drmmode->mode_res->connectors[num]);
	if (!koutput)
		return;

	kencoders = calloc(sizeof(drmModeEncoderPtr), koutput->count_encoders);
	if (!kencoders) {
		goto out_free_encoders;
	}

	for (i = 0; i < koutput->count_encoders; i++) {
		kencoders[i] = drmModeGetEncoder(drmmode->fd, koutput->encoders[i]);
		if (!kencoders[i]) {
			goto out_free_encoders;
		}
	}

	snprintf(name, 32, "%s-%d", output_names[koutput->connector_type], koutput->connector_type_id);
	

	output = xf86OutputCreate (pScrn, &drmmode_output_funcs, name);
	if (!output) {
		goto out_free_encoders;
	}

	drmmode_output = calloc(sizeof(drmmode_output_private_rec), 1);
	if (!drmmode_output) {
		xf86OutputDestroy(output);
		goto out_free_encoders;
	}

	drmmode_output->output_id = drmmode->mode_res->connectors[num];
	drmmode_output->mode_output = koutput;
	drmmode_output->mode_encoders = kencoders;
	drmmode_output->drmmode = drmmode;
	output->mm_width = koutput->mmWidth;
	output->mm_height = koutput->mmHeight;

	output->subpixel_order = subpixel_conv_table[koutput->subpixel];
	output->interlaceAllowed = TRUE;
	output->doubleScanAllowed = TRUE;
	output->driver_private = drmmode_output;
	
	output->possible_crtcs = 0xffffffff;
	for (i = 0; i < koutput->count_encoders; i++) {
		output->possible_crtcs &= kencoders[i]->possible_crtcs;
	}
	/* work out the possible clones later */
	output->possible_clones = 0;

	for (i = 0; i < koutput->count_props; i++) {
		props = drmModeGetProperty(drmmode->fd, koutput->props[i]);
		if (props && (props->flags & DRM_MODE_PROP_ENUM)) {
			if (!strcmp(props->name, "DPMS")) {
				drmmode_output->dpms_enum_id = koutput->props[i];
				drmModeFreeProperty(props);
				break;
			}
			drmModeFreeProperty(props);
		}
	}

	return;
out_free_encoders:
	if (kencoders){
		for (i = 0; i < koutput->count_encoders; i++)
			drmModeFreeEncoder(kencoders[i]);
		free(kencoders);
	}
	drmModeFreeConnector(koutput);
	
}

static Bool
drmmode_xf86crtc_resize (ScrnInfoPtr scrn, int width, int height)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	drmmode_crtc_private_ptr
		    drmmode_crtc = xf86_config->crtc[0]->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	struct qxl_bo *old_front = NULL;
	struct qxl_bo *front_bo;
	qxl_screen_t *qxl = scrn->driverPrivate;
	int cpp = (scrn->bitsPerPixel + 7) / 8;
	int32_t pitch, old_pitch;
	int ret, i;
	uint32_t old_width, old_height, old_fb_id;
	if (scrn->virtualX == width && scrn->virtualY == height)
                return TRUE;

	xf86DrvMsg(scrn->scrnIndex, X_INFO,
                   "Allocate new frame buffer %dx%d stride\n",
                   width, height);

	front_bo = qxl->primary->bo;

	pitch = width * cpp;
	old_width = scrn->virtualX;
	old_height = scrn->virtualY;
	old_pitch = scrn->displayWidth;
	old_fb_id = drmmode->fb_id;
	old_front = front_bo;

	scrn->virtualX = width;
	scrn->virtualY = height;
	scrn->displayWidth = pitch / cpp;

	qxl->primary->bo = qxl->bo_funcs->create_primary(qxl, width, height, pitch, SPICE_SURFACE_FMT_32_xRGB);
	if (!qxl->primary->bo)
		goto fail;

	ret = drmModeAddFB(drmmode->fd,
			   width, height,
			   scrn->depth, scrn->bitsPerPixel,
			   pitch,
			   qxl_kms_bo_get_handle(qxl->primary->bo),
			   &drmmode->fb_id);
	if (ret)
		goto fail;

	for (i = 0; i < xf86_config->num_crtc; i++) {
		xf86CrtcPtr crtc = xf86_config->crtc[i];
		if (!crtc->enabled)
			continue;
		if (!drmmode_set_mode_major(crtc, &crtc->mode, crtc->rotation,
                                            crtc->x, crtc->y))
                    goto fail;
	}

	{
		void *dev_ptr = qxl->bo_funcs->bo_map(qxl->primary->bo);
		uint32_t *dev_addr;
		int format = scrn->bitsPerPixel == 16 ? PIXMAN_x1r5g5b5 : PIXMAN_x8r8g8b8;
		dev_addr = dev_ptr;
		pixman_image_unref(qxl->primary->dev_image);
		pixman_image_unref (qxl->primary->host_image);

		qxl->primary->dev_image = pixman_image_create_bits (format,
								    width,
								    height,
								    (uint32_t *)dev_addr, pitch);

		qxl->primary->host_image = pixman_image_create_bits (format,
								    width,
								    height,
								    NULL, pitch);
	}
		
	/* fixup the surfaces */

	if (old_fb_id)
		drmModeRmFB(drmmode->fd, old_fb_id);
	if (old_front)
		qxl->bo_funcs->bo_decref(qxl, old_front);

	return TRUE;
fail:
	qxl->primary->bo = old_front;
	scrn->virtualX = old_width;
	scrn->virtualY = old_height;
	scrn->displayWidth = old_pitch;
	drmmode->fb_id = old_fb_id;
	return FALSE;
}


static const xf86CrtcConfigFuncsRec drmmode_xf86crtc_config_funcs = {
	drmmode_xf86crtc_resize,
};

Bool drmmode_pre_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int cpp)
{
	int i;

	xf86CrtcConfigInit(pScrn, &drmmode_xf86crtc_config_funcs);

	drmmode->scrn = pScrn;
	drmmode->cpp = cpp;
	drmmode->mode_res = drmModeGetResources(drmmode->fd);
	if (!drmmode->mode_res)
		return FALSE;

	xf86CrtcSetSizeRange(pScrn, 320, 200, drmmode->mode_res->max_width, drmmode->mode_res->max_height);
	for (i = 0; i < drmmode->mode_res->count_crtcs; i++)
	        drmmode_crtc_init(pScrn, drmmode, i);

	for (i = 0; i < drmmode->mode_res->count_connectors; i++)
	    drmmode_output_init(pScrn, drmmode, i);

#if XF86_CRTC_VERSION >= 5
	xf86ProviderSetup(pScrn, NULL, "qxl");
#endif
	xf86InitialConfiguration(pScrn, TRUE);

	return TRUE;
}

#ifdef HAVE_LIBUDEV
static void
drmmode_handle_uevents(int fd, void *closure)
{
	drmmode_ptr drmmode = closure;
	ScrnInfoPtr scrn = drmmode->scrn;
	struct udev_device *dev;
	dev = udev_monitor_receive_device(drmmode->uevent_monitor);
	if (!dev)
		return;

	RRGetInfo(xf86ScrnToScreen(scrn), TRUE);
	udev_device_unref(dev);
}
#endif

void qxl_drmmode_uevent_init(ScrnInfoPtr scrn, drmmode_ptr drmmode)
{
#ifdef HAVE_LIBUDEV
	struct udev *u;
	struct udev_monitor *mon;

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

	drmmode->uevent_handler =
		xf86AddGeneralHandler(udev_monitor_get_fd(mon),
				      drmmode_handle_uevents,
				      drmmode);

	drmmode->uevent_monitor = mon;
#endif
}

void qxl_drmmode_uevent_fini(ScrnInfoPtr scrn, drmmode_ptr drmmode)
{
#ifdef HAVE_LIBUDEV
	if (drmmode->uevent_handler) {
		struct udev *u = udev_monitor_get_udev(drmmode->uevent_monitor);
		xf86RemoveGeneralHandler(drmmode->uevent_handler);

		udev_monitor_unref(drmmode->uevent_monitor);
		udev_unref(u);
	}
#endif
}

#endif
