/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo_driver.c,v 1.1.2.5 1999/06/18 13:08:27 hohndel Exp $ */
/**********************************************************************
Copyright 1998 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/

/*
 * This Precision Insight driver has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
 */

/*
 * These are X and server generic header files.
 */
#include "X.h"
#include "input.h"
#include "screenint.h"

/*
 * These are XFree86-specific header files.
 */
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"
extern vgaHWCursorRec vgaHWCursor;

/*
 * For PCI probing etc.
 */
#include "xf86_PCI.h"
#include "vgaPCI.h"
extern vgaPCIInformation *vgaPCIInfo;

/*
 * If the driver makes use of XF86Config 'Option' flags, the following
 * will be required.
 */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif


/*
 * Driver data structures.
 */
#include "neo.h"
#include "neo_reg.h"
#include "neo_macros.h"

#define NEO_EXT_CR_MAX 0x85
#define NEO_EXT_GR_MAX 0xC7

typedef struct {
    unsigned char CR[NEO_EXT_CR_MAX+1];
    unsigned char GR[NEO_EXT_GR_MAX+1];
} regSaveRec, *regSavePtr;

typedef struct {
    vgaHWRec std;               /* good old IBM VGA */
    unsigned char GeneralLockReg;
    unsigned char ExtCRTDispAddr;
    unsigned char ExtCRTOffset;
    unsigned char SysIfaceCntl1;
    unsigned char SysIfaceCntl2;
    unsigned char ExtColorModeSelect;
    unsigned char SingleAddrPage;
    unsigned char DualAddrPage;
    unsigned char PanelDispCntlReg1;
    unsigned char PanelDispCntlReg2;
    unsigned char PanelDispCntlReg3;
    unsigned char PanelVertCenterReg1;
    unsigned char PanelVertCenterReg2;
    unsigned char PanelVertCenterReg3;
    unsigned char PanelVertCenterReg4;
    unsigned char PanelVertCenterReg5;
    unsigned char PanelHorizCenterReg1;
    unsigned char PanelHorizCenterReg2;
    unsigned char PanelHorizCenterReg3;
    unsigned char PanelHorizCenterReg4;
    unsigned char PanelHorizCenterReg5;
    Bool ProgramVCLK;
    unsigned char VCLK3NumeratorLow;
    unsigned char VCLK3NumeratorHigh;
    unsigned char VCLK3Denominator;
    unsigned char VerticalExt;
    regSavePtr reg;
} vgaNeoRec, *vgaNeoPtr;


/* Globals that are allocated in this module. */
unsigned char* NeoMMIOBase = NULL;
int NeoFifoCount = 0;
int NeoChipset = -1;
int NeoPanelWidth = 0;
int NeoPanelHeight = 0;
int NeoMMIOAddr = 0;
int NeoLinearAddr = 0;
int NeoCursorMemSegment = 0;
int NeoCursorOffset = 0;
regSaveRec NeoSaveRegs;

/*
 * Forward definitions for the functions that make up the driver.  See
 * the definitions of these functions for the real scoop.
 */
static Bool     NeoProbe();
static char *   NeoIdent();
static Bool     NeoClockSelect();
static void     NeoEnterLeave();
static Bool     NeoInit();
static int	NeoValidMode();
static void *   NeoSave();
static void     NeoRestore();
static void     NeoAdjust();
static void	NeoFbInit();
static void	NeoDisplayPowerManagementSet();

/*
 * These are the bank select functions.  There are defined in neo_bank.s
 */
extern void     NeoSetRead();
extern void     NeoSetWrite();
extern void     NeoSetReadWrite();

/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data 
 * that defines how the driver operates.
 */
vgaVideoChipRec NEO = {
    /* 
     * Function pointers
     */
    NeoProbe,
    NeoIdent,
    NeoEnterLeave,
    NeoInit,
    NeoValidMode,
    NeoSave,
    NeoRestore,
    NeoAdjust,
    vgaHWSaveScreen,
    (void (*)())NoopDDA,
    NeoFbInit,
    NeoSetRead,
    NeoSetWrite,
    NeoSetReadWrite,
    /*
     * This is the size of the mapped memory window, usually 64k.
     */
    0x10000,		
    /*
     * This is the size of a video memory bank for this chipset.
     */
    0x10000,
    /*
     * This is the number of bits by which an address is shifted
     * right to determine the bank number for that address.
     */
    16,
    /*
     * This is the bitmask used to determine the address within a
     * specific bank.
     */
    0xFFFF,
    /*
     * These are the bottom and top addresses for reads inside a
     * given bank.
     */
    0x00000, 0x10000,
    /*
     * And corresponding limits for writes.
     */
    0x00000, 0x10000,
    /*
     * Whether this chipset supports a single bank register or
     * seperate read and write bank registers.  Almost all chipsets
     * support two banks, and two banks are almost always faster
     * (Trident 8900C and 9000 are odd exceptions).
     */
    TRUE,
    /*
     * If the chipset requires vertical timing numbers to be divided
     * by two for interlaced modes, set this to VGA_DIVIDE_VERT.
     */
    VGA_NO_DIVIDE_VERT,
    /*
     * This is a dummy initialization for the set of option flags
     * that this driver supports.  It gets filled in properly in the
     * probe function, if the probe succeeds (assuming the driver
     * supports any such flags).
     */
    {0,},
    /*
     * This determines the multiple to which the virtual width of
     * the display must be rounded for the 256-color server.  This
     * will normally be 8, but may be 4 or 16 for some servers.
     */
    8,
    /*
     * If the driver includes support for a linear-mapped frame buffer
     * for the detected configuratio this should be set to TRUE in the
     * Probe or FbInit function.  In most cases it should be FALSE.
     */
    FALSE,
    /*
     * This is the physical base address of the linear-mapped frame
     * buffer (when used).  Set it to 0 when not in use.
     */
    0,
    /*
     * This is the size  of the linear-mapped frame buffer (when used).
     * Set it to 0 when not in use.
     */
    0,
    /*
     * This is TRUE if the driver has support for 16bpp for the detected
     * configuration. It must be set in the Probe function.
     * It most cases it should be FALSE.
     */
    FALSE,
    /*
     * This is TRUE if the driver has support for 24bpp for the detected
     * configuration.
     */
    FALSE,
    /*
     * This is TRUE if the driver has support for 32bpp for the detected
     * configuration.
     */
    FALSE,
    /*
     * This is a pointer to a list of builtin driver modes.
     * This is rarely used, and in must cases, set it to NULL
     */
    NULL,
    /*
     * ClockMulFactor can be used to scale the raw clocks to pixel
     * clocks. This is rarely used, and in most cases, set it to 1. The
     * "raw" pixel clock from the modeline is multiplied by this value,
     * and that value is used for clock selection.
     */
    1,
    /*
     * ClockDivFactor can be used to scale the raw clocks to pixel
     * clocks. This is rarely used, and in most cases, set it to 1. The
     * "raw" pixel clock from the modeline is divided by this value
     * (after having been multiplied by ClockMulFactor), and that value
     * is used for clock selection.
     *
     * When these values are set to a non-1 value, the server will
     * automatically scale pixel clocks (if a discrete clock generator
     * is used) and maximum allowed pixel clock. If e.g. a 16bpp mode
     * requires pixel clock multipled by 2 (to be able to transport data
     * over an 8-bit data bus), setting ClockMulFactor to 2 will
     * automatically scale the maximum allowed ramdac speed with the
     * same factor, and use a clock*2 for the pixel data transfer clock.
     *
     * ClockMulFactor and ClockDivFactor can only be used when the
     * "rules" don't change over time: dynamic changes of these
     * variables will cause inconsistent server behaviour (e.g. it will
     * report wrong pixel clocks and maximum pixel speeds. Pixel
     * multiplexing modes for example can mostly only be enabled if the
     * pixel clock is higher than a certain value, and these modes
     * cannot use ClockDivFactor (they don't need to: the maximum RAMDAC
     * speed remains the same, so there's no use in the server code
     * lowering it).
     *
     * For 24bpp modes over a 16-bit RAMDAC bus, one would set
     * ClockMulFactor to 3, and ClockDivFactor to 2.
     */
    1
};

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */
#define new ((vgaNeoPtr)vgaNewVideoState)


/*
 * NeoIdent --
 *
 * Returns the string name for supported chipset 'n'.  Most drivers only
 * support one chipset, but multiple version may require that the driver
 * identify them individually (e.g. the Trident driver).  The Ident function
 * should return a string if 'n' is valid, or NULL otherwise.  The
 * server will call this function when listing supported chipsets, with 'n' 
 * incrementing from 0, until the function returns NULL.  The 'Probe'
 * function should call this function to get the string name for a chipset
 * and when comparing against an XF86Config-supplied chipset value.  This
 * cuts down on the number of places errors can creep in.
 */
static char *
NeoIdent(n)
int n;
{
    static char *chipsets[] =
    {
	"NM2070",
	"NM2090",
	"NM2093",
	"NM2097",
	"NM2160",
	"NM2200"
    };

    if (n + 1 > sizeof(chipsets) / sizeof(char *))
	return(NULL);
    else
	return(chipsets[n]);
}

/*
 * NeoClockSelect --
 * 
 * This function selects the dot-clock with index 'no'.  In most cases
 * this is done my setting the correct bits in various registers (generic
 * VGA uses two bits in the Miscellaneous Output Register to select from
 * 4 clocks).  Care must be taken to protect any other bits in these
 * registers by fetching their values and masking off the other bits.
 *
 * This function returns FALSE if the passed index is invalid or if the
 * clock can't be set for some reason.
 */
static Bool
NeoClockSelect(no)
int no;
{
    static unsigned char save1;
    unsigned char temp;

    switch(no) {
    case CLK_REG_SAVE:
	/*
	 * Here all of the registers that can be affected by
	 * clock setting should be saved into static variables.
	 */
	save1 = inb(0x3CC);
	/* Any extended registers would go here */
	break;
    case CLK_REG_RESTORE:
	/*
	 * Here all the previously saved registers are restored.
	 */
	outb(0x3C2, save1);
	/* Any extended registers would go here */
	break;
    default:
	/* 
	 * These are the generic two low-order bits of the clock select 
	 */
	temp = inb(0x3CC);
	outb(0x3C2, ( temp & 0xF3) | ((no << 2) & 0x0C));

	/* 
	 * Here is where the high order bit(s) supported by the chipset 
	 * are set.  This is done by fetching the appropriate register,
	 * masking off bits that won't be changing, then shifting and
	 * masking 'no' to set the bits as appropriate.
	 */
    }
    return(TRUE);
}


/*
 * NeoProbe --
 *
 * This is the function that makes a yes/no decision about whether or not
 * a chipset supported by this driver is present or not.  The server will
 * call each driver's probe function in sequence, until one returns TRUE
 * or they all fail.
 *
 * Pretty much any mechanism can be used to determine the presence of the
 * chipset.  If there is a BIOS signature (e.g. ATI, GVGA), it can be read
 * via /dev/mem on most OSs, but some OSs (e.g. Mach) require special
 * handling, and others (e.g. Amoeba) don't allow reading  the BIOS at
 * all.  Hence, this mechanism is discouraged, if other mechanisms can be
 * found.  If the BIOS-reading mechanism must be used, examine the ATI and
 * GVGA drivers for the special code that is needed.  Note that the BIOS 
 * base should not be assumed to be at 0xC0000 (although most are).  Use
 * 'vga256InfoRec.BIOSbase', which will pick up any changes the user may
 * have specified in the XF86Config file.
 *
 * The preferred mechanism for doing this is via register identification.
 * It is important not only the chipset is detected, but also to
 * ensure that other chipsets will not be falsely detected by the probe
 * (this is difficult, but something that the developer should strive for).  
 * For testing registers, there are a set of utility functions in the 
 * "compiler.h" header file.  A good place to find example probing code is
 * in the SuperProbe program, which uses algorithms from the "vgadoc2.zip"
 * package (available on most PC/vga FTP mirror sites, like ftp.uu.net and
 * wuarchive.wustl.edu).
 *
 * Once the chipset has been successfully detected, then the developer needs 
 * to do some other work to find memory, and clocks, etc, and do any other
 * driver-level data-structure initialization may need to be done.
 */
static Bool
NeoProbe()
{
    pciConfigPtr pcr;
    pciConfigPtr neopcr = NULL;
    int i, w;
    unsigned short id;

    /*
     * Set up I/O ports to be used by this card.
     */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

    /*
     * First we attempt to figure out if one of the supported chipsets
     * is present.
     */
    NeoChipset = -1;

    if (vga256InfoRec.chipset) {
	/*
	 * This is the easy case.  The user has specified the
	 * chipset in the XF86Config file.  All we need to do here
	 * is a string comparison against each of the supported
	 * names available from the Ident() function.  If this
	 * driver supports more than one chipset, there would be
	 * nested conditionals here (see the Trident and WD drivers
	 * for examples).
	 */
	char *chipset;
	for (i = 0; (chipset = NeoIdent(i)); i++) {
	    if (!StrCaseCmp(vga256InfoRec.chipset, chipset)) {
		switch (i) {
		case 0 :
		    NeoChipset = PCI_CHIP_NM2070;
		    break;
		case 1 :
		    NeoChipset = PCI_CHIP_NM2090;
		    break;
		case 2 :
		    NeoChipset = PCI_CHIP_NM2093;
		    break;
		case 3 :
		    NeoChipset = PCI_CHIP_NM2097;
		    break;
		case 4 :
		    NeoChipset = PCI_CHIP_NM2160;
		    break;
		case 5 :
		    NeoChipset = PCI_CHIP_NM2200;
		    break;
		}
		break;
	    }
	}

	if (NeoChipset == -1)
	    return (FALSE);

	if (vga256InfoRec.chipID &&
	    (vga256InfoRec.chipID != NeoChipset)) {
	    FatalError("\nNeoMagic chipset (0x%04X) and chipID (0x%04X) "
		       "don't agree.\n",
		       NeoChipset,
		       vga256InfoRec.chipID);
	}
    }

    /*
     * Start with PCI probing, this should get us quite far already.
     */
    i = 0;
    if (vgaPCIInfo && vgaPCIInfo->AllCards) {
	while (pcr = vgaPCIInfo->AllCards[i++]) {
	    if ((pcr->_vendor == PCI_VENDOR_NEOMAGIC) &&
	        (pcr->_command & PCI_CMD_IO_ENABLE) &&
		(pcr->_command & PCI_CMD_MEM_ENABLE)) {

		/* This logic is broken for a multifunction device like the 
                 * NM2200.  Hack around the NM2200 for now by ignoring the 
                 * audio device.  Eventually, this could should be rewritten
                 * to handle multifunction pci devices.  JO, 9 Sept 98
                 */
		if (pcr->_device == 0x8005) continue;

		if (vga256InfoRec.chipID) {
		    ErrorF("%s %s: NeoMagic chipset override, using ChipID "
			   "value (0x%04X) instead of PCI value (0x%04X)\n",
			   XCONFIG_GIVEN,
			   vga256InfoRec.name,
			   vga256InfoRec.chipID,
			   pcr->_device);
		    id = vga256InfoRec.chipID;
		} else if (NeoChipset != -1) {
		    ErrorF("%s %s: NeoMagic chipset override, using %s "
			   "chipset (0x%04X) instead of PCI value (0x%04X)\n",
			   XCONFIG_GIVEN,
			   vga256InfoRec.name,
			   vga256InfoRec.chipset,
			   NeoChipset,
			   pcr->_device);
		    id = NeoChipset;
		} else {
		    id = pcr->_device;
		}

		if (vga256InfoRec.MemBase) {
		    NeoLinearAddr = vga256InfoRec.MemBase;
		} else {
		    NeoLinearAddr = vgaPCIInfo->MemBase;
		}

		switch(id) {
		case PCI_CHIP_NM2070 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(0);
		    NeoMMIOAddr = NeoLinearAddr + 0x100000;
		    break;
		case PCI_CHIP_NM2090 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(1);
		    NeoMMIOAddr = NeoLinearAddr + 0x200000;
		    break;
		case PCI_CHIP_NM2093 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(2);
		    NeoMMIOAddr = NeoLinearAddr + 0x200000;
		    break;
		case PCI_CHIP_NM2097 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(3);
		    NeoMMIOAddr = pcr->_base1;
		    break;
		case PCI_CHIP_NM2160 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(4);
		    NeoMMIOAddr = pcr->_base1;
		    break;
		case PCI_CHIP_NM2200 :
		    NeoChipset = id;
		    vga256InfoRec.chipset = NeoIdent(5);
		    NeoMMIOAddr = pcr->_base1;
		    break;
		}
	    }
	}
    } else {
	/*
	 * Assume we are on a VLB.
	 */
	int addr;

	xf86EnableIOPorts(vga256InfoRec.scrnIndex);
	vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
	outw(GRAX, 0x2609); /* Unlock NeoMagic registers */

	outb(vgaIOBase + 4, 0x1A);
	id = inb(vgaIOBase + 5);

	outb(GRAX, 0x13);
	addr = inb(GRAX+1);

	outw(GRAX, 0x0009); /* Lock NeoMagic registers */
	xf86DisableIOPorts(vga256InfoRec.scrnIndex);

	if (vga256InfoRec.chipID) {
	    ErrorF("%s %s: NeoMagic chipset override, using ChipID "
		   "value (0x%04X) instead of probed value (0x%04X)\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.chipID,
		   id);
	    id = vga256InfoRec.chipID;
	} else if (NeoChipset != -1) {
	    ErrorF("%s %s: NeoMagic chipset override, using %s "
		   "chipset (0x%04X) instead of probed value (0x%04X)\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
		   NeoChipset,
		   id);
	    /* Brute force way to do this, but better to be generic. */
	    switch (NeoChipset) {
	    case PCI_CHIP_NM2070 :
		id = PROBED_NM2070;
		break;
	    case PCI_CHIP_NM2090 :
		id = PROBED_NM2090;
		break;
	    case PCI_CHIP_NM2093 :
		id = PROBED_NM2093;
		break;
	    case PCI_CHIP_NM2097 :
		id = PROBED_NM2097;
		break;
	    case PCI_CHIP_NM2160 :
		id = PROBED_NM2160;
		break;
	    case PCI_CHIP_NM2200 :
		id = PROBED_NM2200;
		break;
	    }
	}

	if (vga256InfoRec.MemBase) {
	    NeoLinearAddr = vga256InfoRec.MemBase;
	} else {
	    NeoLinearAddr = addr << 20;
	}
	NeoMMIOAddr = NeoLinearAddr + 0x100000;

	switch (id) {
	case PROBED_NM2070 :
	    NeoChipset = PCI_CHIP_NM2070;
	    vga256InfoRec.chipset = NeoIdent(0);
	    break;
	case PROBED_NM2090 :
	    NeoChipset = PCI_CHIP_NM2090;
	    vga256InfoRec.chipset = NeoIdent(1);
	    break;
	case PROBED_NM2093 :
	    NeoChipset = PCI_CHIP_NM2093;
	    vga256InfoRec.chipset = NeoIdent(2);
	    break;
	case PROBED_NM2097 :
	    /* NM2097 does not have a VLB interface. */
	    return(FALSE);
	    break;
	case PROBED_NM2160 :
	    /* NM2160 does not have a VLB interface. */
	    return(FALSE);
	    break;
	case PROBED_NM2200 :
	    /* NM2160 does not have a VLB interface. */
	    return(FALSE);
	    break;
	default :
	    /* Cannot find a NeoMagic chip, so return failure. */
	    return(FALSE);
	}
    }

    if (NeoChipset == -1) {
	if (vga256InfoRec.chipset)
	    ErrorF("%s %s: NeoMagic: Unknown chipset\n",
		   XCONFIG_PROBED, vga256InfoRec.name);
	return(FALSE);
    }

    /*
     * Okay, it's a NeoMagic chip.
     */

    /* Check for the MMIO base in the XF86Config file. */
    if (vga256InfoRec.IObase) {
	NeoMMIOAddr = vga256InfoRec.IObase;
    }

    /*
     * If the user has specified the amount of memory in the XF86Config
     * file, we respect that setting.
     */
    if (!vga256InfoRec.videoRam) {
	/*
	 * Otherwise, do whatever chipset-specific things are 
	 * necessary to figure out how much memory (in kBytes) is 
	 * available.
	 */
	switch(NeoChipset) {
	case PCI_CHIP_NM2070 :
	    vga256InfoRec.videoRam = 896;
	    break;
	case PCI_CHIP_NM2090 :
	case PCI_CHIP_NM2093 :
	case PCI_CHIP_NM2097 :
	    vga256InfoRec.videoRam = 1152;
	    break;
	case PCI_CHIP_NM2160 :
	    vga256InfoRec.videoRam = 2048;
	    break;
	case PCI_CHIP_NM2200 :
	    vga256InfoRec.videoRam = 2560;
	    break;
	}
    }

    /*
     * If the user specified a ramdac speed in the XF86Config file, we
     * respect that setting.
     */
    if (vga256InfoRec.dacSpeeds[0]) {
	vga256InfoRec.maxClock = vga256InfoRec.dacSpeeds[0];
    } else {
	switch(NeoChipset) {
	case PCI_CHIP_NM2070 :
	    vga256InfoRec.maxClock = 65000;
	    break;
	case PCI_CHIP_NM2090 :
	case PCI_CHIP_NM2093 :
	case PCI_CHIP_NM2097 :
	    vga256InfoRec.maxClock = 80000;
	    break;
	case PCI_CHIP_NM2160 :
	    vga256InfoRec.maxClock = 90000;
	    break;
	case PCI_CHIP_NM2200 :
	    vga256InfoRec.maxClock = 110000;
	    break;
	}
    }

    /*
     * Last we fill in the remaining data structures.  We set
     * a boolean for whether or not this driver supports banking
     * for the Monochrome server.  And we set up a list of all
     * the option flags that this driver can make use of.
     */
    vga256InfoRec.bankedMono = FALSE;

#ifdef XFreeXDGA
    vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

    /* Initialize the depths possible for each chip. */
    switch(NeoChipset) {
    case PCI_CHIP_NM2070 :
	NEO.ChipHas16bpp = TRUE;
	break;
    case PCI_CHIP_NM2090 :
    case PCI_CHIP_NM2093 :
    case PCI_CHIP_NM2097 :
    case PCI_CHIP_NM2160 :
    case PCI_CHIP_NM2200 :
	NEO.ChipHas16bpp = TRUE;
	NEO.ChipHas24bpp = TRUE;
	break;
    }

    /* allowed options */
    OFLG_SET(OPTION_LINEAR, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_NOLINEAR_MODE, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_NOACCEL, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_SW_CURSOR, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_HW_CURSOR, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_MMIO, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_NO_MMIO, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_INTERN_DISP, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_EXTERN_DISP, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_LCD_CENTER, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_LCD_STRETCH, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_PCI_BURST_ON, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_PCI_BURST_OFF, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_PROG_LCD_MODE_REGS, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_NO_PROG_LCD_MODE_REGS, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_PROG_LCD_MODE_STRETCH, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_NO_PROG_LCD_MODE_STRETCH, &NEO.ChipOptionFlags);
    OFLG_SET(OPTION_OVERRIDE_VALIDATE_MODE, &NEO.ChipOptionFlags);
    
    if (NeoChipset == PCI_CHIP_NM2070 &&
	OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
	FatalError(
	    "\nOption \"nolinear\" does not work for the NM2070.\n");

    /* Make sure users don't set mutually exclusive options!! */
    if (OFLG_ISSET(OPTION_LINEAR, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"linear\" and \"nolinear\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"sw_cursor\" and \"hw_cursor\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_MMIO, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_NO_MMIO, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"mmio\" and \"no_mmio\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_PCI_BURST_OFF, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"pci_burst_on\" and \"pci_burst_off\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_PROG_LCD_MODE_REGS, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_NO_PROG_LCD_MODE_REGS, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"prog_lcd_mode_regs\" and \"no_prog_lcd_mode_regs\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_PROG_LCD_MODE_STRETCH, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_NO_PROG_LCD_MODE_STRETCH, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"prog_lcd_mode_stretch\" and \"no_prog_lcd_mode_stretch\" are incompatible.\n");

    /* 
     * if your driver uses a programmable clockchip, you have
     * to set this option to avoid clock probing etc.
     */
    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

    /* Enable IO ports, etc.*/
    NeoEnterLeave(ENTER);

#ifdef DPMSExtension
    if (NeoChipset != PCI_CHIP_NM2070)
	vga256InfoRec.DPMSSet = NeoDisplayPowerManagementSet;
#endif

    /* Determine panel width -- used in NeoValidMode. */
    outb(GRAX, 0x20);
    w = inb(GRAX+1);
    switch ((w & 0x18) >> 3) {
    case 0x00 :
	NeoPanelWidth  = 640;
	NeoPanelHeight = 480;
	break;
    case 0x01 :
	NeoPanelWidth  = 800;
	NeoPanelHeight = 600;
	break;
    case 0x02 :
	NeoPanelWidth  = 1024;
	NeoPanelHeight = 768;
	break;
    case 0x03 :
        /* 1280x1024 panel support needs to be added */
#ifdef NOT_DONE
	NeoPanelWidth  = 1280;
	NeoPanelHeight = 1024;
	break;
#else
	FatalError("\nOnly 640x480,\n"
                     "     800x600,\n"
                     " and 1024x768 panels are currently supported\n");
#endif
    default :
	NeoPanelWidth  = 640;
	NeoPanelHeight = 480;
	break;
    }

    /* Save all of the CRT and GRA registers */
    for (i = 0x00; i <= NEO_EXT_CR_MAX; i++) {
	outb(vgaIOBase + 4, i); NeoSaveRegs.CR[i] = inb(vgaIOBase + 5);
    }
    for (i = 0x00; i <= NEO_EXT_GR_MAX; i++) {
	outb(vgaIOBase + 4, i); NeoSaveRegs.GR[i] = inb(vgaIOBase + 5);
    }

    /* Block out chips that we currently don't support. */
    if ((NeoChipset != PCI_CHIP_NM2070) &&
        (NeoChipset != PCI_CHIP_NM2090) &&
        (NeoChipset != PCI_CHIP_NM2093) &&
        (NeoChipset != PCI_CHIP_NM2097) &&
        (NeoChipset != PCI_CHIP_NM2160) &&
        (NeoChipset != PCI_CHIP_NM2200)) {
	FatalError(
	    "\nOnly the NM2200 (MagicMedia 256AV),\n"
	      "         NM2160 (MagicGraph 128XD),\n"
              "         NM2097 (MagicGraph 128ZV+),\n"
              "         NM2093 (MagicGraph 128ZV),\n"
              "         NM2090 (MagicGraph 128V),\n"
	      " and the NM2070 (MagicGraph 128) are currently supported.\n");
    }

    return(TRUE);
}

/*
 * NeoEnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down.  Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device.  This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
NeoEnterLeave(enter)
Bool enter;
{
    unsigned char temp;

#ifdef XFreeXDGA
    if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter) {
	if (vgaHWCursor.Initialized) {
	    /* Turn off hardware cursor */
	    NeoHideCursor();
	}
	return;
    }
#endif

    if (enter)
    {
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);

	/* 
	 * This is a global.  The CRTC base address depends on
	 * whether the VGA is functioning in color or mono mode.
	 * This is just a convenient place to initialize this
	 * variable.
	 */
	vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

	outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0x7F);
	outw(GRAX, 0x2609);  
    }
    else
    {
	/*
	 * Here undo what was done above.
	 */
	if (vgaHWCursor.Initialized) {
	    /* Turn off hardware cursor */
	    NeoHideCursor();
	}

	outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);
	outw(GRAX, 0x0009);  

	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

/*
 * NeoCalcVCLK --
 *
 * Determine the closest clock frequency to the one requested.
 */
#define REF_FREQ 14.31818
#define MAX_N 127
#define MAX_D 31
#define MAX_F 1

static void
NeoCalcVCLK(freq)
long freq;
{
    int n, d, f;
    double f_out;
    double f_diff;
    int n_best = 0, d_best = 0, f_best = 0;
    double f_best_diff = 999999.0;
    double f_target = freq/1000.0;

    for (f = 0; f <= MAX_F; f++)
	for (n = 0; n <= MAX_N; n++)
	    for (d = 0; d <= MAX_D; d++) {
		f_out = (n+1.0)/((d+1.0)*(1<<f))*REF_FREQ;
		f_diff = abs(f_out-f_target);
		if (f_diff < f_best_diff) {
		    f_best_diff = f_diff;
		    n_best = n;
		    d_best = d;
		    f_best = f;
		}
	    }

    if (NeoChipset == PCI_CHIP_NM2200) {
        /* NOT_DONE:  We are trying the full range of the 2200 clock.
           We should be able to try n up to 2047 */
	new->VCLK3NumeratorLow  = n_best;
	new->VCLK3NumeratorHigh = (f_best << 7);
    }
    else {
	new->VCLK3NumeratorLow  = n_best | (f_best << 7);
    }
    new->VCLK3Denominator = d_best;
}

/*
 * NeoProgramShadowRegs
 *
 * Setup the shadow registers to their default values.  The NeoSave
 * routines will restore the proper values on server exit.
 */
static void
NeoProgramShadowRegs(regs)
vgaNeoPtr regs;
{
    int i;
    Bool noProgramShadowRegs;

    /*
     * Convoluted logic for shadow register programming.
     *
     * As far as we know, shadow programming is needed for the 2070,
     * but not in stretched modes.  Special case this code.
     */
    switch (NeoChipset) {
    case PCI_CHIP_NM2070:
	/* Program the shadow regs by default */
	noProgramShadowRegs = FALSE;
	if (OFLG_ISSET(OPTION_NO_PROG_LCD_MODE_REGS, &vga256InfoRec.options))
	    noProgramShadowRegs = TRUE;

	if (regs->PanelDispCntlReg2 & 0x84) {
	    /* Don't program by default if in stretch mode */
	    noProgramShadowRegs = TRUE;
	    if (OFLG_ISSET(OPTION_PROG_LCD_MODE_STRETCH, &vga256InfoRec.options))
		noProgramShadowRegs = FALSE;
	}
	break;
    case PCI_CHIP_NM2090:
    case PCI_CHIP_NM2093:
    case PCI_CHIP_NM2097:
    case PCI_CHIP_NM2160:
    case PCI_CHIP_NM2200:
    default:
	/* Don't program the shadow regs by default */
	noProgramShadowRegs = TRUE;
	if (OFLG_ISSET(OPTION_PROG_LCD_MODE_REGS, &vga256InfoRec.options))
	    noProgramShadowRegs = FALSE;

	if (regs->PanelDispCntlReg2 & 0x84) {
	    /* Only change the behavior if an option is set */
	    if (OFLG_ISSET(OPTION_PROG_LCD_MODE_STRETCH, &vga256InfoRec.options))
		noProgramShadowRegs = FALSE;
	    else if (OFLG_ISSET(OPTION_NO_PROG_LCD_MODE_STRETCH, &vga256InfoRec.options))
		noProgramShadowRegs = TRUE;
	}
	break;
    }

    if (noProgramShadowRegs) {
	for (i = 0x40; i <= 0x59; i++) {
	    outb(vgaIOBase + 4, i); outb(vgaIOBase + 5, NeoSaveRegs.CR[i]);
	}
	for (i = 0x60; i <= 0x64; i++) {
	    outb(vgaIOBase + 4, i); outb(vgaIOBase + 5, NeoSaveRegs.CR[i]);
	}
    } else {
	/*
	 * Program the shadow regs based on the panel width.  This works
	 * fine for normal sized panels, but what about the odd ones like
	 * the Libretto 100 which has an 800x480 panel???
	 */
	switch (NeoPanelWidth) {
	case 640 :
	    outw(vgaIOBase + 4, 0x5F40);
	    outw(vgaIOBase + 4, 0x5041);
	    outw(vgaIOBase + 4, 0x0242);
	    outw(vgaIOBase + 4, 0x5543);
	    outw(vgaIOBase + 4, 0x8144);
	    outw(vgaIOBase + 4, 0x0B45);
	    outw(vgaIOBase + 4, 0x2E46);
	    outw(vgaIOBase + 4, 0xEA47);
	    outw(vgaIOBase + 4, 0x0C48);
	    outw(vgaIOBase + 4, 0xE749);
	    outw(vgaIOBase + 4, 0x044A);
	    outw(vgaIOBase + 4, 0x2D4B);
	    outw(vgaIOBase + 4, 0x284C);
	    outw(vgaIOBase + 4, 0x904D);
	    outw(vgaIOBase + 4, 0x2B4E);
	    outw(vgaIOBase + 4, 0xA04F);
	    break;
	case 800 :
	    outw(vgaIOBase + 4, 0x7F40);
	    outw(vgaIOBase + 4, 0x6341);
	    outw(vgaIOBase + 4, 0x0242);
	    outw(vgaIOBase + 4, 0x6C43);
	    outw(vgaIOBase + 4, 0x1C44);
	    outw(vgaIOBase + 4, 0x7245);
	    outw(vgaIOBase + 4, 0xE046);
	    outw(vgaIOBase + 4, 0x5847);
	    outw(vgaIOBase + 4, 0x0C48);
	    outw(vgaIOBase + 4, 0x5749);
	    outw(vgaIOBase + 4, 0x734A);
	    outw(vgaIOBase + 4, 0x3D4B);
	    outw(vgaIOBase + 4, 0x314C);
	    outw(vgaIOBase + 4, 0x014D);
	    outw(vgaIOBase + 4, 0x364E);
	    outw(vgaIOBase + 4, 0x1E4F);
	    if (NeoChipset != PCI_CHIP_NM2070) {
		outw(vgaIOBase + 4, 0x6B50);
		outw(vgaIOBase + 4, 0x4F51);
		outw(vgaIOBase + 4, 0x0E52);
		outw(vgaIOBase + 4, 0x5853);
		outw(vgaIOBase + 4, 0x8854);
		outw(vgaIOBase + 4, 0x3355);
		outw(vgaIOBase + 4, 0x2756);
		outw(vgaIOBase + 4, 0x1657);
		outw(vgaIOBase + 4, 0x2C58);
		outw(vgaIOBase + 4, 0x9459);
	    }
	    break;
	case 1024 :
	    outw(vgaIOBase + 4, 0xA340);
	    outw(vgaIOBase + 4, 0x7F41);
	    outw(vgaIOBase + 4, 0x0642);
	    outw(vgaIOBase + 4, 0x8543);
	    outw(vgaIOBase + 4, 0x9644);
	    outw(vgaIOBase + 4, 0x2445);
	    outw(vgaIOBase + 4, 0xE546);
	    outw(vgaIOBase + 4, 0x0247);
	    outw(vgaIOBase + 4, 0x0848);
	    outw(vgaIOBase + 4, 0xFF49);
	    outw(vgaIOBase + 4, 0x254A);
	    outw(vgaIOBase + 4, 0x4F4B);
	    outw(vgaIOBase + 4, 0x404C);
	    outw(vgaIOBase + 4, 0x004D);
	    outw(vgaIOBase + 4, 0x444E);
	    outw(vgaIOBase + 4, 0x0C4F);
	    outw(vgaIOBase + 4, 0x7A50);
	    outw(vgaIOBase + 4, 0x5651);
	    outw(vgaIOBase + 4, 0x0052);
	    outw(vgaIOBase + 4, 0x5D53);
	    outw(vgaIOBase + 4, 0x0E54);
	    outw(vgaIOBase + 4, 0x3B55);
	    outw(vgaIOBase + 4, 0x2B56);
	    outw(vgaIOBase + 4, 0x0057);
	    outw(vgaIOBase + 4, 0x2F58);
	    outw(vgaIOBase + 4, 0x1859);
	    outw(vgaIOBase + 4, 0x8860);
	    outw(vgaIOBase + 4, 0x6361);
	    outw(vgaIOBase + 4, 0x0B62);
	    outw(vgaIOBase + 4, 0x6963);
	    outw(vgaIOBase + 4, 0x1A64);
	    break;
	case 1280:
#ifdef NOT_DONE
	    outw(vgaIOBase + 4, 0x??40);
            .
            .
            .
	    outw(vgaIOBase + 4, 0x??64);
	    break;
#else
            /* Probe should prevent this case for now */
            FatalError("1280 panel support incomplete\n");
#endif
	}
    }
}

/*
 * NeoRestore --
 *
 * This function restores a video mode.  It basically writes out all of
 * the registers that have previously been saved in the vgaNeoRec data 
 * structure.
 *
 * Note that "Restore" is a little bit incorrect.  This function is also
 * used when the server enters/changes video modes.  The mode definitions 
 * have previously been initialized by the Init() function, below.  */
static void 
NeoRestore(restore)
vgaNeoPtr restore;
{
    int i;
    unsigned char temp;

    vgaProtect(TRUE);		/* Blank the screen */

    outw(GRAX, 0x2609);

    /* Init the shadow registers if necessary */
    NeoProgramShadowRegs(restore);

    /*
     * Whatever code is needed to get things back to bank zero should be
     * placed here.  Things should be in the same state as when the
     * Save/Init was done.
     */
    outw(GRAX, 0x0015);

    outb(GRAX, 0x0A); outb(GRAX+1, restore->GeneralLockReg);

    /*
     * The color mode needs to be set before calling vgaHWRestore
     * to ensure the DAC is initialized properly.
     *
     * NOTE: Make sure we don't change bits make sure we don't change
     * any reserved bits.
     */
    outb(GRAX, 0x90); temp = inb(GRAX+1);
    switch (NeoChipset) {
    case PCI_CHIP_NM2070 :
	temp &= 0xF0; /* Save bits 7:4 */
	temp |= (restore->ExtColorModeSelect & ~0xF0);
	break;
    case PCI_CHIP_NM2090 :
    case PCI_CHIP_NM2093 :
    case PCI_CHIP_NM2097 :
    case PCI_CHIP_NM2160 :
    case PCI_CHIP_NM2200 :
	temp &= 0x70; /* Save bits 6:4 */
	temp |= (restore->ExtColorModeSelect & ~0x70);
	break;
    }
    outb(GRAX, 0x90); outb(GRAX+1, temp);

    /*
     * Disable horizontal and vertical graphics and text expansions so
     * that vgaHWRestore works properly.
     */
    outb(GRAX, 0x25);
    temp = inb(GRAX+1);
    outb(GRAX, 0x25);
    temp &= 0x39;
    outb(GRAX+1, temp);

    /*
     * Sleep for 200ms to make sure that the two operations above have
     * had time to take effect.
     */
    usleep(200000);

    /*
     * This function handles restoring the generic VGA registers.  */
    vgaHWRestore((vgaHWPtr)restore);

    /*
     * Code to restore any SVGA registers that have been saved/modified
     * goes here.  Note that it is allowable, and often correct, to 
     * only modify certain bits in a register by a read/modify/write cycle.
     *
     * A special case - when using an external clock-setting program,
     * this function must not change bits associated with the clock
     * selection.  This condition can be checked by the condition:
     *
     *	if (restore->std.NoClock >= 0)
     *		restore clock-select bits.
     */

    outb(GRAX, 0x0E); outb(GRAX+1, restore->ExtCRTDispAddr);
    outb(GRAX, 0x0F); outb(GRAX+1, restore->ExtCRTOffset);
    outb(GRAX, 0x10); temp = inb(GRAX+1);
    temp &= 0x0F; /* Save bits 3:0 */
    temp |= (restore->SysIfaceCntl1 & ~0x0F);
    outb(GRAX, 0x10); outb(GRAX+1, temp);

    outb(GRAX, 0x11); outb(GRAX+1, restore->SysIfaceCntl2);
    outb(GRAX, 0x15); outb(GRAX+1, restore->SingleAddrPage);
    outb(GRAX, 0x16); outb(GRAX+1, restore->DualAddrPage);
    outb(GRAX, 0x20); temp = inb(GRAX+1);
    switch (NeoChipset) {
    case PCI_CHIP_NM2070 :
	temp &= 0xFC; /* Save bits 7:2 */
	temp |= (restore->PanelDispCntlReg1 & ~0xFC);
	break;
    case PCI_CHIP_NM2090 :
    case PCI_CHIP_NM2093 :
    case PCI_CHIP_NM2097 :
    case PCI_CHIP_NM2160 :
	temp &= 0xDC; /* Save bits 7:6,4:2 */
	temp |= (restore->PanelDispCntlReg1 & ~0xDC);
	break;
    case PCI_CHIP_NM2200 :
	temp &= 0x98; /* Save bits 7,4:3 */
	temp |= (restore->PanelDispCntlReg1 & ~0x98);
	break;
    }
    outb(GRAX, 0x20); outb(GRAX+1, temp);
    outb(GRAX, 0x25); temp = inb(GRAX+1);
    temp &= 0x38; /* Save bits 5:3 */
    temp |= (restore->PanelDispCntlReg2 & ~0x38);
    outb(GRAX, 0x25); outb(GRAX+1, temp);

    if (NeoChipset != PCI_CHIP_NM2070) {
	outb(GRAX, 0x30); temp = inb(GRAX+1);
	temp &= 0xEF; /* Save bits 7:5 and bits 3:0 */
	temp |= (restore->PanelDispCntlReg3 & ~0xEF);
	outb(GRAX, 0x30); outb(GRAX+1, temp);
    }

    outb(GRAX, 0x28); outb(GRAX+1, restore->PanelVertCenterReg1);
    outb(GRAX, 0x29); outb(GRAX+1, restore->PanelVertCenterReg2);
    outb(GRAX, 0x2a); outb(GRAX+1, restore->PanelVertCenterReg3);

    if (NeoChipset != PCI_CHIP_NM2070) {
        outb(GRAX, 0x32); outb(GRAX+1, restore->PanelVertCenterReg4);
        outb(GRAX, 0x33); outb(GRAX+1, restore->PanelHorizCenterReg1);
        outb(GRAX, 0x34); outb(GRAX+1, restore->PanelHorizCenterReg2);
        outb(GRAX, 0x35); outb(GRAX+1, restore->PanelHorizCenterReg3);
    }

    if (NeoChipset == PCI_CHIP_NM2160) {
        outb(GRAX, 0x36); outb(GRAX+1, restore->PanelHorizCenterReg4);
    }

    if (NeoChipset == PCI_CHIP_NM2200) {
        outb(GRAX, 0x36); outb(GRAX+1, restore->PanelHorizCenterReg4);
        outb(GRAX, 0x37); outb(GRAX+1, restore->PanelVertCenterReg5);
        outb(GRAX, 0x38); outb(GRAX+1, restore->PanelHorizCenterReg5);
    }

    /* Program VCLK3 if needed. */
    if (restore->ProgramVCLK) {
	outb(GRAX, 0x9B); outb(GRAX+1, restore->VCLK3NumeratorLow);
	if (NeoChipset == PCI_CHIP_NM2200) {
	    outb(GRAX, 0x8F); temp = inb(GRAX+1);
	    temp &= 0x0F; /* Save bits 3:0 */
	    temp |= (restore->VCLK3NumeratorHigh & ~0x0F);
	    outb(GRAX, 0x8F); outb(GRAX+1, temp);
	}
	outb(GRAX, 0x9F); outb(GRAX+1, restore->VCLK3Denominator);
    }

    if (restore->reg) {
	outb(vgaIOBase + 4, 0x25); outb(vgaIOBase + 5, restore->reg->CR[0x25]);
	outb(vgaIOBase + 4, 0x2F); outb(vgaIOBase + 5, restore->reg->CR[0x2F]);
	for (i = 0x40; i <= 0x59; i++) {
	    outb(vgaIOBase + 4, i); outb(vgaIOBase + 5, restore->reg->CR[i]);
	}
	for (i = 0x60; i <= 0x69; i++) {
	    outb(vgaIOBase + 4, i); outb(vgaIOBase + 5, restore->reg->CR[i]);
	}
	for (i = 0x70; i <= NEO_EXT_CR_MAX; i++) {
	    outb(vgaIOBase + 4, i); outb(vgaIOBase + 5, restore->reg->CR[i]);
	}

	for (i = 0x0a; i <= 0x3f; i++) {
	    outb(GRAX, i); outb(GRAX+1, restore->reg->GR[i]);
	}
	for (i = 0x90; i <= NEO_EXT_GR_MAX; i++) {
	    outb(GRAX, i); outb(GRAX+1, restore->reg->GR[i]);
	}
	xfree(restore->reg);
	restore->reg = NULL;
    }

    /* Program vertical extension register */
    if (NeoChipset == PCI_CHIP_NM2200) {
	outb(vgaIOBase + 4, 0x70); outb(vgaIOBase + 5, restore->VerticalExt);
    }

    vgaProtect(FALSE);		/* Turn on screen */
}

/*
 * NeoSave --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaNeoRec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void *
NeoSave(save)
vgaNeoPtr save;
{
    int i;

    outw(GRAX, 0x2609);

    /*
     * Whatever code is needed to get back to bank zero goes here.
     */
    outw(GRAX, 0x0015);

    /*
     * This function will handle creating the data structure and filling
     * in the generic VGA portion.
     */
    save = (vgaNeoPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaNeoRec));

    /*
     * The port I/O code necessary to read in the extended registers 
     * into the fields of the vgaNeoRec structure goes here.
     */

    outb(GRAX, 0x0A); save->GeneralLockReg           = inb(GRAX+1);
    outb(GRAX, 0x0E); save->ExtCRTDispAddr           = inb(GRAX+1);
    if (NeoChipset != PCI_CHIP_NM2070)
	outb(GRAX, 0x0F); save->ExtCRTOffset         = inb(GRAX+1);
    outb(GRAX, 0x10); save->SysIfaceCntl1            = inb(GRAX+1);
    outb(GRAX, 0x11); save->SysIfaceCntl2            = inb(GRAX+1);
    outb(GRAX, 0x15); save->SingleAddrPage           = inb(GRAX+1);
    outb(GRAX, 0x16); save->DualAddrPage             = inb(GRAX+1);
    outb(GRAX, 0x20); save->PanelDispCntlReg1        = inb(GRAX+1);
    outb(GRAX, 0x25); save->PanelDispCntlReg2        = inb(GRAX+1);
    outb(GRAX, 0x30); save->PanelDispCntlReg3        = inb(GRAX+1);
    outb(GRAX, 0x28); save->PanelVertCenterReg1      = inb(GRAX+1);
    outb(GRAX, 0x29); save->PanelVertCenterReg2      = inb(GRAX+1);
    outb(GRAX, 0x2a); save->PanelVertCenterReg3      = inb(GRAX+1);
    if (NeoChipset != PCI_CHIP_NM2070) {
	outb(GRAX, 0x32); save->PanelVertCenterReg4  = inb(GRAX+1);
	outb(GRAX, 0x33); save->PanelHorizCenterReg1 = inb(GRAX+1);
	outb(GRAX, 0x34); save->PanelHorizCenterReg2 = inb(GRAX+1);
	outb(GRAX, 0x35); save->PanelHorizCenterReg3 = inb(GRAX+1);
    }
    if (NeoChipset == PCI_CHIP_NM2160)
	outb(GRAX, 0x36); save->PanelHorizCenterReg4 = inb(GRAX+1);
    if (NeoChipset == PCI_CHIP_NM2200) {
	outb(GRAX, 0x36); save->PanelHorizCenterReg4 = inb(GRAX+1);
	outb(GRAX, 0x37); save->PanelVertCenterReg5  = inb(GRAX+1);
	outb(GRAX, 0x38); save->PanelHorizCenterReg5 = inb(GRAX+1);
    }
    outb(GRAX, 0x90); save->ExtColorModeSelect       = inb(GRAX+1);
    outb(GRAX, 0x9B); save->VCLK3NumeratorLow        = inb(GRAX+1);
    if (NeoChipset == PCI_CHIP_NM2200)
	outb(GRAX, 0x8F); save->VCLK3NumeratorHigh   = inb(GRAX+1);
    outb(GRAX, 0x9F); save->VCLK3Denominator         = inb(GRAX+1);

    if (save->reg == NULL)
	save->reg = (regSavePtr)xcalloc(1, sizeof(regSaveRec));
    else
	ErrorF("WARNING: Non-NULL reg in NeoSave: reg=0x%08X\n", save->reg);

    outb(vgaIOBase + 4, 0x25); save->reg->CR[0x25] = inb(vgaIOBase + 5);
    outb(vgaIOBase + 4, 0x2F); save->reg->CR[0x2F] = inb(vgaIOBase + 5);
    for (i = 0x40; i <= 0x59; i++) {
	outb(vgaIOBase + 4, i); save->reg->CR[i] = inb(vgaIOBase + 5);
    }
    for (i = 0x60; i <= 0x69; i++) {
	outb(vgaIOBase + 4, i); save->reg->CR[i] = inb(vgaIOBase + 5);
    }
    for (i = 0x70; i <= NEO_EXT_CR_MAX; i++) {
	outb(vgaIOBase + 4, i); save->reg->CR[i] = inb(vgaIOBase + 5);
    }

    for (i = 0x0A; i <= NEO_EXT_GR_MAX; i++) {
	outb(GRAX, i); save->reg->GR[i] = inb(GRAX+1);
    }

    return ((void *) save);
}

/*
 * NeoInit --
 *
 * This is the most important function (after the Probe) function.  This
 * function fills in the vgaNeoRec with all of the register values needed
 * to enable either a 256-color mode (for the color server) or a 16-color
 * mode (for the monochrome server).
 *
 * The 'mode' parameter describes the video mode.  The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.  The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
static Bool
NeoInit(mode)
DisplayModePtr mode;
{
    int i;
    int hoffset, voffset;

    /*
     * This will allocate the datastructure and initialize all of the
     * generic VGA registers.
     */
    if (!vgaHWInit(mode,sizeof(vgaNeoRec)))
	return(FALSE);

    /*
     * Here all of the other fields of 'new' get filled in, to
     * handle the SVGA extended registers.  It is also allowable
     * to override generic registers whenever necessary.
     *
     * A special case - when using an external clock-setting program,
     * this function must not change bits associated with the clock
     * selection.  This condition can be checked by the condition:
     *
     *	if (new->std.NoClock >= 0)
     *		initialize clock-select bits.
     */

    /*
     * Several registers need to be corrected from the default values
     * assigned by vgaHWinit().
     */

    /*
     * The default value assigned by vgaHW.c is 0x41, but this does
     * not work for NeoMagic.
     */
    new->std.Attribute[16] = 0x01;

    switch (vgaBitsPerPixel) {
    case  8 :
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 3;
	new->ExtCRTOffset   = vga256InfoRec.displayWidth >> 11;
	new->ExtColorModeSelect = 0x11;
	break;
    case 15 :
    case 16 :
	if ((xf86weight.red == 5) &&
	    (xf86weight.green == 5) &&
	    (xf86weight.blue == 5)) {
	    /* 15bpp */
	    for (i = 0; i < 64; i++) {
		new->std.DAC[i*3+0] = i << 1;
		new->std.DAC[i*3+1] = i << 1;
		new->std.DAC[i*3+2] = i << 1;
	    }
	    new->ExtColorModeSelect = 0x12;
	} else {
	    /* 16bpp */
	    for (i = 0; i < 64; i++) {
		new->std.DAC[i*3+0] = i << 1;
		new->std.DAC[i*3+1] = i;
		new->std.DAC[i*3+2] = i << 1;
	    }
	    new->ExtColorModeSelect = 0x13;
	}
	/* 15bpp & 16bpp */
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 2;
	new->ExtCRTOffset   = vga256InfoRec.displayWidth >> 10;
	break;
    case 24 :
	for (i = 0; i < 256; i++) {
	    new->std.DAC[i*3+0] = i;
	    new->std.DAC[i*3+1] = i;
	    new->std.DAC[i*3+2] = i;
	}
	new->std.CRTC[0x13] = (vga256InfoRec.displayWidth * 3) >> 3;
	new->ExtCRTOffset   = (vga256InfoRec.displayWidth * 3) >> 11;
	new->ExtColorModeSelect = 0x14;
	break;
    default :
	break;
    }

    new->ExtCRTDispAddr = 0x10;

    /* Vertical Extension */
    new->VerticalExt = (((mode->CrtcVTotal -2) & 0x400) >> 10 )
      | (((mode->CrtcVDisplay -1) & 0x400) >> 9 )
        | (((mode->CrtcVSyncStart) & 0x400) >> 8 )
          | (((mode->CrtcVSyncStart) & 0x400) >> 7 );

    /* Disable read/write bursts if requested. */
    if (OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options)) {
	new->SysIfaceCntl1 = 0x30; 
    } else {
	new->SysIfaceCntl1 = 0x00; 
    }

    /* If they are used, enable linear addressing and/or enable MMIO. */
    new->SysIfaceCntl2 = 0x00;
    if (NEO.ChipUseLinearAddressing)
	new->SysIfaceCntl2 |= 0x80;
    if (NeoMMIOBase)
	new->SysIfaceCntl2 |= 0x40;

    /* Enable any user specified display devices. */
    new->PanelDispCntlReg1 = 0x00;
    if (OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options)) {
	new->PanelDispCntlReg1 |= 0x02;
    }
    if (OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options)) {
	new->PanelDispCntlReg1 |= 0x01;
    }

    /* If the user did not specify any display devices, then... */
    if (new->PanelDispCntlReg1 == 0x00) {
	/* Default to internal (i.e., LCD) only. */
	new->PanelDispCntlReg1 |= 0x02;
    }

    /* If we are using a fixed mode, then tell the chip we are. */
    switch (mode->HDisplay) {
    case 1280:
	new->PanelDispCntlReg1 |= 0x60;
        break;
    case 1024:
	new->PanelDispCntlReg1 |= 0x40;
        break;
    case 800:
	new->PanelDispCntlReg1 |= 0x20;
        break;
    case 640:
    default:
        break;
    }

    /* Setup shadow register locking. */
    switch (new->PanelDispCntlReg1 & 0x03) {
    case 0x01 : /* External CRT only mode: */
	new->GeneralLockReg = 0x00;
	/* We need to program the VCLK for external display only mode. */
	new->ProgramVCLK = TRUE;
	break;
    case 0x02 : /* Internal LCD only mode: */
    case 0x03 : /* Simultaneous internal/external (LCD/CRT) mode: */
	new->GeneralLockReg = 0x01;
	/* Don't program the VCLK when using the LCD. */
	new->ProgramVCLK = FALSE;
	break;
    }

    /*
     * If the screen is to be stretched, turn on stretching for the
     * various modes.
     *
     * OPTION_LCD_STRETCH means stretching should be turned off!
     */
    new->PanelDispCntlReg2 = 0x00;
    new->PanelDispCntlReg3 = 0x00;
    if (!OFLG_ISSET(OPTION_LCD_STRETCH, &vga256InfoRec.options) &&
	(new->PanelDispCntlReg1 & 0x02)) {
	if (mode->HDisplay == NeoPanelWidth) {
	    /*
	     * No stretching required when the requested display width
	     * equals the panel width.
	     */
	    if (vgaHWCursor.Initialized) NeoTempSWCursor(FALSE);
	} else {
	    switch (mode->HDisplay) {
	    case  320 : /* Needs testing.  KEM -- 24 May 98 */
	    case  400 : /* Needs testing.  KEM -- 24 May 98 */
	    case  640 :
	    case  800 :
	    case 1024 :
		new->PanelDispCntlReg2 |= 0xC6;
		if (vgaHWCursor.Initialized) NeoTempSWCursor(TRUE);
		break;
	    default   :
		/* No stretching in these modes. */
		if (vgaHWCursor.Initialized) NeoTempSWCursor(FALSE);
		break;
	    }
	}
    } else if (mode->Flags & V_DBLSCAN) {
	if (vgaHWCursor.Initialized) NeoTempSWCursor(TRUE);
    } else {
	if (vgaHWCursor.Initialized) NeoTempSWCursor(FALSE);
    }

    /*
     * If the screen is to be centerd, turn on the centering for the
     * various modes.
     */
    new->PanelVertCenterReg1  = 0x00;
    new->PanelVertCenterReg2  = 0x00;
    new->PanelVertCenterReg3  = 0x00;
    new->PanelVertCenterReg4  = 0x00;
    new->PanelVertCenterReg5  = 0x00;
    new->PanelHorizCenterReg1 = 0x00;
    new->PanelHorizCenterReg2 = 0x00;
    new->PanelHorizCenterReg3 = 0x00;
    new->PanelHorizCenterReg4 = 0x00;
    new->PanelHorizCenterReg5 = 0x00;

    if (OFLG_ISSET(OPTION_LCD_CENTER, &vga256InfoRec.options) &&
	(new->PanelDispCntlReg1 & 0x02)) {
	if (mode->HDisplay == NeoPanelWidth) {
	    /*
	     * No centering required when the requested display width
	     * equals the panel width.
	     */
	} else {
	    new->PanelDispCntlReg2 |= 0x01;
	    new->PanelDispCntlReg3 |= 0x10;

	    /* Calculate the horizontal and vertical offsets. */
	    if (OFLG_ISSET(OPTION_LCD_STRETCH, &vga256InfoRec.options)) {
		hoffset = ((NeoPanelWidth - mode->HDisplay) >> 4) - 1;
		voffset = ((NeoPanelHeight - mode->VDisplay) >> 1) - 2;
	    } else {
		/* Stretched modes cannot be centered. */
		hoffset = 0;
		voffset = 0;
	    }

	    switch (mode->HDisplay) {
	    case  320 : /* Needs testing.  KEM -- 24 May 98 */
		new->PanelHorizCenterReg3 = hoffset;
		new->PanelVertCenterReg2  = voffset;
		break;
	    case  400 : /* Needs testing.  KEM -- 24 May 98 */
		new->PanelHorizCenterReg4 = hoffset;
		new->PanelVertCenterReg1  = voffset;
		break;
	    case  640 :
		new->PanelHorizCenterReg1 = hoffset;
		new->PanelVertCenterReg3  = voffset;
		break;
	    case  800 :
		new->PanelHorizCenterReg2 = hoffset;
		new->PanelVertCenterReg4  = voffset;
		break;
	    case 1024 :
		new->PanelHorizCenterReg5 = hoffset;
		new->PanelVertCenterReg5  = voffset;
		break;
	    case 1280 :
	    default   :
		/* No centering in these modes. */
		break;
	    }
	}
    }

    /*
     * vgaHWInit just calloced 'new' so reg should be empty.  Just in
     * case it isn't, warn us and clear it anyway.
     */
    if (new->reg) {
	ErrorF("WARNING: Non-NULL reg in NeoInit: reg=0x%08X\n", new->reg);
	xfree(new->reg);
	new->reg = NULL;
    }

    /*
     * Calculate the VCLK that most closely matches the requested dot
     * clock.
     */
    NeoCalcVCLK(vga256InfoRec.clock[new->std.NoClock]);

    /* Since we program the clocks ourselves, always use VCLK3. */
    new->std.MiscOutReg |= 0x0C;

    return(TRUE);
}

/*
* NeoAdjust --
*
* This function is used to initialize the SVGA Start Address - the first
* displayed location in the video memory.  This is used to implement the
* virtual window.
*/
static void 
NeoAdjust(x, y)
int x, y;
{
    /*
     * The calculation for Base works as follows:
     *
     *	(y * virtX) + x ==> the linear starting pixel
     *
     * This number is divided by 8 for the monochrome server, because
     * there are 8 pixels per byte.
     *
     * For the color server, it's a bit more complex.  There is 1 pixel
     * per byte.  In general, the 256-color modes are in word-mode 
     * (16-bit words).  Word-mode vs byte-mode is will vary based on
     * the chipset - refer to the chipset databook.  So the pixel address 
     * must be divided by 2 to get a word address.  In 256-color modes, 
     * the 4 planes are interleaved (i.e. pixels 0,3,7, etc are adjacent 
     * on plane 0). The starting address needs to be as an offset into 
     * plane 0, so the Base address is divided by 4.
     *
     * So:
     *    Monochrome: Base is divided by 8
     *    Color:
     *	if in word mode, Base is divided by 8
     *	if in byte mode, Base is divided by 4
     *
     * The generic VGA only supports 16 bits for the Starting Address.
     * But this is not enough for the extended memory.  SVGA chipsets
     * will have additional bits in their extended registers, which
     * must also be set.
     */
    int Base = (y * vga256InfoRec.displayWidth + x) >> 2;
    int oldExtCRTDispAddr;

    /* Scale Base by the number of bytes per pixel. */
    switch (vgaBitsPerPixel) {
    case  8 :
	break;
    case 15 :
    case 16 :
	Base *= 2;
	break;
    case 24 :
	Base *= 3;
	break;
    default :
	break;
    }
    /*
     * These are the generic starting address registers.
     */
    outw(vgaIOBase + 4, (Base & 0x00FF00) | 0x0C);
    outw(vgaIOBase + 4, ((Base & 0x00FF) << 8) | 0x0D);

    /*
     * Make sure we don't clobber some other bits that might already
     * have been set.  NOTE: NM2200 has a writable bit 3, but it shouldn't
     * be needed.
     */
    outb(GRAX, 0x0E);
    oldExtCRTDispAddr = inb(GRAX+1);
    outw(GRAX,
	 ((((Base >> 16) & 0x07) | (oldExtCRTDispAddr & 0xf8)) << 8) | 0x0E);

    /*
     * This is a workaround for a higher level bug that causes the cursor
     * to be at the wrong position after a virtual screen resolution change
     */
    if (vgaHWCursor.Initialized) {
	NeoRepositionCursor();
    }
}

/*
 * NeoFbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 * For most drivers, this function is not required, and 'NoopDDA' is put
 * in the vgaVideoChipRec structure.
 */
static void
NeoFbInit()
{
    unsigned char w;
    unsigned char type;

    /* Print out some useful information about the system. */
    if (xf86Verbose) {
	ErrorF("%s %s: NeoMagic ",
	       (vga256InfoRec.chipset ? XCONFIG_PROBED : XCONFIG_GIVEN),
	       vga256InfoRec.name);
	switch (NeoChipset) {
	case PCI_CHIP_NM2070 :
	    ErrorF("MagicGraph 128 (NM2070)");
	    break;
	case PCI_CHIP_NM2090 :
	    ErrorF("MagicGraph 128V (NM2090)");
	    break;
	case PCI_CHIP_NM2093 :
	    ErrorF("MagicGraph 128ZV (NM2093)");
	    break;
	case PCI_CHIP_NM2097 :
	    ErrorF("MagicGraph 128ZV+ (NM2097)");
	    break;
	case PCI_CHIP_NM2160 :
	    ErrorF("MagicGraph 128XD (NM2160)");
	    break;
	case PCI_CHIP_NM2200 :
	    ErrorF("MagicMedia 256AV (NM2200)");
	    break;
	}
	ErrorF(" chip\n");

	outb(GRAX, 0x21); type = inb(GRAX+1);
	ErrorF("%s %s: %s: Panel is a %dx%d %s %s display\n",
	       XCONFIG_PROBED,
	       vga256InfoRec.name,
	       vga256InfoRec.chipset,
	       NeoPanelWidth,
	       NeoPanelHeight,
	       (type & 0x02) ? "color" : "monochrome",
	       (type & 0x10) ? "TFT" : "dual scan");

	if (OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) &&
	    OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options)) {
	    ErrorF("%s %s: %s: Simultaneous LCD/CRT display mode\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	} else if (!OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) &&
		   OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options)) {
	    ErrorF("%s %s: %s: External CRT only display mode\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	} else if ((OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) &&
		    !OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options)) ||
		   (!OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) &&
		    !OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options))) {
	    ErrorF("%s %s: %s: Internal LCD only display mode\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	}

	if (OFLG_ISSET(OPTION_LCD_CENTER, &vga256InfoRec.options)) {
	    ErrorF("%s %s: %s: Video modes are centered on the display\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	} else {
	    ErrorF("%s %s: %s: Video modes are displayed in the upper-left corner\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	}

	if (OFLG_ISSET(OPTION_LCD_STRETCH, &vga256InfoRec.options)) {
	    ErrorF("%s %s: %s: Low resolution video modes are not stretched\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	} else {
	    ErrorF("%s %s: %s: Low resolution video modes are stretched\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	}
    }

    /* Map IO registers to virtual address space. */
    if (!OFLG_ISSET(OPTION_NO_MMIO, &vga256InfoRec.options)) {
	NeoMMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
				    (pointer)(NeoMMIOAddr), 0x200000);
	if (!NeoMMIOBase)
	    FatalError("NeoMagic: Can't memory map IO registers, try 'no_mmio' flag\n");
	if (xf86Verbose)
	    ErrorF("%s %s: %s: MMIO registers at 0x%08lX\n",
		   (vga256InfoRec.IObase ? XCONFIG_GIVEN : XCONFIG_PROBED),
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
		   NeoMMIOAddr);
    }

    /* Use linear by default. */
    if (!OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) {
	NEO.ChipLinearBase = NeoLinearAddr;
	NEO.ChipUseLinearAddressing = TRUE;

	/*
	 * Setup the size of the linear frame buffer memory aperature.
	 * NOTE: this must be a power of 2 for the vga support code to operate
	 * properly.
	 */
	switch(NeoChipset) {
	case PCI_CHIP_NM2070 :
	    NEO.ChipLinearSize = 1024*1024; /* exactly 1Meg */
	    break;
	case PCI_CHIP_NM2090 :
	case PCI_CHIP_NM2093 :
	case PCI_CHIP_NM2097 :
	case PCI_CHIP_NM2160 :
	    NEO.ChipLinearSize = 2048*1024; /* exactly 2Meg */
	    break;
	case PCI_CHIP_NM2200 :
	    NEO.ChipLinearSize = 4096*1024; /* exactly 4Meg */
	    break;
	}

	if (xf86Verbose)
	    ErrorF("%s %s: %s: Linear framebuffer at 0x%08lX\n",
		   (vga256InfoRec.MemBase ? XCONFIG_GIVEN : XCONFIG_PROBED),
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
		   NEO.ChipLinearBase);
    }

    /*
     * If memeory is linear, then use HW cursor by default. 
     */
    if (NEO.ChipUseLinearAddressing) {
	if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
	    !OFLG_ISSET(OPTION_NO_MMIO, &vga256InfoRec.options)) {
	    vgaHWCursor.Initialized = TRUE;
            vgaHWCursor.Init = NeoCursorInit;
            vgaHWCursor.Restore = NeoRestoreCursor;
            vgaHWCursor.Warp = NeoWarpCursor;
            vgaHWCursor.QueryBestSize = NeoQueryBestSize;

	    switch(NeoChipset) {
	    case PCI_CHIP_NM2070 :
	    case PCI_CHIP_NM2090 :
	    case PCI_CHIP_NM2093 :
		/* Use the last 2048 bytes of framebuffer for cursor image */
		NeoCursorMemSegment = vga256InfoRec.videoRam - 2;
		NeoCursorOffset = 0x100;
		break;
	    case PCI_CHIP_NM2097 :
	    case PCI_CHIP_NM2160 :
		/* Use the last 1024 bytes of framebuffer for cursor image */
		NeoCursorMemSegment = vga256InfoRec.videoRam - 1;
		NeoCursorOffset = 0x100;
		break;
	    case PCI_CHIP_NM2200 :
		/* Use the last 1024 bytes of framebuffer for cursor image */
		NeoCursorMemSegment = vga256InfoRec.videoRam - 1;
		NeoCursorOffset = 0x1000;
		break;
	    }

	    if (xf86Verbose)
                ErrorF("%s %s: %s: Using hardware cursor\n",
		       (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) ?
			XCONFIG_GIVEN : XCONFIG_PROBED),
		       vga256InfoRec.name,
		       vga256InfoRec.chipset);
	} else {
	    if (xf86Verbose)
		ErrorF("%s %s: %s: Using software cursor\n",
		       (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) ?
			XCONFIG_GIVEN : XCONFIG_PROBED),
		       vga256InfoRec.name,
		       vga256InfoRec.chipset);
	}

	if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) &&
	    !OFLG_ISSET(OPTION_NO_MMIO, &vga256InfoRec.options)) {
	    switch(NeoChipset) {
		case PCI_CHIP_NM2070 :
		    Neo2070AccelInit();
		    break;
		case PCI_CHIP_NM2090 :
		case PCI_CHIP_NM2093 :
		    Neo2090AccelInit();
		    break;
		case PCI_CHIP_NM2097 :
		case PCI_CHIP_NM2160 :
		    Neo2097AccelInit();
		    break;
		case PCI_CHIP_NM2200 :
		    Neo2200AccelInit();
		    break;
	    }
	} else {
	    if (xf86Verbose)
		ErrorF("%s %s: %s: Acceleration disabled\n",
		       (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) ?
			XCONFIG_GIVEN : XCONFIG_PROBED),
		       vga256InfoRec.name,
		       vga256InfoRec.chipset);
	}
    } else {
	if (xf86Verbose) {
	    ErrorF("%s %s: %s: Using software cursor\n",
		   (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) ?
		    XCONFIG_GIVEN : XCONFIG_PROBED),
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	    ErrorF("%s %s: %s: Acceleration disabled\n",
		   (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) ?
		    XCONFIG_GIVEN : XCONFIG_PROBED),
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	}
    }
}

static int
NeoValidMode(mode, verbose, flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
    int maxWidth;

    /*
     * Code to check if a mode is suitable for the selected chipset.
     * In most cases this can just return MODE_OK.
     */

    /*
     * Limit the modes to just those allowed by the various NeoMagic
     * chips.  
     */
    if (NeoChipset == PCI_CHIP_NM2200) {
	maxWidth = 1280;
    }
    else {
	maxWidth = 1024;
    }

    if (mode->HDisplay > maxWidth) {
	if (verbose) {
	    ErrorF("%s %s: %s: Removing mode wider than %d\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
                   maxWidth);
	}
	return(MODE_BAD);
    }

    if (OFLG_ISSET(OPTION_OVERRIDE_VALIDATE_MODE, &vga256InfoRec.options)) {
	if (verbose) {
	    ErrorF("%s %s: %s: WARNING, display mode validation disabled\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
	}
    }
    else {
	/*
	 * When the LCD is active, only allow modes that are (1) equal to
	 * or smaller than the size of the panel and (2) are one of the
	 * following sizes: 1024x768, 800x600, 640x480.
	 */
	if (OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) ||
	   !OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options)) {
	    /* Is the mode larger than the LCD panel? */
	    if ((mode->HDisplay > NeoPanelWidth) ||
		(mode->VDisplay > NeoPanelHeight)) {
		if (verbose) {
		    ErrorF("%s %s: %s: Removing mode (%dx%d) "
			   "larger than the LCD panel (%dx%d)\n",
			   XCONFIG_PROBED,
			   vga256InfoRec.name,
			   vga256InfoRec.chipset,
			   mode->HDisplay,
			   mode->VDisplay,
			   NeoPanelWidth,
			   NeoPanelHeight);
		}
		return(MODE_BAD);
	    }

	    /* Is the mode one of the acceptable sizes? */
	    switch (mode->HDisplay) {
	    case 1280 :
		if (mode->VDisplay == 1024)
		    return(MODE_OK);
		break;
	    case 1024 :
		if (mode->VDisplay == 768)
		    return(MODE_OK);
		break;
	    case  800 :
		if (mode->VDisplay == 600)
		    return(MODE_OK);
		break;
	    case  640 :
		if (mode->VDisplay == 480)
		    return(MODE_OK);
		break;
	    }

	    if (verbose) {
		ErrorF("%s %s: %s: Removing mode (%dx%d) that won't "
		       "display properly on LCD\n",
		       XCONFIG_PROBED,
		       vga256InfoRec.name,
		       vga256InfoRec.chipset,
		       mode->HDisplay,
		       mode->VDisplay);
	    }
	    return(MODE_BAD);
	}
    }

    return(MODE_OK);
}

/*
 * NeoDisplayPowerManagementSet --
 *
 * Sets VESA Display Power Management Signaling (DPMS) Mode.
 */
#ifdef DPMSExtension
static void NeoDisplayPowerManagementSet(PowerManagementMode)
int PowerManagementMode;
{
    unsigned char SEQ01;
    unsigned char LogicPowerMgmt;
    unsigned char LCD_on;

    if (!xf86VTSema)
	return;

    switch (PowerManagementMode) {
    case DPMSModeOn:
	/* Screen: On; HSync: On, VSync: On */
	SEQ01 = 0x00;
	LogicPowerMgmt = 0x00;
	if (OFLG_ISSET(OPTION_INTERN_DISP, &vga256InfoRec.options) ||
	    !OFLG_ISSET(OPTION_EXTERN_DISP, &vga256InfoRec.options))
	    LCD_on = 0x02;
	else
	    LCD_on = 0x00;
	break;
    case DPMSModeStandby:
	/* Screen: Off; HSync: Off, VSync: On */
	SEQ01 = 0x20;
	LogicPowerMgmt = 0x10;
	LCD_on = 0x00;
	break;
    case DPMSModeSuspend:
	/* Screen: Off; HSync: On, VSync: Off */
	SEQ01 = 0x20;
	LogicPowerMgmt = 0x20;
	LCD_on = 0x00;
	break;
    case DPMSModeOff:
	/* Screen: Off; HSync: Off, VSync: Off */
	SEQ01 = 0x20;
	LogicPowerMgmt = 0x30;
	LCD_on = 0x00;
	break;
    }

    /* Turn the screen on/off */
    outb(0x3C4, 0x01);
    SEQ01 |= inb(0x3C5) & ~0x20;
    outb(0x3C5, SEQ01);

    /* Turn the LCD on/off */
    outb(GRAX, 0x20);
    LCD_on |= inb(GRAX+1) & ~0x02;
    outb(GRAX+1, LCD_on);

    /* Set the DPMS mode */
    LogicPowerMgmt |= 0x80;
    outb(GRAX, 0x01);
    LogicPowerMgmt |= inb(GRAX+1) & ~0xF0;
    outb(GRAX+1, LogicPowerMgmt);
}
#endif
