/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiprobe.c,v 1.1.2.11 2000/05/14 02:02:17 tsi Exp $ */
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

#include "ati.h"
#include "atiadapter.h"
#include "atibus.h"
#include "atichip.h"
#include "aticlock.h"
#include "aticonsole.h"
#include "atidac.h"
#include "atidepth.h"
#include "atidsp.h"
#include "atigetmode.h"
#include "atiident.h"
#include "atiio.h"
#include "atiprint.h"
#include "atiprobe.h"
#ifndef MONOVGA
#include "atiscrinit.h"
#endif
#include "ativersion.h"
#include "atividmem.h"
#include "vgaPCI.h"
#include "xf86Procs.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#ifdef XFreeXDGA
#   define _XF86DGA_SERVER_
#   include "extensions/xf86dga.h"
#endif

/* To allow compilation with older vgaPCI.h */
#ifndef PCI_CHIP_RAGE128LE
#define PCI_CHIP_RAGE128LE 0x4C45
#endif
#ifndef PCI_CHIP_RAGE128LF
#define PCI_CHIP_RAGE128LF 0x4C46
#endif

/*
 * This structure is used by ATIProbe in an attempt to define a default video
 * mode when the user has not specified any modes in XF86Config.
 */
static DisplayModeRec DefaultMode;

/*
 * Macros for port definitions.
 */
#define IOByte(_Port)   (_Port)
#define IOWord(_Port)   (_Port), (_Port)+1
#define IOLong(_Port)   (_Port), (_Port)+1, (_Port)+2, (_Port)+3

typedef CARD16 Colour;          /* The correct spelling should be OK :-) */

/*
 * Bit patterns which are extremely unlikely to show up when reading from
 * nonexistant memory (which normally shows up as either all bits set or all
 * bits clear).
 */
static const Colour Test_Pixel[] = {0x5AA5U, 0x55AAU, 0xA55AU, 0xCA53U};

static const struct
{
    int videoRamSize;
    int Miscellaneous_Options_Setting;
    struct
    {
        short int x, y;
    }
    Coordinates[NumberOf(Test_Pixel) + 1];
}
Test_Case[] =
{
    /*
     * Given the engine settings used, only a 4M card will have enough memory
     * to back up the 1025th line of the display.  Since the pixel coordinates
     * are zero-based, line 1024 will be the first one which is only backed on
     * 4M cards.
     *
     * <Mark_Weaver@brown.edu>:
     * In case memory is being wrapped, (0,0) and (0,1024) to make sure they
     * can each hold a unique value.
     */
    {4096, MEM_SIZE_4M, {{0,0}, {0,1024}, {-1,-1}}},

    /*
     * This card has 2M or less.  On a 1M card, the first 2M of the card's
     * memory will have even doublewords backed by physical memory and odd
     * doublewords unbacked.
     *
     * Pixels 0 and 1 of a row will be in the zeroth doubleword, while pixels 2
     * and 3 will be in the first.  Check both pixels 2 and 3 in case this is a
     * pseudo-1M card (one chip pulled to turn a 2M card into a 1M card).
     *
     * <Mark_Weaver@brown.edu>:
     * I don't have a 1M card, so I'm taking a stab in the dark.  Maybe memory
     * wraps every 512 lines, or maybe odd doublewords are aliases of their
     * even doubleword counterparts.  I try everything here.
     */
    {2048, MEM_SIZE_2M, {{0,0}, {0,512}, {2,0}, {3,0}, {-1,-1}}},

    /*
     * This is a either a 1M card or a 512k card.  Test pixel 1, since it is an
     * odd word in an even doubleword.
     *
     * <Mark_Weaver@brown.edu>:
     * This is the same idea as the test above.
     */
    {1024, MEM_SIZE_1M, {{0,0}, {0,256}, {1,0}, {-1,-1}}},

    /*
     * Assume it is a 512k card by default, since that is the minimum
     * configuration.
     */
    {512, MEM_SIZE_512K, {{-1,-1}}}
};

/*
 * ATIMach32ReadPixel --
 *
 * Return the colour of the specified screen location.  Called from
 * ATIMach32videoRam function below.
 */
static Colour
ATIMach32ReadPixel(const short int X, const short int Y)
{
    Colour Pixel_Colour;

    /* Wait for idle engine */
    ProbeWaitIdleEmpty();

    /* Set up engine for pixel read */
    ATIWaitQueue(7);
    outw(RD_MASK, (CARD16)(~0));
    outw(DP_CONFIG, FG_COLOR_SRC_BLIT | DATA_WIDTH | DRAW | DATA_ORDER);
    outw(CUR_X, X);
    outw(CUR_Y, Y);
    outw(DEST_X_START, X);
    outw(DEST_X_END, X + 1);
    outw(DEST_Y_END, Y + 1);

    /* Wait for data to become ready */
    ATIWaitQueue(16);
    WaitDataReady();

    /* Read pixel colour */
    Pixel_Colour = inw(PIX_TRANS);
    ProbeWaitIdleEmpty();
    return Pixel_Colour;
}

/*
 * ATIMach32WritePixel --
 *
 * Set the colour of the specified screen location.  Called from
 * ATIMach32videoRam function below.
 */
static void
ATIMach32WritePixel(const short int X, const short int Y,
                    const Colour Pixel_Colour)
{
    /* Set up engine for pixel write */
    ATIWaitQueue(9);
    outw(WRT_MASK, (CARD16)(~0));
    outw(DP_CONFIG, FG_COLOR_SRC_FG | DRAW | READ_WRITE);
    outw(ALU_FG_FN, MIX_FN_PAINT);
    outw(FRGD_COLOR, Pixel_Colour);
    outw(CUR_X, X);
    outw(CUR_Y, Y);
    outw(DEST_X_START, X);
    outw(DEST_X_END, X + 1);
    outw(DEST_Y_END, Y + 1);
}

/*
 * ATIMach32videoRam --
 *
 * Determine the amount of video memory installed on an 68800-6 based adapter.
 * This is done because these chips exhibit a bug that causes their
 * MISC_OPTIONS register to report 1M rather than the true amount of memory.
 *
 * This function is adapted from a similar function in mach32mem.c written by
 * Robert Wolff, David Dawes and Mark Weaver.
 */
static int
ATIMach32videoRam(void)
{
    CARD16 saved_clock_sel, saved_mem_bndry, saved_misc_options,
        saved_ext_ge_config;
    Colour saved_Pixel[NumberOf(Test_Pixel)];
    unsigned int Case_Number, Pixel_Number;
    CARD16 corrected_misc_options;
    Bool AllPixelsOK;

    /* Save register values to be modified */
    saved_clock_sel = inw(CLOCK_SEL);
    saved_mem_bndry = inw(MEM_BNDRY);
    saved_misc_options = inw(MISC_OPTIONS);
    corrected_misc_options = saved_misc_options & ~MEM_SIZE_ALIAS;
    saved_ext_ge_config = inw(R_EXT_GE_CONFIG);

    /* Wait for enough FIFO entries */
    ATIWaitQueue(7);

    /* Enable accelerator */
    outw(CLOCK_SEL, saved_clock_sel | DISABPASSTHRU);

    /* Make accelerator and VGA share video memory */
    outw(MEM_BNDRY, saved_mem_bndry & ~(MEM_PAGE_BNDRY | MEM_BNDRY_ENA));

    /* Prevent video memory wrap */
    outw(MISC_OPTIONS, corrected_misc_options | MEM_SIZE_4M);

    /*
     * Set up the drawing engine for a pitch of 1024 at 16 bits per pixel.  No
     * need to mess with the CRT because the results of this test are not
     * intended to be seen.
     */
    outw(EXT_GE_CONFIG, PIX_WIDTH_16BPP | ORDER_16BPP_565 | MONITOR_8514 |
        ALIAS_ENA);
    outw(GE_PITCH, 1024 >> 3);
    outw(GE_OFFSET_HI, 0);
    outw(GE_OFFSET_LO, 0);

    for (Case_Number = 0;
         Case_Number < (NumberOf(Test_Case) - 1);
         Case_Number++)
    {
        /* Reduce redundancy as per Mark_Weaver@brown.edu */
#       define TestPixel Test_Case[Case_Number].Coordinates[Pixel_Number]
#       define ForEachTestPixel        \
            for (Pixel_Number = 0;  TestPixel.x >= 0;  Pixel_Number++)

        /* Save pixel colours that will be clobbered */
        ForEachTestPixel
            saved_Pixel[Pixel_Number] =
                ATIMach32ReadPixel(TestPixel.x, TestPixel.y);

        /* Write test patterns */
        ForEachTestPixel
            ATIMach32WritePixel(TestPixel.x, TestPixel.y,
                Test_Pixel[Pixel_Number]);

        /* Test for lost pixels */
        AllPixelsOK = TRUE;
        ForEachTestPixel
            if (ATIMach32ReadPixel(TestPixel.x, TestPixel.y) !=
                Test_Pixel[Pixel_Number])
            {
                AllPixelsOK = FALSE;
                break;
            }

        /* Restore clobbered pixels */
        ForEachTestPixel
            ATIMach32WritePixel(TestPixel.x, TestPixel.y,
                saved_Pixel[Pixel_Number]);

        /* End test on success */
        if (AllPixelsOK)
            break;

        /* Completeness */
#       undef ForEachTestPixel
#       undef TestPixel
    }

    /* Restore what was changed and correct MISC_OPTIONS register */
    ATIWaitQueue(4);
    outw(EXT_GE_CONFIG, saved_ext_ge_config);
    corrected_misc_options |=
        Test_Case[Case_Number].Miscellaneous_Options_Setting;
    outw(MISC_OPTIONS, corrected_misc_options);
    outw(MEM_BNDRY, saved_mem_bndry);
    outw(CLOCK_SEL, saved_clock_sel);

    /* Wait for activity to die down */
    ProbeWaitIdleEmpty();

    /* Tell ATIProbe the REAL story */
    return Test_Case[Case_Number].videoRamSize;
}

/*
 * ATIMach64Probe --
 *
 * This function looks for a Mach64 at a particular I/O base address.  This
 * sets ATIAdapter if a Mach64 is found.
 */
static void
ATIMach64Probe(const CARD16 IO_Base, const CARD8 IO_Decoding,
               const CARD16 ExpectedChipType)
{
    CARD32 IO_Value, saved_bus_cntl, saved_gen_test_cntl;
    CARD16 IO_Port;

    if ((ATIAdapter != ATI_ADAPTER_NONE) || (IO_Base == 0))
        return;

    ATIIOBase = IO_Base;
    ATIIODecoding = IO_Decoding;

    /*
     * Make sure any Mach64 is not in some weird state.  Note that command
     * FIFO errors cannot be reset here because VTB's and later use the same
     * bits for something else, and, at this point, it isn't yet known whether
     * or not such a controller will be detected.  Something based on the
     * expected chip type could be done, I suppose, but it would be kludgy at
     * best, and imprecise at worst.
     */
    ATIIOPortBUS_CNTL = ATIIOPort(BUS_CNTL);
    saved_bus_cntl = inl(ATIIOPortBUS_CNTL);
    outl(ATIIOPortBUS_CNTL, (saved_bus_cntl & ~BUS_HOST_ERR_INT_EN) |
        BUS_HOST_ERR_INT);

    ATIIOPortGEN_TEST_CNTL = ATIIOPort(GEN_TEST_CNTL);
    saved_gen_test_cntl = inl(ATIIOPortGEN_TEST_CNTL);
    IO_Value = saved_gen_test_cntl &
        (GEN_OVR_OUTPUT_EN | GEN_OVR_POLARITY | GEN_CUR_EN | GEN_BLOCK_WR_EN);
    outl(ATIIOPortGEN_TEST_CNTL, IO_Value | GEN_GUI_EN);
    outl(ATIIOPortGEN_TEST_CNTL, IO_Value);
    outl(ATIIOPortGEN_TEST_CNTL, IO_Value | GEN_GUI_EN);

    /* See if a Mach64 answers */
    IO_Port = ATIIOPort(SCRATCH_REG0);
    IO_Value = inl(IO_Port);

    /* Test odd bits */
    outl(IO_Port, 0x55555555UL);
    if (inl(IO_Port) == 0x55555555UL)
    {
        /* Test even bits */
        outl(IO_Port, 0xAAAAAAAAUL);
        if (inl(IO_Port) == 0xAAAAAAAAUL)
        {
            /*
             * *Something* has a R/W 32-bit register at this I/O address.  Try
             * to make sure it's a Mach64.  The following assumes that ATI
             * won't be producing any more adapters that don't register
             * themselves in the PCI configuration space.
             */
            ATIMach64ChipID(ExpectedChipType);
            if ((ATIChip != ATI_CHIP_Mach64) || (IO_Decoding == BLOCK_IO))
                ATIAdapter = ATI_ADAPTER_MACH64;
            else
                ATIChip = ATI_CHIP_NONE;
        }
    }

    /* Restore registers that might have been clobbered */
    outl(IO_Port, IO_Value);
    if (ATIAdapter != ATI_ADAPTER_MACH64)
    {
        outl(ATIIOPortGEN_TEST_CNTL, saved_gen_test_cntl);
        outl(ATIIOPortBUS_CNTL, saved_bus_cntl);
    }
}

/*
 * ATIProbe --
 *
 * This is the function that makes a yes/no decision about whether or not a
 * chipset supported by this driver is present or not.  The server will call
 * each driver's probe function in sequence, until one returns TRUE or they all
 * fail.
 */
Bool
ATIProbe(void)
{
    static const CARD8 ATISignature[] = " 761295520";
#   define Signature_Size   10
#   define Prefix_Size      1024                /* 1kB */
#   define BIOS_SIZE        0x010000            /* 64kB */
#   define BIOS_Signature   0x30U
#   define No_Signature     (Prefix_Size + 1 - Signature_Size)
    CARD8 BIOS[BIOS_SIZE];
#   define BIOSByte(_n)     (*((CARD8  *)(BIOS + (_n))))
#   define BIOSWord(_n)     (*((CARD16 *)(BIOS + (_n))))
#   define BIOSLong(_n)     (*((CARD32 *)(BIOS + (_n))))
    CARD32 IO_Value = 0, IO_Value2;
    unsigned int BIOSSize = 0;
    unsigned int Signature = No_Signature;
    int saved_BIOSbase = vga256InfoRec.BIOSbase;
    int MachvideoRam = 0, VGAvideoRam = 0;
    unsigned int WindowSize;
    CARD8 ATIMachChip = ATI_CHIP_NONE;
    CARD16 ClockDac;
    static const int videoRamSizes[] =
        {0, 256, 512, 1024, 2*1024, 4*1024, 6*1024, 8*1024, 12*1024, 16*1024, 0};
    int Index, Index2;
    unsigned int ROMTable = 0, ClockTable = 0, FrequencyTable = 0;
    unsigned int LCDTable = 0, LCDPanelInfo = 0;
    const DACRec *DAC;
    pciConfigPtr PCIDevice;
    Bool Rage128Seen = FALSE;

    /* Get out if this isn't the driver the user wants */
    if (!ATIIdentProbe())
        return FALSE;

    /* Enable the I/O ports needed for probing */
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);

#if 0
    /*
     * It is quite possible for a system to preclude the existence of a mix of
     * sparse I/O and block I/O devices.  Scan PCI configuration space, if
     * available, for any registered I/O ranges which would in most cases
     * preclude the existence of an 8514/A compatible device.  The following
     * check is a bit of an overkill, but will do for now.
     */
    if (vgaPCIInfo && vgaPCIInfo->AllCards)
    {
        Index = 0;
        while ((PCIDevice = vgaPCIInfo->AllCards[Index++]))
        {
            CARD32 * BasePointer = &PCIDevice->_base0;

            /* Check all six base addresses */
            for (;  BasePointer <= &PCIDevice->_base5;  BasePointer++)
            {
                /*
                 * Skip 8514/A probe if this device has registered an I/O
                 * range.
                 */
                if (*BasePointer & 1U)
                    goto Skip8514Probe;

                /* Allow for 64-bit memory addresses */
                if (*BasePointer & 4U)
                    BasePointer++;
            }
        }
    }
#endif

    /*
     * Save register value to be modified, just in case there is no 8514/A
     * compatible accelerator.  Note that, in more ways than one,
     * SUBSYS_STAT == SUBSYS_CNTL.
     */
    IO_Value = inw(SUBSYS_STAT);
    IO_Value2 = IO_Value & _8PLANE;

    /*
     * Determine if an 8514/A-compatible accelerator is present, making sure
     * it's not in some weird state.
     */
    outw(SUBSYS_CNTL, IO_Value2 | (GPCTRL_RESET | CHPTEST_NORMAL));
    outw(SUBSYS_CNTL, IO_Value2 | (GPCTRL_ENAB | CHPTEST_NORMAL | RVBLNKFLG |
        RPICKFLAG | RINVALIDIO | RGPIDLE));

    IO_Value2 = inw(ERR_TERM);
    outw(ERR_TERM, 0x5A5AU);
    ProbeWaitIdleEmpty();
    if (inw(ERR_TERM) == 0x5A5AU)
    {
        outw(ERR_TERM, 0x2525U);
        ProbeWaitIdleEmpty();
        if (inw(ERR_TERM) == 0x2525U)
            ATIAdapter = ATI_ADAPTER_8514A;
    }
    outw(ERR_TERM, IO_Value2);

    if (ATIAdapter == ATI_ADAPTER_8514A)
    {
        /* Some kind of 8514/A detected */
        ATIChipHasSUBSYS_CNTL = TRUE;

        /* Don't leave any Mach8 or Mach32 in 8514/A mode */
        IO_Value2 = inw(CLOCK_SEL);
        outw(CLOCK_SEL, IO_Value2);
        ProbeWaitIdleEmpty();

        IO_Value2 = inw(ROM_ADDR_1);
        outw(ROM_ADDR_1, 0x5555U);
        ProbeWaitIdleEmpty();
        if (inw(ROM_ADDR_1) == 0x5555U)
        {
            outw(ROM_ADDR_1, 0x2A2AU);
            ProbeWaitIdleEmpty();
            if (inw(ROM_ADDR_1) == 0x2A2AU)
                ATIAdapter = ATI_ADAPTER_MACH8;
        }
        outw(ROM_ADDR_1, IO_Value2);
    }

    if (ATIAdapter == ATI_ADAPTER_MACH8)
    {
        /* ATI Mach8 or Mach32 accelerator detected */
        outw(DESTX_DIASTP, 0xAAAAU);
        ProbeWaitIdleEmpty();
        if (inw(READ_SRC_X) == 0x02AAU)
            ATIAdapter = ATI_ADAPTER_MACH32;

        outw(DESTX_DIASTP, 0x5555U);
        ProbeWaitIdleEmpty();
        if (inw(READ_SRC_X) == 0x0555U)
        {
            if (ATIAdapter != ATI_ADAPTER_MACH32)
                ATIAdapter = ATI_ADAPTER_8514A;
        }
        else
        {
            if (ATIAdapter != ATI_ADAPTER_MACH8)
                ATIAdapter = ATI_ADAPTER_8514A;
        }
    }
    else
    {
        /* Restore register clobbered by 8514/A reset attempt */
        outw(SUBSYS_CNTL, IO_Value);
    }

#if 0
Skip8514Probe:
#endif
    if (ATIAdapter == ATI_ADAPTER_NONE)
    {
        /*
         * Determine if a Mach64 is present.  First, check the user's IObase.
         */
        if (vga256InfoRec.IObase & BLOCK_IO_SELECT)
            ATIMach64Probe(vga256InfoRec.IObase & SPARSE_IO_BASE, SPARSE_IO, 0);
        else if (vga256InfoRec.IObase & SPARSE_IO_SELECT)
            ATIMach64Probe(vga256InfoRec.IObase & BLOCK_IO_BASE, BLOCK_IO, 0);

        /* Check the "standard" sparse I/O bases */
        ATIMach64Probe(0x02ECU, SPARSE_IO, 0);
        ATIMach64Probe(0x01C8U, SPARSE_IO, 0);
        ATIMach64Probe(0x01CCU, SPARSE_IO, 0);

        /* Lastly, check PCI configuration space */
        if (vgaPCIInfo && vgaPCIInfo->AllCards)
        {
            Index = 0;
            while ((ATIAdapter == ATI_ADAPTER_NONE) &&
                   (PCIDevice = vgaPCIInfo->AllCards[Index++]))
            {
                if (PCIDevice->_vendor != PCI_VENDOR_ATI)
                    continue;
                if (PCIDevice->_device == PCI_CHIP_MACH32)
                    continue;

                if ((PCIDevice->_command &
                     (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)) !=
                    (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE))
                    continue;

                /*
                 * The legacy servers provide Rage 128 support elsewhere.
                 */
                if ((PCIDevice->_device == PCI_CHIP_RAGE128RE) ||
                    (PCIDevice->_device == PCI_CHIP_RAGE128RF) ||
                    (PCIDevice->_device == PCI_CHIP_RAGE128RK) ||
                    (PCIDevice->_device == PCI_CHIP_RAGE128RL) ||
                /* Rage 128 PRO */
                    ((PCIDevice->_device >= PCI_CHIP_RAGE128PA) &&
                     (PCIDevice->_device <= PCI_CHIP_RAGE128PX)) ||
                    ((PCIDevice->_device >= PCI_CHIP_RAGE128SE) &&
                     (PCIDevice->_device <= PCI_CHIP_RAGE128SG)) ||
                    ((PCIDevice->_device >= PCI_CHIP_RAGE128SK) &&
                     (PCIDevice->_device <= PCI_CHIP_RAGE128SM)) ||
                /* Rage 128 Mobility */
                    (PCIDevice->_device == PCI_CHIP_RAGE128LE) ||
                    (PCIDevice->_device == PCI_CHIP_RAGE128LF))
                {
                    Rage128Seen = TRUE;
                    continue;
                }

                ATIMach64Probe(PCIDevice->_base1 & BLOCK_IO_BASE, BLOCK_IO,
                    PCIDevice->_device);
            }
        }
    }

    /* Extract various information from any detected accelerator */
    switch (ATIAdapter)
    {
        case ATI_ADAPTER_8514A:
            MachvideoRam = videoRamSizes[GetBits(IO_Value, _8PLANE) + 2];
            ATIMachChip = ATI_CHIP_8514A;
            IO_Value = inb(EXT_CONFIG_3);
            outb(EXT_CONFIG_3, IO_Value & 0x0FU);
            if (!(inb(EXT_CONFIG_3) & 0xF0U))
            {
                outb(EXT_CONFIG_3, IO_Value | 0xF0U);
                if ((inb(EXT_CONFIG_3) & 0xF0U) == 0xF0U)
                    ATIMachChip = ATI_CHIP_CT480;
            }
            outb(EXT_CONFIG_3, IO_Value);
            break;

        case ATI_ADAPTER_MACH8:
            ATIMachChip = ATI_CHIP_38800_1;
            IO_Value = inw(CONFIG_STATUS_1);
            if (IO_Value & MC_BUS)
                ATIBusType = ATI_BUS_MCA16;
            MachvideoRam =
                videoRamSizes[GetBits(IO_Value, MEM_INSTALLED) + 2];
            break;

        case ATI_ADAPTER_MACH32:
            IO_Value = inw(CONFIG_STATUS_1);
            if (!(IO_Value & (_8514_ONLY | CHIP_DIS)))
            {
                ATIVGAAdapter = ATI_ADAPTER_MACH32;
                ATIChipHasVGAWonder = TRUE;
            }
            ATIBusType = GetBits(IO_Value, BUS_TYPE);
            ATIDac = ATI_DAC(GetBits(IO_Value, DACTYPE), 0);

            ATIMach32ChipID();

            ATIMemoryType = GetBits(IO_Value, MEM_TYPE);
            MachvideoRam =
                videoRamSizes[GetBits(inw(MISC_OPTIONS), MEM_SIZE_ALIAS) + 2];

            /*
             * The 68800-6 doesn't necessarily report the correct video memory
             * size.
             */
            if ((ATIChip == ATI_CHIP_68800_6) && (MachvideoRam == 1024))
                MachvideoRam = ATIMach32videoRam();

            vga256InfoRec.BIOSbase = 0x000C0000U +
                (GetBits(inw(ROM_ADDR_1), BIOS_BASE_SEGMENT) << 11);
            break;

        case ATI_ADAPTER_MACH64:
            /* Set general use I/O port numbers */
            ATIIOPortCRTC_H_TOTAL_DISP = ATIIOPort(CRTC_H_TOTAL_DISP);
            ATIIOPortCRTC_H_SYNC_STRT_WID = ATIIOPort(CRTC_H_SYNC_STRT_WID);
            ATIIOPortCRTC_V_TOTAL_DISP = ATIIOPort(CRTC_V_TOTAL_DISP);
            ATIIOPortCRTC_V_SYNC_STRT_WID = ATIIOPort(CRTC_V_SYNC_STRT_WID);
            ATIIOPortCRTC_OFF_PITCH = ATIIOPort(CRTC_OFF_PITCH);
            ATIIOPortCRTC_INT_CNTL = ATIIOPort(CRTC_INT_CNTL);
            ATIIOPortCRTC_GEN_CNTL = ATIIOPort(CRTC_GEN_CNTL);
            ATIIOPortOVR_CLR = ATIIOPort(OVR_CLR);
            ATIIOPortOVR_WID_LEFT_RIGHT = ATIIOPort(OVR_WID_LEFT_RIGHT);
            ATIIOPortOVR_WID_TOP_BOTTOM = ATIIOPort(OVR_WID_TOP_BOTTOM);
            ATIIOPortCLOCK_CNTL = ATIIOPort(CLOCK_CNTL);
            ATIIOPortMEM_INFO = ATIIOPort(MEM_INFO);
            ATIIOPortDAC_REGS = ATIIOPort(DAC_REGS);
            ATIIOPortDAC_CNTL = ATIIOPort(DAC_CNTL);
            ATIIOPortCONFIG_CNTL = ATIIOPort(CONFIG_CNTL);

            IO_Value = inl(ATIIOPortMEM_INFO);
            if (ATIChip >= ATI_CHIP_264VTB)
            {
                IO_Value = GetBits(IO_Value, CTL_MEM_SIZEB);
                if (IO_Value < 8)
                    MachvideoRam = (IO_Value + 1) * 512;
                else if (IO_Value < 12)
                    MachvideoRam = (IO_Value - 3) * 1024;
                else
                    MachvideoRam = (IO_Value - 7) * 2048;
            }
            else
                MachvideoRam =
                    videoRamSizes[GetBits(IO_Value, CTL_MEM_SIZE) + 2];

            IO_Value = inl(ATIIOPort(SCRATCH_REG1));
            IO_Value2 = inl(ATIIOPort(CONFIG_STATUS64_0));
            ATIDac = GetBits(inl(ATIIOPortDAC_CNTL), DAC_TYPE);

            vga256InfoRec.BIOSbase = 0x000C0000U +
                (GetBits(IO_Value, BIOS_BASE_SEGMENT) << 11);

            if (ATIChip < ATI_CHIP_264CT)
            {
                ATIBusType = GetBits(IO_Value2, CFG_BUS_TYPE);
                IO_Value2 &= (CFG_VGA_EN | CFG_CHIP_EN);
                if (ATIChip == ATI_CHIP_88800CX)
                    IO_Value2 |= CFG_VGA_EN;
                if (IO_Value2 == (CFG_VGA_EN | CFG_CHIP_EN))
                {
                    ATIVGAAdapter = ATI_ADAPTER_MACH64;
                    ATIChipHasVGAWonder = TRUE;

                    /*
                     * Apparently, 0x1CE cannot be used for ATI's extended VGA
                     * registers when using block I/O decoding.  Instead, these
                     * registers are tacked on to VGA's Graphics register bank.
                     */
                    if (ATIIODecoding == BLOCK_IO)
                        ATIIOPortVGAWonder = GRAX;
                }

                /* Factor in what the BIOS says the DAC is */
                ATIDac = ATI_DAC(ATIDac,
                    GetBits(IO_Value, BIOS_INIT_DAC_SUBTYPE));

                ATIMemoryType = GetBits(IO_Value2, CFG_MEM_TYPE);
            }
            else
            {
                /*
                 * On VT's and above, it's possible BIOS initialization
                 * disabled VGA functionality through this adapter.  It could
                 * be re-enabled, but this would mean disabling whatever else
                 * is providing the system's VGA.  So, for now, respect the
                 * BIOS setting.  This situation might well be dealt with
                 * differently if or when it shows up during testing or normal
                 * use.
                 */
                if ((ATIChip < ATI_CHIP_264VT) || (IO_Value2 & CFG_VGA_EN_T))
                    ATIVGAAdapter = ATI_ADAPTER_MACH64;

                ATIMemoryType = GetBits(IO_Value2, CFG_MEM_TYPE_T);

                /* Set LCD and TV I/O port numbers */
                if (ATIChip == ATI_CHIP_264LT)
                {
                    ATILCDPanelID = GetBits(IO_Value2, CFG_PANEL_ID);

                    ATIIOPortHORZ_STRETCHING = ATIIOPort(HORZ_STRETCHING);
                    ATIIOPortVERT_STRETCHING = ATIIOPort(VERT_STRETCHING);
                    ATIIOPortLCD_GEN_CTRL = ATIIOPort(LCD_GEN_CTRL);

                    /*
                     * Don't bother with panel support if it's not enabled by
                     * BIOS initialization.
                     */
                    if (!(inl(ATIIOPortLCD_GEN_CTRL) & LCD_ON))
                        ATILCDPanelID = -1;
                }
                else if ((ATIChip == ATI_CHIP_264LTPRO) ||
                         (ATIChip == ATI_CHIP_264XL) ||
                         (ATIChip == ATI_CHIP_MOBILITY))
                {
                    ATILCDPanelID = GetBits(IO_Value2, CFG_PANEL_ID);

                    ATIIOPortTV_OUT_INDEX = ATIIOPort(TV_OUT_INDEX);
                    ATIIOPortTV_OUT_DATA = ATIIOPort(TV_OUT_DATA);
                    ATIIOPortLCD_INDEX = ATIIOPort(LCD_INDEX);
                    ATIIOPortLCD_DATA = ATIIOPort(LCD_DATA);

                    /*
                     * Don't bother with panel support if it's not enabled by
                     * BIOS initialization.
                     */
                    IO_Value = inl(ATIIOPortLCD_INDEX);
                    IO_Value2 = ATIGetLTProLCDReg(LCD_HORZ_STRETCHING);
#if 0
                    if (IO_Value2 & AUTO_HORZ_RATIO)
#endif
                        ATILCDHorizontal =
                            (GetBits(IO_Value2, HORZ_PANEL_SIZE) + 1) << 3;
                    IO_Value2 = ATIGetLTProLCDReg(LCD_EXT_VERT_STRETCH);
#if 0
                    if (IO_Value2 & AUTO_VERT_RATIO)
#endif
                        ATILCDVertical =
                            GetBits(IO_Value2, VERT_PANEL_SIZE) + 1;
                    IO_Value2 = ATIGetLTProLCDReg(LCD_GEN_CNTL);
                    outl(ATIIOPortLCD_INDEX, IO_Value);
                    if (!(IO_Value2 & LCD_ON))
                        ATILCDPanelID = -1;
                }
            }

            /*
             * RAMDAC types 0 & 1 for Mach64's are not the same as for
             * Mach32's.
             */
            if (ATIDac < ATI_DAC_ATI68875)
                ATIDac += ATI_DAC_INTERNAL;
            break;

        default:
            break;
    }

    /* Get video BIOS, *all* of it */
    Index = xf86ReadBIOS(vga256InfoRec.BIOSbase, 0, BIOS, SizeOf(BIOS));

    /* Fill in what cannot be gotten with zeroes */
    if (Index < 0)
        Index = 0;
    for (;  Index < BIOS_SIZE;  Index++)
        BIOS[Index] = 0;

    if ((BIOSByte(0) == 0x55U) && (BIOSByte(1) == 0xAAU))
        BIOSSize = BIOSByte(2) << 9;

    /*
     * Attempt to find the ATI signature in the first 1024 bytes of the video
     * BIOS.
     */
    if (!Rage128Seen)
        for (Signature = 0;  Signature < No_Signature;  Signature++)
            for (Index = 0; BIOS[Signature + Index] == ATISignature[Index];  )
                if (++Index >= Signature_Size)
                    goto signature_found;
    signature_found:;

    /*
     * If no VGA capability has yet been detected, determine if VGA Wonder
     * functionality is possible.
     */
    if ((ATIAdapter <= ATI_ADAPTER_MACH8) &&
        (Signature == BIOS_Signature) &&
        (BIOS[0x40U] == '3'))
    {
        switch (BIOS[0x41U])
        {
            case '1':
                /* This is a Mach8 or VGA Wonder adapter of some kind */
                if ((BIOS[0x43U] >= '1') && (BIOS[0x43U] <= '6'))
                    ATIChip = BIOS[0x43U] - ('1' - ATI_CHIP_18800);

                switch (BIOS[0x43U])
                {
                    case '1':           /* ATI_CHIP_18800 */
                        ATIVGAOffset = 0xB0U;
                        ATIVGAAdapter = ATI_ADAPTER_V3;

                        /* Reset a few things for V3 adapters */
                        ATI.ChipSetRead = ATIV3SetRead;
                        ATI.ChipSetWrite = ATIV3SetWrite;
                        ATI.ChipSetReadWrite = ATIV3SetReadWrite;
                        ATI.ChipUse2Banks = FALSE;
                        break;

                    case '2':           /* ATI_CHIP_18800_1 */
                        ATIVGAOffset = 0xB0U;
                        if (BIOS[0x42U] & 0x10U)
                            ATIVGAAdapter = ATI_ADAPTER_V5;
                        else
                            ATIVGAAdapter = ATI_ADAPTER_V4;

                        /* Reset a few things for V4 and V5 adapters */
                        ATI.ChipSetRead = ATIV4V5SetRead;
                        ATI.ChipSetWrite = ATIV4V5SetWrite;
                        ATI.ChipSetReadWrite = ATIV4V5SetReadWrite;
                        break;

                    case '3':           /* ATI_CHIP_28800_2 */
                    case '4':           /* ATI_CHIP_28800_4 */
                    case '5':           /* ATI_CHIP_28800_5 */
                    case '6':           /* ATI_CHIP_28800_6 */
                        ATIVGAOffset = 0xA0U;
                        ATIVGAAdapter = ATI_ADAPTER_PLUS;
                        if (BIOS[0x44U] & 0x80U)
                        {
                            ATIVGAAdapter = ATI_ADAPTER_XL;
                            ATIDac = ATI_DAC_SC11483;
                        }
                        break;

                    case 'a':           /* A Mach32 with a */
                    case 'b':           /* frontal lobotomy */
                    case 'c':
                        ATIVGAAdapter = ATI_ADAPTER_NONISA;
                        ATIMach32ChipID();
                        ProbeWaitIdleEmpty();
                        if (inw(SUBSYS_STAT) != 0xFFFFU)
                            ATIChipHasSUBSYS_CNTL = TRUE;
                        break;

                    case ' ':           /* A crippled Mach64 */
                        ATIVGAAdapter = ATI_ADAPTER_NONISA;
                        ATIMach64ChipID(0);
                        break;

                    default:
                        break;
                }

                if (ATIVGAAdapter != ATI_ADAPTER_NONE)
                    ATIChipHasVGAWonder = TRUE;
                break;
#if 0
            case '2':
                ATIVGAOffset = 0xB0U;   /* Presumably */
                ATIVGAAdapter = ATI_ADAPTER_EGA_PLUS;
                break;

            case '3':
                ATIVGAOffset = 0xB0U;   /* Presumably */
                ATIVGAAdapter = ATI_ADAPTER_BASIC;
                break;
#endif
            case '?':                   /* A crippled Mach64 */
                ATIVGAAdapter = ATI_ADAPTER_NONISA;
                ATIMach64ChipID(0);
                break;

            default:
                break;
        }

        if (ATIAdapter == ATI_ADAPTER_NONE)
            ATIAdapter = ATIVGAAdapter;
    }

    /*
     * At this point, some adapters should probably be shared with other
     * drivers :-).
     */
    if (Signature != BIOS_Signature)
    {
        if ((ATIVGAAdapter == ATI_ADAPTER_NONE) &&
            (ATIChipSet == ATI_CHIPSET_IBMVGA) &&
            !Rage128Seen)
        {
            /*
             * VGA has one more attribute register than EGA.  See if it can be
             * read and written.
             */
            ATISetVGAIOBase(inb(R_GENMO));
            (void) inb(GENS1(vgaIOBase));
            IO_Value = GetReg(ATTRX, 0x14U | 0x20U);
            outb(ATTRX, IO_Value ^ 0x0FU);
            IO_Value2 = GetReg(ATTRX, 0x14U | 0x20U);
            outb(ATTRX, IO_Value);
            if (IO_Value2 == (IO_Value ^ 0x0FU))
            {
                /* VGA device detected */
                ATIChip = ATI_CHIP_VGA;
                ATIVGAAdapter = ATI_ADAPTER_VGA;
                if (ATIAdapter == ATI_ADAPTER_NONE)
                    ATIAdapter = ATIVGAAdapter;

                /* Disable banking */
                ATI.ChipSetRead = ATI.ChipSetWrite = ATI.ChipSetReadWrite =
                    (BankFunction *)NoopDDA;
            }
        }
        if ((ATIAdapter == ATI_ADAPTER_NONE) ||
            (ATIVGAAdapter == ATI_ADAPTER_NONE))
        {
            if (vga256InfoRec.chipset)
                ErrorF("XF86Config chipset specified as \"%s\",\n but no"
                       " applicable adapter found.\n", vga256InfoRec.chipset);
            xf86DisableIOPorts(vga256InfoRec.scrnIndex);
            vga256InfoRec.BIOSbase = saved_BIOSbase;
            return FALSE;
        }
    }

    /*
     * For Mach64 adapters, pick up, from the BIOS, the type of programmable
     * clock generator (if any), and various information about it.
     */
    if (ATIChip >= ATI_CHIP_88800GXC)
    {
        ROMTable = BIOSWord(0x48U);
        if ((ROMTable + 0x12U) > BIOSSize)
            ROMTable = 0;
        if (ROMTable > 0)
        {
            ClockTable = BIOSWord(ROMTable + 0x10U);
            if ((ClockTable + 0x0CU) > BIOSSize)
                ClockTable = 0;
        }
        if (ClockTable > 0)
        {
            FrequencyTable = BIOSWord(ClockTable - 0x02U);
            if ((FrequencyTable + 0x20U) > BIOSSize)
                FrequencyTable = 0;
            if (FrequencyTable > 0)
                for (Index = 0;  Index < 16;  Index++)
                    ATIBIOSClocks[Index] = (&BIOSWord(FrequencyTable))[Index];
            ATIProgrammableClock = BIOSByte(ClockTable);
            ATIClockNumberToProgramme = BIOSByte(ClockTable + 0x06U);
            if (ATIProgrammableClock < ATI_CLOCK_MAX)
                ATIClockDescriptor += ATIProgrammableClock;
            if ((BIOSWord(ClockTable + 0x08U) / 10) != 143)
            {
                ATIReferenceNumerator = BIOSWord(ClockTable + 0x08U) * 10;
                ATIReferenceDenominator = 1;
            }
        }

        ClockDac = ATIDac;
        switch (ATIProgrammableClock)
        {
            case ATI_CLOCK_ICS2595:
                /*
                 * Pick up reference divider (43 or 46) appropriate to the chip
                 * revision level.
                 */
                if (ClockTable > 0)
                    ATIClockDescriptor->MinM = ATIClockDescriptor->MaxM =
                        BIOSWord(ClockTable + 0x0AU);
                break;

            case ATI_CLOCK_STG1703:
                /* This one's also a RAMDAC */
                ClockDac = ATI_DAC_STG1703;
                break;

            case ATI_CLOCK_CH8398:
                /* This one's also a RAMDAC */
                ClockDac = ATI_DAC_CH8398;
                break;

            case ATI_CLOCK_INTERNAL:
                /*
                 * The reference divider has already been programmed by BIOS
                 * initialization.  Because, there is only one reference
                 * divider for all generated frequencies (including MCLK), it
                 * cannot be changed without reprogramming all clocks every
                 * time one of them needs a different reference divider.
                 *
                 * Besides, it's not a good idea to change the reference
                 * divider.  BIOS initialization sets it to a value that
                 * effectively prevents generating frequencies beyond the
                 * graphics controller's tolerance.
                 */
                ATIClockDescriptor->MinM = ATIClockDescriptor->MaxM =
                    ATIGetMach64PLLReg(PLL_REF_DIV);

                /* The DAC is also integrated */
                if ((ATIDac & ~0x0FU) != ATI_DAC_INTERNAL)
                    ClockDac = ATI_DAC_INTERNAL;

                break;

            case ATI_CLOCK_ATT20C408:
                /* This one's also a RAMDAC */
                ClockDac = ATI_DAC_ATT20C408;
                break;

            case ATI_CLOCK_IBMRGB514:
                /* This one's also a RAMDAC */
                ClockDac = ATI_DAC_IBMRGB514;
                ATIClockNumberToProgramme = 7;
                break;

            default:
                break;
        }

        /*
         * We now have up to two indications of what RAMDAC the adapter uses.
         * They should be the same.  The following test and corresponding
         * action are under construction.
         */
        if (ATIDac != ClockDac)
        {
            ErrorF("Mach64 RAMDAC probe discrepancy detected:\n"
                   "  ATIDac=0x%02X;  ClockDac=0x%02X.\n", ATIDac, ClockDac);

            if (ATIDac == ATI_DAC_IBMRGB514)
            {
                ATIProgrammableClock = ATI_CLOCK_IBMRGB514;
                ATIClockDescriptor = ATIClockDescriptors + ATI_CLOCK_IBMRGB514;
                ATIClockNumberToProgramme = 7;
            }
            else
                ATIDac = ClockDac;      /* For now */
        }

        /*
         * For adapters with supported programmable clock generators, set an
         * initial estimate for maxClock.  This value might be reduced later
         * due to RAMDAC considerations.
         */
        if (ATIClockDescriptor->MaxN > 0)
        {
            int Numerator = ATIClockDescriptor->MaxN * ATIReferenceNumerator;
            int Denominator = ATIClockDescriptor->MinM *
                ATIReferenceDenominator * ATIClockDescriptor->PostDividers[0];

            /*
             * An integrated PLL behaves as though the reference frequency were
             * doubled.
             */
            if (ATIProgrammableClock == ATI_CLOCK_INTERNAL)
                Numerator <<= 1;

            vga256InfoRec.maxClock = (Numerator / (Denominator * 1000)) * 1000;
        }

        /*
         * Use the XF86Config's ChipSet specification to decide which CRTC to
         * use for the video modes generated by the server.
         */
        if (!ATIUsingPlanarModes && (ATIChipSet == ATI_CHIPSET_ATI))
        {
            ATICRTC = ATI_CRTC_MACH64;

            /* Support higher depths on integrated controllers */
            if (ATIChip >= ATI_CHIP_264CT)
            {
                if ((xf86weight.red == 5) && (xf86weight.blue == 5) &&
                    (xf86weight.green >= 5) && (xf86weight.green <= 6))
                    ATI.ChipHas16bpp = TRUE;
                ATI.ChipHas24bpp =
                    ATI.ChipHas32bpp = TRUE;
            }
        }

        /*
         * For LT's and LT Pro's, determine panel dimensions and driving clock.
         */
        if (ATILCDPanelID >= 0)
        {
            LCDTable = BIOSWord(0x78U);
            if ((LCDTable + BIOSByte(LCDTable + 5)) > BIOSSize)
                LCDTable = 0;

            if (LCDTable > 0)
            {
                LCDPanelInfo = BIOSWord(LCDTable + 0x0AU);
                if (((LCDPanelInfo + 0x1DU) > BIOSSize) ||
                    ((BIOSByte(LCDPanelInfo) != ATILCDPanelID) &&
                     (ATILCDPanelID || (BIOSByte(LCDPanelInfo) > 0x1Fu) ||
                      (ATIChip <= ATI_CHIP_264LTPRO))))
                    LCDPanelInfo = 0;
            }

            if (!LCDPanelInfo)
            {
                /*
                 * Scan BIOS for panel info table.
                 */
                for (Index = 0;  Index < (int)(BIOSSize - 0x1DU);  Index++)
                {
                    /* Look for panel ID ... */
                    if ((BIOSByte(Index) != ATILCDPanelID) &&
                        (ATILCDPanelID || (BIOSByte(Index) > 0x1FU) ||
                         (ATIChip <= ATI_CHIP_264LTPRO)))
                        continue;

                    /* ... followed by 24-byte panel model name ... */
                    for (Index2 = 0;  Index2 < 24;  Index2++)
                        if ((CARD8)(BIOSByte(Index + Index2 + 1) - 0x20U) >
                            0x5FU)
                        {
                            Index += Index2;
                            goto NextBIOSByte;
                        }

                    /* ... verify panel width ... */
                    if ((ATILCDHorizontal > 8) &&
                        (ATILCDHorizontal <=
                         (int)(MaxBits(HORZ_PANEL_SIZE) << 3)) &&
                        (ATILCDHorizontal != BIOSWord(Index + 0x19U)))
                        continue;

                    /* ... and verify panel height */
                    if ((ATILCDVertical > 1) &&
                        (ATILCDVertical <= (int)MaxBits(VERT_PANEL_SIZE)) &&
                        (ATILCDVertical != BIOSWord(Index + 0x1BU)))
                        continue;

                    if (LCDPanelInfo)
                    {
                        /*
                         * More than one possibility, but don't care if all
                         * tables describe panels of the same size.
                         */
                        if (BIOSLong(LCDPanelInfo + 0x19U) ==
                            BIOSLong(Index + 0x19U))
                            continue;

                        LCDPanelInfo = 0;
                        break;
                    }

                    LCDPanelInfo = Index;

            NextBIOSByte:  ;
                }
            }

            if (LCDPanelInfo > 0)
            {
                ATILCDPanelID = BIOSByte(LCDPanelInfo);
                ATILCDHorizontal = BIOSWord(LCDPanelInfo + 0x19U);
                ATILCDVertical = BIOSWord(LCDPanelInfo + 0x1BU);

                /* Compute panel clock */
                if (inl(ATIIOPortCRTC_GEN_CNTL) & CRTC_EXT_DISP_EN)
                    Index = GetBits(inb(ATIIOPortCLOCK_CNTL), CLOCK_SELECT);
                else
                    Index = GetBits(inb(R_GENMO), 0x0C);
                ATILCDClock = 2 * ATIGetMach64PLLReg(PLL_VCLK0_FB_DIV + Index);
                ATILCDClock *= ATIReferenceNumerator;
                ATILCDClock /= ATIClockDescriptor->MinM;
                ATILCDClock /= ATIReferenceDenominator;
                Index2 =
                    GetBits(ATIGetMach64PLLReg(PLL_XCLK_CNTL),
                        PLL_VCLK0_XDIV << Index);
                Index2 *= MaxBits(PLL_VCLK0_POST_DIV) + 1;
                Index2 |= GetBits(ATIGetMach64PLLReg(PLL_VCLK_POST_DIV),
                    PLL_VCLK0_POST_DIV << (2 * Index));
                ATILCDClock /= ATIClockDescriptor->PostDividers[Index2];
            }
        }

        /*
         * Decide which aperture(s) to enable to allow CPU access to video
         * memory.
         */
        if ((ATICRTC == ATI_CRTC_MACH64) || (ATIChip >= ATI_CHIP_264CT))
        {
            /* Possibly set up for a linear aperture */
            OFLG_SET(OPTION_NOLINEAR_MODE, &ATI.ChipOptionFlags);

            if (!ATIUsingPlanarModes &&
                !OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options) &&
                xf86LinearVidMem())
            {
                /* Get the adapter's linear aperture configuration */
                IO_Value = inl(ATIIOPortCONFIG_CNTL);
                ATI.ChipLinearBase = GetBits(IO_Value, CFG_MEM_AP_LOC) << 22;
                if ((IO_Value & CFG_MEM_AP_SIZE) != CFG_MEM_AP_SIZE)
                    ATI.ChipLinearSize =
                        GetBits(IO_Value, CFG_MEM_AP_SIZE) << 22;

                /* Except for PCI, allow user override */
                if ((ATIBusType != ATI_BUS_PCI) && (ATIBusType != ATI_BUS_AGP))
                {
                    if (ATIChip >= ATI_CHIP_88800GXE)
                        IO_Value2 = vga256InfoRec.MemBase &
                            ~((unsigned long)((1 << 24) - 1));
                    else if (MachvideoRam >= 4096)
                        IO_Value2 = vga256InfoRec.MemBase &
                            ~((unsigned long)((1 << 23) - 1));
                    else
                        IO_Value2 = vga256InfoRec.MemBase &
                            ~((unsigned long)((1 << 22) - 1));
                    if (IO_Value2 &&
                        (IO_Value2 <=
                            (GetBits(CFG_MEM_AP_LOC, CFG_MEM_AP_LOC) << 22)))
                        ATI.ChipLinearBase = IO_Value2;
                    if ((ATIChip < ATI_CHIP_264CT) && (MachvideoRam < 4096))
                        ATI.ChipLinearSize = 4 * 1024 * 1024;
                    else
                        ATI.ChipLinearSize = 8 * 1024 * 1024;
                }

                if (ATI.ChipLinearBase && ATI.ChipLinearSize)
                    ATI.ChipUseLinearAddressing = TRUE;
            }

            if (ATIVGAAdapter == ATI_ADAPTER_NONE)
            {
                /* Reset banking functions */
                ATI.ChipSetRead = ATI.ChipSetWrite =
                    ATI.ChipSetReadWrite = (BankFunction *)NoopDDA;
            }
            else
            {
                /*
                 * Enable the small dual paged apertures, even if the linear
                 * aperture is available.
                 */
                ATIUsingSmallApertures = TRUE;

                /* Reset banking functions */
                if (ATIUsingPlanarModes)
                {
                    ATI.ChipSetRead = ATIMach64SetReadPlanar;
                    ATI.ChipSetWrite = ATIMach64SetWritePlanar;
                    ATI.ChipSetReadWrite = ATIMach64SetReadWritePlanar;
                }
                else
                {
                    ATI.ChipSetRead = ATIMach64SetReadPacked;
                    ATI.ChipSetWrite = ATIMach64SetWritePacked;
                    ATI.ChipSetReadWrite = ATIMach64SetReadWritePacked;
                }

                /* Set banking port numbers */
                ATIIOPortMEM_VGA_RP_SEL = ATIIOPort(MEM_VGA_RP_SEL);
                ATIIOPortMEM_VGA_WP_SEL = ATIIOPort(MEM_VGA_WP_SEL);
            }
        }
    }

    if ((ATIChipHasVGAWonder) && (ATIChip <= ATI_CHIP_88800GXD))
    {
        /*
         * Set up extended VGA register addressing.  Note that, for Mach64's,
         * only the GX-C & GX-D controllers allow the setting of this address.
         */
        if ((ATIChip < ATI_CHIP_88800GXC) &&
            (Signature == BIOS_Signature) &&
            (BIOSWord(0x10U)) &&
            (!(BIOSWord(0x10U) & ~(SPARSE_IO_BASE | IO_BYTE_SELECT))))
        {
            /* Pick up extended register index I/O port number */
            ATIIOPortVGAWonder = BIOSWord(0x10U);
        }
        PutReg(GRAX, 0x50U, GetByte(ATIIOPortVGAWonder, 0));
        PutReg(GRAX, 0x51U, GetByte(ATIIOPortVGAWonder, 1) | ATIVGAOffset);
    }

    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    ATIEnterLeave(ENTER);               /* Unlock registers */

    /* Sometimes, the BIOS lies about the chip */
    if ((ATIChip >= ATI_CHIP_28800_4) && (ATIChip <= ATI_CHIP_28800_6))
    {
        IO_Value = GetBits(ATIGetExtReg(0xAAU), 0x0FU) +
            (ATI_CHIP_28800_4 - 4);
        if ((IO_Value <= ATI_CHIP_28800_6) && (IO_Value > ATIChip))
            ATIChip = IO_Value;
    }

    if ((xf86Verbose) ||
        (ATIChip == ATI_CHIP_NONE) || (ATIChip == ATI_CHIP_Mach64) ||
        ((ATIVGAAdapter == ATI_ADAPTER_NONE) && (ATICRTC == ATI_CRTC_VGA)))
    {
        ErrorF("Using XFree86 ATI driver version " ATI_VERSION_NAME ".\n");
        ErrorF("%s graphics controller detected.\n", ATIChipNames[ATIChip]);
        if ((ATIChip >= ATI_CHIP_68800) && (ATIChip != ATI_CHIP_68800_3))
        {
            ErrorF("Chip type %04X", ATIChipType);
            if (!(ATIChipType & ~(CHIP_CODE_0 | CHIP_CODE_1)))
                ErrorF(" (%c%c)", GetBits(ATIChipType, CHIP_CODE_1) + 0x41U,
                    GetBits(ATIChipType, CHIP_CODE_0) + 0x41U);
            else if ((ATIChipType & 0x4040U) == 0x4040U)
                ErrorF(" \"%c%c\"", GetByte(ATIChipType, 1),
                    GetByte(ATIChipType, 0));
            if ((ATIChip >= ATI_CHIP_264CT) && (ATIChip != ATI_CHIP_Mach64))
                ErrorF(", version %d, foundry %s", ATIChipVersion,
                    ATIFoundryNames[ATIChipFoundry]);
            ErrorF(", class %d, revision 0x%02X.\n", ATIChipClass,
                ATIChipRevision);
        }
        if (ATIAdapter >= ATI_ADAPTER_MACH8)
        {
            ErrorF("%s interface detected", ATIBusNames[ATIBusType]);
            if (ATIAdapter == ATI_ADAPTER_MACH64)
                ErrorF(";  %s I/O base is 0x%04X",
                    (ATIIODecoding == SPARSE_IO) ? "Sparse" : "Block",
                    ATIIOBase);
            ErrorF(".\n");
        }
        if (ATIMachChip != ATI_CHIP_NONE)
            ErrorF("%s graphics accelerator detected, with %d kB of"
                   " coprocessor memory.\n", ATIChipNames[ATIMachChip],
                MachvideoRam);
        if ((ATIVGAAdapter == ATI_ADAPTER_NONE) &&
            (Signature == BIOS_Signature))
            ErrorF("Unknown chip descriptor in BIOS:  0x%02X%02X%02X%02X.\n",
               BIOS[0x40U], BIOS[0x41U], BIOS[0x42U], BIOS[0x43U]);
        ErrorF("%s video adapter detected.\n", ATIAdapterNames[ATIAdapter]);
    }

    if ((ATIDac & ~0x0FU) == ATI_DAC_INTERNAL)
    {
        if (xf86Verbose)
            ErrorF("Internal RAMDAC (subtype %d) detected.\n", ATIDac & 0x0FU);
    }
    else for (DAC = ATIDACDescriptors;  ;  DAC++)
    {
        if (ATIDac == DAC->DACType)
        {
            if (xf86Verbose)
                ErrorF("%s RAMDAC detected.\n", DAC->DACName);
            break;
        }
        if (ATIDac < DAC->DACType)
        {
            ErrorF("Unknown RAMDAC type (0x%02X) detected.\n", ATIDac);
            break;
        }
    }

    if (ATILCDPanelID >= 0)
    {
        if (LCDPanelInfo <= 0)
        {
            ErrorF("Unable to determine dimensions of panel (ID %d)!\n",
                   ATILCDPanelID);
            ATILCDPanelID = -1;         /* Revert to unsupported status */
        }
        else if (xf86Verbose)
        {
            ErrorF("%dx%d panel (ID %d) detected;  '%.24s'.\n",
                   ATILCDHorizontal, ATILCDVertical, ATILCDPanelID,
                   BIOS + LCDPanelInfo + 1);
            ErrorF("Panel clock is %.3f MHz.\n", (double)ATILCDClock / 1000.0);
        }
    }

    switch (ATIAdapter)
    {
        case ATI_ADAPTER_8514A:
        case ATI_ADAPTER_MACH8:
            /* From now on, ignore any 8514/A or Mach8 accelerator */
            ATIAdapter = ATIVGAAdapter;
            /* Accelerator and VGA cannot share memory */
            MachvideoRam = 0;
            break;

        case ATI_ADAPTER_MACH32:
        case ATI_ADAPTER_MACH64:
            if ((ATIVGAAdapter == ATI_ADAPTER_NONE) &&
                (!ATI.ChipUseLinearAddressing || (ATICRTC == ATI_CRTC_VGA)))
            {
                ErrorF("VGA aperture is not available through this"
                       " adapter.\n");
                ATIEnterLeave(LEAVE);
                vga256InfoRec.BIOSbase = saved_BIOSbase;
                return FALSE;
            }
            if (saved_BIOSbase != vga256InfoRec.BIOSbase)
            {
                ErrorF("BIOS Base Address changed to 0x%08X.\n",
                    vga256InfoRec.BIOSbase);
                OFLG_CLR(XCONFIG_BIOSBASE, &vga256InfoRec.xconfigFlag);
            }
            break;

        default:
            break;
    }

    if (ATIChip == ATI_CHIP_NONE)
        ErrorF("Support for this video adapter is highly experimental!\n");

    if (ATIAdapter == ATI_ADAPTER_VGA)
    {
        if (ATIUsingPlanarModes)
            VGAvideoRam = 256;
        else
            VGAvideoRam = 64;

        /*
         * The XF86Config videoRam must be limited because generic VGA doesn't
         * implement video memory banking.
         */
        if (VGAvideoRam < vga256InfoRec.videoRam)
        {
            ErrorF("XF86Config videoRam specification reduced to %d kB,\n"
                   " because generic VGA does not support banking.\n",
                VGAvideoRam);
            vga256InfoRec.videoRam = VGAvideoRam;
            OFLG_CLR(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag);
        }
    }
    else
    {
        /* Normalize any XF86Config videoRam value */
        if (ATIChip < ATI_CHIP_264VTB)
        {
            for (Index = 0;  videoRamSizes[++Index];  )
                if (vga256InfoRec.videoRam < videoRamSizes[Index])
                    break;
            vga256InfoRec.videoRam = videoRamSizes[Index - 1];
        }
        else
        {
            if (vga256InfoRec.videoRam <= 4096)
                vga256InfoRec.videoRam &= ~(512 - 1);
            else if (vga256InfoRec.videoRam <= 8192)
                vga256InfoRec.videoRam &= ~(1024 - 1);
            else if (vga256InfoRec.videoRam <= 16384)
                vga256InfoRec.videoRam &= ~(2048 - 1);
            else
                vga256InfoRec.videoRam = 16 * 1024;
        }
    }

    /*
     * The default videoRam value is what the accelerator (if any) thinks it
     * has.  Also, allow the user to override the accelerator's value.
     */
    ATIvideoRam = MachvideoRam;
    if (!vga256InfoRec.videoRam)
    {
        /* Normalization might have zeroed XF86Config videoRam value */
        OFLG_CLR(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag);
        vga256InfoRec.videoRam = MachvideoRam;
    }
    else
        MachvideoRam = vga256InfoRec.videoRam;

    if (ATIChipHasVGAWonder)
    {
        /* Find out how much video memory the VGA Wonder side thinks it has */
        if (ATIChip <= ATI_CHIP_18800_1)
        {
            IO_Value = ATIGetExtReg(0xBBU);
            if (IO_Value & 0x20U)
                VGAvideoRam = 512;
            else
                VGAvideoRam = 256;
            if (MachvideoRam > 512)
                MachvideoRam = 512;
        }
        else
        {
            IO_Value = ATIGetExtReg(0xB0U);
            if (IO_Value & 0x08U)
                VGAvideoRam = 1024;
            else if (IO_Value & 0x10U)
                VGAvideoRam = 512;
            else
                VGAvideoRam = 256;
            if (MachvideoRam > 1024)
                MachvideoRam = 1024;
        }
    }

    /*
     * If there's no supported accelerator, default videoRam to what the VGA
     * side believes.
     */
    if (!vga256InfoRec.videoRam)
        ATIvideoRam = vga256InfoRec.videoRam = VGAvideoRam;
    else if ((ATIChip < ATI_CHIP_68800) || (ATIChip > ATI_CHIP_68800AX))
    /*
     * After BIOS initialization, the accelerator (if any) and the VGA won't
     * necessarily agree on the amount of video memory, depending on whether or
     * where the memory boundary is configured.  Any discrepancy will be
     * resolved by ATIInit.
     *
     * However, it's possible that there is more video memory than VGA Wonder
     * can architecturally handle.
     */
    if ((MachvideoRam < vga256InfoRec.videoRam) && (ATICRTC == ATI_CRTC_VGA))
    {
        if (OFLG_ISSET(OPTION_FB_DEBUG, &vga256InfoRec.options))
            ErrorF("Virtual resolutions requiring more than %d kB\n of video"
                   " memory might not function correctly.\n",
                ATIUsing1bppModes ? (MachvideoRam / 4) : MachvideoRam);
        else
        {
            /*
             * Temporary code to disable virtual resolutions that are too
             * large.
             */
            if (OFLG_ISSET(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag))
            {
                ErrorF("XF86Config videoRam specification reduced to %d kB due"
                       " to hardware limitations.\n See README.ati for more"
                       " information.\n", MachvideoRam);
                OFLG_CLR(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag);
            }
            vga256InfoRec.videoRam = MachvideoRam;
        }
    }

    /* VT-B's and later have DSP registers */
    if ((ATIChip >= ATI_CHIP_264VTB) && (ATIIODecoding == BLOCK_IO) &&
        !ATIDSPProbe())
    {
        ATIEnterLeave(LEAVE);
        vga256InfoRec.BIOSbase = saved_BIOSbase;
        return FALSE;
    }

    /* Set up for video memory banking */
    vga256InfoRec.bankedMono = TRUE;
    if (ATIUsingPlanarModes)
    {
        if (vga256InfoRec.videoRam <= 256)
            vga256InfoRec.bankedMono = FALSE;
        else if (ATIChip <= ATI_CHIP_18800_1)
        {
            if (OFLG_ISSET(OPTION_FB_DEBUG, &vga256InfoRec.options))
                ErrorF("Virtual resolutions requiring more than %s kB\n of"
                       " video memory might not function properly.\n See"
                       " README.ati for more information.\n",
                    ATIUsing1bppModes ? "64" : "256");
            else
            {
                /* Temporary code to disable banking in planar modes */
                if (OFLG_ISSET(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag))
                {
                    ErrorF("XF86Config videoRam specification reduced to 256"
                           " kB due to hardware limitations.\n See README.ati"
                           " for more information.\n");
                    OFLG_CLR(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag);
                }
                vga256InfoRec.videoRam = 256;
                vga256InfoRec.bankedMono = FALSE;
            }
        }

        /* Planar modes also need a larger virtual X rounding */
        ATI.ChipRounding = 32;
    }
    else if ((ATIChip >= ATI_CHIP_264CT) || (ATICRTC == ATI_CRTC_MACH64) ||
             ((ATIChip <= ATI_CHIP_18800) && (ATIvideoRam == 256)))
    {
        ATI.ChipRounding = 8;   /* Reduce virtual X rounding requirements */
#       ifdef __TSI__
            /* A temporary kludge */
            if (!ATI.ChipUseLinearAddressing &&
                (vga256InfoRec.bitsPerPixel == 24))
                ATI.ChipRounding = 4096;
#       endif /* __TSI__ */
    }

    if (ATI.ChipUseLinearAddressing)
    {
        MachvideoRam = (ATI.ChipLinearSize >> 10) - 2;  /* 4? */
        if (vga256InfoRec.videoRam > MachvideoRam)
        {
            if (ATIChip < ATI_CHIP_264VTB)
            {
                /*
                 * Don't allow virtual resolution to overlay register
                 * aperture(s).
                 */
                vga256InfoRec.videoRam = MachvideoRam;
                ErrorF("Virtual resolutions will be limited to %dkB to account"
                       " for\n accelerator register aperture.\n", MachvideoRam);
            }
            else
            {
                /*
                 * On VTB's and later, ATIInit disables the primary register
                 * aperture.  This is done so the driver can get at the frame
                 * buffer memory behind it.  For MMIO purposes, the auxillary
                 * register aperture will be used instead.  Also, ignore the
                 * CONFIG_CNTL register's indication of linear aperture size,
                 * as it is insufficient for adapters with more than 8MB of
                 * video memory.
                 */
                if (vga256InfoRec.videoRam > (8 * 1024))
                    ATI.ChipLinearSize = 16 * 1024 * 1024;
            }
        }

        ErrorF("Using %dMB linear aperture at 0x%08X.\n",
            ATI.ChipLinearSize >> 20, ATI.ChipLinearBase);

        /* Only mmap what is needed */
        ATI.ChipLinearSize = vga256InfoRec.videoRam * 1024;
    }

    if (ATIAdapter >= ATI_ADAPTER_MACH32)
    {
        if (ATIChip >= ATI_CHIP_264CT)
        {
            if (xf86Verbose || (ATIMemoryType == MEM_264_NONE) ||
                (ATIMemoryType >= MEM_264_TYPE_7))
                ATIPrintMemoryType(ATIMemoryTypeNames_264xT[ATIMemoryType]);
        }
        else if (ATIChip == ATI_CHIP_88800CX)
        {
            if (xf86Verbose || (ATIMemoryType == MEM_CX_TYPE_2) ||
                (ATIMemoryType >= MEM_CX_TYPE_4))
                ATIPrintMemoryType(ATIMemoryTypeNames_88800CX[ATIMemoryType]);
        }
        else if (ATIChip >= ATI_CHIP_68800)
        {
            if (xf86Verbose || (ATIMemoryType == MEM_MACH_TYPE_7))
                ATIPrintMemoryType(ATIMemoryTypeNames_Mach[ATIMemoryType]);
        }
    }
    else if (ATIAdapter >= ATI_ADAPTER_V3)
        if (xf86Verbose)
            ATIPrintMemoryType((ATIGetExtReg(0xB7U) & 0x04U) ? "DRAM" : "VRAM");

    /* Initialize for ATISwap */
    IO_Value = GetReg(SEQX, 0x04U) & 0x08U;
    ATICurrentPlanes = 1;
    if (!IO_Value)                      /* Adjust for planar modes */
        ATICurrentPlanes = 4;
    WindowSize = ATI.ChipSegmentSize * ATICurrentPlanes;
    ATICurrentBanks = ATIMaximumBanks =
        ATIDivide(vga256InfoRec.videoRam, WindowSize, 10, 1);

    if (!ATIUsingSmallApertures)
    {
        ATISelectBankFunction = ATI.ChipSetReadWrite;
        if (ATIVGAAdapter == ATI_ADAPTER_NONE)
            ATICurrentBanks = 1;
    }
    else if (!(inb(ATIIOPortCONFIG_CNTL) & CFG_MEM_VGA_AP_EN))
    {
        ATICurrentBanks = 1;
        ATISelectBankFunction = (BankFunction *)NoopDDA;
    }
    else if (IO_Value)
        ATISelectBankFunction = ATIMach64SetReadWritePacked;
    else
        ATISelectBankFunction = ATIMach64SetReadWritePlanar;

    /*
     * Set the maximum allowable dot-clock frequency (in kHz).  This is
     * dependent on what the RAMDAC can handle (in non-pixmux mode, for now).
     * For an internal DAC, assume it can handle whatever frequency the
     * internal PLL can produce (with the reference divider set by BIOS
     * initialization), but default maxClock to a lower chip-specific default.
     */
    if ((ATIDac & ~0x0FU) == ATI_DAC_INTERNAL)
    {
        if (vga256InfoRec.dacSpeeds[0] < vga256InfoRec.maxClock)
        {
            int DefaultmaxClock = 135000;

            if ((ATIChip >= ATI_CHIP_264VTB) && (ATIChip != ATI_CHIP_Mach64))
            {
                if (ATIChip >= ATI_CHIP_264VT4)
                    DefaultmaxClock = 230000;
                else if (ATIChip >= ATI_CHIP_264VT3)
                    DefaultmaxClock = 200000;
                else
                    DefaultmaxClock = 170000;
            }
            if (vga256InfoRec.dacSpeeds[0] > DefaultmaxClock)
                vga256InfoRec.maxClock = vga256InfoRec.dacSpeeds[0];
            else if (DefaultmaxClock < vga256InfoRec.maxClock)
                vga256InfoRec.maxClock = DefaultmaxClock;
        }
    }
    else switch (ATIDac)
    {
        case ATI_DAC_STG1700:
        case ATI_DAC_STG1702:
        case ATI_DAC_STG1703:
            vga256InfoRec.maxClock = 110000;
            break;

        default:
            vga256InfoRec.maxClock = 80000;
            break;
    }

    /* Determine available dot clock frequencies */
    ATIClockProbe();

    /*
     * If user did not specify any modes, attempt to create a default mode.
     * Its timings will be taken from the mode in effect on driver entry.
     */
    if ((vga256InfoRec.modes == NULL) && (ATIVGAAdapter != ATI_ADAPTER_NONE))
    {
        const char *Message = NULL;
        int MaxScreen;

#       ifndef BANKEDMONOVGA
            if (ATIUsing1bppModes)
                MaxScreen = (ATI.ChipSegmentSize << 3);
            else
#       endif
            if (ATIUsingPlanarModes)
                MaxScreen = vga256InfoRec.videoRam << 11;
            else
                MaxScreen = (vga256InfoRec.videoRam << 13) /
                    vga256InfoRec.bitsPerPixel;

        /* Get current timings */
        ATIGetMode(&DefaultMode);

        /* Check if generated mode can be used */
        if (!DefaultMode.SynthClock)
            Message = "required dot clock cannot be determined";
        else if ((DefaultMode.SynthClock / 1000) >
                 (((vga256InfoRec.maxClock / 1000) * ATI.ChipClockDivFactor) /
                     ATI.ChipClockMulFactor))
            Message = "required dot clock greater than maxClock";
        else if ((DefaultMode.HDisplay * DefaultMode.VDisplay) > MaxScreen)
            Message = "insufficient video memory";

        if (Message)
            ErrorF("Default %dx%d mode not used:  %s.\n", DefaultMode.HDisplay,
                DefaultMode.VDisplay, Message);
        else
        {
            DefaultMode.prev = DefaultMode.next = ATI.ChipBuiltinModes =
                &DefaultMode;
            DefaultMode.name = "Default mode";
            ErrorF("The following default video mode will be used:\n");
            ATIPrintMode(&DefaultMode);
        }
    }

    /* Set chipset name */
    vga256InfoRec.chipset = ATIChipSetNames[ATIChipSet];

#ifndef MONOVGA
    /* Call ATIScreenInit() to finalize screen initialization */
    vgaSetScreenInitHook(ATIScreenInit);
#endif

    /* Indicate supported options ... */
    if (ATIAdapter > ATI_ADAPTER_VGA)
    {
        OFLG_SET(OPTION_CSYNC,  &ATI.ChipOptionFlags);
        if (ATIChip >= ATI_CHIP_88800GXC)
            OFLG_SET(OPTION_NOLINEAR_MODE, &ATI.ChipOptionFlags);
    }
    OFLG_SET(OPTION_PROBE_CLKS, &ATI.ChipOptionFlags);
    OFLG_SET(OPTION_FB_DEBUG,   &ATI.ChipOptionFlags);     /* For testing */

    /* ... and unsupported ones */
    if (vga256InfoRec.clockprog)
    {
        ErrorF("XF86Config ClockProg specification ignored.\n");
        vga256InfoRec.clockprog = NULL;
    }

    /*
     * Our caller doesn't necessarily get back to us.  So, remove its
     * privileges until it does.
     */
    ATIEnterLeave(LEAVE);

    if (xf86Verbose > 1)
    {
        /* Spill the beans... */
        if (Signature == No_Signature)
            ErrorF("\nNo BIOS signature found.\n");
        else if (Signature != BIOS_Signature)
            ErrorF("\nBIOS signature found at offset 0x%04X.\n", Signature);

        if (ATIChipHasVGAWonder)
            ErrorF("\nThe ATI extended VGA registers are being accessed at I/O"
                   " port 0x%04X.\n", ATIIOPortVGAWonder);

        if ((ATIChip < ATI_CHIP_88800GXC) && (Signature == BIOS_Signature))
        {
            ErrorF("\n   Signature code:                \"%c%c\"",
               BIOS[0x40U], BIOS[0x41U]);
            ErrorF("\n   BIOS version:                  %d.%d\n",
               BIOS[0x4CU], BIOS[0x4DU]);

            ErrorF("\n   Byte at offset 0x42 =          0x%02X\n",
               BIOS[0x42U]);
            ErrorF("   8 and 16 bit ROM supported:    %s\n",
               BIOS[0x42U] & 0x01U ? "Yes" : "No");
            ErrorF("   Mouse chip present:            %s\n",
               BIOS[0x42U] & 0x02U ? "Yes" : "No");
            ErrorF("   Inport compatible mouse port:  %s\n",
               BIOS[0x42U] & 0x04U ? "Yes" : "No");
            ErrorF("   Micro Channel supported:       %s\n",
               BIOS[0x42U] & 0x08U ? "Yes" : "No");
            ErrorF("   Clock chip present:            %s\n",
               BIOS[0x42U] & 0x10U ? "Yes" : "No");
            ErrorF("   Uses C000:0000 to D000:FFFF:   %s\n",
               BIOS[0x42U] & 0x80U ? "Yes" : "No");

            ErrorF("\n   Byte at offset 0x44 =          0x%02X\n",
               BIOS[0x44U]);
            ErrorF("   Supports 70Hz non-interlaced:  %s\n",
               BIOS[0x44U] & 0x01U ? "No" : "Yes");
            ErrorF("   Supports Korean characters:    %s\n",
               BIOS[0x44U] & 0x02U ? "Yes" : "No");
            ErrorF("   Uses 45Mhz memory clock:       %s\n",
               BIOS[0x44U] & 0x04U ? "Yes" : "No");
            ErrorF("   Supports zero wait states:     %s\n",
               BIOS[0x44U] & 0x08U ? "Yes" : "No");
            ErrorF("   Uses paged ROMs:               %s\n",
               BIOS[0x44U] & 0x10U ? "Yes" : "No");
            ErrorF("   8514/A hardware on adapter:    %s\n",
               BIOS[0x44U] & 0x40U ? "No" : "Yes");
            ErrorF("   32K colour DAC on adapter:     %s\n",
               BIOS[0x44U] & 0x80U ? "Yes" : "No");
        }

        ATIPrintBIOS(BIOS, 0, Prefix_Size);

        if (ROMTable > 0)
            ATIPrintBIOS(BIOS, ROMTable, ROMTable + 0x16U);
        if (ClockTable > 0)
            ATIPrintBIOS(BIOS, ClockTable - 0x06U, ClockTable + 0x1EU);
        if (LCDTable > 0)
            ATIPrintBIOS(BIOS, LCDTable, LCDTable + 0x1AU);
        if (LCDPanelInfo > 0)
            ATIPrintBIOS(BIOS, LCDPanelInfo - 0x60U, LCDPanelInfo + 0x40U);

        if (xf86Verbose > 2)
        {
            ErrorF("\n On server entry:\n");

            xf86EnableIOPorts(vga256InfoRec.scrnIndex);
            ATIPrintRegisters();
            xf86DisableIOPorts(vga256InfoRec.scrnIndex);
        }
    }

#   ifdef XFreeXDGA
        if (!ATIUsing1bppModes)
            vga256InfoRec.directMode = XF86DGADirectPresent;
#   endif

    return TRUE;
}
