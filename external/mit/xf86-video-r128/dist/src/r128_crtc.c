/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>

#include "xf86.h"
#include "xf86Modes.h"
#include "X11/extensions/dpmsconst.h"

#include "r128.h"
#include "r128_probe.h"
#include "r128_reg.h"

static void r128_crtc_load_lut(xf86CrtcPtr crtc);

static void r128_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
    int mask;
    ScrnInfoPtr pScrn = crtc->scrn;
    R128InfoPtr info = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;

    /* XXX: The HSYNC and VSYNC bits for CRTC2 don't exist on the r128? */
    mask = r128_crtc->crtc_id ? R128_CRTC2_DISP_DIS : (R128_CRTC_DISPLAY_DIS | R128_CRTC_HSYNC_DIS | R128_CRTC_VSYNC_DIS);

    switch (mode) {
    case DPMSModeOn:
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, 0, ~mask);
        } else {
            OUTREGP(R128_CRTC_EXT_CNTL, 0, ~mask);
        }
        break;
    case DPMSModeStandby:
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, R128_CRTC2_DISP_DIS, ~mask);
        } else {
            OUTREGP(R128_CRTC_EXT_CNTL, (R128_CRTC_DISPLAY_DIS | R128_CRTC_HSYNC_DIS), ~mask);
        }
        break;
    case DPMSModeSuspend:
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, R128_CRTC2_DISP_DIS, ~mask);
        } else {
            OUTREGP(R128_CRTC_EXT_CNTL, (R128_CRTC_DISPLAY_DIS | R128_CRTC_VSYNC_DIS), ~mask);
        }
        break;
    case DPMSModeOff:
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, mask, ~mask);
        } else {
            OUTREGP(R128_CRTC_EXT_CNTL, mask, ~mask);
        }
        break;
    }

    if (mode != DPMSModeOn) {
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, 0, ~R128_CRTC2_EN);
        } else {
            OUTREGP(R128_CRTC_GEN_CNTL, 0, ~R128_CRTC_EN);
        }
    } else {
        if (r128_crtc->crtc_id) {
            OUTREGP(R128_CRTC2_GEN_CNTL, R128_CRTC2_EN, ~R128_CRTC2_EN);
        } else {
            OUTREGP(R128_CRTC_GEN_CNTL, R128_CRTC_EN, ~R128_CRTC_EN);
        }
    }

    if (mode != DPMSModeOff)
        r128_crtc_load_lut(crtc);
}

void r128_crtc_load_lut(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    R128InfoPtr info = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;
    int i;

    if (!crtc->enabled)
        return;

    PAL_SELECT(r128_crtc->crtc_id);

    for (i = 0; i < 256; i++) {
        OUTPAL(i, r128_crtc->lut_r[i], r128_crtc->lut_g[i], r128_crtc->lut_b[i]);
    }
}

static Bool r128_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void r128_crtc_mode_prepare(xf86CrtcPtr crtc)
{
    r128_crtc_dpms(crtc, DPMSModeOff);
}

static void r128_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjusted_mode, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;
    R128InfoPtr info = R128PTR(pScrn);
    double dot_clock = adjusted_mode->Clock / 1000.0;

    if (r128_crtc->cursor_offset) r128_crtc_hide_cursor(crtc);
    xf86PrintModeline(pScrn->scrnIndex, adjusted_mode);
    R128InitCommonRegisters(&info->ModeReg, info);

    switch (r128_crtc->crtc_id) {
    case 0:
        R128InitCrtcRegisters(crtc, &info->ModeReg, adjusted_mode);
	R128InitCrtcBase(crtc, &info->ModeReg, x, y);
        if (dot_clock) {
            R128InitPLLRegisters(crtc, &info->ModeReg, &info->pll, dot_clock);
            R128InitDDARegisters(crtc, &info->ModeReg, &info->pll, adjusted_mode);
        } else {
            info->ModeReg.ppll_ref_div         = info->SavedReg.ppll_ref_div;
            info->ModeReg.ppll_div_3           = info->SavedReg.ppll_div_3;
            info->ModeReg.htotal_cntl          = info->SavedReg.htotal_cntl;
            info->ModeReg.dda_config           = info->SavedReg.dda_config;
            info->ModeReg.dda_on_off           = info->SavedReg.dda_on_off;
        }
        break;
    case 1:
        R128InitCrtc2Registers(crtc, &info->ModeReg, adjusted_mode);
	R128InitCrtc2Base(crtc, &info->ModeReg, x, y);
        if (dot_clock) {
            R128InitPLL2Registers(crtc, &info->ModeReg, &info->pll, dot_clock);
            R128InitDDA2Registers(crtc, &info->ModeReg, &info->pll, adjusted_mode);
        }
        break;
    }

    R128RestoreCommonRegisters(pScrn, &info->ModeReg);

    switch (r128_crtc->crtc_id) {
    case 0:
        R128RestoreDDARegisters(pScrn, &info->ModeReg);
        R128RestoreCrtcRegisters(pScrn, &info->ModeReg);
        R128RestorePLLRegisters(pScrn, &info->ModeReg);
        break;
    case 1:
        R128RestoreDDA2Registers(pScrn, &info->ModeReg);
        R128RestoreCrtc2Registers(pScrn, &info->ModeReg);
        R128RestorePLL2Registers(pScrn, &info->ModeReg);
	break;
    }

    if (r128_crtc->cursor_offset) r128_crtc_show_cursor(crtc);
}

static void r128_crtc_mode_commit(xf86CrtcPtr crtc)
{
    r128_crtc_dpms(crtc, DPMSModeOn);
}

static void r128_crtc_gamma_set(xf86CrtcPtr crtc, uint16_t *red, uint16_t *green, uint16_t *blue, int size)
{
    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;
    int i;

    for (i = 0; i < 256; i++) {
        r128_crtc->lut_r[i] = red[i] >> 8;
        r128_crtc->lut_g[i] = green[i] >> 8;
        r128_crtc->lut_b[i] = blue[i] >> 8;
    }

    r128_crtc_load_lut(crtc);
}

static Bool r128_crtc_lock(xf86CrtcPtr crtc)
{
    ScrnInfoPtr   pScrn   = crtc->scrn;
    ScreenPtr     pScreen = xf86ScrnToScreen(pScrn);
    R128InfoPtr   info    = R128PTR(pScrn);

#ifdef HAVE_XAA_H
    if (info->accel) info->accel->Sync(pScrn);
#endif
#ifdef USE_EXA
    if (info->ExaDriver) exaWaitSync(pScreen);
#endif

    return FALSE;
}

static void r128_crtc_unlock(xf86CrtcPtr crtc)
{
    ScrnInfoPtr   pScrn   = crtc->scrn;
    ScreenPtr     pScreen = xf86ScrnToScreen(pScrn);
    R128InfoPtr   info    = R128PTR(pScrn);

#ifdef HAVE_XAA_H
    if (info->accel) info->accel->Sync(pScrn);
#endif
#ifdef USE_EXA
    if (info->ExaDriver) exaWaitSync(pScreen);
#endif
}

static void *r128_crtc_shadow_allocate(xf86CrtcPtr crtc, int width, int height)
{
    ScrnInfoPtr   pScrn   = crtc->scrn;
    R128InfoPtr   info    = R128PTR(pScrn);

    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;
    unsigned long rotate_offset = 0;
    unsigned long rotate_pitch;
    int cpp = pScrn->bitsPerPixel / 8;
    int align = 4096;
    int size;

    rotate_pitch = pScrn->displayWidth * cpp;
    size = rotate_pitch * height;
    rotate_offset = R128AllocateMemory(pScrn, &(r128_crtc->rotate_mem), size, align, TRUE);

    /* If allocations failed or if there was no accel. */
    if (rotate_offset == 0)
        return NULL;

    return info->FB + rotate_offset;
}

static PixmapPtr r128_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    PixmapPtr rotate_pixmap;
    unsigned long rotate_pitch;
    int cpp = pScrn->bitsPerPixel / 8;

    if (!data) data = r128_crtc_shadow_allocate(crtc, width, height);

    rotate_pitch = pScrn->displayWidth * cpp;
    rotate_pixmap = GetScratchPixmapHeader(xf86ScrnToScreen(pScrn),
                                           width, height,
                                           pScrn->depth,
                                           pScrn->bitsPerPixel,
                                           rotate_pitch,
                                           data);

    if (rotate_pixmap == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "Couldn't allocate shadow memory for rotated CRTC\n");
        return NULL;
    }

    return rotate_pixmap;
}

static void r128_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
    ScrnInfoPtr   pScrn   = crtc->scrn;
    ScreenPtr     pScreen = xf86ScrnToScreen(pScrn);
    R128InfoPtr   info    = R128PTR(pScrn);

    R128CrtcPrivatePtr r128_crtc = crtc->driver_private;

    if (rotate_pixmap) FreeScratchPixmapHeader(rotate_pixmap);

    if (data && r128_crtc->rotate_mem != NULL) {
#ifdef USE_EXA
        if (info->ExaDriver)
            exaOffscreenFree(pScreen, (ExaOffscreenArea *) r128_crtc->rotate_mem);
#endif
#ifdef HAVE_XAA_H
        if (info->accel)
            xf86FreeOffscreenLinear((FBLinearPtr) r128_crtc->rotate_mem);
#endif
        r128_crtc->rotate_mem = NULL;
    }
}

static const xf86CrtcFuncsRec r128_crtc_funcs = {
    .dpms = r128_crtc_dpms,
    .save = NULL,
    .restore = NULL,
    .mode_fixup = r128_crtc_mode_fixup,
    .prepare = r128_crtc_mode_prepare,
    .mode_set = r128_crtc_mode_set,
    .commit = r128_crtc_mode_commit,
    .gamma_set = r128_crtc_gamma_set,
    .lock = r128_crtc_lock,
    .unlock = r128_crtc_unlock,
    .shadow_create = r128_crtc_shadow_create,
    .shadow_allocate = r128_crtc_shadow_allocate,
    .shadow_destroy = r128_crtc_shadow_destroy,
    .set_cursor_colors = r128_crtc_set_cursor_colors,
    .set_cursor_position = r128_crtc_set_cursor_position,
    .show_cursor = r128_crtc_show_cursor,
    .hide_cursor = r128_crtc_hide_cursor,
    .load_cursor_image = r128_crtc_load_cursor_image,
    .destroy = NULL,
};

Bool R128AllocateControllers(ScrnInfoPtr pScrn)
{
    R128EntPtr pR128Ent = R128EntPriv(pScrn);

    if (pR128Ent->Controller[0])
        return TRUE;

    pR128Ent->pCrtc[0] = xf86CrtcCreate(pScrn, &r128_crtc_funcs);
    if (!pR128Ent->pCrtc[0])
        return FALSE;

    pR128Ent->Controller[0] = xnfcalloc(sizeof(R128CrtcPrivateRec), 1);
    if (!pR128Ent->Controller[0])
        return FALSE;

    pR128Ent->pCrtc[0]->driver_private = pR128Ent->Controller[0];
    pR128Ent->Controller[0]->crtc_id = 0;

    if (!pR128Ent->HasCRTC2)
        return TRUE;

    pR128Ent->pCrtc[1] = xf86CrtcCreate(pScrn, &r128_crtc_funcs);
    if (!pR128Ent->pCrtc[1])
        return FALSE;

    pR128Ent->Controller[1] = xnfcalloc(sizeof(R128CrtcPrivateRec), 1);
    if (!pR128Ent->Controller[1]) {
        free(pR128Ent->Controller[0]);
        return FALSE;
    }

    pR128Ent->pCrtc[1]->driver_private = pR128Ent->Controller[1];
    pR128Ent->Controller[1]->crtc_id = 1;

    return TRUE;
}

void R128Blank(ScrnInfoPtr pScrn)
{
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    xf86OutputPtr output;
    xf86CrtcPtr crtc;
    int o, c;

    for (c = 0; c < xf86_config->num_crtc; c++) {
        crtc = xf86_config->crtc[c];
        for (o = 0; o < xf86_config->num_output; o++) {
            output = xf86_config->output[o];
            if (output->crtc != crtc)
                continue;

            output->funcs->dpms(output, DPMSModeOff);
        }
        crtc->funcs->dpms(crtc, DPMSModeOff);
    }
}

void R128Unblank(ScrnInfoPtr pScrn)
{
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    xf86OutputPtr output;
    xf86CrtcPtr crtc;
    int o, c;

    for (c = 0; c < xf86_config->num_crtc; c++) {
        crtc = xf86_config->crtc[c];
        if (!crtc->enabled)
            continue;
        crtc->funcs->dpms(crtc, DPMSModeOn);
        for (o = 0; o < xf86_config->num_output; o++) {
            output = xf86_config->output[o];
            if (output->crtc != crtc)
                continue;

            output->funcs->dpms(output, DPMSModeOn);
        }
    }
}
