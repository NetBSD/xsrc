/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticonsole.c,v 1.1.2.5 2000/05/14 02:02:14 tsi Exp $ */
/*
 * Copyright 1997 through 2000 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#include "atiadapter.h"
#include "atichip.h"
#include "aticonsole.h"
#include "atidepth.h"
#include "atiio.h"
#include "xf86_OSproc.h"

#ifdef XFreeXDGA
#   define _XF86DGA_SERVER_
#   include "extensions/xf86dga.h"
#endif

/*
 * ATIEnterLeave --
 *
 * This function is called when the virtual terminal on which the server is
 * running is entered or left, as well as when the server starts up and is shut
 * down.  Its function is to obtain and relinquish I/O permissions for the SVGA
 * device.  This includes unlocking access to any registers that may be
 * protected on the chipset, and locking those registers again on exit.
 */
void
ATIEnterLeave(const Bool enter)
{
    static CARD8 saved_a6, saved_ab,
        saved_b1, saved_b4, saved_b5, saved_b6, saved_b8, saved_b9, saved_be;
    static CARD16 saved_clock_sel, saved_misc_options, saved_mem_bndry,
        saved_mem_cfg;
    static CARD32 saved_bus_cntl, saved_config_cntl, saved_crtc_gen_cntl,
        saved_mem_info, saved_gen_test_cntl, saved_dac_cntl,
        saved_crtc_int_cntl, saved_lcd_index;

    static Bool entered = LEAVE;
    CARD32 tmp, lcd_gen_ctrl = 0, saved_lcd_gen_ctrl = 0;

#   ifdef XFreeXDGA
        if ((enter == LEAVE) && !ATIUsing1bppModes &&
            (vga256InfoRec.directMode & XF86DGADirectGraphics))
            return;
#   endif

    if (enter == entered)
        return;
    entered = enter;

    if (enter == ENTER)
    {
        xf86EnableIOPorts(vga256InfoRec.scrnIndex);

        if (ATIChipHasSUBSYS_CNTL)
        {
            /* Save register values to be modified */
            saved_clock_sel = inw(CLOCK_SEL);
            if (ATIChip >= ATI_CHIP_68800)
            {
                saved_misc_options = inw(MISC_OPTIONS);
                saved_mem_bndry = inw(MEM_BNDRY);
                saved_mem_cfg = inw(MEM_CFG);
            }

            tmp = inw(SUBSYS_STAT) & _8PLANE;

            /* Reset the 8514/A and disable all interrupts */
            outw(SUBSYS_CNTL, tmp | (GPCTRL_RESET | CHPTEST_NORMAL));
            outw(SUBSYS_CNTL, tmp | (GPCTRL_ENAB | CHPTEST_NORMAL | RVBLNKFLG |
                RPICKFLAG | RINVALIDIO | RGPIDLE));

            /* Ensure VGA is enabled */
            outw(CLOCK_SEL, saved_clock_sel & ~DISABPASSTHRU);
            if (ATIChip >= ATI_CHIP_68800)
            {
                outw(MISC_OPTIONS, saved_misc_options &
                    ~(DISABLE_VGA | DISABLE_DAC));

                /* Disable any video memory boundary */
                outw(MEM_BNDRY, saved_mem_bndry &
                    ~(MEM_PAGE_BNDRY | MEM_BNDRY_ENA));

                /* Disable direct video memory aperture */
                outw(MEM_CFG, saved_mem_cfg &
                    ~(MEM_APERT_SEL | MEM_APERT_PAGE | MEM_APERT_LOC));
            }

            /* Wait for all activity to die down */
            ProbeWaitIdleEmpty();
        }
        else if (ATIChip >= ATI_CHIP_88800GXC)
        {
            /* Save register values to be modified */
            saved_config_cntl = inl(ATIIOPortCONFIG_CNTL);
            saved_dac_cntl = inl(ATIIOPortDAC_CNTL);

            /* Reset everything */
            saved_bus_cntl = (inl(ATIIOPortBUS_CNTL) & ~BUS_HOST_ERR_INT_EN) |
                BUS_HOST_ERR_INT;
            if (ATIChip < ATI_CHIP_264VTB)
                saved_bus_cntl = (saved_bus_cntl & ~BUS_FIFO_ERR_INT_EN) |
                     BUS_FIFO_ERR_INT;
            outl(ATIIOPortBUS_CNTL, (saved_bus_cntl & ~BUS_ROM_DIS) |
                SetBits(15, BUS_FIFO_WS));
            saved_crtc_int_cntl = inl(ATIIOPortCRTC_INT_CNTL);
            outl(ATIIOPortCRTC_INT_CNTL,
                (saved_crtc_int_cntl & ~CRTC_INT_ENS) | CRTC_INT_ACKS);
            saved_gen_test_cntl = inl(ATIIOPortGEN_TEST_CNTL) &
                (GEN_OVR_OUTPUT_EN | GEN_OVR_POLARITY | GEN_CUR_EN |
                    GEN_BLOCK_WR_EN);
            tmp = saved_gen_test_cntl & ~GEN_CUR_EN;
            outl(ATIIOPortGEN_TEST_CNTL, tmp | GEN_GUI_EN);
            outl(ATIIOPortGEN_TEST_CNTL, tmp);
            outl(ATIIOPortGEN_TEST_CNTL, tmp | GEN_GUI_EN);
            tmp = saved_crtc_gen_cntl = inl(ATIIOPortCRTC_GEN_CNTL) &
                ~(CRTC_EN | CRTC_LOCK_REGS);
            if (ATIChip >= ATI_CHIP_264XL)
                tmp = (tmp & ~CRTC_INT_ENS_X) | CRTC_INT_ACKS_X;
            outl(ATIIOPortCRTC_GEN_CNTL, tmp | CRTC_EN);
            outl(ATIIOPortCRTC_GEN_CNTL, tmp);
            outl(ATIIOPortCRTC_GEN_CNTL, tmp | CRTC_EN);
            if (ATILCDPanelID >= 0)
            {
                saved_lcd_index = inl(ATIIOPortLCD_INDEX);
                if (ATIChip >= ATI_CHIP_264XL)
                    outl(ATIIOPortLCD_INDEX, saved_lcd_index &
                        ~(LCD_MONDET_INT_EN | LCD_MONDET_INT));
            }

            /* Ensure VGA aperture is enabled */
            outl(ATIIOPortDAC_CNTL, saved_dac_cntl | DAC_VGA_ADR_EN);
            outl(ATIIOPortCONFIG_CNTL, saved_config_cntl & ~CFG_VGA_DIS);
            if (ATIChip < ATI_CHIP_264CT)
            {
                saved_mem_info = inl(ATIIOPortMEM_INFO);
                outl(ATIIOPortMEM_INFO, saved_mem_info &
                    ~(CTL_MEM_BNDRY | CTL_MEM_BNDRY_EN));
            }
        }

        if (ATIVGAAdapter != ATI_ADAPTER_NONE)
        {
            if (ATIChipHasVGAWonder)
            {
                /*
                 * Ensure all registers are read/write and disable all non-VGA
                 * emulations.
                 */
                saved_b1 = ATIGetExtReg(0xB1U);
                ATIModifyExtReg(0xB1U, saved_b1, 0xFCU, 0x00U);
                saved_b4 = ATIGetExtReg(0xB4U);
                ATIModifyExtReg(0xB4U, saved_b4, 0x00U, 0x00U);
                saved_b5 = ATIGetExtReg(0xB5U);
                ATIModifyExtReg(0xB5U, saved_b5, 0xBFU, 0x00U);
                saved_b6 = ATIGetExtReg(0xB6U);
                ATIModifyExtReg(0xB6U, saved_b6, 0xDDU, 0x00U);
                saved_b8 = ATIGetExtReg(0xB8U);
                ATIModifyExtReg(0xB8U, saved_b8, 0xC0U, 0x00U);
                saved_b9 = ATIGetExtReg(0xB9U);
                ATIModifyExtReg(0xB9U, saved_b9, 0x7FU, 0x00U);
                if (ATIChip > ATI_CHIP_18800)
                {
                    saved_be = ATIGetExtReg(0xBEU);
                    ATIModifyExtReg(0xBEU, saved_be, 0xFAU, 0x01U);
                    if (ATIChip >= ATI_CHIP_28800_2)
                    {
                        saved_a6 = ATIGetExtReg(0xA6U);
                        ATIModifyExtReg(0xA6U, saved_a6, 0x7FU, 0x00U);
                        saved_ab = ATIGetExtReg(0xABU);
                        ATIModifyExtReg(0xABU, saved_ab, 0xE7U, 0x00U);
                    }
                }
            }

            if (ATILCDPanelID >= 0)
            {
                if (ATIChip == ATI_CHIP_264LT)
                {
                    saved_lcd_gen_ctrl = inl(ATIIOPortLCD_GEN_CTRL);

                    /* Setup to unlock non-shadow registers */
                    lcd_gen_ctrl = saved_lcd_gen_ctrl &
                        ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN);
                    outl(ATIIOPortLCD_GEN_CTRL, lcd_gen_ctrl);
                }
                else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                            (ATIChip == ATI_CHIP_264XL) ||
                            (ATIChip == ATI_CHIP_MOBILITY)) */
                {
                    saved_lcd_gen_ctrl = ATIGetLTProLCDReg(LCD_GEN_CNTL);

                    /* Setup to unlock shadow registers */
                    lcd_gen_ctrl = saved_lcd_gen_ctrl &
                        ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN);
                    ATIPutLTProLCDReg(LCD_GEN_CNTL, lcd_gen_ctrl);
                }
            }

        UnlockShadowVGA:
            ATISetVGAIOBase(inb(R_GENMO));

            /*
             * There's a bizarre interaction here.  If bit 0x80 of CRTC[17] is
             * on, then CRTC[3] is read-only.  If bit 0x80 of CRTC[3] is off,
             * then CRTC[17] is write-only (or a read attempt actually returns
             * bits from C/EGA's light pen position).  This means that if both
             * conditions are met, CRTC[17]'s value on server entry cannot be
             * retrieved.
             */

            tmp = GetReg(CRTX(vgaIOBase), 0x03U);
            if ((tmp & 0x80U) ||
                ((outb(CRTD(vgaIOBase), tmp | 0x80U),
                    tmp = inb(CRTD(vgaIOBase))) & 0x80U))
            {
                /* CRTC[16-17] should be readable */
                tmp = GetReg(CRTX(vgaIOBase), 0x11U);
                if (tmp & 0x80U)        /* Unprotect CRTC[0-7] */
                    outb(CRTD(vgaIOBase), tmp & 0x7FU);
            }
            else
            {
                /*
                 * Could not make CRTC[17] readable, so unprotect CRTC[0-7]
                 * replacing VSyncEnd with zero.  This zero will be replaced
                 * after acquiring the needed access.
                 */
                unsigned int VSyncEnd, VBlankStart, VBlankEnd;
                CARD8 crt07, crt09;

                PutReg(CRTX(vgaIOBase), 0x11U, 0x20U);
                /* Make CRTC[16-17] readable */
                PutReg(CRTX(vgaIOBase), 0x03U, tmp | 0x80U);
                /* Make vertical synch pulse as wide as possible */
                crt07 = GetReg(CRTX(vgaIOBase), 0x07U);
                crt09 = GetReg(CRTX(vgaIOBase), 0x09U);
                VBlankStart = (((crt09 & 0x20U) << 4) |
                    ((crt07 & 0x08U) << 5) |
                    GetReg(CRTX(vgaIOBase), 0x15U)) + 1;
                VBlankEnd = (VBlankStart & 0x300U) |
                    GetReg(CRTX(vgaIOBase), 0x16U);
                if (VBlankEnd <= VBlankStart)
                    VBlankEnd += 0x0100U;
                VSyncEnd = (((crt07 & 0x80U) << 2) | ((crt07 & 0x04U) << 6) |
                    GetReg(CRTX(vgaIOBase), 0x10U)) + 0x0FU;
                if (VSyncEnd >= VBlankEnd)
                    VSyncEnd = VBlankEnd - 1;
                PutReg(CRTX(vgaIOBase), 0x11U, (VSyncEnd & 0x0FU) | 0x20U);
            }

            if (ATILCDPanelID >= 0)
            {
                Bool DoShadow = TRUE;

                lcd_gen_ctrl ^= (SHADOW_EN | SHADOW_RW_EN);
                if (!(lcd_gen_ctrl & (SHADOW_EN | SHADOW_RW_EN)))
                {
                    DoShadow = FALSE;
                    lcd_gen_ctrl = saved_lcd_gen_ctrl;
                }

                /*
                 * Setup to unlock shadow registers or restore previous
                 * selection.
                 */
                if (ATIChip == ATI_CHIP_264LT)
                    outl(ATIIOPortLCD_GEN_CTRL, lcd_gen_ctrl);
                else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                            (ATIChip == ATI_CHIP_264XL) ||
                            (ATIChip == ATI_CHIP_MOBILITY)) */
                {
                    ATIPutLTProLCDReg(LCD_GEN_CNTL, lcd_gen_ctrl);

                    /* Restore LCD index */
                    outb(ATIIOPortLCD_INDEX, GetByte(saved_lcd_index, 0));
                }

                if (DoShadow)
                    goto UnlockShadowVGA;       /* Unlock shadow registers */
            }
        }
    }
    else
    {
        if (ATIVGAAdapter != ATI_ADAPTER_NONE)
        {
            if (ATILCDPanelID >= 0)
            {
                if (ATIChip == ATI_CHIP_264LT)
                {
                    saved_lcd_gen_ctrl = inl(ATIIOPortLCD_GEN_CTRL);

                    /* Setup to lock non-shadow registers */
                    lcd_gen_ctrl = saved_lcd_gen_ctrl &
                        ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN);
                    outl(ATIIOPortLCD_GEN_CTRL, lcd_gen_ctrl);
                }
                else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                            (ATIChip == ATI_CHIP_264XL) ||
                            (ATIChip == ATI_CHIP_MOBILITY)) */
                {
                    saved_lcd_gen_ctrl = ATIGetLTProLCDReg(LCD_GEN_CNTL);

                    /* Setup to lock shadow registers */
                    lcd_gen_ctrl = saved_lcd_gen_ctrl &
                        ~(CRTC_RW_SELECT | SHADOW_EN | SHADOW_RW_EN);
                    ATIPutLTProLCDReg(LCD_GEN_CNTL, lcd_gen_ctrl);
                }
            }

        LockShadowVGA:
            ATISetVGAIOBase(inb(R_GENMO));

            /* Protect CRTC[0-7] */
            tmp = GetReg(CRTX(vgaIOBase), 0x11U);
            outb(CRTD(vgaIOBase), tmp | 0x80U);

            if (ATILCDPanelID >= 0)
            {
                Bool DoShadow = TRUE;

                lcd_gen_ctrl ^= (SHADOW_EN | SHADOW_RW_EN);
                if (!(lcd_gen_ctrl & (SHADOW_EN | SHADOW_RW_EN)))
                {
                    DoShadow = FALSE;
                    lcd_gen_ctrl = saved_lcd_gen_ctrl;
                }

                /*
                 * Setup to lock shadow registers or restore previous
                 * selection.
                 */
                if (ATIChip == ATI_CHIP_264LT)
                    outl(ATIIOPortLCD_GEN_CTRL, lcd_gen_ctrl);
                else /* if ((ATIChip == ATI_CHIP_264LTPRO) ||
                            (ATIChip == ATI_CHIP_264XL) ||
                            (ATIChip == ATI_CHIP_MOBILITY)) */
                {
                    ATIPutLTProLCDReg(LCD_GEN_CNTL, lcd_gen_ctrl);

                    /* Restore LCD index */
                    outb(ATIIOPortLCD_INDEX, GetByte(saved_lcd_index, 0));
                }

                if (DoShadow)
                    goto LockShadowVGA;       /* Lock shadow registers */
            }

            if (ATIChipHasVGAWonder)
            {
                /*
                 * Restore emulation and protection bits in ATI extended VGA
                 * registers.
                 */
                ATIModifyExtReg(0xB1U, -1, 0xFCU, saved_b1);
                ATIModifyExtReg(0xB4U, -1, 0x00U, saved_b4);
                ATIModifyExtReg(0xB5U, -1, 0xBFU, saved_b5);
                ATIModifyExtReg(0xB6U, -1, 0xDDU, saved_b6);
                ATIModifyExtReg(0xB8U, -1, 0xC0U, saved_b8 & 0x03U);
                ATIModifyExtReg(0xB9U, -1, 0x7FU, saved_b9);
                if (ATIChip > ATI_CHIP_18800)
                {
                    ATIModifyExtReg(0xBEU, -1, 0xFAU, saved_be);
                    if (ATIChip >= ATI_CHIP_28800_2)
                    {
                        ATIModifyExtReg(0xA6U, -1, 0x7FU, saved_a6);
                        ATIModifyExtReg(0xABU, -1, 0xE7U, saved_ab);
                    }
                }
            }
        }

        if (ATIChipHasSUBSYS_CNTL)
        {
            tmp = inw(SUBSYS_STAT) & _8PLANE;

            /* Reset the 8514/A and disable all interrupts */
            outw(SUBSYS_CNTL, tmp | (GPCTRL_RESET | CHPTEST_NORMAL));
            outw(SUBSYS_CNTL, tmp | (GPCTRL_ENAB | CHPTEST_NORMAL | RVBLNKFLG |
                RPICKFLAG | RINVALIDIO | RGPIDLE));

            /* Restore modified accelerator registers */
            outw(CLOCK_SEL, saved_clock_sel);
            if (ATIChip >= ATI_CHIP_68800)
            {
                outw(MISC_OPTIONS, saved_misc_options);
                outw(MEM_BNDRY, saved_mem_bndry);
                outw(MEM_CFG, saved_mem_cfg);
            }

            /* Wait for all activity to die down */
            ProbeWaitIdleEmpty();
        }
        else if (ATIChip >= ATI_CHIP_88800GXC)
        {
            /* Reset everything */
            outl(ATIIOPortBUS_CNTL, saved_bus_cntl);
            outl(ATIIOPortCRTC_INT_CNTL, saved_crtc_int_cntl);
            outl(ATIIOPortGEN_TEST_CNTL, saved_gen_test_cntl | GEN_GUI_EN);
            outl(ATIIOPortGEN_TEST_CNTL, saved_gen_test_cntl);
            outl(ATIIOPortGEN_TEST_CNTL, saved_gen_test_cntl | GEN_GUI_EN);
            outl(ATIIOPortCRTC_GEN_CNTL, saved_crtc_gen_cntl | CRTC_EN);
            outl(ATIIOPortCRTC_GEN_CNTL, saved_crtc_gen_cntl);
            outl(ATIIOPortCRTC_GEN_CNTL, saved_crtc_gen_cntl | CRTC_EN);

            /* Restore registers */
            outl(ATIIOPortCONFIG_CNTL, saved_config_cntl);
            outl(ATIIOPortDAC_CNTL, saved_dac_cntl);
            if (ATIChip < ATI_CHIP_264CT)
                outl(ATIIOPortMEM_INFO, saved_mem_info);
            else if (ATILCDPanelID >= 0)
                outl(ATIIOPortLCD_INDEX, saved_lcd_index);
        }

        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}
