/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticrtc.c,v 1.1.2.4 2000/05/14 02:02:14 tsi Exp $ */
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
#include "atiadapter.h"
#include "atiadjust.h"
#include "atichip.h"
#include "aticlock.h"
#include "aticonsole.h"
#include "atidac.h"
#include "atidepth.h"
#include "atidsp.h"
#include "atiio.h"
#include "atimach64.h"
#include "atiprint.h"
#include "ativga.h"
#include "atividmem.h"
#include "atiwonder.h"
#include "xf86Procs.h"

/* The CRTC to use for server generated video modes */
CARD8 ATICRTC = ATI_CRTC_VGA;

/* The current video mode */
ATIHWPtr ATICurrentHWPtr;

/* LCD porch data */
static int ATILCDHSyncStart, ATILCDHSyncWidth, ATILCDHBlankWidth,
           ATILCDVSyncStart, ATILCDVSyncWidth, ATILCDVBlankWidth;

/*
 * ATICopyVGAMemory --
 *
 * This function is called by ATISwap to copy to/from one or all banks of a VGA
 * plane.
 */
static void
ATICopyVGAMemory(void **saveptr, void **from, void **to)
{
    unsigned int Bank;

    for (Bank = 0;  Bank < ATICurrentBanks;  Bank++)
    {
        ATISelectBank(Bank);
        (void) memmove(*to, *from, ATI.ChipSegmentSize);
        *saveptr = (char *)(*saveptr) + ATI.ChipSegmentSize;
    }
}

/*
 * ATISwap --
 *
 * This function saves/restores video memory contents during sequencer resets.
 * This is used to remember the mode on server entry and during mode switches.
 */
static void
ATISwap(ATIHWPtr mode, Bool ToFB)
{
    void *save, **from, **to;
    CARD8 seq2, seq4, gra1, gra3, gra4, gra5, gra6, gra8;
    unsigned int Plane = 0, PlaneMask = 1;

    /*
     * This is only done for non-accelerator modes.  If the video state on
     * server entry was an accelerator mode, the application that relinquished
     * the console had better do the Right Thing (tm) anyway by saving and
     * restoring its own video memory.
     */
    if (mode->crtc != ATI_CRTC_VGA)
        return;

    /*
     * There's also no need to do this if the VGA aperture isn't accessible
     * through the adapter being driven.
     */
    if ( /* (mode->crtc == ATI_CRTC_VGA) && */
        (mode != ATINewHWPtr) && (ATIVGAAdapter == ATI_ADAPTER_NONE))
        return;

    if (ToFB)
    {
        if (!mode->frame_buffer)
            return;

        from = &save;
        to = &vgaBase;
    }
    else
    {
        /* Allocate the memory */
        if (!mode->frame_buffer)
        {
            mode->frame_buffer =
                (pointer)xalloc(ATI.ChipSegmentSize * ATICurrentPlanes *
                    ATICurrentBanks);
            if (!mode->frame_buffer)
            {
                ErrorF("Warning:  Temporary frame buffer could not be"
                       " allocated.\n");
                return;
            }
        }

        from = &vgaBase;
        to = &save;
    }

    /* Save register values to be changed */
    seq2 = GetReg(SEQX, 0x02U);
    seq4 = GetReg(SEQX, 0x04U);
    gra1 = GetReg(GRAX, 0x01U);
    gra3 = GetReg(GRAX, 0x03U);
    gra5 = GetReg(GRAX, 0x05U);
    gra6 = GetReg(GRAX, 0x06U);
    gra8 = GetReg(GRAX, 0x08U);

    save = mode->frame_buffer;

    /* Temporarily normalize the current mode */
    if (gra1 != 0x00U)
        PutReg(GRAX, 0x01U, 0x00U);
    if (gra3 != 0x00U)
        PutReg(GRAX, 0x03U, 0x00U);
    if (gra6 != 0x05U)
        PutReg(GRAX, 0x06U, 0x05U);
    if (gra8 != 0xFFU)
        PutReg(GRAX, 0x08U, 0xFFU);

    if (seq4 & 0x08U)
    {
        /* Setup packed mode memory */
        if (seq2 != 0x0FU)
            PutReg(SEQX, 0x02U, 0x0FU);
        if (seq4 != 0x0AU)
            PutReg(SEQX, 0x04U, 0x0AU);
        if (ATIChip < ATI_CHIP_264CT)
        {
            if (gra5 != 0x00U)
                PutReg(GRAX, 0x05U, 0x00U);
        }
        else
        {
            if (gra5 != 0x40U)
                PutReg(GRAX, 0x05U, 0x40U);
        }

        ATICopyVGAMemory(&save, from, to);

        if (seq2 != 0x0FU)
            PutReg(SEQX, 0x02U, seq2);
        if (seq4 != 0x0AU)
            PutReg(SEQX, 0x04U, seq4);
        if (ATIChip < ATI_CHIP_264CT)
        {
            if (gra5 != 0x00U)
                PutReg(GRAX, 0x05U, gra5);
        }
        else
        {
            if (gra5 != 0x40U)
                PutReg(GRAX, 0x05U, gra5);
        }
    }
    else
    {
        gra4 = GetReg(GRAX, 0x04U);

        /* Setup planar mode memory */
        if (seq4 != 0x06U)
            PutReg(SEQX, 0x04U, 0x06U);
        if (gra5 != 0x00U)
            PutReg(GRAX, 0x05U, 0x00U);

        for (;  Plane < ATICurrentPlanes;  Plane++)
        {
            PutReg(SEQX, 0x02U, PlaneMask);
            PutReg(GRAX, 0x04U, Plane);
            ATICopyVGAMemory(&save, from, to);
            PlaneMask <<= 1;
        }

        PutReg(SEQX, 0x02U, seq2);
        if (seq4 != 0x06U)
            PutReg(SEQX, 0x04U, seq4);
        PutReg(GRAX, 0x04U, gra4);
        if (gra5 != 0x00U)
            PutReg(GRAX, 0x05U, gra5);
    }

    /* Restore registers */
    if (gra1 != 0x00U)
        PutReg(GRAX, 0x01U, gra1);
    if (gra3 != 0x00U)
        PutReg(GRAX, 0x03U, gra3);
    if (gra6 != 0x05U)
        PutReg(GRAX, 0x06U, gra6);
    if (gra8 != 0xFFU)
        PutReg(GRAX, 0x08U, gra8);

    ATISelectBank(0);                   /* Reset to bank 0 */

    /*
     * If restoring video memory for a server video mode, free the frame buffer
     * save area.
     */
    if (ToFB && (mode != (ATIHWPtr)vgaOrigVideoState))
    {
        xfree(mode->frame_buffer);
        mode->frame_buffer = NULL;
    }
}

/*
 * ATISave --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the ATIHWRec data structure.  There is in general no need to mask out
 * bits here - just read the registers.
 */
void *
ATISave(void *data)
{
    ATIHWPtr save = data;
    int      Index;

    /* If need be, allocate the data structure */
    if (!save)
        save = (ATIHWPtr)xcalloc(1, SizeOf(ATIHWRec));

    /* Unlock registers */
    ATIEnterLeave(ENTER);

    /* Figure out what CRTC got us into this mess */
    save->crtc = ATI_CRTC_VGA;
#if 0   /* Not yet */
    if (ATIChipHasSUBSYS_CNTL)
    {
    }
    else
#endif
    if (ATIChip >= ATI_CHIP_88800GXC)
    {
        save->crtc_gen_cntl = inl(ATIIOPortCRTC_GEN_CNTL);
        if (save->crtc_gen_cntl & CRTC_EXT_DISP_EN)
            save->crtc = ATI_CRTC_MACH64;
    }

    ATISelectBank(0);                   /* Get back to bank 0 */
    save->bank_function = ATISelectBankFunction;
    save->banks = ATICurrentBanks;
    save->planes = ATICurrentPlanes;

    /* Save clock data */
    ATIClockSave(save);
    if (ATIChip >= ATI_CHIP_264CT)
    {
        save->pll_vclk_cntl = ATIGetMach64PLLReg(PLL_VCLK_CNTL) |
            PLL_VCLK_RESET;
        save->pll_vclk_post_div = ATIGetMach64PLLReg(PLL_VCLK_POST_DIV);
        save->pll_vclk0_fb_div = ATIGetMach64PLLReg(PLL_VCLK0_FB_DIV);
        save->pll_vclk1_fb_div = ATIGetMach64PLLReg(PLL_VCLK1_FB_DIV);
        save->pll_vclk2_fb_div = ATIGetMach64PLLReg(PLL_VCLK2_FB_DIV);
        save->pll_vclk3_fb_div = ATIGetMach64PLLReg(PLL_VCLK3_FB_DIV);
        save->pll_xclk_cntl = ATIGetMach64PLLReg(PLL_XCLK_CNTL);
        if (ATIChip >= ATI_CHIP_264LT)
            save->pll_ext_vpll_cntl = ATIGetMach64PLLReg(PLL_EXT_VPLL_CNTL);
    }

    /* Save LCD registers */
    if (ATILCDPanelID >= 0)
    {
        if (ATIChip == ATI_CHIP_264LT)
        {
            save->horz_stretching = inl(ATIIOPortHORZ_STRETCHING);
            save->vert_stretching = inl(ATIIOPortVERT_STRETCHING);
            save->lcd_gen_ctrl = inl(ATIIOPortLCD_GEN_CTRL);

            /* Setup to save non-shadow registers */
            outl(ATIIOPortLCD_GEN_CTRL, save->lcd_gen_ctrl &
                ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN));
        }
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
        {
            save->lcd_index = inl(ATIIOPortLCD_INDEX);
            save->config_panel = ATIGetLTProLCDReg(LCD_CONFIG_PANEL);
            save->lcd_gen_ctrl = ATIGetLTProLCDReg(LCD_GEN_CNTL);
            save->horz_stretching = ATIGetLTProLCDReg(LCD_HORZ_STRETCHING);
            save->vert_stretching = ATIGetLTProLCDReg(LCD_VERT_STRETCHING);
            save->ext_vert_stretch = ATIGetLTProLCDReg(LCD_EXT_VERT_STRETCH);

            /* Setup to save non-shadow registers */
            ATIPutLTProLCDReg(LCD_GEN_CNTL, save->lcd_gen_ctrl &
                ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN));
        }
    }

    switch (save->crtc)
    {
        case ATI_CRTC_VGA:
            if (ATIVGAAdapter != ATI_ADAPTER_NONE)
            {
                /* Save VGA Wonder registers */
                if (ATIChipHasVGAWonder)
                    ATIVGAWonderSave(save);

                /* Save VGA registers */
                ATIVGASave(save);
            }

            if (ATIChip >= ATI_CHIP_88800GXC)
            {
                save->crtc_off_pitch = inl(ATIIOPortCRTC_OFF_PITCH);
                save->config_cntl = inl(ATIIOPortCONFIG_CNTL);
                save->mem_vga_wp_sel = inl(ATIIOPortMEM_VGA_WP_SEL);
                save->mem_vga_rp_sel = inl(ATIIOPortMEM_VGA_RP_SEL);
                save->dac_cntl = inl(ATIIOPortDAC_CNTL);
                if (ATIChip >= ATI_CHIP_264VTB)
                    save->bus_cntl = inl(ATIIOPortBUS_CNTL);
            }
            break;

        case ATI_CRTC_MACH64:
            /* Save VGA registers */
            ATIVGASave(save);

            /* Save Mach64 data */
            ATIMach64Save(save);
            break;

        default:
            break;
    }

    if ((ATIChip >= ATI_CHIP_264VTB) && (ATIIODecoding == BLOCK_IO))
        ATIDSPSave(save);

    if (ATILCDPanelID >= 0)
    {
        /* Switch to shadow registers */
        if (ATIChip == ATI_CHIP_264LT)
            outl(ATIIOPortLCD_GEN_CTRL,
                (save->lcd_gen_ctrl & ~CRTC_RW_SELECT) |
                (SHADOW_EN | SHADOW_RW_EN));
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
            ATIPutLTProLCDReg(LCD_GEN_CNTL,
                (save->lcd_gen_ctrl & ~CRTC_RW_SELECT) |
                (SHADOW_EN | SHADOW_RW_EN));

        /* Save shadow VGA CRTC registers */
        for (Index = 0;  Index < NumberOf(save->shadow_vga);  Index++)
            save->shadow_vga[Index] = GetReg(CRTX(vgaIOBase), Index);

        /* Save shadow Mach64 CRTC registers */
        save->shadow_h_total_disp = inl(ATIIOPortCRTC_H_TOTAL_DISP);
        save->shadow_h_sync_strt_wid = inl(ATIIOPortCRTC_H_SYNC_STRT_WID);
        save->shadow_v_total_disp = inl(ATIIOPortCRTC_V_TOTAL_DISP);
        save->shadow_v_sync_strt_wid = inl(ATIIOPortCRTC_V_SYNC_STRT_WID);

        /* Restore CRTC selection and shadow state */
        if (ATIChip == ATI_CHIP_264LT)
            outl(ATIIOPortLCD_GEN_CTRL, save->lcd_gen_ctrl);
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
        {
            ATIPutLTProLCDReg(LCD_GEN_CNTL, save->lcd_gen_ctrl);
            outl(ATIIOPortLCD_INDEX, save->lcd_index);
        }
    }

    /* Save RAMDAC state */
    ATIDACSave(save);

    /*
     * The server has already saved video memory when switching out of its
     * virtual console, so don't do it again.
     */
    if (save != ATINewHWPtr)
    {
        ATICurrentHWPtr = save;         /* Keep track of current mode */
        save->mode = NULL;              /* No corresponding mode line */
        save->FeedbackDivider = 0;      /* Don't programme clock */

        ATISwap(save, FALSE);           /* Save video memory */
    }

    (void) inb(GENS1(vgaIOBase));       /* Reset flip-flop */
    outb(ATTRX, 0x20U);                 /* Turn on PAS */

    SetTimeSinceLastInputEvent();

    return save;
}

/*
 * ATIInit --
 *
 * This function fills in the ATIHWRec with all of the register values needed
 * to enable a video mode.  It's important that this be done without modifying
 * the current video state.
 */
Bool
ATIInit(DisplayModePtr mode)
{
    CARD32 lcd_index;
    int    Index;

    /* Unlock registers */
    ATIEnterLeave(ENTER);

    if (ATINewHWPtr == NULL)
    {
        /* Initialize ATIAdjust */
        ATIAdjustInit();

        /*
         * Check limits related to the virtual width.  A better place for this
         * would be in ATIValidMode were it not for the fact that the virtual
         * width isn't necessarily known then.
         */
        switch (ATICRTC)
        {
            case ATI_CRTC_VGA:
                if (vga256InfoRec.displayWidth >= 4096)
                {
                    ErrorF("Virtual resolution is too wide.\n");
                    return FALSE;
                }

                if (ATIUsing1bppModes)
                {
                    /*
                     * Prune interlaced modes if the virtual width is too
                     * large.
                     */
                    if ((ATIChip <= ATI_CHIP_28800_6) &&
                        (vga256InfoRec.displayWidth >= 2048))
                    {
                        DisplayModePtr Next, Deleted = NULL;
                        DisplayModePtr Original = mode;

                        for (;  mode;  mode = Next)
                        {
                            Next = mode->next;
                            if (Next == vga256InfoRec.modes)
                                Next = NULL;
                            if (!(mode->Flags & V_INTERLACE))
                                continue;
                            if (!Deleted)
                            {
                                Deleted = mode;
                                ErrorF("Interlaced modes are not supported at"
                                       " this virtual width.\n See README.ati"
                                       " for more information.\n");
                            }
                            xf86DeleteMode(&vga256InfoRec, mode);
                        }

                        /* Reset to first remaining mode */
                        if (!(mode = vga256InfoRec.modes))
                        {
                            ErrorF("Oops!  No modes left!\n");
                            return FALSE;
                        }

                        /* The physical screen dimensions might have changed */
                        if (mode != Original)
                            xf86InitViewport(&vga256InfoRec);
                    }
                }
                else if (!ATIUsingPlanarModes)
                /* Packed modes have lower limits in some cases */
                if ((vga256InfoRec.displayWidth >= 2048) &&
                    ((ATIChip >= ATI_CHIP_264CT) ||
                     ((ATIChip <= ATI_CHIP_18800) && (ATIvideoRam == 256))))
                {
                    ErrorF("Virtual resolution is too wide.\n");
                    return FALSE;
                }
                break;

            case ATI_CRTC_MACH64:
                if (vga256InfoRec.displayWidth <=
                    (int)(GetBits(CRTC_PITCH, CRTC_PITCH) << 3))
                    break;
                ErrorF("Virtual resolution is too wide.\n");
                return FALSE;

            default:
                break;
        }

        /*
         * Allocate and clear the data structure.  Then, initialize it with the
         * data that is to remain constant for all modes used by the server.
         */
        vgaNewVideoState = (void *)xcalloc(1, SizeOf(ATIHWRec));

        /* Set the CRTC that will be used to generate the modes */
        ATINewHWPtr->crtc = ATICRTC;

        /* Set clock maps */
        ATINewHWPtr->ClockMap = ATIClockMap;
        ATINewHWPtr->ClockUnMap = ATIClockUnMap;

        /* Setup for ATISwap */
        ATINewHWPtr->bank_function = ATI.ChipSetReadWrite;
        ATINewHWPtr->banks = ATIMaximumBanks;
        if (ATIUsingPlanarModes)
            ATINewHWPtr->planes = 4;
        else
            ATINewHWPtr->planes = 1;

        switch (ATICRTC)
        {
            case ATI_CRTC_VGA:
                /* Fill in VGA Wonder data */
                if (ATIChipHasVGAWonder)
                    ATIVGAWonderInit(NULL);

                /* Fill in VGA data */
                ATIVGAInit(NULL);

                if (ATIChip >= ATI_CHIP_264CT)
                {
                    ATINewHWPtr->config_cntl = inl(ATIIOPortCONFIG_CNTL);
                    ATINewHWPtr->mem_vga_wp_sel =
                        /* SetBits(0, MEM_VGA_WPS0) + */
                        SetBits(ATINewHWPtr->planes, MEM_VGA_WPS1);
                    ATINewHWPtr->mem_vga_rp_sel =
                        /* SetBits(0, MEM_VGA_RPS0) + */
                        SetBits(ATINewHWPtr->planes, MEM_VGA_RPS1);
                    ATINewHWPtr->dac_cntl = inl(ATIIOPortDAC_CNTL);
                    if (vga256InfoRec.depth > 8)
                        ATINewHWPtr->dac_cntl |= DAC_8BIT_EN;
                    else
                        ATINewHWPtr->dac_cntl &= ~DAC_8BIT_EN;
                    if (ATIUsingSmallApertures)
                        ATINewHWPtr->config_cntl |= CFG_MEM_VGA_AP_EN;
                    else
                        ATINewHWPtr->config_cntl &= ~CFG_MEM_VGA_AP_EN;
                    if (ATIChip >= ATI_CHIP_264VTB)
                        ATINewHWPtr->bus_cntl = (inl(ATIIOPortBUS_CNTL) &
                            ~(BUS_HOST_ERR_INT_EN | BUS_ROM_DIS)) |
                            (BUS_HOST_ERR_INT | BUS_APER_REG_DIS);
                }
                break;

            case ATI_CRTC_MACH64:
                /* Fill in VGA Wonder data */
                if (ATIChipHasVGAWonder)
                    ATIVGAWonderInit(NULL);

                /* Fill in mode-independent VGA data */
                ATIVGAInit(NULL);

                /* Fill in Mach64 accelerator data */
                ATIMach64Init(NULL);
                break;

            default:
                break;
        }

        if (ATIChip >= ATI_CHIP_264CT)
        {
            /* Ensure proper VPLL clock source */
            ATINewHWPtr->pll_vclk_cntl = ATIGetMach64PLLReg(PLL_VCLK_CNTL) |
                (PLL_VCLK_SRC_SEL | PLL_VCLK_RESET);

            /* Set provisional values for certain other PLL registers */
            ATINewHWPtr->pll_vclk_post_div =
                ATIGetMach64PLLReg(PLL_VCLK_POST_DIV);
            ATINewHWPtr->pll_vclk0_fb_div =
                ATIGetMach64PLLReg(PLL_VCLK0_FB_DIV);
            ATINewHWPtr->pll_vclk1_fb_div =
                ATIGetMach64PLLReg(PLL_VCLK1_FB_DIV);
            ATINewHWPtr->pll_vclk2_fb_div =
                ATIGetMach64PLLReg(PLL_VCLK2_FB_DIV);
            ATINewHWPtr->pll_vclk3_fb_div =
                ATIGetMach64PLLReg(PLL_VCLK3_FB_DIV);
            ATINewHWPtr->pll_xclk_cntl = ATIGetMach64PLLReg(PLL_XCLK_CNTL);

            /* For now disable extended reference and feedback dividers */
            if (ATIChip >= ATI_CHIP_264LT)
                ATINewHWPtr->pll_ext_vpll_cntl =
                    ATIGetMach64PLLReg(PLL_EXT_VPLL_CNTL) &
                        ~(PLL_EXT_VPLL_EN | PLL_EXT_VPLL_VGA_EN |
                          PLL_EXT_VPLL_INSYNC);
        }

        if (ATILCDPanelID >= 0)
        {
            int HDisplay, VDisplay;

            if (ATIChip == ATI_CHIP_264LT)
            {
                ATINewHWPtr->horz_stretching = inl(ATIIOPortHORZ_STRETCHING);
                ATINewHWPtr->vert_stretching = inl(ATIIOPortVERT_STRETCHING);
                ATINewHWPtr->lcd_gen_ctrl = inl(ATIIOPortLCD_GEN_CTRL);
            }
            else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                        (ATIChip == ATI_CHIP_264XL) ||
                        (ATIChip == ATI_CHIP_MOBILITY)) */
            {
                lcd_index = inl(ATIIOPortLCD_INDEX);
                ATINewHWPtr->lcd_index = (lcd_index &
                    ~(LCD_REG_INDEX | LCD_DISPLAY_DIS | LCD_SRC_SEL)) |
                    (LCD_SRC_SEL_CRTC1 | LCD_CRTC2_DISPLAY_DIS);
                ATINewHWPtr->config_panel =
                    ATIGetLTProLCDReg(LCD_CONFIG_PANEL) | DONT_SHADOW_HEND;
                ATINewHWPtr->lcd_gen_ctrl = ATIGetLTProLCDReg(LCD_GEN_CNTL);
                ATINewHWPtr->horz_stretching =
                    ATIGetLTProLCDReg(LCD_HORZ_STRETCHING);
                ATINewHWPtr->vert_stretching =
                    ATIGetLTProLCDReg(LCD_VERT_STRETCHING);
                outl(ATIIOPortLCD_INDEX, lcd_index);
            }

            /*
             * Use primary CRTC to drive the panel.  Turn off CRT interface.
             */
            ATINewHWPtr->lcd_gen_ctrl = (ATINewHWPtr->lcd_gen_ctrl &
                ~(CRT_ON | HORZ_DIVBY2_EN | DISABLE_PCLK_RESET |
                  DIS_HOR_CRT_DIVBY2 | VCLK_DAC_PM_EN | XTALIN_PM_EN |
                  CRTC_RW_SELECT | USE_SHADOWED_VEND | USE_SHADOWED_ROWCUR |
                  SHADOW_EN | SHADOW_RW_EN)) |
                (DONT_SHADOW_VPAR | LOCK_8DOT);

            /*
             * Determine porch data.  The intent here is to produce stretched
             * modes that approximate the horizontal sync and vertical refresh
             * rates of the mode on server entry (which, BTW, might not have
             * been saved yet).  Use the newly allocated ATIHWRec for temporary
             * register storage.
             */
            if (inl(ATIIOPortCRTC_GEN_CNTL) & CRTC_EXT_DISP_EN)
            {
                ATINewHWPtr->crtc_h_total_disp =
                    inl(ATIIOPortCRTC_H_TOTAL_DISP);
                ATINewHWPtr->crtc_h_sync_strt_wid =
                    inl(ATIIOPortCRTC_H_SYNC_STRT_WID);
                ATINewHWPtr->crtc_v_total_disp =
                    inl(ATIIOPortCRTC_V_TOTAL_DISP);
                ATINewHWPtr->crtc_v_sync_strt_wid =
                    inl(ATIIOPortCRTC_V_SYNC_STRT_WID);

                HDisplay =
                    GetBits(ATINewHWPtr->crtc_h_total_disp, CRTC_H_DISP);
                VDisplay =
                    GetBits(ATINewHWPtr->crtc_v_total_disp, CRTC_V_DISP);

                ATILCDHSyncStart =
                    (GetBits(ATINewHWPtr->crtc_h_sync_strt_wid,
                        CRTC_H_SYNC_STRT_HI) * (MaxBits(CRTC_H_SYNC_STRT) + 1))
                    +
                    GetBits(ATINewHWPtr->crtc_h_sync_strt_wid,
                        CRTC_H_SYNC_STRT) - HDisplay;
                ATILCDHSyncWidth =
                    GetBits(ATINewHWPtr->crtc_h_sync_strt_wid,
                        CRTC_H_SYNC_WID);
                ATILCDHBlankWidth =
                    GetBits(ATINewHWPtr->crtc_h_total_disp, CRTC_H_TOTAL) -
                    HDisplay;
                ATILCDVSyncStart =
                    GetBits(ATINewHWPtr->crtc_v_sync_strt_wid,
                        CRTC_V_SYNC_STRT) - VDisplay;
                ATILCDVSyncWidth =
                    GetBits(ATINewHWPtr->crtc_v_sync_strt_wid,
                        CRTC_V_SYNC_WID);
                ATILCDVBlankWidth =
                    GetBits(ATINewHWPtr->crtc_v_total_disp, CRTC_V_TOTAL) -
                    VDisplay;
            }
            else
            {
                ATINewHWPtr->std.CRTC[0] = GetReg(CRTX(vgaIOBase), 0x00U);
                ATINewHWPtr->std.CRTC[1] = GetReg(CRTX(vgaIOBase), 0x01U);
                ATINewHWPtr->std.CRTC[4] = GetReg(CRTX(vgaIOBase), 0x04U);
                ATINewHWPtr->std.CRTC[5] = GetReg(CRTX(vgaIOBase), 0x05U);
                ATINewHWPtr->std.CRTC[6] = GetReg(CRTX(vgaIOBase), 0x06U);
                ATINewHWPtr->std.CRTC[7] = GetReg(CRTX(vgaIOBase), 0x07U);
                ATINewHWPtr->std.CRTC[16] = GetReg(CRTX(vgaIOBase), 0x10U);
                ATINewHWPtr->std.CRTC[17] = GetReg(CRTX(vgaIOBase), 0x11U);
                ATINewHWPtr->std.CRTC[18] = GetReg(CRTX(vgaIOBase), 0x12U);

                HDisplay = ATINewHWPtr->std.CRTC[1] + 1;
                VDisplay = (((ATINewHWPtr->std.CRTC[7] << 3) & 0x0200U) |
                            ((ATINewHWPtr->std.CRTC[7] << 7) & 0x0100U) |
                            ATINewHWPtr->std.CRTC[18]) + 1;

                ATILCDHSyncStart = ATINewHWPtr->std.CRTC[4] - HDisplay;
                ATILCDHSyncWidth = 0x1FU &
                    (ATINewHWPtr->std.CRTC[5] - ATINewHWPtr->std.CRTC[4]);
                ATILCDHBlankWidth = ATINewHWPtr->std.CRTC[0] + 5 - HDisplay;
                ATILCDVSyncStart =
                    (((ATINewHWPtr->std.CRTC[7] << 2) & 0x0200U) |
                     ((ATINewHWPtr->std.CRTC[7] << 6) & 0x0100U) |
                     ATINewHWPtr->std.CRTC[16]) - VDisplay;
                ATILCDVSyncWidth = 0x0FU &
                    (ATINewHWPtr->std.CRTC[17] - ATINewHWPtr->std.CRTC[16]);
                ATILCDVBlankWidth =
                    (((ATINewHWPtr->std.CRTC[7] << 4) & 0x0200U) |
                     ((ATINewHWPtr->std.CRTC[7] << 8) & 0x0100U) |
                     ATINewHWPtr->std.CRTC[6]) + 2 - VDisplay;
            }

            HDisplay <<= 3;
            ATILCDHSyncStart <<= 3;
            ATILCDHSyncWidth <<= 3;
            ATILCDHBlankWidth <<= 3;

            /* If the mode on entry isn't stretched, adjust timings */
            if (!(ATINewHWPtr->horz_stretching & HORZ_STRETCH_EN) &&
                ((HDisplay = ATILCDHorizontal - HDisplay) > 0))
            {
                if ((ATILCDHSyncStart -= HDisplay) < 0)
                    ATILCDHSyncStart = 0;
                ATILCDHBlankWidth -= HDisplay;
                HDisplay = ATILCDHSyncStart + ATILCDHSyncWidth;
                if (ATILCDHBlankWidth < HDisplay)
                    ATILCDHBlankWidth = HDisplay;
            }

            if (!(ATINewHWPtr->vert_stretching & VERT_STRETCH_EN) &&
                ((VDisplay = ATILCDVertical - VDisplay) > 0))
            {
                if ((ATILCDVSyncStart -= VDisplay) < 0)
                    ATILCDVSyncStart = 0;
                ATILCDVBlankWidth -= VDisplay;
                VDisplay = ATILCDVSyncStart + ATILCDVSyncWidth;
                if (ATILCDVBlankWidth < VDisplay)
                    ATILCDVBlankWidth = VDisplay;
            }
        }

        /* Set RAMDAC data */
        ATIDACInit(NULL);
    }

    ATINewHWPtr->mode = mode;           /* Link with mode line */

    if (!(mode->CrtcHAdjusted) && /* !(mode->CrtcVAdjusted) && */
        (ATILCDPanelID >= 0))
    {
        /* Clobber mode timings */
        mode->Flags &= ~(V_DBLSCAN | V_INTERLACE | V_CLKDIV2);

        mode->HSyncStart = mode->HDisplay + ATILCDHSyncStart;
        mode->HSyncEnd = mode->HSyncStart + ATILCDHSyncWidth;
        mode->HTotal = mode->HDisplay + ATILCDHBlankWidth;

        mode->VSyncStart = mode->VDisplay + ATILCDVSyncStart;
        mode->VSyncEnd = mode->VSyncStart + ATILCDVSyncWidth;
        mode->VTotal = mode->VDisplay + ATILCDVBlankWidth;
    }

    switch (ATINewHWPtr->crtc)
    {
        case ATI_CRTC_VGA:
            /* Fill in VGA data */
            ATIVGAInit(mode);

            /* Fill in VGA Wonder data */
            if (ATIChipHasVGAWonder)
                ATIVGAWonderInit(mode);

            if (ATIChip >= ATI_CHIP_88800GXC)
            {
                ATINewHWPtr->crtc_gen_cntl = inl(ATIIOPortCRTC_GEN_CNTL) &
                    ~(CRTC_DBL_SCAN_EN | CRTC_INTERLACE_EN |
                      CRTC_HSYNC_DIS | CRTC_VSYNC_DIS | CRTC_CSYNC_EN |
                      CRTC_PIX_BY_2_EN | CRTC_DISPLAY_DIS |
                      CRTC_VGA_XOVERSCAN | CRTC_VGA_128KAP_PAGING |
                      CRTC_VFC_SYNC_TRISTATE |
                      CRTC_LOCK_REGS |  /* Already off, but ... */
                      CRTC_SYNC_TRISTATE | CRTC_EXT_DISP_EN |
                      CRTC_DISP_REQ_EN | CRTC_VGA_LINEAR | CRTC_VGA_TEXT_132 |
                      CRTC_CUR_B_TEST);
#if 0           /* This isn't needed, but is kept for reference */
                if (mode->Flags & V_DBLSCAN)
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_DBL_SCAN_EN;
#endif
                if (mode->Flags & V_INTERLACE)
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_INTERLACE_EN;
                if ((mode->Flags & (V_CSYNC | V_PCSYNC)) ||
                    (OFLG_ISSET(OPTION_CSYNC, &vga256InfoRec.options)))
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_CSYNC_EN;
                if (ATIUsingPlanarModes)
                {
                    ATINewHWPtr->crtc_gen_cntl |= CRTC_EN | CRTC_CNT_EN;
                    ATINewHWPtr->crtc_off_pitch =
                        SetBits(vga256InfoRec.displayWidth >> 4, CRTC_PITCH);
                }
                else
                {
                    ATINewHWPtr->crtc_gen_cntl |=
                        CRTC_EN | CRTC_VGA_LINEAR | CRTC_CNT_EN;
                    ATINewHWPtr->crtc_off_pitch =
                        SetBits(vga256InfoRec.displayWidth >> 3, CRTC_PITCH);
                }
            }
            break;

        case ATI_CRTC_MACH64:
            /* Fill in Mach64 accelerator data */
            ATIMach64Init(mode);
            break;

        default:
            break;
    }

    /* Set LCD register values */
    if (ATILCDPanelID >= 0)
    {
        if (ATIChip == ATI_CHIP_264LT)
            ATINewHWPtr->horz_stretching = inl(ATIIOPortHORZ_STRETCHING);
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
        {
            lcd_index = inl(ATIIOPortLCD_INDEX);
            ATINewHWPtr->horz_stretching =
                ATIGetLTProLCDReg(LCD_HORZ_STRETCHING);
            ATINewHWPtr->ext_vert_stretch =
                ATIGetLTProLCDReg(LCD_EXT_VERT_STRETCH) &
                  ~(AUTO_VERT_RATIO | VERT_STRETCH_MODE);

            /*
             * FIXME:  On a 1024x768 panel, vertical blending does not work
             *         when HDisplay is greater than 896.
             */
            if ((mode->HDisplay < ATILCDHorizontal) &&
                (mode->VDisplay < ATILCDVertical))
                ATINewHWPtr->ext_vert_stretch |= VERT_STRETCH_MODE;

            outl(ATIIOPortLCD_INDEX, lcd_index);
        }

        ATINewHWPtr->horz_stretching &=
            ~(HORZ_STRETCH_RATIO | HORZ_STRETCH_LOOP | AUTO_HORZ_RATIO |
              HORZ_STRETCH_MODE | HORZ_STRETCH_EN);
        if (mode->HDisplay < ATILCDHorizontal)
            ATINewHWPtr->horz_stretching |=
                SetBits(((mode->HDisplay & ~7) *
                    (MaxBits(HORZ_STRETCH_BLEND) + 1)) /
                    ATILCDHorizontal, HORZ_STRETCH_BLEND) |
                (HORZ_STRETCH_MODE | HORZ_STRETCH_EN);

        if (mode->VDisplay >= ATILCDVertical)
            ATINewHWPtr->vert_stretching = 0;
        else
        {
            ATINewHWPtr->vert_stretching =
                SetBits((mode->VDisplay * (MaxBits(VERT_STRETCH_RATIO0) + 1)) /
                    ATILCDVertical, VERT_STRETCH_RATIO0);
            ATINewHWPtr->vert_stretching |=
                VERT_STRETCH_USE0 | VERT_STRETCH_EN;
        }

        /* Copy non-shadow CRTC register values to the shadow set */
        for (Index = 0;  Index < NumberOf(ATINewHWPtr->std.CRTC);  Index++)
            ATINewHWPtr->shadow_vga[Index] = ATINewHWPtr->std.CRTC[Index];

        ATINewHWPtr->shadow_h_total_disp = ATINewHWPtr->crtc_h_total_disp;
        ATINewHWPtr->shadow_h_sync_strt_wid =
            ATINewHWPtr->crtc_h_sync_strt_wid;
        ATINewHWPtr->shadow_v_total_disp = ATINewHWPtr->crtc_v_total_disp;
        ATINewHWPtr->shadow_v_sync_strt_wid =
            ATINewHWPtr->crtc_v_sync_strt_wid;
    }

    /* Fill in RAMDAC data */
    ATIDACInit(mode);

    /* Setup clock programming and selection */
    return ATIClockInit(mode);
}

/*
 * ATIRestore --
 *
 * This function sets a video mode.  It basically writes out all of the
 * registers that have previously been saved in the ATIHWRec data structure.
 *
 * Note that "Restore" is slightly incorrect.  This function is also used when
 * the server enters/changes video modes.  The mode definitions have previously
 * been initialized by the ATIInit() function.
 */
void
ATIRestore(void *data)
{
    ATIHWPtr restore = data;
    int      Index;

    /* Unlock registers */
    ATIEnterLeave(ENTER);

    ATISelectBank(0);                   /* Get back to bank 0 */

    /*
     * If switching from one server-generated mode to another, preserve video
     * memory contents across sequencer resets.  This is only necessary for
     * 18800 and 18800-1 adapters.
     */
    if ((ATIChip <= ATI_CHIP_18800_1) &&
        (ATICurrentHWPtr != (ATIHWPtr)vgaOrigVideoState) &&
        (restore != (ATIHWPtr)vgaOrigVideoState))
        ATISwap(restore, FALSE);
    ATICurrentHWPtr = restore;

    /* Reset ATISwap setup to that needed by the mode to be restored */
    ATISelectBankFunction = restore->bank_function;
    ATICurrentBanks = restore->banks;
    ATICurrentPlanes = restore->planes;

    if (ATIChip >= ATI_CHIP_264CT)
    {
        ATIPutMach64PLLReg(PLL_VCLK_CNTL, restore->pll_vclk_cntl);
        ATIPutMach64PLLReg(PLL_VCLK_POST_DIV, restore->pll_vclk_post_div);
        ATIPutMach64PLLReg(PLL_VCLK0_FB_DIV, restore->pll_vclk0_fb_div);
        ATIPutMach64PLLReg(PLL_VCLK1_FB_DIV, restore->pll_vclk1_fb_div);
        ATIPutMach64PLLReg(PLL_VCLK2_FB_DIV, restore->pll_vclk2_fb_div);
        ATIPutMach64PLLReg(PLL_VCLK3_FB_DIV, restore->pll_vclk3_fb_div);
        ATIPutMach64PLLReg(PLL_XCLK_CNTL, restore->pll_xclk_cntl);
        if (ATIChip >= ATI_CHIP_264LT)
            ATIPutMach64PLLReg(PLL_EXT_VPLL_CNTL, restore->pll_ext_vpll_cntl);
        ATIPutMach64PLLReg(PLL_VCLK_CNTL,
            restore->pll_vclk_cntl & ~PLL_VCLK_RESET);
    }

    /* Load LCD registers */
    if (ATILCDPanelID >= 0)
    {
        /* Stop CRTC */
        outl(ATIIOPortCRTC_GEN_CNTL, restore->crtc_gen_cntl &
            ~(CRTC_EXT_DISP_EN | CRTC_EN));

        if (ATIChip == ATI_CHIP_264LT)
        {
            /* Update non-shadow registers first */
            outl(ATIIOPortLCD_GEN_CTRL, restore->lcd_gen_ctrl &
                ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
                  SHADOW_RW_EN));

            /* Temporarily disable stretching */
            outl(ATIIOPortHORZ_STRETCHING, restore->horz_stretching &
                ~(HORZ_STRETCH_MODE | HORZ_STRETCH_EN));
            outl(ATIIOPortVERT_STRETCHING, restore->vert_stretching &
                ~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
                  VERT_STRETCH_USE0 | VERT_STRETCH_EN));
        }
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
        {
            /* Update non-shadow registers first */
            ATIPutLTProLCDReg(LCD_CONFIG_PANEL, restore->config_panel);
            ATIPutLTProLCDReg(LCD_GEN_CNTL, restore->lcd_gen_ctrl &
                ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
                  SHADOW_RW_EN));

            /* Temporarily disable stretching */
            ATIPutLTProLCDReg(LCD_HORZ_STRETCHING, restore->horz_stretching &
                ~(HORZ_STRETCH_MODE | HORZ_STRETCH_EN));
            ATIPutLTProLCDReg(LCD_VERT_STRETCHING, restore->vert_stretching &
                ~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
                  VERT_STRETCH_USE0 | VERT_STRETCH_EN));
        }
    }

    switch (restore->crtc)
    {
        case ATI_CRTC_VGA:
            if (ATIVGAAdapter != ATI_ADAPTER_NONE)
            {
                ATISetVGAIOBase(restore->std.MiscOutReg);

                if (ATIChip >= ATI_CHIP_88800GXC)
                    outl(ATIIOPortCRTC_GEN_CNTL,
                        restore->crtc_gen_cntl & ~CRTC_EN);

                /* Start sequencer reset */
                PutReg(SEQX, 0x00U, 0x00U);
            }

            /* Set the pixel clock */
            if ((restore->FeedbackDivider > 0) &&
                (ATIProgrammableClock != ATI_CLOCK_FIXED))
                ATIClockRestore(restore);

            if (ATIVGAAdapter != ATI_ADAPTER_NONE)
            {
                /* Restore VGA Wonder registers */
                if (ATIChipHasVGAWonder)
                    ATIVGAWonderRestore(restore);

                /* Load VGA device */
                ATIVGARestore(restore);
            }

            if (ATIChip >= ATI_CHIP_88800GXC)
            {
                outl(ATIIOPortCRTC_GEN_CNTL, restore->crtc_gen_cntl);
                outl(ATIIOPortMEM_VGA_WP_SEL, restore->mem_vga_wp_sel);
                outl(ATIIOPortMEM_VGA_RP_SEL, restore->mem_vga_rp_sel);
                if (ATIChip >= ATI_CHIP_264CT)
                {
                    outl(ATIIOPortCRTC_OFF_PITCH, restore->crtc_off_pitch);
                    outl(ATIIOPortDAC_CNTL, restore->dac_cntl);
                    outl(ATIIOPortCONFIG_CNTL, restore->config_cntl);
                    outl(ATIIOPortBUS_CNTL, restore->bus_cntl);
                }
            }

            if (ATIVGAAdapter != ATI_ADAPTER_NONE)
            {
                /* Give LUT access to CRTC */
                (void) inb(GENS1(vgaIOBase));
                outb(ATTRX, 0x20U);
            }
            break;

        case ATI_CRTC_MACH64:
            /* Load Mach64 CRTC registers */
            ATIMach64Restore(restore);

            if (ATIUsingSmallApertures)
            {
                /* Oddly enough, these need to be set also, maybe others */
                PutReg(SEQX, 0x02U, restore->std.Sequencer[2]);
                PutReg(SEQX, 0x04U, restore->std.Sequencer[4]);
                PutReg(GRAX, 0x06U, restore->std.Graphics[6]);
                if (ATIChipHasVGAWonder)
                    ATIModifyExtReg(0xB6, -1, 0x00U, restore->b6);
            }
            break;

        default:
            break;
    }

    if (ATILCDPanelID >= 0)
    {
        if (!restore->mode ||
            !OFLG_ISSET(OPTION_FB_DEBUG, &vga256InfoRec.options))
        {
            /* Switch to shadow registers */
            if (ATIChip == ATI_CHIP_264LT)
                outl(ATIIOPortLCD_GEN_CTRL,
                    (restore->lcd_gen_ctrl &
                     ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
                    (SHADOW_EN | SHADOW_RW_EN));
            else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                        (ATIChip == ATI_CHIP_264XL) ||
                        (ATIChip == ATI_CHIP_MOBILITY)) */
                ATIPutLTProLCDReg(LCD_GEN_CNTL,
                    (restore->lcd_gen_ctrl &
                     ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
                    (SHADOW_EN | SHADOW_RW_EN));

            switch (restore->crtc)
            {
                case ATI_CRTC_VGA:
                    /* Restore shadow VGA CRTC registers */
                    for (Index = 0;
                         Index < NumberOf(restore->shadow_vga);
                         Index++)
                        PutReg(CRTX(vgaIOBase), Index,
                            restore->shadow_vga[Index]);
                    break;

                case ATI_CRTC_MACH64:
                    /* Restore shadow Mach64 CRTC registers */
                    outl(ATIIOPortCRTC_H_TOTAL_DISP,
                        restore->shadow_h_total_disp);
                    outl(ATIIOPortCRTC_H_SYNC_STRT_WID,
                        restore->shadow_h_sync_strt_wid);
                    outl(ATIIOPortCRTC_V_TOTAL_DISP,
                        restore->shadow_v_total_disp);
                    outl(ATIIOPortCRTC_V_SYNC_STRT_WID,
                        restore->shadow_v_sync_strt_wid);
                    break;

                default:
                    break;
            }
        }

        /* Restore CRTC selection and shadow state & enable stretching */
        if (ATIChip == ATI_CHIP_264LT)
        {
            outl(ATIIOPortLCD_GEN_CTRL, restore->lcd_gen_ctrl);
            outl(ATIIOPortHORZ_STRETCHING, restore->horz_stretching);
            outl(ATIIOPortVERT_STRETCHING, restore->vert_stretching);
        }
        else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                    (ATIChip == ATI_CHIP_264XL) ||
                    (ATIChip == ATI_CHIP_MOBILITY)) */
        {
            ATIPutLTProLCDReg(LCD_GEN_CNTL, restore->lcd_gen_ctrl);
            ATIPutLTProLCDReg(LCD_HORZ_STRETCHING, restore->horz_stretching);
            ATIPutLTProLCDReg(LCD_VERT_STRETCHING, restore->vert_stretching);
            ATIPutLTProLCDReg(LCD_EXT_VERT_STRETCH, restore->ext_vert_stretch);
            outl(ATIIOPortLCD_INDEX, restore->lcd_index);
        }
    }

    /*
     * Set DSP registers.  Note that sequencer resets clear the DSP_CONFIG
     * register.
     */
    if ((ATIChip >= ATI_CHIP_264VTB) && (ATIIODecoding == BLOCK_IO))
        ATIDSPRestore(restore);

    /* Load RAMDAC */
    ATIDACRestore(restore);

    ATISwap(restore, TRUE);             /* Restore video memory */

    SetTimeSinceLastInputEvent();

    if ((xf86Verbose > 2) && (restore->mode))
    {
        ErrorF("\n After setting mode \"%s\":\n\n", restore->mode->name);
        ATIPrintMode(restore->mode);
        ATIPrintRegisters();
    }
}
