/* $XConsortium: ati_driver.c /main/9 1996/01/12 12:16:31 kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/ati_driver.c,v 3.38 1996/10/16 14:42:27 dawes Exp $ */
/*
 * Copyright 1994 through 1996 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

/*************************************************************************/

/*
 * Author:  Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * This is the ATI driver for XFree86.
 *
 * Someone once said "No man is an island", and I am most certainly no
 * exception.  Contributions to this and previous versions of this driver by
 * the following are hereby acknowledged:
 *
 * Thomas Roell, roell@informatik.tu-muenchen.de
 * Per Lindqvist, pgd@compuram.bbt.se
 * Doug Evans, dje@cygnus.com
 * Rik Faith, faith@cs.unc.edu
 * Arthur Tateishi, ruhtra@turing.toronto.edu
 * Alain Hebert, aal@broue.rot.qc.ca
 * Ton van Rosmalen, ton@stack.urc.tue.nl
 * David Chambers, davidc@netcom.com
 * William Shubert, wms@ssd.intel.com
 * ATI Technologies Incorporated
 * Robert Wolff
 * David Dawes, dawes@xfree86.org
 * Mark Weaver, Mark_Weaver@brown.edu
 * Hans Nasten, nasten@everyware.se
 * Kevin Martin, martin@cs.unc.edu
 * Frederic Rienthaler, root@mojo.synapse.com
 * Marc Bolduc, bolduc@cim.mcgill.ca
 * Reuben Sumner, rasumner@undergrad.math.uwaterloo.ca
 * Benjamin T. Yang, risk@uclink.berkeley.edu
 * James Fast Kane, jfk2@engr.uark.edu
 * Randall Hopper, rhh@ct.picker.com
 *
 * ... and, many, many others from around the world.
 *
 * In addition, this work would not have been possible without the active
 * support, both moral and otherwise, of the staff and management of Computing
 * and Network Services at the University of Alberta, in Edmonton, Alberta,
 * Canada.
 *
 * The driver is intended to support ATI VGA Wonder series of adapters and its
 * OEM counterpart, the VGA1024 series.  It will also work with Mach32's and
 * Mach64's but will not use their accelerated features.  This includes
 * Mach64's based on the 264xT series of integrated controllers.
 *
 * The ATI x8800 chips use special registers for their extended VGA features.
 * These registers are accessible through an index I/O port and a data I/O
 * port.  BIOS initialization stores the index port number in the Graphics
 * register bank (0x03CE), indices 0x50 and 0x51.  Unfortunately, these
 * registers are write-only (a.k.a. black holes).  On all but Mach64', the
 * index port number can be found in the short integer at offset 0x10 in the
 * BIOS.  For Mach64's, this driver will use 0x01CE as the index port number.
 * The data port number is one more than the index port number (i.e. 0x01CF).
 * These ports differ slightly in their I/O behaviour from the normal VGA ones:
 *
 *    write:  outw(0x01CE, (data << 8) | index);
 *    read:   outb(0x01CE, index);  data = inb(0x01CF);
 *
 * Two consecutive byte-writes will not work.  Furthermore an index written to
 * 0x01CE is usable only once.  Note also that the setting of ATI extended
 * registers (especially those with clock selection bits) should be bracketed
 * by a sequencer reset.
 *
 * The number of these extended VGA registers varies by chipset.  The 18800
 * series have 16, the 28800 series have 32, while Mach32's and Mach64's have
 * 64.  The last 16 on each have almost identical definitions.  Thus, the BIOS
 * (and this driver) sets up an indexing scheme whereby the last 16 extended
 * VGA registers are accessed at indices 0xB0 through 0xBF on all chipsets.
 *
 * Note that Mach64's based on the 264xT integrated controllers do not
 * implement the ATI extended VGA registers described above.
 *
 * Adapters prior to V5 use 4 crystals.  Adapters V5 and later use a clock
 * generator chip.  V3 and V4 adapters differ when it comes to choosing clock
 * frequencies.
 *
 * VGA Wonder V3/V4 Adapter Clock Frequencies
 * R E G I S T E R S
 * 1CE(*)    3C2     3C2    Frequency
 * B2h/BEh
 * Bit 6/4  Bit 3   Bit 2   (MHz)
 * ------- ------- -------  -------
 *    0       0       0     50.000
 *    0       0       1     56.644
 *    0       1       0     Spare 1
 *    0       1       1     44.900
 *    1       0       0     44.900
 *    1       0       1     50.000
 *    1       1       0     Spare 2
 *    1       1       1     36.000
 *
 * (*):  V3 uses index B2h, bit 6;  V4 uses index BEh, bit 4
 *
 * V5, PLUS, XL and XL24 usually have an ATI 18810 clock generator chip, but
 * some have an ATI 18811-0, and it's quite conceivable that some exist with
 * ATI 18811-1's or ATI 18811-2's.  Mach32 adapters are known to use any one of
 * these clock generators.  Mach32 adapters also use a different dot clock
 * ordering.  ATI says there is no reliable way for the driver to determine
 * which clock generator is on the adapter, but this driver will do its best to
 * do so anyway.
 *
 * VGA Wonder V5/PLUS/XL/XL24 Clock Frequencies
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)   18811-0  18811-1
 *  Bit 1   Bit 4   Bit 3   Bit 2    18810   18812-0  18811-2
 * ------- ------- ------- -------  -------  -------  -------
 *    0       0       0       0      30.240   30.240  135.000
 *    0       0       0       1      32.000   32.000   32.000
 *    0       0       1       0      37.500  110.000  110.000
 *    0       0       1       1      39.000   80.000   80.000
 *    0       1       0       0      42.954   42.954  100.000
 *    0       1       0       1      48.771   48.771  126.000
 *    0       1       1       0        (*1)   92.400   92.400
 *    0       1       1       1      36.000   36.000   36.000
 *    1       0       0       0      40.000   39.910   39.910
 *    1       0       0       1      56.644   44.900   44.900
 *    1       0       1       0      75.000   75.000   75.000
 *    1       0       1       1      65.000   65.000   65.000
 *    1       1       0       0      50.350   50.350   50.350
 *    1       1       0       1      56.640   56.640   56.640
 *    1       1       1       0        (*2)     (*3)     (*3)
 *    1       1       1       1      44.900   44.900   44.900
 *
 * (*1) External 0 (supposedly 16.657 Mhz)
 * (*2) External 1 (supposedly 28.322 MHz)
 * (*3) This setting doesn't seem to generate anything
 *
 * Mach32 Clock Frequencies
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)   18811-0  18811-1
 *  Bit 1   Bit 4   Bit 3   Bit 2    18810   18812-0  18811-2
 * ------- ------- ------- -------  -------  -------  -------
 *    0       0       0       0      42.954   42.954  100.000
 *    0       0       0       1      48.771   48.771  126.000
 *    0       0       1       0        (*1)   92.400   92.400
 *    0       0       1       1      36.000   36.000   36.000
 *    0       1       0       0      30.240   30.240  135.000
 *    0       1       0       1      32.000   32.000   32.000
 *    0       1       1       0      37.500  110.000  110.000
 *    0       1       1       1      39.000   80.000   80.000
 *    1       0       0       0      50.350   50.350   50.350
 *    1       0       0       1      56.640   56.640   56.640
 *    1       0       1       0        (*2)     (*3)     (*3)
 *    1       0       1       1      44.900   44.900   44.900
 *    1       1       0       0      40.000   39.910   39.910
 *    1       1       0       1      56.644   44.900   44.900
 *    1       1       1       0      75.000   75.000   75.000
 *    1       1       1       1      65.000   65.000   65.000
 *
 * (*1) External 0 (supposedly 16.657 Mhz)
 * (*2) External 1 (supposedly 28.322 MHz)
 * (*3) This setting doesn't seem to generate anything
 *
 * Note that, to reduce confusion, this driver masks out the different clock
 * ordering.
 *
 * For all adapters, these frequencies can be divided by 1 or 2.  For all
 * adapters, except Mach32's and Mach64's, frequencies can also be divided by 3
 * or 4.
 *
 *      Register 1CE, index B8h
 *       Bit 7    Bit 6
 *      -------  -------
 *         0        0           Divide by 1
 *         0        1           Divide by 2
 *         1        0           Divide by 3
 *         1        1           Divide by 4
 *
 * With respect to clocks, Mach64's are entirely different animals.
 *
 * The oldest Mach64's use one of the non-programmable clock generators
 * described above.  In this case, the driver will handle clocks in much the
 * same way as it would for a Mach32.
 *
 * All other Mach64 adapters use a programmable clock generator.  BIOS
 * initialization programmes an initial set of frequencies.  Two of these are
 * reserved to allow for the setting of modes that do not use a frequency from
 * this initial set.  One of these reserved slots is used by the BIOS mode set
 * routine, the other by the particular accelerated driver used (MS-Windows,
 * AutoCAD, etc.).  The slots reserved in this way are dependent on the
 * particular clock generator used by the adapter.
 *
 * If the driver does not support the adapter's clock generator, it will try to
 * match the (probed or specified) clocks to one of the following sets.
 *
 * Mach64 Clock Frequencies for unsupported programmable clock generators
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)
 *  Bit 1   Bit 4   Bit 3   Bit 2    Set 1    Set 2    Set 3
 * ------- ------- ------- -------  -------  -------  -------
 *    0       0       0       0      50.350   25.180   25.180
 *    0       0       0       1      56.640   28.320   28.320
 *    0       0       1       0      63.000   31.500    0.000
 *    0       0       1       1      72.000   36.000    0.000
 *    0       1       0       0       0.000    0.000    0.000
 *    0       1       0       1     110.000  110.000    0.000
 *    0       1       1       0     126.000  126.000    0.000
 *    0       1       1       1     135.000  135.000    0.000
 *    1       0       0       0      40.000   40.000    0.000
 *    1       0       0       1      44.900   44.900    0.000
 *    1       0       1       0      49.500   49.500    0.000
 *    1       0       1       1      50.000   50.000    0.000
 *    1       1       0       0       0.000    0.000    0.000
 *    1       1       0       1      80.000   80.000    0.000
 *    1       1       1       0      75.000   75.000    0.000
 *    1       1       1       1      65.000   65.000    0.000
 *
 * The driver will never select a setting of 0.000 MHz.  The above comments on
 * clock ordering and clock divider apply here also.
 *
 * For all supported programmable clock generators, the driver will ignore any
 * XF86Config clock line and programme, as needed, the clock number reserved by
 * the BIOS for accelerated drivers.  The driver's mode initialization routine
 * finds integers N, M and D such that
 *
 *      157.5 * N    -D
 *      --------- * 2    MHz
 *        11 * M
 *
 * best approximates the mode's clock frequency.  Note that (157.5/11) MHz is
 * 14.31818... MHz, which is the crystal-generated reference frequency used by
 * all clock generators currently known to exist on Mach64 adapters.  Different
 * clock generators have different restrictions on the value N, M and D can
 * assume.  The driver contains an internal table to record these restrictions
 * (among other things).  The resulting values of N, M and D are then encoded
 * in a generator-specific way and used to programme the clock.  The Mach64's
 * clock divider is not used in this case.
 */

/*************************************************************************/

/*
 * These are XFree86-specific header files.
 */
#include "xf86Version.h"
#include "xf86Procs.h"
#include "compiler.h"
#include "xf86_HWlib.h"
#include "xf86_Config.h"
#include "vga.h"
#include "vgaPCI.h"
#include "regati.h"

#if defined(XFreeXDGA) && !defined(MONOVGA)
#       define _XF86DGA_SERVER_
#       include "extensions/xf86dga.h"
#endif

/* *** Temporary *** */
#ifndef FunctionPrototype
#if NeedFunctionPrototypes
#define FunctionPrototype(FunctionName, FunctionArgumentTypes) \
        FunctionName FunctionArgumentTypes
#else
#define FunctionPrototype(FunctionName, FunctionArgumentTypes) \
        FunctionName ()
#endif
#endif
/* *** Temporary *** */

/*
 * Driver data structures.
 */
typedef struct
{
        vgaHWRec std;               /* IBM VGA */

        unsigned char             a3,         a6, a7,
                                  ab, ac, ad, ae,
                      b0, b1, b2, b3,     b5, b6,
                      b8, b9, ba,         bd, be, bf;

        unsigned char crtc_gen_cntl0, crtc_gen_cntl3;
        unsigned short int second_aperture_bank;
        unsigned long int crtc_off_pitch;

        DisplayModePtr mode;        /* The corresponding mode line */

        int ClockProgramme;         /* Clock programme word */
} vgaATIRec, *vgaATIPtr;

/*
 * Forward definitions for the functions that make up the driver.  See the
 * definitions of these functions for the real scoop.
 */
static Bool     FunctionPrototype(ATIProbe, (void));
static char *   FunctionPrototype(ATIIdent, (const unsigned int));
static void     FunctionPrototype(ATIEnterLeave, (const Bool));
static Bool     FunctionPrototype(ATIInit, (DisplayModePtr));
static int      FunctionPrototype(ATIValidMode, (DisplayModePtr, Bool));
static void *   FunctionPrototype(ATISave, (vgaATIPtr));
static void     FunctionPrototype(ATIRestore, (vgaATIPtr));
static void     FunctionPrototype(ATIAdjust,
                                  (const unsigned int, const unsigned int));
static void     FunctionPrototype(ATISaveScreen, (const Bool));
static void     FunctionPrototype(ATIGetMode, (DisplayModePtr));
/*
 * These are the bank select functions and are defined in bank.s.
 */
extern void     FunctionPrototype(ATISetRead, (const unsigned int));
extern void     FunctionPrototype(ATISetWrite, (const unsigned int));
extern void     FunctionPrototype(ATISetReadWrite, (const unsigned int));
/*
 * These are the bank selection functions for V3 adapters and are defined in
 * bank.s.
 */
extern void     FunctionPrototype(ATIV3SetRead, (const unsigned int));
extern void     FunctionPrototype(ATIV3SetWrite, (const unsigned int));
extern void     FunctionPrototype(ATIV3SetReadWrite, (const unsigned int));
/*
 * These are the bank selection functions for V4 and V5 adapters and are
 * defined in bank.s.
 */
extern void     FunctionPrototype(ATIV4V5SetRead, (const unsigned int));
extern void     FunctionPrototype(ATIV4V5SetWrite, (const unsigned int));
extern void     FunctionPrototype(ATIV4V5SetReadWrite, (const unsigned int));
/*
 * These are the bank selection functions for a Mach64's small dual paged
 * apertures, and are defined in bank.s.
 */
extern void     FunctionPrototype(ATIMach64SetRead, (const unsigned int));
extern void     FunctionPrototype(ATIMach64SetWrite, (const unsigned int));
extern void     FunctionPrototype(ATIMach64SetReadWrite, (const unsigned int));
/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data that
 * defines how the driver operates.
 */
vgaVideoChipRec ATI =
{
        ATIProbe,               /* Probe */
        ATIIdent,               /* Ident */
        ATIEnterLeave,          /* EnterLeave */
        ATIInit,                /* Init */
        ATIValidMode,           /* ValidMode */
        ATISave,                /* Save */
        ATIRestore,             /* Restore */
        ATIAdjust,              /* Adjust */
        ATISaveScreen,          /* SaveScreen */
        ATIGetMode,             /* GetMode */
        (void (*)())NoopDDA,    /* FbInit */
        ATISetRead,             /* SetRead */
        ATISetWrite,            /* SetWrite */
        ATISetReadWrite,        /* SetReadWrite */
        0x10000,                /* Mapped memory window size (64k) */
        0x10000,                /* Video memory bank size (64k) */
        16,                     /* Shift factor to get bank number */
        0xFFFF,                 /* Bit mask for address within a bank */
        0x00000, 0x10000,       /* Boundaries for reads within a bank */
        0x00000, 0x10000,       /* Boundaries for writes within a bank */
        TRUE,                   /* Read & write banks can be different */
        VGA_DIVIDE_VERT,        /* Divide interlaced vertical timings */
        {{0,}},                 /* Options are set by ATIProbe */
        16,                     /* Virtual X rounding */
        FALSE,                  /* No linear frame buffer */
        0,                      /* Linear frame buffer base address */
        0,                      /* Linear frame buffer size */
        FALSE,                  /* No support for 16 bits per pixel (yet) */
        FALSE,                  /* No support for 24 bits per pixel (yet) */
        FALSE,                  /* No support for 32 bits per pixel (yet) */
        NULL,                   /* List of builtin modes */
        1,                      /* Clock scaling factor */
};

/*
 * This is a convenience macro, so that entries in the driver structure can
 * simply be dereferenced with 'new->xxx'.
 */
#define new ((vgaATIPtr)vgaNewVideoState)

/*
 * This structure is used by ATIProbe in an attempt to define a default video
 * mode when the user has not specified any modes in XF86Config.
 */
static DisplayModeRec DefaultMode;

/*
 * Define a table to map mode flag values to XF86Config tokens.
 */
typedef struct
{
      int flag, token;
} TokenTabRec, *TokenTabPtr;

static TokenTabRec TokenTab[] =
{
      {V_PHSYNC,    TT_PHSYNC},
      {V_NHSYNC,    TT_NHSYNC},
      {V_PVSYNC,    TT_PVSYNC},
      {V_NVSYNC,    TT_NVSYNC},
      {V_PCSYNC,    TT_PCSYNC},
      {V_NCSYNC,    TT_NCSYNC},
      {V_INTERLACE, TT_INTERLACE},
      {V_DBLSCAN,   TT_DBLSCAN},
      {V_CSYNC,     TT_CSYNC},
      {0,           0}
};

/*
 * Macros for port definitions.
 */
#define IOByte(Port)    (Port)
#define IOWord(Port)    (Port), (Port)+1
#define IOLong(Port)    (Port), (Port)+1, (Port)+2, (Port)+3

/*
 * This driver needs non-standard I/O ports.  The first two are determined by
 * ATIProbe and are initialized to their most probable values here.
 */
static unsigned ATI_IOPorts[] =
{
        /* ATI VGA Wonder extended registers */
        IOWord(0x01CEU),

        /* 8514/A register base */
        IOWord(0x02E8U),

        /* Standard ATI accelerator (Mach8/32/64) register bases */
        IOLong(0x02ECU),

        /* Alternate Mach64 register bases (sparse I/O) */
        IOLong(0x01C8U), IOLong(0x01CCU),

        /* This is sufficient to allow access to all ports above 0x03FF */
        IOByte(0x0400U),

        /* System timer registers */
        IOByte(0x40U), IOByte(0x43U)
};
#define Num_ATI_IOPorts (sizeof(ATI_IOPorts) / sizeof(ATI_IOPorts[0]))

#undef IOByte
#undef IOWord
#undef IOLong

/*
 * The following are port numbers used by bank.s.  They must be short int's.
 */
unsigned short int ATIVGAPort      = 0x01CEU;
unsigned short int ATIMach64RPPort = 0;  /* Set by ATIProbe */
unsigned short int ATIMach64WPPort = 0;  /* Set by ATIProbe */

/* Other Mach64 port numbers.  These are set by ATIProbe */
static unsigned short int
        CRTC_OFF_PITCH_IOPort,
        CRTC_GEN_CNTL_IOPort,
        CLOCK_CNTL_IOPort,
        BUS_CNTL_IOPort,
        MEM_INFO_IOPort,
        DAC_REGS_IOPort,
        DAC_CNTL_IOPort,
        GEN_TEST_CNTL_IOPort,
        CONFIG_CNTL_IOPort;

/* I/O decoding definitions */
#define SPARSE_IO       0
#define BLOCK_IO        1
static unsigned short int ATIIOBase;
static unsigned char ATIIODecoding;
#define ATIIOPort(PortTag)                                                   \
        (((ATIIODecoding == SPARSE_IO) ?                                     \
          (((PortTag) & SPARSE_IO_SELECT) | ((PortTag) & IO_BYTE_SELECT)) :  \
          (((PortTag) & BLOCK_IO_SELECT)  | ((PortTag) & IO_BYTE_SELECT))) | \
         ATIIOBase)

extern unsigned char ATIB2Reg;          /* The B2 mirror in bank.s */
static unsigned char ATIVGAOffset = 0x80U;    /* Index offset for ATIVGAPort */

/*
 * Odds and ends to ease reading and writting of registers.
 */
#define GetReg(Register, Index)                                 \
        (                                                       \
                outb(Register, Index),                          \
                inb(Register + 1)                               \
        )
#define PutReg(Register, Index, Value)                          \
        outw(Register, ((Value) << 8) | (Index))
#define ATIGetExtReg(Index)                                     \
        GetReg(ATIVGAPort, Index)
#define ATIPutExtReg(Index, Value)                              \
        PutReg(ATIVGAPort, Index, Value)

static void
ATIAccessMach64PLLReg(const unsigned char Index, const Bool Write)
{
        unsigned char clock_cntl1;
        clock_cntl1 = inb(CLOCK_CNTL_IOPort + 1) &
                ~GetBits(PLL_WR_EN | PLL_ADDR, 0xFFU << (1 * 8));
        /* Set PLL register to be read or written */
        outb(CLOCK_CNTL_IOPort + 1, clock_cntl1 |
                GetBits(SetBits(Index, PLL_ADDR) | SetBits(Write, PLL_WR_EN),
                        0xFFU << (1 * 8)));
}

#define ATIGetMach64PLLReg(Index)                               \
        (                                                       \
                ATIAccessMach64PLLReg(Index, FALSE),            \
                inb(CLOCK_CNTL_IOPort + 2)                      \
        )
#define ATIPutMach64PLLReg(Index, Value)                        \
        (                                                       \
                ATIAccessMach64PLLReg(Index, TRUE),             \
                outb(CLOCK_CNTL_IOPort + 2, Value)              \
        )

/* Odds and ends */
static unsigned char Chip_Has_SUBSYS_CNTL  = FALSE;
static unsigned char Chip_Has_VGA_Wonder   = FALSE;
static unsigned char Using_Small_Apertures = FALSE;
static unsigned short int ChipType = 0, ChipClass = 0, ChipRevision = 0;

/*
 * Chip-related definitions.
 */
#define ATI_CHIP_NONE      0
#define ATI_CHIP_18800     1
#define ATI_CHIP_18800_1   2
#define ATI_CHIP_28800_2   3
#define ATI_CHIP_28800_4   4
#define ATI_CHIP_28800_5   5
#define ATI_CHIP_28800_6   6
#define ATI_CHIP_8514A     7    /* 8514/A */
#define ATI_CHIP_CT480     8    /* 8514/A clone */
#define ATI_CHIP_38800_1   9    /* Mach8 */
#define ATI_CHIP_68800    10    /* Mach32 */
#define ATI_CHIP_68800_3  11    /* Mach32 */
#define ATI_CHIP_68800_6  12    /* Mach32 */
#define ATI_CHIP_68800LX  13    /* Mach32 */
#define ATI_CHIP_68800AX  14    /* Mach32 */
#define ATI_CHIP_88800GXC 15    /* Mach64 */
#define ATI_CHIP_88800GXD 16    /* Mach64 */
#define ATI_CHIP_88800GXE 17    /* Mach64 */
#define ATI_CHIP_88800GXF 18    /* Mach64 */
#define ATI_CHIP_88800GX  19    /* Mach64 */
#define ATI_CHIP_88800CX  20    /* Mach64 */
#define ATI_CHIP_264CT    21    /* Mach64 */
#define ATI_CHIP_264ET    22    /* Mach64 */
#define ATI_CHIP_264VT    23    /* Mach64 */
#define ATI_CHIP_264GT    24    /* Mach64 */
#define ATI_CHIP_88800    25    /* Mach64 */
static unsigned char ATIChip = ATI_CHIP_NONE;
static const char *ChipNames[] =
{
        "Unknown",
        "ATI 18800",
        "ATI 18800-1",
        "ATI 28800-2",
        "ATI 28800-4",
        "ATI 28800-5",
        "ATI 28800-6",
        "IBM 8514/A",
        "Chips & Technologies 82C480",
        "ATI 38800-1",
        "ATI 68800",
        "ATI 68800-3",
        "ATI 68800-6",
        "ATI 68800LX",
        "ATI 68800AX",
        "ATI 88800GX-C",
        "ATI 88800GX-D",
        "ATI 88800GX-E",
        "ATI 88800GX-F",
        "ATI 88800GX",
        "ATI 88800CX",
        "ATI 264CT",
        "ATI 264ET",
        "ATI 264VT",
        "ATI 264GT",
        "ATI unknown Mach64",
};

/*
 * Adapter-related definitions.
 */
#define ATI_ADAPTER_NONE        0
#define ATI_ADAPTER_EGA         1
#define ATI_ADAPTER_EGA_PLUS    2
#define ATI_ADAPTER_BASIC       3
#define ATI_ADAPTER_V3          4
#define ATI_ADAPTER_V4          5
#define ATI_ADAPTER_V5          6
#define ATI_ADAPTER_PLUS        7
#define ATI_ADAPTER_XL          8
#define ATI_ADAPTER_NONISA      9
#define ATI_ADAPTER_8514A      10
#define ATI_ADAPTER_MACH8      11
#define ATI_ADAPTER_MACH32     12
#define ATI_ADAPTER_MACH64     13
static unsigned char ATIAdapter = ATI_ADAPTER_NONE;
static unsigned char ATIVGAAdapter = ATI_ADAPTER_NONE;
static const char *AdapterNames[] =
{
        "Unknown",
        "ATI EGA Wonder800",
        "ATI EGA Wonder800+",
        "ATI VGA Basic16",
        "ATI VGA Wonder V3",
        "ATI VGA Wonder V4",
        "ATI VGA Wonder V5",
        "ATI VGA Wonder+",
        "ATI VGA Wonder XL or XL24",
        "ATI VGA Wonder VLB or PCI",
        "IBM 8514/A or compatible",
        "ATI Mach8",
        "ATI Mach32",
        "ATI Mach64"
};

/*
 * RAMDAC-related definitions.
 */
#define ATI_DAC(Type, Subtype)  (((Type) << 4) | (Subtype))
#define ATI_DAC_ATI68830        ATI_DAC(0x0U, 0x0U)
#define ATI_DAC_SC11483         ATI_DAC(0x1U, 0x0U)
#define ATI_DAC_ATI68875        ATI_DAC(0x2U, 0x0U)
#define ATI_DAC_TVP3026_A       ATI_DAC(0x2U, 0x7U)
#define ATI_DAC_GENERIC         ATI_DAC(0x3U, 0x0U)
#define ATI_DAC_BT481           ATI_DAC(0x4U, 0x0U)
#define ATI_DAC_ATT20C491       ATI_DAC(0x4U, 0x1U)
#define ATI_DAC_SC15026         ATI_DAC(0x4U, 0x2U)
#define ATI_DAC_MU9C1880        ATI_DAC(0x4U, 0x3U)
#define ATI_DAC_IMSG174         ATI_DAC(0x4U, 0x4U)
#define ATI_DAC_ATI68860_B      ATI_DAC(0x5U, 0x0U)
#define ATI_DAC_ATI68860_C      ATI_DAC(0x5U, 0x1U)
#define ATI_DAC_TVP3026_B       ATI_DAC(0x5U, 0x7U)
#define ATI_DAC_STG1700         ATI_DAC(0x6U, 0x0U)
#define ATI_DAC_ATT20C498       ATI_DAC(0x6U, 0x1U)
#define ATI_DAC_STG1702         ATI_DAC(0x7U, 0x0U)
#define ATI_DAC_SC15021         ATI_DAC(0x7U, 0x1U)
#define ATI_DAC_ATT21C498       ATI_DAC(0x7U, 0x2U)
#define ATI_DAC_STG1703         ATI_DAC(0x7U, 0x3U)
#define ATI_DAC_CH8398          ATI_DAC(0x7U, 0x4U)
#define ATI_DAC_ATT20C408       ATI_DAC(0x7U, 0x5U)
#define ATI_DAC_INTERNAL        ATI_DAC(0x8U, 0x0U)
#define ATI_DAC_IBMRGB514       ATI_DAC(0x9U, 0x0U)
#define ATI_DAC_UNKNOWN        (ATI_DAC((GetBits(DACTYPE, DACTYPE) << 2) + 3, \
                                        GetBits(BIOS_INIT_DAC_SUBTYPE,        \
                                                BIOS_INIT_DAC_SUBTYPE)))
static unsigned short int ATIDac = ATI_DAC_GENERIC;
typedef struct
{
        const int DACType;
        const char *DACName;
} DACRec;
static const DACRec DACDescriptors[] =
{       /* Keep this table in ascending DACType order */
        {ATI_DAC_ATI68830,      "ATI 68830 or similar"},
        {ATI_DAC_SC11483,       "Sierra 11483 or similar"},
        {ATI_DAC_ATI68875,      "ATI 68875 or similar"},
        {ATI_DAC_TVP3026_A,     "TI ViewPoint3026 or similar"},
        {ATI_DAC_GENERIC,       "Brooktree 476 or similar"},
        {ATI_DAC_BT481,         "Brooktree 481 or similar"},
        {ATI_DAC_ATT20C491,     "AT&T 20C491 or similar"},
        {ATI_DAC_SC15026,       "Sierra 15026 or similar"},
        {ATI_DAC_MU9C1880,      "Music 9C1880 or similar"},
        {ATI_DAC_IMSG174,       "Inmos G174 or similar"},
        {ATI_DAC_ATI68860_B,    "ATI 68860 (Revision B) or similar"},
        {ATI_DAC_ATI68860_C,    "ATI 68860 (Revision C) or similar"},
        {ATI_DAC_TVP3026_B,     "TI ViewPoint3026 or similar"},
        {ATI_DAC_STG1700,       "SGS-Thompson 1700 or similar"},
        {ATI_DAC_ATT20C498,     "AT&T 20C498 or similar"},
        {ATI_DAC_STG1702,       "SGS-Thompson 1702 or similar"},
        {ATI_DAC_SC15021,       "Sierra 15021 or similar"},
        {ATI_DAC_ATT21C498,     "AT&T 21C498 or similar"},
        {ATI_DAC_STG1703,       "SGS-Thompson 1703 or similar"},
        {ATI_DAC_CH8398,        "Chrontel 8398 or similar"},
        {ATI_DAC_ATT20C408,     "AT&T 20C408 or similar"},
        {ATI_DAC_INTERNAL,      "Internal"},
        {ATI_DAC_IBMRGB514,     "IBM RGB 514 or similar"},
        {ATI_DAC_UNKNOWN,       "Unknown"}      /* Must be last */
};

/*
 * Definitions related to non-programmable clock generators.
 */
#define ATI_CLOCK_NONE      0    /* Must be zero */
#define ATI_CLOCK_CRYSTALS  1    /* Must be one */
#define ATI_CLOCK_18810     2
#define ATI_CLOCK_18811_0   3
#define ATI_CLOCK_18811_1   4
#define ATI_CLOCK_MACH64A   5
#define ATI_CLOCK_MACH64B   6
#define ATI_CLOCK_MACH64C   7
static unsigned char ATIClock = ATI_CLOCK_NONE;
static const char *ClockNames[] =
{
        "unknown",
        "crystals",
        "ATI 18810 or similar",
        "ATI 18811-0 or similar",
        "ATI 18811-1 or similar",
        "Programmable (BIOS setting 1)",
        "Programmable (BIOS setting 2)",
        "Programmable (BIOS setting 3)"
};

/*
 * Definitions related to programmable clock generators.
 */
#define ATI_CLOCK_FIXED     0   /* Further described above */
#define ATI_CLOCK_ICS2595   1
#define ATI_CLOCK_STG1703   2
#define ATI_CLOCK_CH8398    3
#define ATI_CLOCK_INTERNAL  4
#define ATI_CLOCK_ATT20C408 5
#define ATI_CLOCK_IBMRGB514 6
static unsigned char ATIProgrammableClock = ATI_CLOCK_FIXED;
typedef struct
{
        unsigned short int MinN, MaxN;  /* Numerator and ... */
        unsigned int NMask;             /* ... where it goes and ... */
        unsigned short int NAdjust;     /* ... its adjustment and ... */
        unsigned short int N1, N2;      /* ... its restrictions */
        unsigned short int MinM, MaxM;  /* Denominator and ... */
        unsigned int MMask;             /* ... where it goes and ... */
        unsigned short int MAdjust;     /* ... its adjustment */
        short int MinD, MaxD;           /* Divider and ... */
        unsigned int DMask;             /* ... where it goes */
        unsigned int IMask;             /* Where clock index goes */
        unsigned int XMask;             /* XOR this to finish programme word */
        char *ClockName;
} ClockRec, *ClockPtr;
static ClockRec ClockDescriptors[] =
{
        {
                   0,   0, 0x000000,   0, 1, 1,
                   1,   1, 0x000000,   0,
                   0,   0, 0x000000,
                 0x000000, 0x000000, "Non-programmable"
        },
        {
                 257, 512, 0x01FE00, 257, 1, 1,
                  46,  46, 0x000000,   0,
                   0,   3, 0x0C0000,
                 0x0001F0, 0x3C0000, "ATI 18818 or ICS 2595 or similar"
        },
        {
                   2, 129, 0x00FF00,   2, 1, 1,
                   8,  14, 0x00001F,   2,
                   0,   7, 0x0000E0,
                 0x000000, 0x000000, "SGS-Thompson 1703 or similar"
        },
        {
                   8, 263, 0x00FF00,   8, 8, 9,
                   4,  12, 0x00003F,   2,
                   0,   3, 0x0000C0,
                 0x000000, 0x000000, "Chrontel 8398 or similar"
        },
        {
                   2, 255, 0x0000FF,   0, 1, 1,
                  45,  45, 0x000000,   0,
                  -1,   2, 0x000300,
                 0x000000, 0x000000, "Internal"
        },
        {
                   2, 257, 0x00FF00,   2, 1, 1,
                   2,  32, 0x00003F,   2,
                   0,   3, 0x0000C0,
                 0x000000, 0x000000, "AT&T 20C408 or similar"
        },
        {
                  65, 128, 0x003F00,  65, 1, 1,
                   2,  31, 0x00003F,   0,
                   0,   3, 0x00C000,
                 0x000000, 0x00C000, "IBM RGB 514 or similar"
        }
};
#define Number_Of_Programmable_Clock_Generators         \
        (sizeof(ClockDescriptors) / sizeof(ClockDescriptors[0]))
static int ATIClockNumberToProgramme = -1;
static ClockPtr ATIClockDescriptor = ClockDescriptors;
static unsigned short int ATIBIOSClocks[16] =
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*
 * The driver will attempt to match fixed clocks to one of the following
 * specifications.
 */
static const int
ATICrystalFrequencies[] =
{
         50000,  56644,      0,  44900,  44900,  50000,      0,  36000,
            -1
},
ATI18810Frequencies[] =
{
         30240,  32000,  37500,  39000,  42954,  48771,      0,  36000,
         40000,  56644,  75000,  65000,  50350,  56640,      0,  44900
},
ATI188110Frequencies[] =
{
         30240,  32000, 110000,  80000,  42954,  48771,  92400,  36000,
         39910,  44900,  75000,  65000,  50350,  56640,      0,  44900
},
ATI188111Frequencies[] =
{
        135000,  32000, 110000,  80000, 100000, 126000,  92400,  36000,
         39910,  44900,  75000,  65000,  50350,  56640,      0,  44900
},
ATIMach64AFrequencies[] =
{
             0, 110000, 126000, 135000,  50350,  56640,  63000,  72000,
             0,  80000,  75000,  65000,  40000,  44900,  49500,  50000
},
ATIMach64BFrequencies[] =
{
             0, 110000, 126000, 135000,  25180,  28320,  31500,  36000,
             0,  80000,  75000,  65000,  40000,  44900,  49500,  50000
},
ATIMach64CFrequencies[] =
{
             0,      0,      0,      0,  25180,  28320,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0
},
*ClockLine[] =
{
        NULL,
        ATICrystalFrequencies,
        ATI18810Frequencies,
        ATI188110Frequencies,
        ATI188111Frequencies,
        ATIMach64AFrequencies,
        ATIMach64BFrequencies,
        ATIMach64CFrequencies,
        NULL
};

/*
 * The driver will reject XF86Config clocks lines that start with, or are an
 * initial subset of, one of the following.
 */
static const int
ATIVGAClocks[] =
{
         25175,  28322,
            -1
},
ATIPre_2_1_1_Clocks_A[] =       /* Based on 18810 */
{
         18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
         30240,  32000,  37500,  39000,  40000,  56644,  75000,  65000,
            -1
},
ATIPre_2_1_1_Clocks_B[] =       /* Based on 18811-0 */
{
         18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
         30240,  32000, 110000,  80000,  39910,  44900,  75000,  65000,
            -1
},
ATIPre_2_1_1_Clocks_C[] =       /* Based on 18811-1 (or -2) */
{
         18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
        135000,  32000, 110000,  80000,  39910,  44900,  75000,  65000,
            -1
},
ATIPre_2_1_1_Clocks_D[] =       /* Based on programmable setting 1 */
{
         36000,  25000,  20000,  22450,  72000,  50000,  40000,  44900,
             0, 110000, 126000, 135000,      0,  80000,  75000,  65000,
            -1
},
ATIPre_2_1_1_Clocks_E[] =       /* Based on programmable setting 2 */
{
         18000,  25000,  20000,  22450,  36000,  50000,  40000,  44900,
             0, 110000, 126000, 135000,      0,  80000,  75000,  65000,
            -1
},
*InvalidClockLine[] =
{
        NULL,
        ATIVGAClocks,
        ATIPre_2_1_1_Clocks_A,
        ATIPre_2_1_1_Clocks_B,
        ATIPre_2_1_1_Clocks_C,
        ATIPre_2_1_1_Clocks_D,
        ATIPre_2_1_1_Clocks_E,
        NULL
};

/*
 * Clock maps.
 */
static const unsigned char Clock_Maps[][16] =
{
        /* Null map */
        {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15},
        /* Mach{8,32,64} <-> VGA Wonder map */
        {  4,  5,  6,  7,  0,  1,  2,  3, 12, 13, 14, 15,  8,  9, 10, 11},
        /* Accelerator <-> VGA Wonder map */
        {  0,  1,  2,  3,  8,  9, 10, 11,  4,  5,  6,  7, 12, 13, 14, 15},
        /* Accelerator -> VGA map */
        {  4,  5,  6,  7, 12, 13, 14, 15,  0,  1,  2,  3,  8,  9, 10, 11},
        /* VGA -> Accelerator map */
        {  8,  9, 10, 11,  0,  1,  2,  3, 12, 13, 14, 15,  4,  5,  6,  7}
};
#define Number_Of_Clock_Maps (sizeof(Clock_Maps) / sizeof(Clock_Maps[0]))
#define ATIVGAWonderClockMap    Clock_Maps[0]
#define ATIMachClockMap         Clock_Maps[1]
#define ATIProgrammableClockMap Clock_Maps[2]
static const unsigned char *ATIClockMap = ATIVGAWonderClockMap;

/*
 * ATIIdent --
 *
 * Returns a string name for this driver or NULL.
 */
static char *
ATIIdent(const unsigned int n)
{
        static const char *chipsets[] = {"ati"};

        if (n >= (sizeof(chipsets) / sizeof(char *)))
                return (NULL);
        else
                return ((char *)chipsets[n]);
}

/*
 * ATIPrintBIOS --
 *
 * Display part of the video BIOS when the server is invoked with -verbose.
 */
static void
ATIPrintBIOS(const unsigned char *BIOS,
             const unsigned int Start,
             const unsigned int End)
{
        unsigned int Index = Start & ~(16 - 1);

        ErrorF("\n BIOS data at 0x%08X:", Start + vga256InfoRec.BIOSbase);
        for (;  Index < End;  Index++)
        {
                if ((Index % 4) == 0)
                {
                        if ((Index % 16) == 0)
                                ErrorF("\n 0x%08X:",
                                        Index + vga256InfoRec.BIOSbase);
                        ErrorF(" ");
                }
                if (Index < Start)
                        ErrorF("  ");
                else
                        ErrorF("%02X", BIOS[Index]);
        }
        ErrorF("\n");
}

/*
 * ATIPrintIndexedRegisters --
 *
 * Display a set of indexed byte-size registers when the server is invoked with
 * -verbose.
 */
static void
ATIPrintIndexedRegisters(const unsigned short int Port,
                         const unsigned char Start_Index,
                         const unsigned char End_Index, const char * Name,
                         const unsigned short int GenS1)
{
        int Index;

        ErrorF("\n\n %s register values:", Name);
        for (Index = Start_Index;  Index < End_Index;  Index++)
        {
                if((Index % 4) == 0)
                {
                        if ((Index % 16) == 0)
                                ErrorF("\n 0x%02X:", Index);
                        ErrorF(" ");
                }
                if (Port == ATTRX)
                        (void) inb(GenS1);      /* Reset flip-flop */
                ErrorF("%02X", GetReg(Port, Index));
        }

        if (Port == ATTRX)
        {
                (void) inb(GenS1);              /* Reset flip-flop */
                outb(ATTRX, 0x20U);             /* Turn on PAS bit */
        }
}

/*
 * ATIPrintRegisters --
 *
 * Display various registers when the server is invoked with -verbose.
 */
static void
ATIPrintRegisters(void)
{
        int Index, Step, Limit;
        unsigned char misc = inb(R_GENMO);

        ErrorF("\n\n Miscellaneous output register value: 0x%02X.",
                misc);

        if (misc & 0x01U)
        {
                ATIPrintIndexedRegisters(CRTX(ColourIOBase), 0, 64,
                        "Colour CRT controller", 0);
                ATIPrintIndexedRegisters(ATTRX, 0, 32, "Attribute controller",
                        GENS1(ColourIOBase));
        }
        else
        {
                ATIPrintIndexedRegisters(CRTX(MonochromeIOBase), 0, 64,
                        "Monochrome CRT controller", 0);
                ATIPrintIndexedRegisters(ATTRX, 0, 32, "Attribute controller",
                        GENS1(MonochromeIOBase));
        }

        ATIPrintIndexedRegisters(GRAX, 0, 16, "Graphics controller", 0);
        ATIPrintIndexedRegisters(SEQX, 0, 8, "Sequencer", 0);

        if (Chip_Has_VGA_Wonder)
                ATIPrintIndexedRegisters(ATIVGAPort,
                        xf86ProbeOnly ? 0x80U : ATIVGAOffset, 0xC0U,
                        "ATI Extended VGA", 0);

        if (ATIChip >= ATI_CHIP_88800GXC)
        {
                ErrorF("\n\n Mach64 %s registers:",
                       (ATIIODecoding == SPARSE_IO) ? "sparse" : "block");
                Limit = ATIIOPort(IOPortTag(0x1FU, 0x3FU));
                Step = ATIIOPort(IOPortTag(0x01U, 0x01U)) - ATIIOBase;
                for (Index = ATIIOBase;  Index <= Limit;  Index += Step)
                {
                        if (!(((Index - ATIIOBase) / Step) & 0x03U))
                                ErrorF("\n 0x%04X:", Index);
                        if (Index == DAC_REGS_IOPort)
                                ErrorF(" %02X%02X%02X%02X",
                                        inb(DAC_REGS_IOPort + 3),
                                        inb(DAC_REGS_IOPort + 2),
                                        inb(DAC_REGS_IOPort + 1),
                                        inb(DAC_REGS_IOPort));
                        else
                                ErrorF(" %08X", inl(Index));
                }
        }

        ErrorF("\n\n");
}

#define ATIV3Delay                                              \
        {                                                       \
                int counter;                                    \
                for (counter = 0;  counter < 512;  counter++)   \
                        /* (void) inb(GENS1(vgaIOBase)) */;     \
        }

/*
 * ATIModifyExtReg --
 *
 * This routine is called to modify certain bits in an ATI extended VGA
 * register while preserving its other bits.  The routine will not write the
 * register if it turns out its value would not change.  This helps prevent
 * server hangs on older adapters.
 */
static void
ATIModifyExtReg(const unsigned char Index,
                int Current_Value,
                const unsigned char Current_Mask,
                unsigned char New_Value)
{
        /* Possibly retrieve the current value */
        if (Current_Value < 0)
                Current_Value = ATIGetExtReg(Index);

        /* Compute new value */
        New_Value &= (unsigned char)(~Current_Mask);
        New_Value |= Current_Value & Current_Mask;

        /* Check if value will be changed */
        if (Current_Value != New_Value)
        {
                /*
                 * The following is taken from ATI's VGA Wonder programmer's
                 * reference manual which says that this is needed to "ensure
                 * the proper state of the 8/16 bit ROM toggle".  I suspect a
                 * timing glitch appeared in the 18800 after its die was cast.
                 * 18800-1 and later chips do not exhibit this problem.
                 */
                if ((ATIChip <= ATI_CHIP_18800) && (Index == 0xB2U) &&
                   ((New_Value ^ 0x40U) & Current_Value & 0x40U))
                {
                        unsigned char misc = inb(R_GENMO);
                        unsigned char bb = ATIGetExtReg(0xBBU);
                        outb(GENMO, (misc & 0xF3U) | 0x04U |
                                ((bb & 0x10U) >> 1));
                        Current_Value &= (unsigned char)(~0x40U);
                        ATIPutExtReg(0xB2U, Current_Value);
                        ATIV3Delay;
                        outb(GENMO, misc);
                        ATIV3Delay;
                        if (Current_Value != New_Value)
                                ATIPutExtReg(0xB2U, New_Value);
                }
                else
                        ATIPutExtReg(Index, New_Value);
        }
}

/*
 * ATIClockSelect --
 *
 * This function selects the dot-clock with index 'no'.  This is done by
 * setting bits in various registers (generic VGA uses two bits in the
 * Miscellaneous Output Register to select from 4 clocks).  Care is taken to
 * protect any other bits in these registers by fetching their values and
 * masking off the other bits.
 */
static Bool
ATIClockSelect(int no)
{
        static unsigned char saved_misc,
                saved_b2, saved_b5, saved_b8, saved_b9, saved_be;
        unsigned char misc;

        switch(no)
        {
                case CLK_REG_SAVE:
                        /*
                         * Here all of the registers that can be affected by
                         * clock setting are saved into static variables.
                         */
                        saved_misc = inb(R_GENMO);

                        if (Chip_Has_VGA_Wonder)
                        {
                                saved_b5 = ATIGetExtReg(0xB5U);
                                saved_b8 = ATIGetExtReg(0xB8U);

                                if (ATIChip <= ATI_CHIP_18800)
                                        saved_b2 = ATIGetExtReg(0xB2U);
                                else
                                {
                                        saved_be = ATIGetExtReg(0xBEU);
                                        if (ATIAdapter != ATI_ADAPTER_V4)
                                                saved_b9 = ATIGetExtReg(0xB9U);
                                }

                                /*
                                 * Ensure clock interface is properly
                                 * configured.
                                 */
                                ATIModifyExtReg(0xB5U, saved_b5, 0x7FU, 0x00U);
                        }
                        break;

                case CLK_REG_RESTORE:
                        /*
                         * Here all the previously saved registers are
                         * restored.
                         */
                        ATISaveScreen(SS_START);

                        if (Chip_Has_VGA_Wonder)
                        {
                                if (ATIChip <= ATI_CHIP_18800)
                                        ATIModifyExtReg(0xB2U, -1, 0x00U,
                                                saved_b2);
                                else
                                {
                                        ATIModifyExtReg(0xBEU, -1, 0x00U,
                                                saved_be);
                                        if (ATIAdapter != ATI_ADAPTER_V4)
                                                ATIModifyExtReg(0xB9U, -1,
                                                        0x00U, saved_b9);
                                }
                                ATIModifyExtReg(0xB5U, -1, 0x00U, saved_b5);
                                ATIModifyExtReg(0xB8U, -1, 0x00U, saved_b8);
                        }

                        outb(GENMO, saved_misc);
                        ATISaveScreen(SS_FINISH);

                        break;

                default:
                        /*
                         * Possibly, remap clock number.
                         */
                        no = ATIClockMap[no & 0x0FU] | (no & ~0x0FU);

                        /*
                         * On adapters with crystals, switching to one of the
                         * spare assignments doesn't do anything (i.e. the
                         * previous setting remains in effect).  So, disable
                         * their selection.
                         */
                        if (((no & 0x03U) == 0x02U) &&
                           ((ATIChip <= ATI_CHIP_18800) ||
                            (ATIAdapter == ATI_ADAPTER_V4)))
                                return (FALSE);

                        /*
                         * Set the generic two low-order bits of the clock
                         * select.
                         */
                        misc = (inb(R_GENMO) & 0xF3U) | ((no << 2) & 0x0CU);

                        if (Chip_Has_VGA_Wonder)
                        {
                                /* Set the high order bits */
                                if (ATIChip <= ATI_CHIP_18800)
                                        ATIModifyExtReg(0xB2U, -1, 0xBFU,
                                                (no << 4));
                                else
                                {
                                        ATIModifyExtReg(0xBEU, -1, 0xEFU,
                                                (no << 2));
                                        if (ATIAdapter != ATI_ADAPTER_V4)
                                        {
                                                no >>= 1;
                                                ATIModifyExtReg(0xB9U, -1,
                                                        0xFDU, no >> 1);
                                        }
                                }

                                /* Set clock divider bits */
                                ATIModifyExtReg(0xB8U, -1, 0x00U,
                                        (no << 3) & 0xC0U);
                        }
                        /*
                         * Reject clocks that cannot be selected.
                         */
                        else if (no & 0xFCU)
                                return (FALSE);

                        /* Must set miscellaneous output register last */
                        outb(GENMO, misc);

                        break;
        }
        return (TRUE);
}

/*
 * ATIMatchClockLine --
 *
 * This function tries to match the XF86Config clocks to one of an array of
 * clock lines.  It returns a clock line number (or 0).
 */
static int
ATIMatchClockLine(const int **Clock_Line, const unsigned int Number_Of_Clocks,
                  const int Calibration_Clock_Number, const int Clock_Map)
{
        int Clock_Chip = 0, Clock_Chip_Index = 0;
        int Number_Of_Matching_Clocks = 0;
        int Minimum_Gap = CLOCK_TOLERANCE + 1;

        /* If checking for XF86Config clock order, skip crystals */
        if (Clock_Map)
                Clock_Chip_Index++;

        for (;  Clock_Line[++Clock_Chip_Index];  )
        {
                int Maximum_Gap = 0, Clock_Count = 0, Clock_Index = 0;

                for (;  Clock_Index < Number_Of_Clocks;  Clock_Index++)
                {
                        int Gap, XF86Config_Clock, Specification_Clock;

                        Specification_Clock =
                                Clock_Line[Clock_Chip_Index]
                                          [Clock_Maps[Clock_Map][Clock_Index]];
                        if (Specification_Clock < 0)
                                break;
                        if (!Specification_Clock ||
                           (Specification_Clock > vga256InfoRec.maxClock))
                                continue;

                        XF86Config_Clock = vga256InfoRec.clock[Clock_Index];
                        if (!XF86Config_Clock ||
                           (XF86Config_Clock > vga256InfoRec.maxClock))
                                continue;

                        Gap = abs(XF86Config_Clock - Specification_Clock);
                        if (Gap >= Minimum_Gap)
                                goto skip_this_clock_generator;
                        if (!Gap)
                        {
                                if (Clock_Index == Calibration_Clock_Number)
                                        continue;
                        }
                        else if (Gap > Maximum_Gap)
                                Maximum_Gap = Gap;
                        Clock_Count++;
                }

                if (Clock_Count <= Number_Of_Matching_Clocks)
                        continue;
                Number_Of_Matching_Clocks = Clock_Count;
                Clock_Chip = Clock_Chip_Index;
                if (!(Minimum_Gap = Maximum_Gap))
                        break;
        skip_this_clock_generator: ;
        }

        return Clock_Chip;
}

/*
 * ATIGetClocks --
 *
 * This function is called by ATIProbe and handles the XF86Config clocks line
 * (or lack thereof).
 */
static void
ATIGetClocks(void)
{
        int Number_Of_Undivided_Clocks, Number_Of_Dividers, Number_Of_Clocks;
        int Calibration_Clock_Number, Calibration_Clock_Value;
        int Clock_Index, Specification_Clock, Clock_Map = 0;

        /*
         * Decide what to do about the XF86Config clocks for programmable clock
         * generators.
         */
        if (ATIProgrammableClock != ATI_CLOCK_FIXED)
        {
                /*
                 * Check for those that are not (yet) handled.
                 */
                if (ATIProgrammableClock >=
                    Number_Of_Programmable_Clock_Generators)
                        ErrorF("Unknown programmable clock generator type"
                               " (0x%02X) detected.\n", ATIProgrammableClock);
                else if (ATIClockDescriptor->MaxN <= 0)
                        ErrorF("Unsupported programmable clock generator"
                               " detected:  %s.",
                               ATIClockDescriptor->ClockName);
                else
                {
                        /*
                         * Recognize supported clock generators.  This involves
                         * telling the rest of the server about it and
                         * (re-)initializing the XF86Config clocks line.
                         */
                        OFLG_SET(CLOCK_OPTION_PROGRAMABLE,
                                &vga256InfoRec.clockOptions);

                        /* Set internal clock ordering */
                        ATIClockMap = ATIProgrammableClockMap;

                        if (xf86Verbose)
                                ErrorF("%s programmable clock generator"
                                       " detected.\n",
                                        ATIClockDescriptor->ClockName);

                        /* Clobber XF86Config clock line */
                        if (OFLG_ISSET(XCONFIG_CLOCKS,
                            &vga256InfoRec.xconfigFlag))
                        {
                                OFLG_CLR(XCONFIG_CLOCKS,
                                        &vga256InfoRec.xconfigFlag);
                                ErrorF("XF86Config clocks specification"
                                       " ignored.\n");
                        }
                        if (ATIProgrammableClock == ATI_CLOCK_CH8398)
                        {       /* The first two are fixed */
                                vga256InfoRec.clocks = 2;
                                vga256InfoRec.clock[0] = 25175;
                                vga256InfoRec.clock[1] = 28322;
                        }
                        else
                                vga256InfoRec.clocks = 0;

                        return;         /* to ATIProbe */
                }
        }

        /*
         * Determine the number of clock values the adapter should be able to
         * generate and the dot clock to use for probe calibration.
         */
probe_clocks:
        Number_Of_Dividers = 4;
        if ((ATIChip <= ATI_CHIP_18800) || (ATIAdapter == ATI_ADAPTER_V4))
        {
                Number_Of_Undivided_Clocks = 8;
                /* Actually, any undivided clock will do */
                Calibration_Clock_Number = 7;
                Calibration_Clock_Value = 36000;
        }
        else
        {
                Number_Of_Undivided_Clocks = 16;
                Calibration_Clock_Number = 10 /* or 11 */;
                Calibration_Clock_Value = 75000 /* or 65000 */;
                if (ATIChip >= ATI_CHIP_68800)
                        Number_Of_Dividers = 2;
        }
        Number_Of_Clocks = Number_Of_Undivided_Clocks * Number_Of_Dividers;

        /*
         * Respect any XF86Config clocks line.  Well, that's the theory,
         * anyway.  In practice, however, the regular use of probed values is
         * widespread, at times causing otherwise inexplicable results.  So,
         * attempt to normalize the clocks to known (specification) values.
         */
        if ((!vga256InfoRec.clocks) || xf86ProbeOnly ||
            (OFLG_ISSET(OPTION_PROBE_CLKS, &vga256InfoRec.options)))
        {
                if (ATIProgrammableClock != ATI_CLOCK_FIXED)
                {
                        /*
                         * For unsupported programmable clock generators, pick
                         * the highest frequency set by BIOS initialization for
                         * clock calibration.
                         */
                        Calibration_Clock_Number = Calibration_Clock_Value = 0;
                        for (Clock_Index = 0;
                             Clock_Index < Number_Of_Undivided_Clocks;
                             Clock_Index++)
                                if (Calibration_Clock_Value <
                                    ATIBIOSClocks[Clock_Index])
                                {
                                        Calibration_Clock_Number = Clock_Index;
                                        Calibration_Clock_Value =
                                                ATIBIOSClocks[Clock_Index];
                                }
                        Calibration_Clock_Number =
                                ATIClockMap[Calibration_Clock_Number];
                        Calibration_Clock_Value *= 10;
                }

                /*
                 * Probe the adapter for clock values.  Note that vgaGetClocks
                 * cannot be used for this purpose because it assumes clock
                 * 1 is 28.322 MHz.  Instead call xf86GetClocks directly
                 * passing it slightly different parameters.
                 */
                xf86GetClocks(Number_Of_Clocks, ATIClockSelect,
                        vgaProtect, vgaSaveScreen,
                        GENS1(vgaIOBase), 0x08,
                        Calibration_Clock_Number, Calibration_Clock_Value,
                        &vga256InfoRec);

                /* Tell user clocks were probed, instead of supplied */
                OFLG_CLR(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag);

                /*
                 * Attempt to match probed clocks to a known specification.
                 */
                ATIClock = ATIMatchClockLine(ClockLine,
                        Number_Of_Undivided_Clocks,
                        Calibration_Clock_Number, 0);

                if ((ATIChip <= ATI_CHIP_18800) ||
                    (ATIAdapter == ATI_ADAPTER_V4))
                {
                        /*
                         * V3 and V4 adapters don't have clock chips.
                         */
                        if (ATIClock > ATI_CLOCK_CRYSTALS)
                                ATIClock = ATI_CLOCK_NONE;
                }
                else
                {
                        /*
                         * All others don't have crystals.
                         */
                        if (ATIClock == ATI_CLOCK_CRYSTALS)
                                ATIClock = ATI_CLOCK_NONE;
                }
        }
        else
        {
                /*
                 * Allow for an initial subset of specification clocks.  Can't
                 * allow for any more than that though...
                 */
                if (Number_Of_Clocks > vga256InfoRec.clocks)
                {
                        Number_Of_Clocks = vga256InfoRec.clocks;
                        if (Number_Of_Undivided_Clocks > Number_Of_Clocks)
                                Number_Of_Undivided_Clocks = Number_Of_Clocks;
                }
                else if (Number_Of_Clocks < vga256InfoRec.clocks)
                        vga256InfoRec.clocks = Number_Of_Clocks;

                /*
                 * Attempt to match clocks to a known specification.
                 */
                ATIClock = ATIMatchClockLine(ClockLine,
                        Number_Of_Undivided_Clocks, -1, 0);

                if (ATIClock == ATI_CLOCK_NONE)
                {
                        /*
                         * Reject certain clock lines that are obviously wrong.
                         * This includes the standard VGA clocks, and clock
                         * lines that could have been used with the pre-2.1.1
                         * driver.
                         */
                        if (ATIMatchClockLine(InvalidClockLine,
                                              Number_Of_Clocks, -1, 0))
                                vga256InfoRec.clocks = 0;
                        else if ((ATIChip > ATI_CHIP_18800) &&
                                 (ATIAdapter != ATI_ADAPTER_V4))
                        /*
                         * Check for clocks that are specified in the wrong
                         * order.  This is meant to catch those who are trying
                         * to use the clock order intended for the accelerated
                         * servers.
                         */
                        for (;
                             (++Clock_Map, Clock_Map %= Number_Of_Clock_Maps);
                            )
                                if ((ATIClock = ATIMatchClockLine(ClockLine,
                                        Number_Of_Undivided_Clocks,
                                        -1, Clock_Map)))
                                {
                                        ErrorF("XF86Config clocks ordering"
                                               " incorrect.  Clocks will be"
                                               " reordered.\nSee README.ati"
                                               " for more information.\n");
                                        break;
                                 }
                }
                else
                /*
                 * Ensure crystals are not matched to clock chips, and vice
                 * versa.
                 */
                if ((ATIChip <= ATI_CHIP_18800) ||
                    (ATIAdapter == ATI_ADAPTER_V4))
                {
                        if (ATIClock > ATI_CLOCK_CRYSTALS)
                                vga256InfoRec.clocks = 0;
                }
                else
                {
                        if (ATIClock == ATI_CLOCK_CRYSTALS)
                                vga256InfoRec.clocks = 0;
                }

                if (!vga256InfoRec.clocks)
                {
                        ErrorF("Invalid or obsolete XF86Config clocks line"
                               " rejected.\nClocks will be probed.  See"
                               " README.ati for more information.\n");
                        goto probe_clocks;
                }
        }

        if (ATIProgrammableClock != ATI_CLOCK_FIXED)
                ATIProgrammableClock = ATI_CLOCK_FIXED;
        else if (ATIClock == ATI_CLOCK_NONE)
                ErrorF("Unknown clock generator detected.\n");
        else if (xf86Verbose)
                if (ATIClock == ATI_CLOCK_CRYSTALS)
                        ErrorF("This adapter uses crystals to generate clock"
                               " frequencies.\n");
                else
                        ErrorF("%s clock chip detected.\n",
                                ClockNames[ATIClock]);

        if (ATIClock == ATI_CLOCK_NONE)
                return;                 /* Don't touch the clocks */

        /*
         * Replace the undivided clocks with specification values.
         */
        for (Clock_Index = 0;
             Clock_Index < Number_Of_Undivided_Clocks;
             Clock_Index++)
        {
                /*
                 * Don't replace clocks that are probed, documented, or set by
                 * the user to zero.  One exception is that we need to override
                 * the user's value for the spare settings on a crystal-based
                 * adapter.  Another exception is when the user specifies the
                 * clock ordering intended for the accelerated servers.
                 */
                Specification_Clock = ClockLine[ATIClock][Clock_Index];
                if (Specification_Clock < 0)
                        break;
                if (!Clock_Map)
                {
                        if (!vga256InfoRec.clock[Clock_Index])
                                continue;
                        if ((!Specification_Clock) &&
                            (ATIClock != ATI_CLOCK_CRYSTALS))
                                continue;
                }
                vga256InfoRec.clock[Clock_Index] = Specification_Clock;
        }

        /*
         * Adjust the divided clocks.
         */
        for (Clock_Index = Number_Of_Undivided_Clocks;
             Clock_Index < Number_Of_Clocks;
             Clock_Index++)
        {
                vga256InfoRec.clock[Clock_Index] =
                        vga256InfoRec.clock[Clock_Index %
                                Number_Of_Undivided_Clocks] /
                        ((Clock_Index / Number_Of_Undivided_Clocks) + 1);
        }
}

/*
 * ATIMach32ChipID --
 *
 * Set variables whose value is dependent upon an 68800's CHIP_ID register.
 */
static void
ATIMach32ChipID(void)
{
        unsigned int IO_Value = inw(CHIP_ID);
        ChipType     = GetBits(IO_Value, CHIP_CODE_0 | CHIP_CODE_1);
        ChipClass    = GetBits(IO_Value, CHIP_CLASS);
        ChipRevision = GetBits(IO_Value, CHIP_REV);
        if (IO_Value == 0xFFFFU)
                IO_Value = 0;
        switch (GetBits(IO_Value, CHIP_CODE_0 | CHIP_CODE_1))
        {
                case 0x0000U:
                        ATIChip = ATI_CHIP_68800_3;
                        break;

                case 0x02F7U:
                        ATIChip = ATI_CHIP_68800_6;
                        break;

                case 0x0177U:
                        ATIChip = ATI_CHIP_68800LX;
                        break;

                case 0x0017U:
                        ATIChip = ATI_CHIP_68800AX;
                        break;

                default:
                        ATIChip = ATI_CHIP_68800;
                        break;
        }

        /*
         * When selecting clocks through the ATI extended VGA registers, all
         * 68800's use a different clock ordering than earlier controllers.
         */
        ATIClockMap = ATIMachClockMap;
}

/*
 * ATIMach64ChipID --
 *
 * Set variables whose value is dependent upon a Mach64's CONFIG_CHIP_ID
 * register.
 */
static void
ATIMach64ChipID(void)
{
        unsigned int IO_Value = inl(ATIIOPort(CONFIG_CHIP_ID));
        ChipType     = GetBits(IO_Value, 0xFFFFU);
        ChipClass    = GetBits(IO_Value, CFG_CHIP_CLASS);
        ChipRevision = GetBits(IO_Value, CFG_CHIP_REV);
        switch (ChipType)
        {
                case 0x4758U:
                        ChipType = 0x00D7U;
                case 0x00D7U:
                        switch (ChipRevision)
                        {
                                case 0x00U:
                                        ATIChip = ATI_CHIP_88800GXC;
                                        break;

                                case 0x01U:
                                        ATIChip = ATI_CHIP_88800GXD;
                                        break;

                                case 0x02U:
                                        ATIChip = ATI_CHIP_88800GXE;
                                        break;

                                case 0x03U:
                                        ATIChip = ATI_CHIP_88800GXF;
                                        break;

                                default:
                                        ATIChip = ATI_CHIP_88800GX;
                                        break;
                        }
                        break;

                case 0x4358U:
                        ChipType = 0x0057U;
                case 0x0057U:
                        ATIChip = ATI_CHIP_88800CX;
                        break;

                case 0x4354U:
                        ChipType = 0x0053U;
                case 0x0053U:
                        ATIChip = ATI_CHIP_264CT;
                        break;

                case 0x4554U:
                        ChipType = 0x0093U;
                case 0x0093U:
                        ATIChip = ATI_CHIP_264ET;
                        break;

                case 0x5654U:
                        ChipType = 0x02B3U;
                case 0x02B3U:
                        ATIChip = ATI_CHIP_264VT;
                        break;

                case 0x4754U:
                        ChipType = 0x00D3U;
                case 0x00D3U:
                        ATIChip = ATI_CHIP_264GT;
                        break;

                default:
                        ATIChip = ATI_CHIP_88800;
                        break;
        }

        /*
         * When selecting clocks through the ATI extended VGA registers,
         * Mach64's use a different clock ordering than 28800 controllers.
         */
        ATIClockMap = ATIMachClockMap;
}

typedef unsigned short int Colour; /* The correct spelling should be OK :-) */

/*
 * Bit patterns which are extremely unlikely to show up when reading from
 * nonexistant memory (which normally shows up as either all bits set or all
 * bits clear).
 */
static const Colour Test_Pixel[] = {0x5AA5U, 0x55AAU, 0xA55AU, 0xCA53U};

#define Number_Of_Test_Pixels (sizeof(Test_Pixel) / sizeof(Test_Pixel[0]))

static const struct
{
        int videoRamSize;
        int Miscellaneous_Options_Setting;
        struct
        {
                short int x, y;
        }
        Coordinates[Number_Of_Test_Pixels + 1];
}
Test_Case[] =
{
        /*
         * Given the engine settings used, only a 4M card will have enough
         * memory to back up the 1025th line of the display.  Since the pixel
         * coordinates are zero-based, line 1024 will be the first one which
         * is only backed on 4M cards.
         *
         * <Mark_Weaver@brown.edu>:
         * In case memory is being wrapped, (0,0) and (0,1024) to make sure
         * they can each hold a unique value.
         */
        {4096, MEM_SIZE_4M, {{0,0}, {0,1024}, {-1,-1}}},

        /*
         * This card has 2M or less.  On a 1M card, the first 2M of the card's
         * memory will have even doublewords backed by physical memory and odd
         * doublewords unbacked.
         *
         * Pixels 0 and 1 of a row will be in the zeroth doubleword, while
         * pixels 2 and 3 will be in the first.  Check both pixels 2 and 3 in
         * case this is a pseudo-1M card (one chip pulled to turn a 2M card
         * into a 1M card).
         *
         * <Mark_Weaver@brown.edu>:
         * I don't have a 1M card, so I'm taking a stab in the dark.  Maybe
         * memory wraps every 512 lines, or maybe odd doublewords are aliases
         * of their even doubleword counterparts.  I try everything here.
         */
        {2048, MEM_SIZE_2M, {{0,0}, {0,512}, {2,0}, {3,0}, {-1,-1}}},

        /*
         * This is a either a 1M card or a 512k card.  Test pixel 1, since it
         * is an odd word in an even doubleword.
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

#define Number_Of_Test_Cases (sizeof(Test_Case) / sizeof(Test_Case[0]))

/*
 * ATIMach32ReadPixel --
 *
 * Return the colour of the specified screen location.  Called from
 * ATIMach32videoRam routine below.
 */
static Colour
ATIMach32ReadPixel(const short int X, const short int Y)
{
        Colour Pixel_Colour;

        /* Wait for idle engine */
        ProbeWaitIdleEmpty();

        /* Set up engine for pixel read */
        ATIWaitQueue(7);
        outw(RD_MASK, (unsigned short int)(~0));
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
 * ATIMach32videoRam routine below.
 */
static void
ATIMach32WritePixel(const short int X, const short int Y,
                    const Colour Pixel_Colour)
{
        /* Set up engine for pixel write */
        ATIWaitQueue(9);
        outw(WRT_MASK, (unsigned short int)(~0));
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
 * This routine is adapted from a similar routine in mach32mem.c written by
 * Robert Wolff, David Dawes and Mark Weaver.
 */
static int
ATIMach32videoRam(void)
{
        unsigned short int saved_clock_sel, saved_mem_bndry,
                saved_misc_options, saved_ext_ge_config;
        Colour saved_Pixel[Number_Of_Test_Pixels];
        int Case_Number, Pixel_Number;
        unsigned short int corrected_misc_options;
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
         * Set up the drawing engine for a pitch of 1024 at 16 bits per pixel.
         * No need to mess with the CRT because the results of this test are
         * not intended to be seen.
         */
        outw(EXT_GE_CONFIG, PIX_WIDTH_16BPP | ORDER_16BPP_565 |
                MONITOR_8514 | ALIAS_ENA);
        outw(GE_PITCH, 1024 >> 3);
        outw(GE_OFFSET_HI, 0);
        outw(GE_OFFSET_LO, 0);

        for (Case_Number = 0;
             Case_Number < (Number_Of_Test_Cases - 1);
             Case_Number++)
        {
                /* Reduce redundancy as per Mark_Weaver@brown.edu */
#               define TestPixel               \
                        Test_Case[Case_Number].Coordinates[Pixel_Number]
#               define ForEachTestPixel        \
                        for (Pixel_Number = 0; \
                             TestPixel.x >= 0; \
                             Pixel_Number++)

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
#               undef ForEachTestPixel
#               undef TestPixel
        }

        /* Restore what was changed and correct MISC_OPTIONS register */
        ATIWaitQueue(4);
        outw(EXT_GE_CONFIG, saved_ext_ge_config);
        corrected_misc_options |=
                Test_Case[Case_Number].Miscellaneous_Options_Setting;
        if (corrected_misc_options != saved_misc_options)
                outw(MISC_OPTIONS, corrected_misc_options);
        outw(MEM_BNDRY, saved_mem_bndry);

        /* Re-enable VGA passthrough */
        outw(CLOCK_SEL, saved_clock_sel & ~DISABPASSTHRU);

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
ATIMach64Probe(const unsigned short int IO_Base,
               const unsigned char IO_Decoding)
{
        unsigned int IO_Value, saved_bus_cntl, saved_gen_test_cntl;
        unsigned short int IO_Port;

        if ((ATIAdapter != ATI_ADAPTER_NONE) || (IO_Base == 0))
                return;

        ATIIOBase = IO_Base;
        ATIIODecoding = IO_Decoding;

        /* Make sure any Mach64 is not in some weird state */
        BUS_CNTL_IOPort = ATIIOPort(BUS_CNTL);
        saved_bus_cntl = inl(BUS_CNTL_IOPort);
        outl(BUS_CNTL_IOPort, (saved_bus_cntl &
                ~(BUS_ROM_DIS | BUS_FIFO_ERR_INT_EN |
                  BUS_HOST_ERR_INT_EN)) |
                (BUS_FIFO_ERR_INT | BUS_HOST_ERR_INT |
                 SetBits(15, BUS_FIFO_WS)));

        GEN_TEST_CNTL_IOPort = ATIIOPort(GEN_TEST_CNTL);
        saved_gen_test_cntl = inl(GEN_TEST_CNTL_IOPort);
        IO_Value = saved_gen_test_cntl &
                (GEN_OVR_OUTPUT_EN | GEN_OVR_POLARITY |
                 GEN_CUR_EN | GEN_BLOCK_WR_EN);
        outl(GEN_TEST_CNTL_IOPort, IO_Value | GEN_GUI_EN);
        outl(GEN_TEST_CNTL_IOPort, IO_Value);
        outl(GEN_TEST_CNTL_IOPort, IO_Value | GEN_GUI_EN);

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
                         * *Something* has a R/W 32-bit register at this I/O
                         * address.  Try to make sure it's a Mach64.  The
                         * following assumes that ATI won't be producing any
                         * more adapters that don't register themselves in the
                         * PCI configuration space.
                         */
                        ATIMach64ChipID();
                        if ((ATIChip != ATI_CHIP_88800) ||
                            (IO_Decoding == BLOCK_IO))
                                ATIAdapter = ATI_ADAPTER_MACH64;
                        else
                        {
                                ATIChip = ATI_CHIP_NONE;
                                ATIClockMap = ATIVGAWonderClockMap;
                        }
                }
        }

        /* Restore registers that might have been clobbered */
        outl(IO_Port, IO_Value);
        if (ATIAdapter != ATI_ADAPTER_MACH64)
        {
                outl(GEN_TEST_CNTL_IOPort, saved_gen_test_cntl);
                outl(BUS_CNTL_IOPort, saved_bus_cntl);
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
static Bool
ATIProbe(void)
{
        static const unsigned char ATISignature[] = " 761295520";
#       define Signature_Size   10
#       define Prefix_Size      1024    /* 1kB */
#       define BIOS_SIZE        0x8000U /* 32kB */
#       define BIOS_Signature   0x30U
#       define No_Signature     (Prefix_Size + 1 - Signature_Size)
        unsigned char BIOS[BIOS_SIZE];
#       define BIOSByte(n)      (*((unsigned char      *)(BIOS + (n))))
#       define BIOSWord(n)      (*((unsigned short int *)(BIOS + (n))))
        unsigned int IO_Value, IO_Value2;
        unsigned int Signature = No_Signature;
        int saved_BIOSbase = vga256InfoRec.BIOSbase;
        int MachvideoRam = 0;
        int VGAWondervideoRam = 0;
        unsigned char ATIMachChip = ATI_CHIP_NONE;
        unsigned short int ClockDac;
        static const int videoRamSizes[] =
                {0, 256, 512, 1024, 2*1024, 4*1024, 6*1024, 8*1024, 12*1024,
                 16*1024, 0};
        TokenTabPtr TokenEntry;
        int mode_flags;
        int ROMTable = 0, ClockTable = 0, FrequencyTable = 0, Index;
        const DACRec *DAC;
        pciConfigPtr PCIDevice;

        /*
         * Get out if this isn't the driver the user wants.
         */
        if (vga256InfoRec.chipset &&
            StrCaseCmp(vga256InfoRec.chipset, ATIIdent(0)))
        {
                static const char *chipsets[] =
                        {"vgawonder", "mach8", "mach32", "mach64"};

                /* Check for some other chipset names that need changing */
                for (Index = 0;
                     StrCaseCmp(vga256InfoRec.chipset, chipsets[Index]);  )
                        if (++Index >=
                            (sizeof(chipsets) / sizeof(chipsets[0])))
                                return (FALSE);
                if (xf86Verbose > 1)
                        ErrorF("ChipSet specification changed from \"%s%s",
                               chipsets[Index], "\" to \"ati\".\n");
                OFLG_CLR(XCONFIG_CHIPSET, &vga256InfoRec.xconfigFlag);
        }

        /*
         * Enable the I/O ports needed for probing.
         */
        xf86ClearIOPortList(vga256InfoRec.scrnIndex);
        xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
        xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_ATI_IOPorts, ATI_IOPorts);
        xf86EnableIOPorts(vga256InfoRec.scrnIndex);

        /*
         * Save register value to be modified, just in case there is no 8514/A
         * compatible accelerator.
         */
        IO_Value = inw(SUBSYS_STAT);
        IO_Value2 = IO_Value & _8PLANE;

        /*
         * Determine if an 8514/A-compatible accelerator is present, making
         * sure it's not in some weird state.
         */
        outw(SUBSYS_CNTL, IO_Value2 | (GPCTRL_RESET | CHPTEST_NORMAL));
        outw(SUBSYS_CNTL, IO_Value2 | (GPCTRL_ENAB | CHPTEST_NORMAL |
                RVBLNKFLG | RPICKFLAG | RINVALIDIO | RGPIDLE));

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
                Chip_Has_SUBSYS_CNTL = TRUE;

                /*
                 * Don't leave any Mach8 or Mach32 in 8514/A mode.
                 */
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

        else if (ATIAdapter == ATI_ADAPTER_NONE)
        {
                /*
                 * Restore register clobbered by 8514/A reset attempt.
                 */
                outw(SUBSYS_CNTL, IO_Value);

                /* Determine if a Mach64 is present */
                /* First, check the user's IObase */
                if (vga256InfoRec.IObase & BLOCK_IO_SELECT)
                        ATIMach64Probe(vga256InfoRec.IObase & SPARSE_IO_BASE,
                                SPARSE_IO);
                else
                if (vga256InfoRec.IObase & SPARSE_IO_SELECT)
                        ATIMach64Probe(vga256InfoRec.IObase & BLOCK_IO_BASE,
                                BLOCK_IO);

                /* Check the "standard" sparse I/O bases */
                ATIMach64Probe(0x02ECU, SPARSE_IO);
                ATIMach64Probe(0x01C8U, SPARSE_IO);
                ATIMach64Probe(0x01CCU, SPARSE_IO);

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
                                ATIMach64Probe(PCIDevice->_base1 &
                                        BLOCK_IO_BASE, BLOCK_IO);
                        }
                }
        }

        /*
         * Extract various information from any detected accelerator.
         */
        switch (ATIAdapter)
        {
                case ATI_ADAPTER_8514A:
                        MachvideoRam =
                                videoRamSizes[GetBits(IO_Value, _8PLANE) + 2];
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
                        MachvideoRam = videoRamSizes[
                                GetBits(inw(CONFIG_STATUS_1),
                                        MEM_INSTALLED) + 2];
                        break;

                case ATI_ADAPTER_MACH32:
                        IO_Value = inw(CONFIG_STATUS_1);
                        if (!(IO_Value & (_8514_ONLY | CHIP_DIS)))
                        {
                                ATIVGAAdapter = ATI_ADAPTER_MACH32;
                                Chip_Has_VGA_Wonder = TRUE;
                        }
                        ATIDac = ATI_DAC(GetBits(IO_Value, DACTYPE), 0);

                        ATIMach32ChipID();

                        MachvideoRam = videoRamSizes[GetBits(inw(MISC_OPTIONS),
                                MEM_SIZE_ALIAS) + 2];

                        /*
                         * The 68800-6 doesn't necessarily report the correct
                         * video memory size.
                         */
                        if ((ATIChip == ATI_CHIP_68800_6) &&
                            (MachvideoRam == 1024))
                                MachvideoRam = ATIMach32videoRam();

                        vga256InfoRec.BIOSbase = 0x000C0000U +
                                (GetBits(inw(ROM_ADDR_1), BIOS_BASE_SEGMENT) <<
                                        11);
                        break;

                case ATI_ADAPTER_MACH64:
                        if (ATIChip <= ATI_CHIP_88800CX)
                        {
                                IO_Value = inl(ATIIOPort(CONFIG_STATUS64_0)) &
                                        (CFG_VGA_EN | CFG_CHIP_EN);
                                if (ATIChip == ATI_CHIP_88800CX)
                                        IO_Value |= CFG_VGA_EN;
                                if (IO_Value == (CFG_VGA_EN | CFG_CHIP_EN))
                                {
                                        ATIVGAAdapter = ATI_ADAPTER_MACH64;
                                        Chip_Has_VGA_Wonder = TRUE;

                                        /*
                                         * Apparently, 0x1CE cannot be used for
                                         * ATI's extended VGA registers when
                                         * using block I/O decoding.  Instead,
                                         * these registers are tacked on to
                                         * VGA's Graphics register bank.
                                         */
                                        if (ATIIODecoding == BLOCK_IO)
                                                ATIVGAPort = GRAX;
                                }
                        }
                        else
                        {
                                ATIVGAAdapter = ATI_ADAPTER_MACH64;

                                /*
                                 * For the integrated controllers, use the
                                 * small dual paged apertures to simulate a
                                 * standard VGA window.  Otherwise, virtual
                                 * resolutions would be limited to the first
                                 * (VGA standard) 256kB of video memory.
                                 */
                                Using_Small_Apertures = TRUE;

                                /* Reset banking functions */
                                ATI.ChipSetRead = ATIMach64SetRead;
                                ATI.ChipSetWrite = ATIMach64SetWrite;
                                ATI.ChipSetReadWrite = ATIMach64SetReadWrite;

                                /* Set banking ports */
                                ATIMach64RPPort = ATIIOPort(MEM_VGA_RP_SEL);
                                ATIMach64WPPort = ATIIOPort(MEM_VGA_WP_SEL);

                                /* Don't divide interlaced timings */
                                ATI.ChipInterlaceType = VGA_NO_DIVIDE_VERT;
                        }

                        /* Set general use I/O port numbers */
                        CRTC_OFF_PITCH_IOPort = ATIIOPort(CRTC_OFF_PITCH);
                        CRTC_GEN_CNTL_IOPort = ATIIOPort(CRTC_GEN_CNTL);
                        CLOCK_CNTL_IOPort = ATIIOPort(CLOCK_CNTL);
                        MEM_INFO_IOPort = ATIIOPort(MEM_INFO);
                        DAC_REGS_IOPort = ATIIOPort(DAC_REGS);
                        DAC_CNTL_IOPort = ATIIOPort(DAC_CNTL);
                        CONFIG_CNTL_IOPort = ATIIOPort(CONFIG_CNTL);

                        IO_Value = inl(ATIIOPort(SCRATCH_REG1));
                        ATIDac = ATI_DAC(
                                GetBits(inl(DAC_CNTL_IOPort), DAC_TYPE),
                                GetBits(IO_Value, BIOS_INIT_DAC_SUBTYPE));
                        if (ATIDac < ATI_DAC_ATI68875)
                                ATIDac += ATI_DAC_INTERNAL;

                        MachvideoRam = videoRamSizes[GetBits(
                                inl(MEM_INFO_IOPort), CTL_MEM_SIZE) + 2];

                        vga256InfoRec.BIOSbase = 0x000C0000U +
                                (GetBits(IO_Value, BIOS_BASE_SEGMENT) << 11);
                        break;

                default:
                        break;
        }

        /*
         * Get BIOS, *all* of it.
         */
        Index = xf86ReadBIOS(vga256InfoRec.BIOSbase, 0, BIOS, sizeof(BIOS));

        /* Fill in what cannot be gotten with zeroes */
        for (;  Index < BIOS_SIZE;  Index++)
                if (Index > 0)
                        BIOS[Index] = 0;

        /*
         * Attempt to find the ATI signature in the first 1024 bytes of the
         * video BIOS.
         */
        for (Signature = 0;  Signature < No_Signature;  )
        {
                for (Index = 0;  Index < Signature_Size;  Index++)
                        if (BIOS[Signature + Index] != ATISignature[Index])
                        {
                                Signature++;
                                break;
                        }
                if (Index >= Signature_Size)
                        break;
        }

        /*
         * If no VGA capability has yet been detected, determine if VGA Wonder
         * functionality is possible.
         */
        if ((ATIAdapter <= ATI_ADAPTER_MACH8) &&
            (Signature == BIOS_Signature) &&
            (BIOS[0x40U] == '3'))
        {
                Chip_Has_VGA_Wonder = TRUE;     /* ... more than likely ... */

                switch (BIOS[0x41U])
                {
                        case '1':
                                /*
                                 * This is a Mach8 or VGA Wonder adapter of
                                 * some kind.
                                 */
                                if ((BIOS[0x43U] >= '1') &&
                                    (BIOS[0x43U] <= '6'))
                                        ATIChip = BIOS[0x43U] -
                                                ('1' - ATI_CHIP_18800);

                                switch (BIOS[0x43U])
                                {
                                        case '1':       /* ATI_CHIP_18800 */
                                                ATIVGAOffset = 0xB0U;
                                                ATIVGAAdapter = ATI_ADAPTER_V3;
                                                /*
                                                 * Reset a few things for V3
                                                 * adapters.
                                                 */
                                                ATI.ChipSetRead = ATIV3SetRead;
                                                ATI.ChipSetWrite =
                                                        ATIV3SetWrite;
                                                ATI.ChipSetReadWrite =
                                                        ATIV3SetReadWrite;
                                                ATI.ChipUse2Banks = FALSE;
                                                break;

                                        case '2':       /* ATI_CHIP_18800_1 */
                                                ATIVGAOffset = 0xB0U;
                                                if (BIOS[0x42U] & 0x10U)
                                                        ATIVGAAdapter =
                                                                ATI_ADAPTER_V5;
                                                else
                                                        ATIVGAAdapter =
                                                                ATI_ADAPTER_V4;
                                                /*
                                                 * Reset a few things for V4
                                                 * and V5 adapters.
                                                 */
                                                ATI.ChipSetRead =
                                                        ATIV4V5SetRead;
                                                ATI.ChipSetWrite =
                                                        ATIV4V5SetWrite;
                                                ATI.ChipSetReadWrite =
                                                        ATIV4V5SetReadWrite;
                                                break;

                                        case '3':       /* ATI_CHIP_28800_2 */
                                        case '4':       /* ATI_CHIP_28800_4 */
                                        case '5':       /* ATI_CHIP_28800_5 */
                                        case '6':       /* ATI_CHIP_28800_6 */
                                                ATIVGAOffset = 0xA0U;
                                                ATIVGAAdapter =
                                                        ATI_ADAPTER_PLUS;
                                                if (BIOS[0x44U] & 0x80U)
                                                {
                                                        ATIVGAAdapter =
                                                                ATI_ADAPTER_XL;
                                                        ATIDac =
                                                                ATI_DAC_SC11483;
                                                }
                                                break;

                                        case 'a':       /* A Mach32 with a */
                                        case 'b':       /* frontal lobotomy */
                                        case 'c':
                                                ATIVGAAdapter =
                                                        ATI_ADAPTER_NONISA;
                                                ATIMach32ChipID();
                                                ProbeWaitIdleEmpty();
                                                if (inw(SUBSYS_STAT) !=
                                                    0xFFFFU)
                                                        Chip_Has_SUBSYS_CNTL =
                                                                TRUE;
                                                break;

                                        case ' ':       /* A crippled Mach64 */
                                                ATIVGAAdapter =
                                                        ATI_ADAPTER_NONISA;
                                                ATIMach64ChipID();
                                                break;

                                        default:
                                                break;
                                }
                                break;

                        case '2':
                                ATIVGAOffset = 0xB0U;   /* Presumably */
                                ATIVGAAdapter = ATI_ADAPTER_EGA_PLUS;
                                break;

                        case '3':
                                ATIVGAOffset = 0xB0U;   /* Presumably */
                                ATIVGAAdapter = ATI_ADAPTER_BASIC;
                                break;

                        case '?':       /* A crippled Mach64 */
                                ATIVGAAdapter = ATI_ADAPTER_NONISA;
                                ATIMach64ChipID();
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
                if ((ATIAdapter == ATI_ADAPTER_NONE) ||
                    (ATIVGAAdapter == ATI_ADAPTER_NONE))
                {
                        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
                        vga256InfoRec.BIOSbase = saved_BIOSbase;
                        return (FALSE);
                }
        }

        /*
         * For Mach64 adapters, pick up, from the BIOS, the type of
         * programmable clock chip (if any), and various information about it.
         */
        if (ATIChip >= ATI_CHIP_88800GXC)
        {
                ROMTable = BIOSWord(0x48U);
                if ((ROMTable + 0x12U) > BIOS_SIZE)
                        ROMTable = 0;
                if (ROMTable > 0)
                {
                        ClockTable = BIOSWord(ROMTable + 0x10U);
                        if ((ClockTable + 0x0CU) > BIOS_SIZE)
                                ClockTable = 0;
                }
                if (ClockTable > 0)
                {
                        FrequencyTable = BIOSWord(ClockTable - 0x02U);
                        if ((FrequencyTable + 0x20U) > BIOS_SIZE)
                                FrequencyTable = 0;
                        if (FrequencyTable > 0)
                                for (Index = 0;  Index < 16;  Index++)
                                    ATIBIOSClocks[Index] =
                                        (&BIOSWord(FrequencyTable))[Index];
                        ATIProgrammableClock = BIOSByte(ClockTable);
                        ATIClockNumberToProgramme =
                                BIOSByte(ClockTable + 0x06U);
                        if (ATIProgrammableClock <
                            Number_Of_Programmable_Clock_Generators)
                                ATIClockDescriptor += ATIProgrammableClock;
                }

                ClockDac = ATIDac;
                switch (ATIProgrammableClock)
                {
                        case ATI_CLOCK_ICS2595:
                                /*
                                 * Pick up reference divider (43 or 46)
                                 * appropriate to the chip revision level.
                                 */
                                ATIClockDescriptor->MinM =
                                ATIClockDescriptor->MaxM =
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
                                 * The reference divider has already been
                                 * programmed by BIOS initialization.  Because,
                                 * there is only one reference divider for all
                                 * generated frequencies (including MCLK), it
                                 * cannot be changed without reprogramming all
                                 * clocks every time one of them needs a
                                 * different reference divider.
                                 *
                                 * Besides, it's not a good idea to change the
                                 * reference divider.  BIOS initialization sets
                                 * it to a value that effectively prevents
                                 * generating frequencies beyond the graphics
                                 * controller's tolerance.
                                 */
                                ATIClockDescriptor->MinM =
                                ATIClockDescriptor->MaxM =
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
                                break;

                        default:
                                break;
                }

                /*
                 * We now have up to two indications of what RAMDAC the adapter
                 * uses.  They should be the same.  The following test and
                 * corresponding action are under construction.
                 */
                if (ATIDac != ClockDac)
                {
                        ErrorF("Mach64 RAMDAC probe discrepancy detected:\n"
                               "  ATIDac=0x%02X;  ClockDac=0x%02X.\n",
                                ATIDac, ClockDac);

                        if (ATIDac == ATI_DAC_IBMRGB514)
                        {
                                ATIProgrammableClock = ATI_CLOCK_IBMRGB514;
                                ATIClockDescriptor =
                                        ClockDescriptors + ATI_CLOCK_IBMRGB514;
                                ATIClockNumberToProgramme = 7;
                        }
                        else
                                ATIDac = ClockDac;      /* For now */
                }

                /*
                 * For adapters with supported programmable clock generators,
                 * set an initial estimate for maxClock.  This value might be
                 * reduced later due to RAMDAC considerations.
                 */
                if (ATIClockDescriptor->MaxN > 0)
                {
                        int Numerator = ATIClockDescriptor->MaxN * 157500,
                            Denominator = ATIClockDescriptor->MinM * 11000;
                        if (ATIClockDescriptor->MinD > 0)
                                Denominator <<= ATIClockDescriptor->MinD;
                        else if (ATIClockDescriptor->MinD < 0)
                                Numerator <<= -ATIClockDescriptor->MinD;
                        vga256InfoRec.maxClock =
                                (Numerator / Denominator) * 1000;
                }
        }

        if (Chip_Has_VGA_Wonder)
        {
                /*
                 * Set up extended VGA register addressing.  Note that, for
                 * Mach64's, only the GX-C & GX-D controllers allow the setting
                 * of this address.
                 */
                if (ATIChip <= ATI_CHIP_88800GXD)
                {
                        if ((ATIChip < ATI_CHIP_88800GXC) &&
                            (Signature == BIOS_Signature) &&
                            (BIOSWord(0x10U)) &&
                            (!(BIOSWord(0x10U) &
                              ~(SPARSE_IO_BASE | IO_BYTE_SELECT))))
                        {
                                /*
                                 * Pick up extended register index I/O port
                                 * number.
                                 */
                                ATIVGAPort = BIOSWord(0x10U);
                        }
                        PutReg(GRAX, 0x50U, ATIVGAPort & 0xFFU);
                        PutReg(GRAX, 0x51U, ATIVGAOffset | (ATIVGAPort >> 8));
                }
                ATI_IOPorts[0] = ATIVGAPort;
                ATI_IOPorts[1] = ATIVGAPort + 1;
        }

        /*
         * Rebuild I/O port list.  Some of its entries might have changed.
         */
        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
        xf86ClearIOPortList(vga256InfoRec.scrnIndex);
        xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
        xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_ATI_IOPorts, ATI_IOPorts);

        ATIEnterLeave(ENTER);           /* Unlock registers */

        /*
         * Sometimes, the BIOS lies about the chip.
         */
        if ((ATIChip >= ATI_CHIP_28800_4) &&
                (ATIChip <= ATI_CHIP_28800_6))
        {
                IO_Value = GetBits(ATIGetExtReg(0xAAU), 0x0FU);
                if ((IO_Value < 7) && (IO_Value > ATIChip))
                        ATIChip = IO_Value;
        }

        if ((xf86Verbose) ||
            (ATIVGAAdapter == ATI_ADAPTER_NONE) ||
            (ATIChip == ATI_CHIP_NONE))
        {
                ErrorF("%s graphics controller detected.\n",
                        ChipNames[ATIChip]);
                if ((ATIChip >= ATI_CHIP_68800) &&
                    (ATIChip != ATI_CHIP_68800_3))
                {
                        ErrorF("Chip type %04X", ChipType);
                        if (!(ChipType & ~(CHIP_CODE_0 | CHIP_CODE_1)))
                                ErrorF(" (%c%c)",
                                        GetBits(ChipType, CHIP_CODE_1) + 0x41U,
                                        GetBits(ChipType, CHIP_CODE_0) + 0x41U);
                        ErrorF(", class %d, revision 0x%X.\n",
                                ChipClass, ChipRevision);
                        if (ATIAdapter == ATI_ADAPTER_MACH64)
                                ErrorF("%s I/O base is 0x%04X\n",
                                       (ATIIODecoding == SPARSE_IO) ?
                                        "Sparse" : "Block", ATIIOBase);
                }
                if (ATIMachChip != ATI_CHIP_NONE)
                        ErrorF("%s graphics accelerator detected, with %dkB of"
                               " coprocessor memory.\n",
                                ChipNames[ATIMachChip], MachvideoRam);
                if ((ATIVGAAdapter == ATI_ADAPTER_NONE) &&
                    (Signature == BIOS_Signature))
                        ErrorF("Unknown chip descriptor in BIOS:"
                               "  0x%02X%02X%02X%02X.\n",
                               BIOS[0x40U], BIOS[0x41U],
                               BIOS[0x42U], BIOS[0x43U]);
                ErrorF("%s video adapter detected.\n",
                        AdapterNames[ATIAdapter]);
        }

        if ((ATIDac & ~0x0FU) == ATI_DAC_INTERNAL)
        {
                if (xf86Verbose)
                        ErrorF("Internal RAMDAC (subtype %d) detected.\n",
                                ATIDac & 0x0FU);
        }
        else
        for (DAC = DACDescriptors;  ;  DAC++)
        {
                if (ATIDac == DAC->DACType)
                {
                        if (xf86Verbose)
                                ErrorF("%s RAMDAC detected.\n", DAC->DACName);
                        break;
                }
                if (ATIDac < DAC->DACType)
                {
                        ErrorF("Unknown RAMDAC type (0x%02X) detected.\n",
                                ATIDac);
                        break;
                }
        }

        switch (ATIAdapter)
        {
                case ATI_ADAPTER_8514A:
                case ATI_ADAPTER_MACH8:
                        /*
                         * From now on, ignore any 8514/A or Mach8 accelerator.
                         */
                        ATIAdapter = ATIVGAAdapter;
                        /* Accelerator and VGA cannot share memory */
                        MachvideoRam = 0;
                        break;

                case ATI_ADAPTER_MACH32:
                case ATI_ADAPTER_MACH64:
                        if (ATIVGAAdapter == ATI_ADAPTER_NONE)
                        {
                                ErrorF("VGA capability is not available.\n");
                                ATIEnterLeave(LEAVE);
                                vga256InfoRec.BIOSbase = saved_BIOSbase;
                                return (FALSE);
                        }
                        if (saved_BIOSbase != vga256InfoRec.BIOSbase)
                        {
                                ErrorF("BIOS Base Address changed to"
                                       " 0x%08X.\n", vga256InfoRec.BIOSbase);
                                OFLG_CLR(XCONFIG_BIOSBASE,
                                        &vga256InfoRec.xconfigFlag);
                        }
                        break;

                default:
                        break;
        }

        if (ATIChip == ATI_CHIP_NONE)
                ErrorF("Support for this video adapter is highly"
                       " experimental!\n");

        /*
         * Normalize any XF86Config videoRam value.
         */
        for (Index = 0;  videoRamSizes[++Index];  )
                if (vga256InfoRec.videoRam < videoRamSizes[Index])
                        break;
        vga256InfoRec.videoRam = videoRamSizes[Index - 1];

        /*
         * The default videoRam value is what the accelerator (if any) thinks
         * it has.  Also, allow the user to override the accelerator's value.
         */
        if (vga256InfoRec.videoRam == 0)
        {
                /* Normalization might have zeroed XF86Config videoRam value */
                OFLG_CLR(XCONFIG_VIDEORAM, &vga256InfoRec.xconfigFlag);
                vga256InfoRec.videoRam = MachvideoRam;
        }
        else
                MachvideoRam = vga256InfoRec.videoRam;

        if (Chip_Has_VGA_Wonder)
        {
                /*
                 * Find out how much video memory the VGA Wonder side thinks it
                 * has.
                 */
                if (ATIChip <= ATI_CHIP_18800_1)
                {
                        IO_Value = ATIGetExtReg(0xBBU);
                        if (IO_Value & 0x20U)
                                VGAWondervideoRam = 512;
                        else
                                VGAWondervideoRam = 256;
                        if (MachvideoRam > 512)
                                MachvideoRam = 512;
                }
                else
                {
                        IO_Value = ATIGetExtReg(0xB0U);
                        if (IO_Value & 0x08U)
                                VGAWondervideoRam = 1024;
                        else if (IO_Value & 0x10U)
                                VGAWondervideoRam = 512;
                        else
                                VGAWondervideoRam = 256;
                        if (MachvideoRam > 1024)
                                MachvideoRam = 1024;
                }
        }

        /*
         * If there's no supported accelerator, default videoRam to what the
         * VGA Wonder side believes.
         */
        if (!vga256InfoRec.videoRam)
                vga256InfoRec.videoRam = VGAWondervideoRam;
        else if ((ATIChip < ATI_CHIP_68800) || (ATIChip > ATI_CHIP_68800AX))
        /*
         * After BIOS initialization, the accelerator (if any) and the VGA
         * won't necessarily agree on the amount of video memory, depending on
         * whether or where the memory boundary is configured.  Any discrepancy
         * will be resolved by ATIInit.
         *
         * However, it's possible that there is more video memory than VGA
         * Wonder can architecturally handle.  If so, spit out a warning.
         */
        if (MachvideoRam < vga256InfoRec.videoRam)
                ErrorF("Virtual resolutions requiring more than %d kB\n of"
                       " video memory might not function correctly.\n",
                       MachvideoRam);

        /*
         * Set the maximum allowable dot-clock frequency (in kHz).  This is
         * dependent on what the RAMDAC can handle (in non-pixmux mode, for
         * now).  For an internal DAC, assume it can handle whatever frequency
         * the internal PLL can produce, but default maxClock to 135MHz.
         */
        if ((ATIDac & ~0x0FU) == ATI_DAC_INTERNAL)
        {
                if (vga256InfoRec.dacSpeed < vga256InfoRec.maxClock)
                        vga256InfoRec.maxClock = vga256InfoRec.dacSpeed;
                if (vga256InfoRec.maxClock < 135000)
                        vga256InfoRec.maxClock = 135000;
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

        /*
         * Determine available dot clock frequencies.
         */
        ATIGetClocks();

        /*
         * If user did not specify any modes, attempt to create a default mode.
         * Its timings will be taken from the mode in effect on driver entry.
         */
        if (vga256InfoRec.modes == NULL)
        {
                char *Message = NULL;

                /*
                 * This duplicates vgaProbe's needmem variable.
                 */
#               if defined(MONOVGA) && !defined(BANKEDMONOVGA)
#                       define needmem (ATI.ChipMapSize << 3)
#               elif defined(MONOVGA) || defined(XF86VGA16)
#                       define needmem (vga256InfoRec.videoRam << 11)
#               else
#                       define needmem ((vga256InfoRec.videoRam << 13) / \
                                vgaBitsPerPixel)
#               endif

                /*
                 * Get current timings.
                 */
                ATIGetMode(&DefaultMode);

                /*
                 * Check if generated mode can be used.
                 */
                if (!DefaultMode.SynthClock)
                        Message = "required dot clock cannot be determined";
                else if ((DefaultMode.SynthClock / 1000) >
                   ((vga256InfoRec.maxClock / 1000) /
                     ATI.ChipClockScaleFactor))
                        Message = "required dot clock greater than maxClock";
                else
                if ((DefaultMode.HDisplay * DefaultMode.VDisplay) > needmem)
                        Message = "insufficient video memory";

                if (Message)
                        ErrorF("Default %dx%d mode not used:  %s.\n",
                                DefaultMode.HDisplay, DefaultMode.VDisplay,
                                Message);
                else
                {
                        DefaultMode.prev = DefaultMode.next =
                                ATI.ChipBuiltinModes = &DefaultMode;
                        DefaultMode.name = "Default mode";
                        ErrorF("The following default video mode will be"
                               " used:\n"
                               " Dot clock:           %7.3fMHz\n"
                               " Horizontal timings:  %4d %4d %4d %4d\n"
                               " Vertical timings:    %4d %4d %4d %4d\n",
                               DefaultMode.SynthClock / 1000.0,
                               DefaultMode.HDisplay,
                               DefaultMode.HSyncStart,
                               DefaultMode.HSyncEnd,
                               DefaultMode.HTotal,
                               DefaultMode.VDisplay,
                               DefaultMode.VSyncStart,
                               DefaultMode.VSyncEnd,
                               DefaultMode.VTotal);
                        mode_flags = DefaultMode.Flags;
                        if (mode_flags & V_HSKEW)
                        {
                                mode_flags &= ~V_HSKEW;
                                ErrorF(" Horizontal skew:     %4d\n",
                                       DefaultMode.HSkew);
                        }
                        ErrorF(" Flags:              ");
                        for (TokenEntry = TokenTab;
                             TokenEntry->flag;
                             TokenEntry++)
                                if (mode_flags & TokenEntry->flag)
                                {
                                        ErrorF(" %s",
                                               xf86TokenToString(TimingTab,
                                                        TokenEntry->token));
                                        mode_flags &= ~TokenEntry->flag;
                                        if (!mode_flags)
                                                break;
                                }
                        ErrorF("\n");
                }

                /*
                 * Completeness.
                 */
#               undef needmem
        }

        /*
         * Set chipset name.
         */
        vga256InfoRec.chipset = ATIIdent(0);

        /*
         * Tell monochrome and 16-colour servers banked operation is
         * supported.
         */
        vga256InfoRec.bankedMono = TRUE;

        /*
         * Indicate supported options ...
         */
        OFLG_SET(OPTION_PROBE_CLKS, &ATI.ChipOptionFlags);
        OFLG_SET(OPTION_CSYNC,      &ATI.ChipOptionFlags);

        /*
         * ... and unsupported ones.
         */
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
                /*
                 * Spill the beans...
                 */
                if (Signature == No_Signature)
                        ErrorF("\nNo BIOS signature found.\n");
                else if (Signature != BIOS_Signature)
                        ErrorF("\nBIOS signature found at offset 0x%04X.\n",
                                Signature);

                if (Chip_Has_VGA_Wonder)
                        ErrorF("\nThe ATI extended VGA registers are being"
                               " accessed at I/O port 0x%04X.\n", ATIVGAPort);

                if ((ATIChip < ATI_CHIP_88800GXC) &&
                    (Signature == BIOS_Signature))
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
                        ATIPrintBIOS(BIOS, ClockTable - 0x06U,
                                ClockTable + 0x1EU);

                if (xf86Verbose > 2)
                {
                        ErrorF("\n On server entry:");

                        xf86EnableIOPorts(vga256InfoRec.scrnIndex);
                        ATIPrintRegisters();
                        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
                }
        }

#       if defined(XFreeXDGA) && !defined(MONOVGA)
                vga256InfoRec.directMode = XF86DGADirectPresent;
#       endif

        /*
         * Return success.
         */
        return (TRUE);
}

/*
 * ATIEnterLeave --
 *
 * This function is called when the virtual terminal on which the server is
 * running is entered or left, as well as when the server starts up and is shut
 * down.  Its function is to obtain and relinquish I/O permissions for the SVGA
 * device.  This includes unlocking access to any registers that may be
 * protected on the chipset, and locking those registers again on exit.
 */
static void
ATIEnterLeave(const Bool enter)
{
        static unsigned char saved_a6, saved_ab,
                saved_b1, saved_b4, saved_b5, saved_b6,
                saved_b8, saved_b9, saved_be;
        static unsigned short int saved_clock_sel, saved_misc_options,
                saved_mem_bndry, saved_mem_cfg;
        static unsigned int saved_config_cntl, saved_crtc_gen_cntl,
                saved_mem_info, saved_gen_test_cntl, saved_dac_cntl;

        static Bool entered = LEAVE;
        unsigned int tmp;

#       if defined(XFreeXDGA) && !defined(MONOVGA)
                if ((enter == LEAVE) &&
                    (vga256InfoRec.directMode & XF86DGADirectGraphics))
                        return;
#       endif

        if (enter == entered)
                return;
        entered = enter;

        if (enter == ENTER)
        {
                xf86EnableIOPorts(vga256InfoRec.scrnIndex);

                if (Chip_Has_SUBSYS_CNTL)
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
                        outw(SUBSYS_CNTL,
                                tmp | (GPCTRL_RESET | CHPTEST_NORMAL));
                        outw(SUBSYS_CNTL, tmp | (GPCTRL_ENAB | CHPTEST_NORMAL |
                                RVBLNKFLG | RPICKFLAG | RINVALIDIO | RGPIDLE));

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
                                        ~(MEM_APERT_SEL | MEM_APERT_PAGE |
                                                MEM_APERT_LOC));
                        }

                        /* Wait for all activity to die down */
                        ProbeWaitIdleEmpty();
                }
                else if (ATIChip >= ATI_CHIP_88800GXC)
                {
                        /* Save register values to be modified */
                        saved_config_cntl = inl(CONFIG_CNTL_IOPort);
                        saved_mem_info = inl(MEM_INFO_IOPort);
                        saved_dac_cntl = inl(DAC_CNTL_IOPort);

                        /* Reset everything */
                        tmp = inl(BUS_CNTL_IOPort);
                        outl(BUS_CNTL_IOPort, (tmp &
                                ~(BUS_ROM_DIS | BUS_FIFO_ERR_INT_EN |
                                        BUS_HOST_ERR_INT_EN)) |
                                (BUS_FIFO_ERR_INT | BUS_HOST_ERR_INT |
                                 SetBits(15, BUS_FIFO_WS)));
                        saved_gen_test_cntl = inl(GEN_TEST_CNTL_IOPort) &
                                (GEN_OVR_OUTPUT_EN | GEN_OVR_POLARITY |
                                 GEN_CUR_EN | GEN_BLOCK_WR_EN);
                        tmp = saved_gen_test_cntl & ~GEN_CUR_EN;
                        outl(GEN_TEST_CNTL_IOPort, tmp | GEN_GUI_EN);
                        outl(GEN_TEST_CNTL_IOPort, tmp);
                        outl(GEN_TEST_CNTL_IOPort, tmp | GEN_GUI_EN);
                        saved_crtc_gen_cntl = inl(CRTC_GEN_CNTL_IOPort) &
                                ~CRTC_EN;
                        tmp = saved_crtc_gen_cntl & ~CRTC_EXT_DISP_EN;
                        outl(CRTC_GEN_CNTL_IOPort, tmp | CRTC_EN);
                        outl(CRTC_GEN_CNTL_IOPort, tmp);
                        outl(CRTC_GEN_CNTL_IOPort, tmp | CRTC_EN);

                        /* Ensure VGA aperture is enabled */
                        outl(DAC_CNTL_IOPort, saved_dac_cntl | DAC_VGA_ADR_EN);
                        tmp = saved_config_cntl &
                                ~(CFG_MEM_AP_SIZE | CFG_VGA_DIS |
                                        CFG_MEM_VGA_AP_EN);
                        if (Using_Small_Apertures)
                                tmp |= CFG_MEM_VGA_AP_EN;
                        outl(CONFIG_CNTL_IOPort, tmp);
                        outl(MEM_INFO_IOPort, saved_mem_info &
                                ~(CTL_MEM_BNDRY | CTL_MEM_BNDRY_EN));
                }

                if (Chip_Has_VGA_Wonder)
                {
                        /*
                         * Ensure all registers are read/write and disable all
                         * non-VGA emulations.
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
                                        ATIModifyExtReg(0xA6U, saved_a6, 0x7FU,
                                                0x00U);
                                        saved_ab = ATIGetExtReg(0xABU);
                                        ATIModifyExtReg(0xABU, saved_ab, 0xE7U,
                                                0x00U);
                                }
                        }
                }

                vgaIOBase = (inb(R_GENMO) & 0x01U) ?
                        ColourIOBase : MonochromeIOBase;

                /*
                 * There's a bizarre interaction here.  If bit 0x80 of CRTC[17]
                 * is on, then CRTC[3] is read-only.  If bit 0x80 of CRTC[3] is
                 * off, then CRTC[17] is write-only (or a read attempt actually
                 * returns bits from C/EGA's light pen position).  This means
                 * that if both conditions are met, CRTC[17]'s value on server
                 * entry cannot be retrieved.
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
                         * Could not make CRTC[17] readable, so unprotect
                         * CRTC[0-7] replacing VSyncEnd with zero.  This zero
                         * will be replaced after acquiring the needed access.
                         */
                        unsigned int VSyncEnd, VBlankStart, VBlankEnd;
                        unsigned char crt07, crt09;

                        PutReg(CRTX(vgaIOBase), 0x11U, 0x20U);
                        /* Make CRTC[16-17] readable */
                        PutReg(CRTX(vgaIOBase), 0x03U, tmp | 0x80U);
                        /* Make vertical synch pulse as wide as possible */
                        crt07 = GetReg(CRTX(vgaIOBase), 0x07U);
                        crt09 = GetReg(CRTX(vgaIOBase), 0x09U);
                        VBlankStart =
                                (((crt09 & 0x20U) << 4) |
                                 ((crt07 & 0x08U) << 5) |
                                 GetReg(CRTX(vgaIOBase), 0x15U)) + 1;
                        VBlankEnd =
                                (VBlankStart & 0x380U) |
                                (GetReg(CRTX(vgaIOBase), 0x16U) & 0x7FU);
                        if (VBlankEnd <= VBlankStart)
                                VBlankEnd += 0x80U;
                        VSyncEnd =
                                (((crt07 & 0x80U) << 2) |
                                 ((crt07 & 0x04U) << 6) |
                                 GetReg(CRTX(vgaIOBase), 0x10U)) + 0x0FU;
                        if (VSyncEnd >= VBlankEnd)
                                VSyncEnd = VBlankEnd - 1;
                        PutReg(CRTX(vgaIOBase), 0x11U,
                                (VSyncEnd & 0x0FU) | 0x20U);
                }
        }
        else
        {
                vgaIOBase = (inb(R_GENMO) & 0x01U) ?
                        ColourIOBase : MonochromeIOBase;

                /* Protect CRTC[0-7] */
                tmp = GetReg(CRTX(vgaIOBase), 0x11U);
                outb(CRTD(vgaIOBase), tmp | 0x80U);

                if (Chip_Has_VGA_Wonder)
                {
                        /*
                         * Restore emulation and protection bits in ATI
                         * extended VGA registers.
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
                                        ATIModifyExtReg(0xA6U, -1, 0x7FU,
                                                saved_a6);
                                        ATIModifyExtReg(0xABU, -1, 0xE7U,
                                                saved_ab);
                                }
                        }
                }

                if (Chip_Has_SUBSYS_CNTL)
                {
                        tmp = inw(SUBSYS_STAT) & _8PLANE;

                        /* Reset the 8514/A and disable all interrupts */
                        outw(SUBSYS_CNTL,
                                tmp | (GPCTRL_RESET | CHPTEST_NORMAL));
                        outw(SUBSYS_CNTL, tmp | (GPCTRL_ENAB | CHPTEST_NORMAL |
                                RVBLNKFLG | RPICKFLAG | RINVALIDIO | RGPIDLE));

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
                        tmp = inl(BUS_CNTL_IOPort);
                        outl(BUS_CNTL_IOPort, (tmp &
                                ~(BUS_ROM_DIS | BUS_FIFO_ERR_INT_EN |
                                        BUS_HOST_ERR_INT_EN)) |
                                (BUS_FIFO_ERR_INT | BUS_HOST_ERR_INT |
                                 SetBits(15, BUS_FIFO_WS)));
                        outl(GEN_TEST_CNTL_IOPort,
                                saved_gen_test_cntl | GEN_GUI_EN);
                        outl(GEN_TEST_CNTL_IOPort, saved_gen_test_cntl);
                        outl(GEN_TEST_CNTL_IOPort,
                                saved_gen_test_cntl | GEN_GUI_EN);
                        outl(CRTC_GEN_CNTL_IOPort,
                                saved_crtc_gen_cntl | CRTC_EN);
                        outl(CRTC_GEN_CNTL_IOPort, saved_crtc_gen_cntl);
                        outl(CRTC_GEN_CNTL_IOPort,
                                saved_crtc_gen_cntl | CRTC_EN);

                        /* Restore registers */
                        outl(CONFIG_CNTL_IOPort, saved_config_cntl);
                        outl(MEM_INFO_IOPort, saved_mem_info);
                        outl(DAC_CNTL_IOPort, saved_dac_cntl);
                }

                xf86DisableIOPorts(vga256InfoRec.scrnIndex);
        }
}

/*
 * ATIAccessBottomBank --
 *
 * Force memory aperture to bank zero of video memory.
 */
static void
ATIAccessBottomBank(unsigned char *ae, unsigned char *b2)
{
        if (Chip_Has_VGA_Wonder)
        {
                *b2 = ATIGetExtReg(0xB2U);
                if (ATIChip <= ATI_CHIP_18800)
                {
                        if (*b2 & 0x1EU)
                        {
                                *b2 &= 0xE1U;
                                ATIPutExtReg(0xB2U, *b2);
                        }
                }
                else
                {
                        if (*b2)
                        {
                                *b2 = 0;
                                ATIPutExtReg(0xB2U, 0U);
                        }
                        ATIB2Reg = 0;
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                *ae = ATIGetExtReg(0xAEU);
                                if (*ae & 0x0FU)
                                {
                                        *ae &= 0xF0U;
                                        ATIPutExtReg(0xAEU, *ae);
                                }
                        }
                }
        }
}

/*
 * ATIGetTimer --
 *
 * This routine returns the current system timer tick count.
 */
static unsigned short int
ATIGetTimer(void)
{
        unsigned short int Timer;

        outb(0x43U, 0U);
        Timer = inb(0x40U);
        Timer |= (inb(0x40U) << 8);
        return(Timer);
}

/*
 * ATITimerWait --
 *
 * This routine waits for a specified number of system timer ticks.
 */
static void
ATITimerWait(unsigned int Count)
{
        unsigned short int Current = ATIGetTimer();
        unsigned int TargetTime = Count + Current;
        unsigned short int TargetCount = TargetTime & 0xFFFFU,
                           WrapCount = TargetTime >> 16;

        while ((WrapCount) || (Current < TargetCount))
        {
                unsigned short int Next = ATIGetTimer();

                if (Next <= Current)
                        if (!(WrapCount--))
                                break;
                Current = Next;
        }
}

#define ATISleep(microseconds)  ATITimerWait(((microseconds) * 5) >> 1)

/*
 * ATIGetMach64DACCmdReg --
 *
 * Setup to access a RAMDAC's command register.
 */
static unsigned char
ATIGetMach64DACCmdReg(void)
{
        (void) inb(DAC_REGS_IOPort);            /* Reset to PEL mode */
        (void) inb(DAC_REGS_IOPort + 2);        /* Get command register */
        (void) inb(DAC_REGS_IOPort + 2);
        (void) inb(DAC_REGS_IOPort + 2);
        return (inb(DAC_REGS_IOPort + 2));
}

/*
 * ATIRestore --
 *
 * This function restores a video mode.  It basically writes out all of the
 * registers that have previously been saved in the vgaATIRec data structure.
 *
 * Note that "Restore" is slightly incorrect.  This function is also used when
 * the server enters/changes video modes.  The mode definitions have previously
 * been initialized by the Init() function, below.
 */
static void
ATIRestore(vgaATIPtr restore)
{
        unsigned char ae, b2, be = 0;

        /* Unlock registers */
        ATIEnterLeave(ENTER);

        /* Get (back) to bank 0 */
        ATIAccessBottomBank(&ae, &b2);
        if (ATIChip >= ATI_CHIP_264CT)
        {
                outb(CRTC_GEN_CNTL_IOPort, restore->crtc_gen_cntl0);
                outb(CRTC_GEN_CNTL_IOPort + 3, restore->crtc_gen_cntl3);
        }
        if (Using_Small_Apertures)
        {
                outw(ATIMach64RPPort, 0);
                outw(ATIMach64RPPort + 2, restore->second_aperture_bank);
                outw(ATIMach64WPPort, 0);
                outw(ATIMach64WPPort + 2, restore->second_aperture_bank);
        }

        if ((restore->ClockProgramme >= 0) &&
            (ATIProgrammableClock != ATI_CLOCK_FIXED))
        {
                /*
                 * Programme the clock.
                 */
                unsigned char saved_clock_cntl, saved_crtc_gen_cntl;
                unsigned char tmp, tmp2;
                int Programme;

                /* Temporarily switch to accelerator mode */
                saved_crtc_gen_cntl = inb(CRTC_GEN_CNTL_IOPort + 3);
                outb(CRTC_GEN_CNTL_IOPort + 3, saved_crtc_gen_cntl |
                        (CRTC_EXT_DISP_EN >> (3 * 8)));

                switch (ATIProgrammableClock)
                {
                        case ATI_CLOCK_ICS2595:
                                saved_clock_cntl = inb(CLOCK_CNTL_IOPort);

                                ATISleep(50000);        /* 50 milliseconds */

                                (void) xf86DisableInterrupts();

                                /* Send all 20 bits of programme word */
                                Programme = restore->ClockProgramme;
                                while (Programme >= CLOCK_BIT)
                                {
                                        tmp = (Programme & CLOCK_BIT) |
                                                CLOCK_STROBE;
                                        outb(CLOCK_CNTL_IOPort, tmp);
                                        ATISleep(26);   /* 26 microseconds */
                                        outb(CLOCK_CNTL_IOPort,
                                                tmp | CLOCK_PULSE);
                                        ATISleep(26);   /* 26 microseconds */
                                        Programme >>= 1;
                                }

                                xf86EnableInterrupts();

                                /* Restore register */
                                outb(CLOCK_CNTL_IOPort,
                                        saved_clock_cntl | CLOCK_STROBE);
                                break;

                        case ATI_CLOCK_STG1703:
                                (void) ATIGetMach64DACCmdReg();
                                (void) inb(DAC_REGS_IOPort + 2);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->std.NoClock << 1) + 0x20U);
                                outb(DAC_REGS_IOPort + 2, 0);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme >> 8) & 0xFFU);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme     ) & 0xFFU);
                                break;

                        case ATI_CLOCK_CH8398:
                                tmp = inb(DAC_CNTL_IOPort);
                                outb(DAC_CNTL_IOPort, tmp |
                                        (DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3));
                                outb(DAC_REGS_IOPort, restore->std.NoClock);
                                outb(DAC_REGS_IOPort + 1,
                                        (restore->ClockProgramme >> 8) & 0xFFU);
                                outb(DAC_REGS_IOPort + 1,
                                        (restore->ClockProgramme     ) & 0xFFU);
                                outb(DAC_CNTL_IOPort, (tmp & ~DAC_EXT_SEL_RS2) |
                                        DAC_EXT_SEL_RS3);
                                break;

                        case ATI_CLOCK_INTERNAL:
                                /* Set post-divider */
                                tmp2 = restore->std.NoClock << 1;
                                tmp = ATIGetMach64PLLReg(PLL_VCLK_POST_DIV);
                                tmp &= ~(0x03U << tmp2);
                                tmp |= (restore->ClockProgramme >> 8) << tmp2;
                                ATIPutMach64PLLReg(PLL_VCLK_POST_DIV, tmp);

                                /* Set feedback divider */
                                tmp = PLL_VCLK0_FB_DIV + restore->std.NoClock;
                                ATIPutMach64PLLReg(tmp,
                                        GetBits(restore->ClockProgramme,
                                        0xFFU));

                                /* Reset write bit */
                                ATIAccessMach64PLLReg(tmp, FALSE);
                                break;

                        case ATI_CLOCK_ATT20C408:
                                (void) ATIGetMach64DACCmdReg();
                                tmp = inb(DAC_REGS_IOPort + 2);
                                (void) ATIGetMach64DACCmdReg();
                                outb(DAC_REGS_IOPort + 2, tmp | 1);
                                outb(DAC_REGS_IOPort, 1);
                                outb(DAC_REGS_IOPort + 2, tmp | 9);
                                ATISleep(400);          /* 400 microseconds */
                                tmp2 = (restore->std.NoClock << 2) + 0x40U;
                                outb(DAC_REGS_IOPort, tmp2);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme >> 8) & 0xFFU);
                                outb(DAC_REGS_IOPort, ++tmp2);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme     ) & 0xFFU);
                                outb(DAC_REGS_IOPort, ++tmp2);
                                outb(DAC_REGS_IOPort + 2, 0x77U);
                                ATISleep(400);          /* 400 microseconds */
                                outb(DAC_REGS_IOPort, 1);
                                outb(DAC_REGS_IOPort + 2, tmp);
                                break;

                        case ATI_CLOCK_IBMRGB514:
                                tmp = inb(DAC_CNTL_IOPort);
                                outb(DAC_CNTL_IOPort, (tmp & ~DAC_EXT_SEL_RS3) |
                                        DAC_EXT_SEL_RS2);
                                tmp = (restore->std.NoClock << 1) + 0x20U;
                                outb(DAC_REGS_IOPort, tmp);
                                outb(DAC_REGS_IOPort + 1, 0);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme >> 8) & 0xFFU);
                                outb(DAC_REGS_IOPort, tmp + 1);
                                outb(DAC_REGS_IOPort + 1, 0);
                                outb(DAC_REGS_IOPort + 2,
                                        (restore->ClockProgramme     ) & 0xFFU);
                                break;

                        default:
                                break;
                }

                (void) inb(DAC_REGS_IOPort);   /* Clear DAC counter */

                /* Restore register */
                outl(CRTC_GEN_CNTL_IOPort + 3, saved_crtc_gen_cntl);
        }

        if (Chip_Has_VGA_Wonder)
        {
                ATISaveScreen(SS_START);        /* Start sequencer reset */

                /* Restore ATI registers */
                if (ATIChip <= ATI_CHIP_18800)
                        ATIModifyExtReg(0xB2U, b2, 0x00U, restore->b2);
                else
                {
                        ATIModifyExtReg(0xBEU, be, 0x00U, restore->be);
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                ATIModifyExtReg(0xBFU, -1, 0x00U, restore->bf);
                                ATIModifyExtReg(0xA3U, -1, 0x00U, restore->a3);
                                ATIModifyExtReg(0xA6U, -1, 0x00U, restore->a6);
                                ATIModifyExtReg(0xA7U, -1, 0x00U, restore->a7);
                                ATIModifyExtReg(0xABU, -1, 0x00U, restore->ab);
                                ATIModifyExtReg(0xACU, -1, 0x00U, restore->ac);
                                ATIModifyExtReg(0xADU, -1, 0x00U, restore->ad);
                                ATIModifyExtReg(0xAEU, ae, 0x00U, restore->ae);
                        }
                }
                ATIModifyExtReg(0xB0U, -1, 0x00U, restore->b0);
                ATIModifyExtReg(0xB1U, -1, 0x00U, restore->b1);
                ATIModifyExtReg(0xB3U, -1, 0x00U, restore->b3);
                ATIModifyExtReg(0xB5U, -1, 0x00U, restore->b5);
                ATIModifyExtReg(0xB6U, -1, 0x00U, restore->b6);
                ATIModifyExtReg(0xB8U, -1, 0x00U, restore->b8);
                ATIModifyExtReg(0xB9U, -1, 0x00U, restore->b9);
                ATIModifyExtReg(0xBAU, -1, 0x00U, restore->ba);
                ATIModifyExtReg(0xBDU, -1, 0x00U, restore->bd);

#if 0
                outb(GENMO, restore->std.MiscOutReg);
                ATISaveScreen(SS_FINISH);
#endif
        }

        /*
         * Restore the generic VGA registers.
         */
        vgaHWRestore((vgaHWPtr)restore);

        if (ATIChip >= ATI_CHIP_264CT)
                outl(CRTC_OFF_PITCH_IOPort, restore->crtc_off_pitch);

        if ((xf86Verbose > 2) && (restore->mode))
        {
                ErrorF("\n After setting mode \"%s\":", restore->mode->name);
                ATIPrintRegisters();
        }
}

/*
 * ATISave --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaATIRec data structure.  There is in general no need to mask out
 * bits here - just read the registers.
 */
static void *
ATISave(vgaATIPtr save)
{
        unsigned char ae, b2;   /* The oddballs */
        unsigned short int second_aperture_bank = 0;

        /* Unlock registers */
        ATIEnterLeave(ENTER);

        /* Get back to bank zero */
        ATIAccessBottomBank(&ae, &b2);
        if (Using_Small_Apertures)
        {
                second_aperture_bank =
                        inw(ATIMach64RPPort + 2) - inw(ATIMach64RPPort);
                outw(ATIMach64RPPort, 0);
                outw(ATIMach64RPPort + 2, second_aperture_bank);
                outw(ATIMach64WPPort, 0);
                outw(ATIMach64WPPort + 2, second_aperture_bank);
        }

        /*
         * vgaHWSave creates the data structure and fills in the generic VGA
         * portion.
         */
        save = (vgaATIPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaATIRec));

        save->mode = NULL;              /* No corresponding mode line */
        save->ClockProgramme = -1;      /* Don't programme clock */

        if (Chip_Has_VGA_Wonder)
        {
                /* Save ATI-specific registers */
                save->b0 = ATIGetExtReg(0xB0U);
                save->b1 = ATIGetExtReg(0xB1U);
                save->b2 = b2;
                save->b3 = ATIGetExtReg(0xB3U);
                save->b5 = ATIGetExtReg(0xB5U);
                save->b6 = ATIGetExtReg(0xB6U);
                save->b8 = ATIGetExtReg(0xB8U);
                save->b9 = ATIGetExtReg(0xB9U);
                save->ba = ATIGetExtReg(0xBAU);
                save->bd = ATIGetExtReg(0xBDU);
                if (ATIChip > ATI_CHIP_18800)
                {
                        save->be = ATIGetExtReg(0xBEU);
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                save->bf = ATIGetExtReg(0xBFU);
                                save->a3 = ATIGetExtReg(0xA3U);
                                save->a6 = ATIGetExtReg(0xA6U);
                                save->a7 = ATIGetExtReg(0xA7U);
                                save->ab = ATIGetExtReg(0xABU);
                                save->ac = ATIGetExtReg(0xACU);
                                save->ad = ATIGetExtReg(0xADU);
                                save->ae = ae;
                        }
                }
        }

        if (ATIChip >= ATI_CHIP_264CT)
        {
                save->crtc_gen_cntl0 = inb(CRTC_GEN_CNTL_IOPort);
                save->crtc_gen_cntl3 = inb(CRTC_GEN_CNTL_IOPort + 3);
                save->crtc_off_pitch = inl(CRTC_OFF_PITCH_IOPort);
        }

        if (Using_Small_Apertures)
                save->second_aperture_bank = second_aperture_bank;

        return ((void *) save);
}

/*
 * ATIInit --
 *
 * This is the most important function (after the Probe function).  This
 * function fills in the vgaATIRec with all of the register values needed to
 * enable a video mode.
 */
static Bool
ATIInit(DisplayModePtr mode)
{
        int saved_mode_flags, Clock;

        /*
         * Unlock registers.
         */
        ATIEnterLeave(ENTER);

        /*
         * ATI adapters have a bit that multiplies all vertical timing values
         * by 2.  This feature is only used if it's actually needed (i.e. when
         * VTotal > 1024).  If the feature is needed, fake out an interlaced
         * mode and let vgaHWInit divide things by two.  Note that this
         * prevents the (incorrect) use of this feature with interlaced modes.
         */
        saved_mode_flags = mode->Flags;
        if (mode->VTotal > 1024)
        {
                mode->Flags |= V_INTERLACE;
                vgaInterlaceType = VGA_DIVIDE_VERT;
        }

        /* Set up default horizontal display enable skew */
        if ((ATIChip >= ATI_CHIP_28800_2) && (ATIChip <= ATI_CHIP_28800_6) &&
            !(mode->Flags & V_HSKEW))
        {
                /*
                 * Modes using the higher clock frequencies need a non-zero
                 * Display Enable Skew.  The following number has been
                 * empirically determined to be somewhere between 4.2 and 4.7
                 * MHz.
                 */
#               define Display_Enable_Skew_Threshold 4500

                /* Set a reasonable default Display Enable Skew */
                mode->HSkew = mode->CrtcHSkew =
                        vga256InfoRec.clock[mode->Clock] /
                                Display_Enable_Skew_Threshold;
        }

        /*
         * This will allocate the data structure and initialize all of the
         * generic VGA registers.
         */
        if (!vgaHWInit(mode,sizeof(vgaATIRec)))
        {
                mode->Flags = saved_mode_flags;
                vgaInterlaceType = ATI.ChipInterlaceType;
                return (FALSE);
        }

        new->mode = mode;                       /* Link with mode line */
        vgaInterlaceType = ATI.ChipInterlaceType;

        /*
         * Override a few things.
         */
#       if !defined(MONOVGA) && !defined(XF86VGA16)
                new->std.Sequencer[4] = 0x0AU;  /* instead of 0x0EU */
                new->std.Attribute[16] = 0x01U; /* instead of 0x41U */
                new->std.CRTC[23] = 0xE3U;      /* instead of 0xC3U */
                if (ATIChip >= ATI_CHIP_264CT)
                        new->std.CRTC[19] = vga256InfoRec.displayWidth >> 3;
                else
                {
                        new->std.Graphics[5] = 0x00U;   /* instead of 0x40U */
                        if ((ATIChip <= ATI_CHIP_18800) &&
                            (vga256InfoRec.videoRam == 256))
                                new->std.CRTC[19] =
                                        vga256InfoRec.displayWidth >> 3;
                }
#       endif
        if (saved_mode_flags != mode->Flags)
        {
                /* Use "double vertical timings" bit */
                new->std.CRTC[23] |= 0x04U;
                mode->Flags = saved_mode_flags;
        }

        /*
         * Determine available clock.
         */
        if ((ATIProgrammableClock == ATI_CLOCK_FIXED) ||
            ((ATIProgrammableClock == ATI_CLOCK_CH8398) && (mode->Clock < 2)))
        {
                /*
                 * Use a fixed clock.
                 */
                Clock = mode->Clock;
                new->ClockProgramme = -1;
        }
        else
        {
                /* A macro for integer division with rounding */
#               define Divide(n, m)     (((n) + ((m) >> 1)) / (m))

                /*
                 * Generate clock programme word.
                 */
                int N, M, D;
                int BestN = ATIClockDescriptor->MaxN,
                    BestM = ATIClockDescriptor->MinM,
                    BestD = ATIClockDescriptor->MinD;
                int N1, UndividedClock, MinimumGap;
                int Frequency, Multiple;        /* Used as temporaries */

                /* Use units of 1kHz throughout */
                if (mode == ATI.ChipBuiltinModes)
                        UndividedClock = mode->SynthClock;
                else
                        UndividedClock = vga256InfoRec.clock[mode->Clock];

                /* Start with the maximum divider */
                for (D = 0;
                    (D < ATIClockDescriptor->MaxD) &&
                            (UndividedClock < 0x20000000U);
                    D++)
                        UndividedClock <<= 1;
                if (D >= ATIClockDescriptor->MinD)
                {
                        MinimumGap = UndividedClock;

                        /* Loop through denominators */
                        for (M = ATIClockDescriptor->MinM;
                             M <= ATIClockDescriptor->MaxM;
                             M++)
                        {
                                /* Set maximum frequency at this M */
                                Multiple = M * 11;
                                Frequency = Divide((ATIClockDescriptor->MaxN *
                                        157500) + 78750, Multiple);

                                /*
                                 * Adjust divider to bring frequency within
                                 * range.
                                 */
                                while ((D > ATIClockDescriptor->MinD) &&
                                       (UndividedClock > Frequency))
                                {
                                        UndividedClock >>= 1;
                                        MinimumGap = (MinimumGap + 1) >> 1;
                                        D--;
                                }

                                /*
                                 * Calculate closest N and apply its
                                 * restrictions.
                                 */
                                N = Divide(UndividedClock * Multiple, 157500);
                                if (N < ATIClockDescriptor->MinN)
                                        N = ATIClockDescriptor->MinN;
                                else if (N > ATIClockDescriptor->MaxN)
                                        N = ATIClockDescriptor->MaxN;
                                N -= ATIClockDescriptor->NAdjust;
                                N1 = (N / ATIClockDescriptor->N1) *
                                        ATIClockDescriptor->N2;
                                if (N > N1)
                                        N = ((N1 + ATIClockDescriptor->N1) /
                                                ATIClockDescriptor->N1) *
                                                ATIClockDescriptor->N1;
                                N += ATIClockDescriptor->NAdjust;
                                N1 += ATIClockDescriptor->NAdjust;

                                for (;  ;  N = N1)
                                {
                                        /* Pick the closest setting */
                                        Frequency =
                                                Divide(N * 157500, Multiple) -
                                                UndividedClock;
                                        if (Frequency < 0)
                                                Frequency = -Frequency;
                                        if (Frequency < MinimumGap)
                                        {
                                                /* Save settings */
                                                BestN = N;
                                                BestM = M;
                                                BestD = D;
                                                MinimumGap = Frequency;
                                        }

                                        if (N <= N1)
                                                break;
                                }
                        }
                }

                Multiple = BestM * 11;
                Frequency = BestN * 157500;
                if (BestD > 0)
                        Multiple <<= BestD;
                else if (BestD < 0)
                        Frequency <<= -BestD;
                UndividedClock = Divide(Frequency, Multiple);
                if (abs(UndividedClock - vga256InfoRec.clock[mode->Clock]) >
                    CLOCK_TOLERANCE)
                {
                        ErrorF("Unable to programme clock %.3fMHz for mode"
                               " %s.\n",
                                 vga256InfoRec.clock[mode->Clock] / 1000.0,
                                 mode->name);
                        return (FALSE);
                }
                vga256InfoRec.clock[mode->Clock] = UndividedClock;
                Clock = ATIClockNumberToProgramme;

                /* Build programme word */
                new->ClockProgramme =
                       (SetBits(BestN - ATIClockDescriptor->NAdjust,
                                ATIClockDescriptor->NMask) |
                        SetBits(BestM - ATIClockDescriptor->MAdjust,
                                ATIClockDescriptor->MMask) |
                        SetBits(BestD - ATIClockDescriptor->MinD,
                                ATIClockDescriptor->DMask) |
                        SetBits(Clock, ATIClockDescriptor->IMask)) ^
                        ATIClockDescriptor->XMask;
                if (xf86Verbose > 1)
                {
                        ErrorF("\nProgramming clock %d to %.3fMHz for mode"
                               " %s.\n", mode->Clock, UndividedClock / 1000.0,
                                mode->name);
                        ErrorF(" N=%d, M=%d, D=%d, L=%d, Programme=0x%06X.\n",
                                BestN, BestM, BestD, Clock,
                                new->ClockProgramme);
                }

#               undef Divide    /* Completeness */
        }

        if (Chip_Has_VGA_Wonder)
        {
                /* Set up ATI registers */
#               if defined(MONOVGA) || defined(XF86VGA16)
                        new->b0 = 0x00U;
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                if (vga256InfoRec.videoRam > 512)
                                        new->b0 |= 0x08U;
                                else if (vga256InfoRec.videoRam > 256)
                                        new->b0 |= 0x10U;
                        }
                        else
                        {
                                if (vga256InfoRec.videoRam > 256)
                                        new->b0 |= 0x08U;
                        }
#               else
                        new->b0 = 0x20U;
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                if (vga256InfoRec.videoRam > 512)
                                        new->b0 |= 0x08U;
                                else if (vga256InfoRec.videoRam > 256)
                                        new->b0 |= 0x10U;
                        }
                        else
                        {
                                if (vga256InfoRec.videoRam > 256)
                                        new->b0 |= 0x10U;
                                else
                                        new->b0 |= 0x06U;
                        }
#               endif
                new->b1 = (ATIGetExtReg(0xB1U) & 0x04U)       ;
                new->b3 = (ATIGetExtReg(0xB3U) & 0x20U)       ;
                new->b5 = 0U;
#               if defined(MONOVGA) || defined(XF86VGA16)
                        new->b6 = 0x40U;
#               else
                        new->b6 = 0x04U;
#               endif
                if ((ATIChip >= ATI_CHIP_28800_2) &&
                    (vga256InfoRec.videoRam > 256))
                        new->b6 |= 0x01U;
                new->b8 = (ATIGetExtReg(0xB8U) & 0xC0U)       ;
                new->b9 = (ATIGetExtReg(0xB9U) & 0x7FU)       ;
                if (ATIChip <= ATI_CHIP_18800)
                        new->ba = 0x08U;
                else
                        new->ba = 0U;
                new->bd = (ATIGetExtReg(0xBDU) & 0x02U)       ;
                if (ATIChip <= ATI_CHIP_18800)
                        new->b2 = (ATIGetExtReg(0xB2U) & 0xC0U)       ;
                else
                {
                        new->b2 = 0U;
                        new->be = (ATIGetExtReg(0xBEU) & 0x30U) | 0x09U;
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                new->bf = (ATIGetExtReg(0xBFU) & 0x5FU)        ;
                                new->a3 = (ATIGetExtReg(0xA3U) & 0x67U)        ;
                                new->a6 = (ATIGetExtReg(0xA6U) & 0x38U) | 0x04U;
                                new->a7 = (ATIGetExtReg(0xA7U) & 0xBEU)        ;
                                new->ab = (ATIGetExtReg(0xABU) & 0xE7U)        ;
                                new->ac = (ATIGetExtReg(0xACU) & 0x8EU)        ;
                                new->ad = 0U;
                                new->ae = (ATIGetExtReg(0xAEU) & 0xF0U)        ;
                        }
                }
                if (mode->Flags & V_INTERLACE)  /* Enable interlacing */
                        if (ATIChip <= ATI_CHIP_18800)
                                new->b2 |= 0x01U;
                        else
                                new->be |= 0x02U;
                if (mode->Flags & V_DBLSCAN)
                        new->b1 |= 0x08U;       /* Enable double scanning */
                if ((OFLG_ISSET(OPTION_CSYNC, &vga256InfoRec.options)) ||
                    (mode->Flags & (V_CSYNC | V_PCSYNC)))
                        new->bd |= 0x08U;       /* Enable composite synch */
                if (mode->Flags & V_NCSYNC)
                        new->bd |= 0x09U;       /* Invert csynch polarity */
                if (mode->CrtcHSkew > 0)
                if (mode->CrtcHSkew <= 3)
                        new->b5 |= 0x01U;
                else if (ATIChip >= ATI_CHIP_28800_2)
                switch ((mode->CrtcHSkew + 4) >> 3)
                {
                        case 1:         /* Use ATI override */
                                new->std.CRTC[3] &= ~0x60U;
                                new->b0 |= 0x01U;
                                break;
                        case 2:         /* Use ATI override */
                                new->std.CRTC[3] &= ~0x60U;
                                new->a6 |= 0x01U;
                                break;
                        case 4:
                                new->a7 |= 0x40U;
                                break;
                        case 5:
                                new->ac |= 0x10U;
                                break;
                        case 6:
                                new->ac |= 0x20U;
                                break;
                        default:
                                break;
                }
        }

        if (ATIChip >= ATI_CHIP_264CT)
        {
                new->crtc_gen_cntl0 = 0;
#if 0
                if (mode->Flags & V_DBLSCAN)
                        new->crtc_gen_cntl0 |= CRTC_DBL_SCAN_EN;
#endif
                if (mode->Flags & V_INTERLACE)
                        new->crtc_gen_cntl0 |= CRTC_INTERLACE_EN;
                if ((mode->Flags & (V_CSYNC | V_PCSYNC)) ||
                    (OFLG_ISSET(OPTION_CSYNC, &vga256InfoRec.options)))
                        new->crtc_gen_cntl0 |= CRTC_CSYNC_EN;
                new->crtc_gen_cntl3 = inb(CRTC_GEN_CNTL_IOPort + 3) &
                        (CRTC_VSYNC_FALL_EDGE >> (8*3));
#               if defined(MONOVGA) || defined(XF86VGA16)
                        new->crtc_gen_cntl3 |=
                                ((CRTC_EN | CRTC_CNT_EN) >> (8*3));
                        new->crtc_off_pitch =
                                SetBits(vga256InfoRec.displayWidth >> 4,
                                        CRTC_PITCH);
#               else
                        new->crtc_gen_cntl3 |=
                                ((CRTC_EN | CRTC_VGA_LINEAR | CRTC_CNT_EN) >>
                                        (8*3));
                        new->crtc_off_pitch =
                                SetBits(vga256InfoRec.displayWidth >> 3,
                                        CRTC_PITCH);
#               endif
        }

        if (Using_Small_Apertures)
        {
#               if defined(MONOVGA) || defined(XF86VGA16)
                        new->second_aperture_bank = 4;
#               else
                        new->second_aperture_bank = 1;
#               endif
        }

        /*
         * Set clock select bits, possibly remapping them.
         */
        new->std.NoClock = Clock;       /* Save pre-map clock number */
        Clock = ATIClockMap[Clock & 0x0FU] | (Clock & ~0x0FU);

        /*
         * Set generic clock select bits.
         */
        new->std.MiscOutReg = (new->std.MiscOutReg & 0xF3U) |
                ((Clock << 2) & 0x0CU);

        if (Chip_Has_VGA_Wonder)
        {
                /* Set ATI clock select bits */
                if (ATIChip <= ATI_CHIP_18800)
                        new->b2 = (new->b2 & 0xBFU) | ((Clock << 4) & 0x40U);
                else
                {
                        new->be = (new->be & 0xEFU) | ((Clock << 2) & 0x10U);
                        if (ATIAdapter != ATI_ADAPTER_V4)
                        {
                                Clock >>= 1;
                                new->b9 = (new->b9 & 0xFDU) |
                                        ((Clock >> 1) & 0x02U);
                        }
                }

                /* Set clock divider bits */
                new->b8 = (new->b8 & 0x3FU) | ((Clock << 3) & 0xC0U);
        }

        return (TRUE);
}

/*
 * ATIAdjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in video memory.  This is used to implement the virtual
 * window.
 */
static void
ATIAdjust(const unsigned int x, const unsigned int y)
{
        int Base = (y * vga256InfoRec.displayWidth + x) >> 3;

        /*
         * Unlock registers.
         */
        ATIEnterLeave(ENTER);

        if (ATIChip < ATI_CHIP_264CT)
        {
                PutReg(CRTX(vgaIOBase), 0x0CU, (Base & 0x00FF00U) >> 8);
                PutReg(CRTX(vgaIOBase), 0x0DU, (Base &   0x00FFU)     );

                if (Chip_Has_VGA_Wonder)
                {
                        if (ATIChip <= ATI_CHIP_18800_1)
                                ATIModifyExtReg(0xB0U, -1, 0x3FU, Base >> 10);
                        else
                        {
                                ATIModifyExtReg(0xB0U, -1, 0xBFU, Base >> 10);
                                ATIModifyExtReg(0xA3U, -1, 0xEFU, Base >> 13);

                                /*
                                 * I don't know if this also applies to
                                 * Mach64's, but give it a shot...
                                 */
                                if (ATIChip >= ATI_CHIP_68800)
                                        ATIModifyExtReg(0xADU, -1, 0xF3U,
                                                Base >> 16);
                        }
                }
        }
        else
        {
                /*
                 * On integrated controllers, there is only one CRTC which is
                 * simultaneously accessible through both VGA and accelerator
                 * I/O ports.  Given VGA's architectural limitations, setting
                 * the CRTC's offset register to more than 256k needs to be
                 * done through the accelerator port.
                 */
#               if defined(MONOVGA) || defined(XF86VGA16)
                        outl(CRTC_OFF_PITCH_IOPort,
                                SetBits(vga256InfoRec.displayWidth >> 4,
                                        CRTC_PITCH) |
                                SetBits(Base, CRTC_OFFSET));
#               else
                        outl(CRTC_OFF_PITCH_IOPort,
                                SetBits(vga256InfoRec.displayWidth >> 3,
                                        CRTC_PITCH) |
                                SetBits(Base << 1, CRTC_OFFSET));
#               endif
        }

#if defined(XFreeXDGA) && !defined(MONOVGA)
	if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
		/* Wait until vertical retrace is in progress. */
		while (inb(vgaIOBase + 0xA) & 0x08);
		while (!(inb(vgaIOBase + 0xA) & 0x08));
	}
#endif
}

/*
 * ATIGetMode --
 *
 * This function will read the current SVGA register settings and produce a
 * filled-in DisplayModeRec containing the current mode.
 */
static void
ATIGetMode(DisplayModePtr mode)
{
        int ShiftCount = 0;
        unsigned char misc;
        unsigned char crt00, crt01, crt03, crt04, crt05, crt06, crt07, crt09,
                crt10, crt11, crt12, crt17;
        unsigned char a6 = 0, a7 = 0, ac = 0,
                      b0 = 0, b1 = 0, b2 = 0, b5 = 0,
                      b8 = 0, b9 = 0, bd = 0, be = 0;
        unsigned char crtc_gen_cntl0 = 0;

        /*
         * Unlock registers.
         */
        ATIEnterLeave(ENTER);

        /* Initialize */
        mode->Clock = 0;

        /*
         * First, get the needed register values.
         */
        misc = inb(R_GENMO);
        vgaIOBase = ((misc & 0x01U) * (ColourIOBase - MonochromeIOBase)) +
                MonochromeIOBase;

        crt00 = GetReg(CRTX(vgaIOBase), 0x00U);
        crt01 = GetReg(CRTX(vgaIOBase), 0x01U);
        crt03 = GetReg(CRTX(vgaIOBase), 0x03U);
        crt04 = GetReg(CRTX(vgaIOBase), 0x04U);
        crt05 = GetReg(CRTX(vgaIOBase), 0x05U);
        crt06 = GetReg(CRTX(vgaIOBase), 0x06U);
        crt07 = GetReg(CRTX(vgaIOBase), 0x07U);
        crt09 = GetReg(CRTX(vgaIOBase), 0x09U);
        crt10 = GetReg(CRTX(vgaIOBase), 0x10U);
        crt11 = GetReg(CRTX(vgaIOBase), 0x11U);
        crt12 = GetReg(CRTX(vgaIOBase), 0x12U);
        crt17 = GetReg(CRTX(vgaIOBase), 0x17U);

        if (ATIChip >= ATI_CHIP_264CT)
                crtc_gen_cntl0 = inb(CRTC_GEN_CNTL_IOPort);

        if (Chip_Has_VGA_Wonder)
        {
                b0 = ATIGetExtReg(0xB0U);
                b1 = ATIGetExtReg(0xB1U);
                b5 = ATIGetExtReg(0xB5U);
                b8 = ATIGetExtReg(0xB8U);
                b9 = ATIGetExtReg(0xB9U);
                bd = ATIGetExtReg(0xBDU);
                if (ATIChip <= ATI_CHIP_18800)
                        b2 = ATIGetExtReg(0xB2U);
                else
                {
                        be = ATIGetExtReg(0xBEU);
                        if (ATIChip >= ATI_CHIP_28800_2)
                        {
                                a6 = ATIGetExtReg(0xA6U);
                                a7 = ATIGetExtReg(0xA7U);
                                ac = ATIGetExtReg(0xACU);
                        }
                }

                /* Set clock number */
                mode->Clock = (b8 & 0xC0U) >> 3;        /* Clock divider */
                if (ATIChip <= ATI_CHIP_18800)
                        mode->Clock |= (b2 & 0x40U) >> 4;
                else
                {
                        if (ATIAdapter != ATI_ADAPTER_V4)
                        {
                                mode->Clock |= (b9 & 0x02U) << 1;
                                mode->Clock <<= 1;
                        }
                        mode->Clock |= (be & 0x10U) >> 2;
                }
        }
        mode->Clock |= (misc & 0x0CU) >> 2;             /* VGA clock select */
        mode->Clock = ATIClockMap[mode->Clock & 0x0FU] |
                (mode->Clock & ~0x0FU);
        if (ATIProgrammableClock == ATI_CLOCK_FIXED)
                mode->SynthClock = vga256InfoRec.clock[mode->Clock];
        else
                mode->SynthClock = (ATIBIOSClocks[mode->Clock & 0x0FU] * 10) /
                        ((mode->Clock >> 4) + 1);

        /*
         * Set horizontal display end.
         */
        mode->CrtcHDisplay = mode->HDisplay = (crt01 + 1) << 3;

        /*
         * Set horizontal synch pulse start.
         */
        mode->CrtcHSyncStart = mode->HSyncStart = crt04 << 3;

        /*
         * Set horizontal synch pulse end.
         */
        crt05 = (crt04 & 0xE0U) | (crt05 & 0x1FU);
        if (crt05 <= crt04)
                crt05 += 0x20U;
        mode->CrtcHSyncEnd = mode->HSyncEnd = crt05 << 3;

        /*
         * Set horizontal total.
         */
        mode->CrtcHTotal = mode->HTotal = (crt00 + 5) << 3;

        /*
         * Set horizontal display enable skew.
         */
        mode->HSkew = (crt03 & 0x60U) >> 2;
        /* Assume ATI extended VGA registers override standard VGA */
        if (b5 & 0x01U)
                mode->HSkew = 1;
        if (b0 & 0x01U)
                mode->HSkew = 1 << 3;
        if (a6 & 0x01U)
                mode->HSkew = 2 << 3;
        if (a7 & 0x40U)
                mode->HSkew = 4 << 3;
        if (ac & 0x10U)
                mode->HSkew = 5 << 3;
        if (ac & 0x20U)
                mode->HSkew = 6 << 3;
        mode->CrtcHSkew = mode->HSkew;

        /*
         * Set vertical display end.
         */
        mode->CrtcVDisplay = mode->VDisplay =
                (((crt07 & 0x40U) << 3) | ((crt07 & 0x02U) << 7) | crt12) + 1;

        /*
         * Set vertical synch pulse start.
         */
        mode->CrtcVSyncStart = mode->VSyncStart =
                (((crt07 & 0x80U) << 2) | ((crt07 & 0x04U) << 6) | crt10);

        /*
         * Set vertical synch pulse end.
         */
        mode->VSyncEnd = (mode->VSyncStart & 0x3F0U) | (crt11 & 0x0FU);
        if (mode->VSyncEnd <= mode->VSyncStart)
                mode->VSyncEnd += 0x10U;
        mode->CrtcVSyncEnd = mode->VSyncEnd;

        /*
         * Set vertical total.
         */
        mode->CrtcVTotal = mode->VTotal =
                (((crt07 & 0x20U) << 4) | ((crt07 & 0x01U) << 8) | crt06) + 2;

        mode->CrtcVAdjusted = TRUE;

        /*
         * Set flags.
         */
        if (misc & 0x40U)
                mode->Flags = V_NHSYNC;
        else
                mode->Flags = V_PHSYNC;
        if (misc & 0x80U)
                mode->Flags |= V_NVSYNC;
        else
                mode->Flags |= V_PVSYNC;
        if (crt09 & 0x80U)
                mode->Flags |= V_DBLSCAN;
        if (mode->HSkew)
                mode->Flags |= V_HSKEW;
        if (Chip_Has_VGA_Wonder)
        {
                if (ATIChip <= ATI_CHIP_18800)
                {
                        if (b2 & 0x01U)
                                mode->Flags |= V_INTERLACE;
                }
                else
                {
                        if (be & 0x02U)
                                mode->Flags |= V_INTERLACE;
                }
                if (b1 & 0x08U)
                        mode->Flags |= V_DBLSCAN;
                if ((bd & 0x09U) == 0x09U)
                        mode->Flags |= V_NCSYNC;
                else if (bd & 0x08U)
                        mode->Flags |= V_PCSYNC;
        }
        if (ATIChip >= ATI_CHIP_264CT)
        {
                if (crtc_gen_cntl0 & CRTC_DBL_SCAN_EN)
                        mode->Flags |= V_DBLSCAN;
                if (crtc_gen_cntl0 & CRTC_INTERLACE_EN)
                        mode->Flags |= V_INTERLACE;
                if (crtc_gen_cntl0 & CRTC_CSYNC_EN)
                        mode->Flags |= V_PCSYNC;
        }

        /*
         * Adjust vertical timings.
         */
        if ((ATI.ChipInterlaceType == VGA_DIVIDE_VERT) &&
            (mode->Flags & V_INTERLACE))
                ShiftCount++;
        if (mode->Flags & V_DBLSCAN)
                ShiftCount--;
        if (b1 & 0x40U)
                ShiftCount--;
        if (crt17 & 0x04U)
                ShiftCount++;
        if (ShiftCount > 0)
        {
                mode->VDisplay <<= ShiftCount;
                mode->VSyncStart <<= ShiftCount;
                mode->VSyncEnd <<= ShiftCount;
                mode->VTotal <<= ShiftCount;
        }
        else if (ShiftCount < 0)
        {
                mode->VDisplay >>= -ShiftCount;
                mode->VSyncStart >>= -ShiftCount;
                mode->VSyncEnd >>= -ShiftCount;
                mode->VTotal >>= -ShiftCount;
        }
}

/*
 * ATISaveScreen --
 *
 * This performs a sequencer reset.
 */
static void
ATISaveScreen(const Bool start)
{
        static Bool started = SS_FINISH;

        if (start == started)
                return;
        started = start;

        if (start == SS_START)                  /* Start synchronous reset */
                PutReg(SEQX, 0x00U, 0x02U);
        else                                    /* End synchronous reset */
                PutReg(SEQX, 0x00U, 0x03U);
}

/*
 * ATIValidMode --
 *
 * This is only a dummy place-holder for now.
 */
static int
ATIValidMode(DisplayModePtr mode, Bool verbose)
{
        return (MODE_OK);
}
