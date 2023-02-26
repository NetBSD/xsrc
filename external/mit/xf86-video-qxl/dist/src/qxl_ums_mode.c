/*
 * Copyright 2008 Red Hat, Inc.
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

/* code to handle UMS modesetting */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "qxl.h"

/* These constants govern which modes are reported to X as preferred */
#define DEFAULT_WIDTH       1024
#define DEFAULT_HEIGHT       768

static void qxl_update_monitors_config (qxl_screen_t *qxl);

static DisplayModePtr
screen_create_mode (ScrnInfoPtr pScrn, int width, int height, int type)
{
    DisplayModePtr mode;

    mode = xnfcalloc (1, sizeof (DisplayModeRec));

    mode->status = MODE_OK;
    mode->type = type;
    mode->HDisplay   = width;
    mode->HSyncStart = (width * 105 / 100 + 7) & ~7;
    mode->HSyncEnd   = (width * 115 / 100 + 7) & ~7;
    mode->HTotal     = (width * 130 / 100 + 7) & ~7;
    mode->VDisplay   = height;
    mode->VSyncStart = height + 1;
    mode->VSyncEnd   = height + 4;
    mode->VTotal     = height * 1035 / 1000;
    mode->Clock = mode->HTotal * mode->VTotal * 60 / 1000;
    mode->Flags = V_NHSYNC | V_PVSYNC;

    xf86SetModeDefaultName (mode);
    xf86SetModeCrtc (mode, pScrn->adjustFlags); /* needed? xf86-video-modesetting does this */

    return mode;
}

static DisplayModePtr
qxl_add_mode (qxl_screen_t *qxl, ScrnInfoPtr pScrn, int width, int height, int type)
{
    DisplayModePtr mode;

    mode = screen_create_mode (pScrn, width, height, type);
    pScrn->modes = qxl->x_modes = xf86ModesAdd (qxl->x_modes, mode);

    return mode;
}

static int
check_crtc (qxl_screen_t *qxl)
{
    int i, count = 0;
    xf86CrtcPtr crtc;

    if (qxl->crtcs == NULL) {
        return 0;
    }

    for (i = 0 ; i < qxl->num_heads; ++i)
    {
	crtc = qxl->crtcs[i];

	if (!crtc->enabled || crtc->mode.CrtcHDisplay == 0 ||
	    crtc->mode.CrtcVDisplay == 0)
	{
	    continue;
	}
	count++;
    }

#if 0
    if (count == 0)
    {
	ErrorF ("check crtc failed, count == 0!!\n");
	BREAKPOINT ();
    }
#endif

    return count;
}

static void
qxl_update_monitors_config (qxl_screen_t *qxl)
{
    int i;
    QXLHead *head;
    xf86CrtcPtr crtc;
    qxl_output_private *qxl_output;
    QXLRam * ram = get_ram_header (qxl);

    if (check_crtc (qxl) == 0)
        return;

    qxl->monitors_config->count = 0;
    qxl->monitors_config->max_allowed = qxl->num_heads;
    for (i = 0 ; i < qxl->num_heads; ++i)
    {
	head = &qxl->monitors_config->heads[qxl->monitors_config->count];
	crtc = qxl->crtcs[i];
	qxl_output = qxl->outputs[i]->driver_private;
	head->id = i;
	head->surface_id = 0;
	head->flags = 0;

	if (!crtc->enabled || crtc->mode.CrtcHDisplay == 0 ||
	    crtc->mode.CrtcVDisplay == 0)
	{
	    head->width = head->height = head->x = head->y = 0;
	    qxl_output->status = XF86OutputStatusDisconnected;
	}
	else
	{
	    head->width = crtc->mode.CrtcHDisplay;
	    head->height = crtc->mode.CrtcVDisplay;
	    head->x = crtc->x;
	    head->y = crtc->y;
	    qxl->monitors_config->count++;
	    qxl_output->status = XF86OutputStatusConnected;
	}
    }
    /* initialize when actually used, memslots should be initialized by now */
    if (ram->monitors_config == 0)
    {
	ram->monitors_config = physical_address (qxl, qxl->monitors_config,
	                                         qxl->main_mem_slot);
    }

    qxl_io_monitors_config_async (qxl);
}

static Bool
crtc_set_mode_major (xf86CrtcPtr crtc, DisplayModePtr mode,
                     Rotation rotation, int x, int y)
{
    qxl_crtc_private *crtc_private = crtc->driver_private;
    qxl_screen_t *    qxl = crtc_private->qxl;

    if (crtc == qxl->crtcs[0] && mode == NULL)
    {
	/* disallow disabling of monitor 0 mode */
	ErrorF ("%s: not allowing crtc 0 disablement\n", __func__);
	return FALSE;
    }

    crtc->mode = *mode;
    crtc->x = x;
    crtc->y = y;
    crtc->rotation = rotation;
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC (1, 5, 99, 0, 0)
    crtc->transformPresent = FALSE;
#endif
    /* TODO set EDID here */
    return TRUE;
}

Bool
qxl_create_desired_modes (qxl_screen_t *qxl)
{
    int i;
    xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR (qxl->pScrn);

    CHECK_POINT ();

    for (i = 0 ; i < config->num_crtc; ++i)
    {
	xf86CrtcPtr crtc = config->crtc[i];
	if (!crtc->enabled)
	    continue;

	if (!crtc_set_mode_major (
		crtc, &crtc->desiredMode, crtc->desiredRotation,
		crtc->desiredX, crtc->desiredY))
	{
	    return FALSE;
	}
    }

    qxl_update_monitors_config(qxl);
    return TRUE;
}

void
qxl_update_edid (qxl_screen_t *qxl)
{
    xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR (qxl->pScrn);
    int i;

    for (i = 0; i < config->num_crtc; ++i)
    {
	xf86CrtcPtr crtc = config->crtc[i];

	if (!crtc->enabled)
	    continue;

	/* TODO set EDID here */
    }
}


static DisplayModePtr
qxl_output_get_modes (xf86OutputPtr output)
{
    qxl_output_private *qxl_output = output->driver_private;
    DisplayModePtr      modes = xf86DuplicateModes (qxl_output->qxl->pScrn, qxl_output->qxl->x_modes);

    /* xf86ProbeOutputModes owns this memory */
    return modes;
}

static void
qxl_output_destroy (xf86OutputPtr output)
{
    qxl_output_private *qxl_output = output->driver_private;

    xf86DrvMsg (qxl_output->qxl->pScrn->scrnIndex, X_INFO,
                "%s", __func__);
}

static void
qxl_output_dpms (xf86OutputPtr output, int mode)
{
}

static void
qxl_output_create_resources (xf86OutputPtr output)
{
}

static Bool
qxl_output_set_property (xf86OutputPtr output, Atom property,
                         RRPropertyValuePtr value)
{
    /* EDID data is stored in the "EDID" atom property, we must return
     * TRUE here for that. No penalty to say ok to everything else. */
    return TRUE;
}

static Bool
qxl_output_get_property (xf86OutputPtr output, Atom property)
{
    return TRUE;
}

static xf86OutputStatus
qxl_output_detect (xf86OutputPtr output)
{
    qxl_output_private *qxl_output = output->driver_private;

    return qxl_output->status;
}

static Bool
qxl_output_mode_valid (xf86OutputPtr output, DisplayModePtr pModes)
{
    return MODE_OK;
}

static const xf86OutputFuncsRec qxl_output_funcs = {
    .dpms = qxl_output_dpms,
    .create_resources = qxl_output_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property = qxl_output_set_property,
    .get_property = qxl_output_get_property,
#endif
    .detect = qxl_output_detect,
    .mode_valid = qxl_output_mode_valid,

    .get_modes = qxl_output_get_modes,
    .destroy = qxl_output_destroy
};


static void
qxl_crtc_dpms (xf86CrtcPtr crtc, int mode)
{
}

static Bool
qxl_crtc_set_mode_major (xf86CrtcPtr crtc, DisplayModePtr mode,
                         Rotation rotation, int x, int y)
{
    qxl_crtc_private *crtc_private = crtc->driver_private;
    qxl_screen_t *    qxl = crtc_private->qxl;

    CHECK_POINT ();

    if (!crtc_set_mode_major (crtc, mode, rotation, x, y))
	return FALSE;

    qxl_update_monitors_config (qxl);

    return TRUE;
}

static void
qxl_crtc_set_cursor_colors (xf86CrtcPtr crtc, int bg, int fg)
{
}

static void
qxl_crtc_set_cursor_position (xf86CrtcPtr crtc, int x, int y)
{
}

static void
qxl_crtc_load_cursor_argb (xf86CrtcPtr crtc, CARD32 *image)
{
}

static void
qxl_crtc_hide_cursor (xf86CrtcPtr crtc)
{
}

static void
qxl_crtc_show_cursor (xf86CrtcPtr crtc)
{
}

static void
qxl_crtc_gamma_set (xf86CrtcPtr crtc, uint16_t *red, uint16_t *green,
                    uint16_t *blue, int size)
{
}

static void
qxl_crtc_destroy (xf86CrtcPtr crtc)
{
    qxl_crtc_private *crtc_private = crtc->driver_private;
    qxl_screen_t *    qxl = crtc_private->qxl;

    xf86DrvMsg (qxl->pScrn->scrnIndex, X_INFO, "%s\n", __func__);
}

static Bool
qxl_crtc_lock (xf86CrtcPtr crtc)
{
    qxl_crtc_private *crtc_private = crtc->driver_private;
    qxl_screen_t *    qxl = crtc_private->qxl;

    xf86DrvMsg (qxl->pScrn->scrnIndex, X_INFO, "%s\n", __func__);
    return TRUE;
}

static void
qxl_crtc_unlock (xf86CrtcPtr crtc)
{
    qxl_crtc_private *crtc_private = crtc->driver_private;
    qxl_screen_t *    qxl = crtc_private->qxl;

    xf86DrvMsg (qxl->pScrn->scrnIndex, X_INFO, "%s\n", __func__);
    qxl_update_monitors_config (qxl);
}

static const xf86CrtcFuncsRec qxl_crtc_funcs = {
    .dpms = qxl_crtc_dpms,
    .set_mode_major = qxl_crtc_set_mode_major,
    .set_cursor_colors = qxl_crtc_set_cursor_colors,
    .set_cursor_position = qxl_crtc_set_cursor_position,
    .show_cursor = qxl_crtc_show_cursor,
    .hide_cursor = qxl_crtc_hide_cursor,
    .load_cursor_argb = qxl_crtc_load_cursor_argb,
    .lock = qxl_crtc_lock,
    .unlock = qxl_crtc_unlock,

    .gamma_set = qxl_crtc_gamma_set,
    .destroy = qxl_crtc_destroy,
};


static Bool
qxl_xf86crtc_resize (ScrnInfoPtr scrn, int width, int height)
{
    qxl_screen_t *qxl = scrn->driverPrivate;

    xf86DrvMsg (scrn->scrnIndex, X_INFO, "%s: Placeholder resize %dx%d\n",
                __func__, width, height);
    if (!qxl_resize_primary (qxl, width, height))
	return FALSE;

    scrn->virtualX = width;
    scrn->virtualY = height;

    // when starting, no monitor is enabled, and count == 0
    // we want to avoid server/client freaking out with temporary config
    qxl_update_monitors_config (qxl);

    return TRUE;
}

static const xf86CrtcConfigFuncsRec qxl_xf86crtc_config_funcs = {
    qxl_xf86crtc_resize
};

void
qxl_initialize_x_modes (qxl_screen_t *qxl, ScrnInfoPtr pScrn,
                        unsigned int *max_x, unsigned int *max_y)
{
    int i;
    int size;
    int preferred_flag;

    *max_x = *max_y = 0;
    /* Create a list of modes used by the qxl_output_get_modes */
    for (i = 0; i < qxl->num_modes; i++)
    {
	if (qxl->modes[i].orientation == 0)
	{
	    size = qxl->modes[i].y_res * qxl->modes[i].stride;
	    if (size > qxl->surface0_size)
	    {
		ErrorF ("skipping mode %dx%d not fitting in surface0\n",
		        qxl->modes[i].x_res, qxl->modes[i].y_res);
		continue;
	    }

            if (qxl->modes[i].x_res == DEFAULT_WIDTH && qxl->modes[i].y_res == DEFAULT_HEIGHT)
                preferred_flag = M_T_PREFERRED;
            else
                preferred_flag = 0;

	    qxl_add_mode (qxl, pScrn, qxl->modes[i].x_res, qxl->modes[i].y_res,
	                  M_T_DRIVER | preferred_flag);

	    if (qxl->modes[i].x_res > *max_x)
		*max_x = qxl->modes[i].x_res;
	    if (qxl->modes[i].y_res > *max_y)
		*max_y = qxl->modes[i].y_res;
	}
    }
}

void
qxl_init_randr (ScrnInfoPtr pScrn, qxl_screen_t *qxl)
{
    char                name[32];
    qxl_output_private *qxl_output;
    qxl_crtc_private *  qxl_crtc;
    int                 i;
    xf86OutputPtr       output;

    xf86CrtcConfigInit (pScrn, &qxl_xf86crtc_config_funcs);

    /* CHECKME: This is actually redundant, it's overwritten by a later call via
     * xf86InitialConfiguration */
    xf86CrtcSetSizeRange (pScrn, 320, 200, 8192, 8192);

    qxl->crtcs = xnfcalloc (sizeof (xf86CrtcPtr), qxl->num_heads);
    qxl->outputs = xnfcalloc (sizeof (xf86OutputPtr), qxl->num_heads);

    for (i = 0 ; i < qxl->num_heads; ++i)
    {
	qxl->crtcs[i] = xf86CrtcCreate (pScrn, &qxl_crtc_funcs);
	if (!qxl->crtcs[i])
	    xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "failed to create Crtc %d", i);

	qxl_crtc = xnfcalloc (sizeof (qxl_crtc_private), 1);
	qxl->crtcs[i]->driver_private = qxl_crtc;
	qxl_crtc->head = i;
	qxl_crtc->qxl = qxl;
	snprintf (name, sizeof (name), "qxl-%d", i);
	qxl->outputs[i] = output = xf86OutputCreate (pScrn, &qxl_output_funcs, name);
	if (!output)
	    xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "failed to create Output %d", i);

	output->possible_crtcs = (1 << i); /* bitrange of allowed outputs - do a 1:1 */
	output->possible_clones = 0; /* TODO: not? */
	qxl_output = xnfcalloc (sizeof (qxl_output_private), 1);
	output->driver_private = qxl_output;
	qxl_output->head = i;
	qxl_output->qxl = qxl;
	qxl_output->status = i ? XF86OutputStatusDisconnected : XF86OutputStatusConnected;
	qxl_crtc->output = output;
    }

    xf86InitialConfiguration (pScrn, TRUE);

    qxl->virtual_x = pScrn->virtualX;
    qxl->virtual_y = pScrn->virtualY;
    /* all crtcs are enabled here, but their mode is 0,
       resulting monitor config empty atm */
}
