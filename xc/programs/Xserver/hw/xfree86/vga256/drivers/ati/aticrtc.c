/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticrtc.c,v 1.1.2.1 1998/02/01 16:41:48 robin Exp $ */
/*
 * Copyright 1997,1998 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

        /* Set RAMDAC data */
        ATIDACInit(NULL);
    }

    ATINewHWPtr->mode = mode;           /* Link with mode line */

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
