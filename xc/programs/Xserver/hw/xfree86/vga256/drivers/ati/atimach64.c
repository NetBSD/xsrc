/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atimach64.c,v 1.1.2.4 2000/05/14 02:02:16 tsi Exp $ */
/*
 * Copyright 1997 through 1999 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "ati.h"
#include "atibus.h"
#include "atichip.h"
#include "aticlock.h"
#include "atiio.h"
#include "atimach64.h"
#include "atividmem.h"

/*
 * ATIMach64Save --
 *
 * This function is called to save a video mode that uses a Mach64
 * accelerator's CRTC.
 */
void
ATIMach64Save(ATIHWPtr save)
{
    save->crtc_h_total_disp = inl(ATIIOPortCRTC_H_TOTAL_DISP);
    save->crtc_h_sync_strt_wid = inl(ATIIOPortCRTC_H_SYNC_STRT_WID);
    save->crtc_v_total_disp = inl(ATIIOPortCRTC_V_TOTAL_DISP);
    save->crtc_v_sync_strt_wid = inl(ATIIOPortCRTC_V_SYNC_STRT_WID);

    save->crtc_off_pitch = inl(ATIIOPortCRTC_OFF_PITCH);

    save->crtc_gen_cntl = inl(ATIIOPortCRTC_GEN_CNTL);

    save->ovr_clr = inl(ATIIOPortOVR_CLR);
    save->ovr_wid_left_right = inl(ATIIOPortOVR_WID_LEFT_RIGHT);
    save->ovr_wid_top_bottom = inl(ATIIOPortOVR_WID_TOP_BOTTOM);

    save->clock_cntl = inl(ATIIOPortCLOCK_CNTL);

    save->bus_cntl = inl(ATIIOPortBUS_CNTL);

    save->mem_vga_wp_sel = inl(ATIIOPortMEM_VGA_WP_SEL);
    save->mem_vga_rp_sel = inl(ATIIOPortMEM_VGA_RP_SEL);

    save->dac_cntl = inl(ATIIOPortDAC_CNTL);

    save->config_cntl = inl(ATIIOPortCONFIG_CNTL);
}

/*
 * ATIMach64Init --
 *
 * This function fills in the Mach64 portion of an ATIHWRec.
 */
void
ATIMach64Init(DisplayModePtr mode)
{
    int VDisplay;

    if (!mode)                          /* Fill in common data */
    {
        ATINewHWPtr->crtc_off_pitch =
            SetBits(vga256InfoRec.displayWidth >> 3, CRTC_PITCH);

        ATINewHWPtr->bus_cntl = (inl(ATIIOPortBUS_CNTL) &
            ~BUS_HOST_ERR_INT_EN) | BUS_HOST_ERR_INT;
        if (ATIChip < ATI_CHIP_264VTB)
            ATINewHWPtr->bus_cntl = (ATINewHWPtr->bus_cntl &
                ~(BUS_FIFO_ERR_INT_EN | BUS_ROM_DIS)) |
                (SetBits(15, BUS_FIFO_WS) | BUS_FIFO_ERR_INT);
        else
            ATINewHWPtr->bus_cntl |= BUS_APER_REG_DIS;

        ATINewHWPtr->mem_vga_wp_sel =
            /* SetBits(0, MEM_VGA_WPS0) | */
            SetBits(ATINewHWPtr->planes, MEM_VGA_WPS1);
        ATINewHWPtr->mem_vga_rp_sel =
            /* SetBits(0, MEM_VGA_RPS0) | */
            SetBits(ATINewHWPtr->planes, MEM_VGA_RPS1);

        ATINewHWPtr->dac_cntl = inl(ATIIOPortDAC_CNTL);
        if (vga256InfoRec.depth > 8)
            ATINewHWPtr->dac_cntl |= DAC_8BIT_EN;
        else
            ATINewHWPtr->dac_cntl &= ~DAC_8BIT_EN;

        ATINewHWPtr->config_cntl = inl(ATIIOPortCONFIG_CNTL);
        if (ATIUsingSmallApertures)
            ATINewHWPtr->config_cntl |= CFG_MEM_VGA_AP_EN;
        else
            ATINewHWPtr->config_cntl &= ~CFG_MEM_VGA_AP_EN;
        if (ATI.ChipUseLinearAddressing &&
            (ATIBusType != ATI_BUS_PCI) && (ATIBusType != ATI_BUS_AGP))
        {
            /* Replace linear aperture size and address */
            ATINewHWPtr->config_cntl &= ~(CFG_MEM_AP_LOC | CFG_MEM_AP_SIZE);
            ATINewHWPtr->config_cntl |=
                SetBits(ATI.ChipLinearBase >> 22, CFG_MEM_AP_LOC);
            if ((ATIChip < ATI_CHIP_264CT) && (ATIvideoRam < 4096))
                ATINewHWPtr->config_cntl |= SetBits(1, CFG_MEM_AP_SIZE);
            else
                ATINewHWPtr->config_cntl |= SetBits(2, CFG_MEM_AP_SIZE);
        }
    }
    else
    {
        /* Adjust mode timings and fill in mode-specific data */
        if (!mode->CrtcHAdjusted)
        {
            mode->CrtcHAdjusted = TRUE;
            mode->CrtcHDisplay = mode->HDisplay;
            mode->CrtcHSyncStart = mode->HSyncStart;
            mode->CrtcHSyncEnd = mode->HSyncEnd;
            mode->CrtcHTotal = mode->HTotal;
            mode->CrtcHDisplay >>= 3;
            mode->CrtcHSyncStart >>= 3;
            mode->CrtcHSyncEnd >>= 3;
            mode->CrtcHTotal >>= 3;
            mode->CrtcHDisplay--;
            mode->CrtcHSyncStart--;
            mode->CrtcHSyncEnd--;
            mode->CrtcHTotal--;
            /* Make adjustments if sync width is out-of-bounds */
            if ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) >
                 (int)MaxBits(CRTC_H_SYNC_WID))
                mode->CrtcHSyncEnd = mode->CrtcHSyncStart +
                    MaxBits(CRTC_H_SYNC_WID);
            else if (mode->CrtcHSyncStart == mode->CrtcHSyncEnd)
            {
                if (mode->CrtcHDisplay < mode->CrtcHSyncStart)
                    mode->CrtcHSyncStart--;
                else if (mode->CrtcHSyncEnd < mode->CrtcHTotal)
                    mode->CrtcHSyncEnd++;
            }
        }

        /*
         * Ignore any vertical adjustments that have already been made.
         * Doing so fixes a minor bug in doublescanned modes.
         */
        mode->CrtcVDisplay = mode->VDisplay;
        mode->CrtcVSyncStart = mode->VSyncStart;
        mode->CrtcVSyncEnd = mode->VSyncEnd;
        mode->CrtcVTotal = mode->VTotal;

        if ((mode->Flags & V_DBLSCAN) && (ATIChip >= ATI_CHIP_264CT))
        {
            mode->CrtcVDisplay <<= 1;
            mode->CrtcVSyncStart <<= 1;
            mode->CrtcVSyncEnd <<= 1;
            mode->CrtcVTotal <<= 1;
        }
        mode->CrtcVDisplay--;
        mode->CrtcVSyncStart--;
        mode->CrtcVSyncEnd--;
        mode->CrtcVTotal--;
        /* Make sure sync pulse is not too wide */
        if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) >
             (int)MaxBits(CRTC_V_SYNC_WID))
            mode->CrtcVSyncEnd = mode->CrtcVSyncStart +
                MaxBits(CRTC_V_SYNC_WID);
        mode->CrtcVAdjusted = TRUE;

        /*
         * Might as well default to the same as VGA with respect to sync
         * polarities.
         */
        if ((!(mode->Flags & (V_PHSYNC | V_NHSYNC))) ||
            (!(mode->Flags & (V_PVSYNC | V_NVSYNC))))
        {
            mode->Flags &= ~(V_PHSYNC | V_NHSYNC | V_PVSYNC | V_NVSYNC);

            if (ATILCDPanelID >= 0)
                VDisplay = ATILCDVertical;
            else
                VDisplay = mode->CrtcVDisplay;

            if (VDisplay < 400)
                mode->Flags |= V_PHSYNC | V_NVSYNC;
            else if (VDisplay < 480)
                mode->Flags |= V_NHSYNC | V_PVSYNC;
            else if (VDisplay < 768)
                mode->Flags |= V_NHSYNC | V_NVSYNC;
            else
                mode->Flags |= V_PHSYNC | V_PVSYNC;
        }

        /* Build register contents */
        ATINewHWPtr->crtc_h_total_disp =
            SetBits(mode->CrtcHTotal, CRTC_H_TOTAL) |
                SetBits(mode->CrtcHDisplay, CRTC_H_DISP);
        ATINewHWPtr->crtc_h_sync_strt_wid =
            SetBits(mode->CrtcHSyncStart, CRTC_H_SYNC_STRT) |
                SetBits(mode->CrtcHSkew, CRTC_H_SYNC_DLY) |     /* ? */
                SetBits(GetBits(mode->CrtcHSyncStart, 0x0100U),
                    CRTC_H_SYNC_STRT_HI) |
                SetBits(mode->CrtcHSyncEnd - mode->CrtcHSyncStart,
                    CRTC_H_SYNC_WID);
        if (mode->Flags & V_NHSYNC)
            ATINewHWPtr->crtc_h_sync_strt_wid |= CRTC_H_SYNC_POL;

        ATINewHWPtr->crtc_v_total_disp =
            SetBits(mode->CrtcVTotal, CRTC_V_TOTAL) |
                SetBits(mode->CrtcVDisplay, CRTC_V_DISP);
        ATINewHWPtr->crtc_v_sync_strt_wid =
            SetBits(mode->CrtcVSyncStart, CRTC_V_SYNC_STRT) |
                SetBits(mode->CrtcVSyncEnd - mode->CrtcVSyncStart,
                    CRTC_V_SYNC_WID);
        if (mode->Flags & V_NVSYNC)
            ATINewHWPtr->crtc_v_sync_strt_wid |= CRTC_V_SYNC_POL;

        ATINewHWPtr->crtc_gen_cntl = inl(ATIIOPortCRTC_GEN_CNTL) &
            ~(CRTC_DBL_SCAN_EN | CRTC_INTERLACE_EN |
              CRTC_HSYNC_DIS | CRTC_VSYNC_DIS | CRTC_CSYNC_EN |
              CRTC_PIX_BY_2_EN | CRTC_DISPLAY_DIS | CRTC_VGA_XOVERSCAN |
              CRTC_PIX_WIDTH | CRTC_BYTE_PIX_ORDER | CRTC_FIFO_LWM |
              CRTC_VGA_128KAP_PAGING | CRTC_VFC_SYNC_TRISTATE |
              CRTC_LOCK_REGS |          /* Already off, but ... */
              CRTC_SYNC_TRISTATE | CRTC_DISP_REQ_EN |
              CRTC_VGA_TEXT_132 | CRTC_CUR_B_TEST);
        ATINewHWPtr->crtc_gen_cntl |= CRTC_EXT_DISP_EN | CRTC_EN |
            CRTC_VGA_LINEAR | CRTC_CNT_EN;
        switch (vga256InfoRec.depth)
        {
            case 1:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_1BPP;
                break;
            case 4:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_4BPP;
                break;
            case 8:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_8BPP;
                break;
            case 15:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_15BPP;
                break;
            case 16:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_16BPP;
                break;
            case 24:
                if (vga256InfoRec.bitsPerPixel == 24)
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_24BPP;
                else if (vga256InfoRec.bitsPerPixel == 32)
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_32BPP;
                break;
            case 32:
                ATINewHWPtr->crtc_gen_cntl |= CRTC_PIX_WIDTH_32BPP;
                break;
            default:
                break;
        }
        if (mode->Flags & V_DBLSCAN)
            ATINewHWPtr->crtc_gen_cntl |= CRTC_DBL_SCAN_EN;
        if (mode->Flags & V_INTERLACE)
            ATINewHWPtr->crtc_gen_cntl |= CRTC_INTERLACE_EN;
        if ((mode->Flags & (V_CSYNC | V_PCSYNC)) ||
            (OFLG_ISSET(OPTION_CSYNC, &vga256InfoRec.options)))
            ATINewHWPtr->crtc_gen_cntl |= CRTC_CSYNC_EN;
        /* For now, set display FIFO low water mark as high as possible */
        if (ATIChip < ATI_CHIP_264VTB)
            ATINewHWPtr->crtc_gen_cntl |= CRTC_FIFO_LWM;
    }
}

/*
 * ATIMach64Restore --
 *
 * This function is called to load a Mach64 accelerator's CRTC.
 */
void
ATIMach64Restore(ATIHWPtr restore)
{
    /* First, turn off the display */
    outl(ATIIOPortCRTC_GEN_CNTL, restore->crtc_gen_cntl & ~CRTC_EN);

    if ((restore->FeedbackDivider > 0) &&
        (ATIProgrammableClock != ATI_CLOCK_NONE))
        ATIClockRestore(restore);       /* Programme clock */

    /* Load Mach64 CRTC registers */
    outl(ATIIOPortCRTC_H_TOTAL_DISP, restore->crtc_h_total_disp);
    outl(ATIIOPortCRTC_H_SYNC_STRT_WID, restore->crtc_h_sync_strt_wid);
    outl(ATIIOPortCRTC_V_TOTAL_DISP, restore->crtc_v_total_disp);
    outl(ATIIOPortCRTC_V_SYNC_STRT_WID, restore->crtc_v_sync_strt_wid);
    outl(ATIIOPortCRTC_OFF_PITCH, restore->crtc_off_pitch);

    /* Set pixel clock */
    outl(ATIIOPortCLOCK_CNTL, restore->clock_cntl | CLOCK_STROBE);

    /* Load overscan registers */
    outl(ATIIOPortOVR_CLR, restore->ovr_clr);
    outl(ATIIOPortOVR_WID_LEFT_RIGHT, restore->ovr_wid_left_right);
    outl(ATIIOPortOVR_WID_TOP_BOTTOM, restore->ovr_wid_top_bottom);

    /* Finalize CRTC setup and turn on the screen */
    outl(ATIIOPortCRTC_GEN_CNTL, restore->crtc_gen_cntl);

    /* Aperture setup */
    outl(ATIIOPortBUS_CNTL, restore->bus_cntl);

    outl(ATIIOPortMEM_VGA_WP_SEL, restore->mem_vga_wp_sel);
    outl(ATIIOPortMEM_VGA_RP_SEL, restore->mem_vga_rp_sel);

    outl(ATIIOPortDAC_CNTL, restore->dac_cntl);

    outl(ATIIOPortCONFIG_CNTL, restore->config_cntl);
}
