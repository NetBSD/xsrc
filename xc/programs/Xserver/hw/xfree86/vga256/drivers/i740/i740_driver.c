/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i740/i740_driver.c,v 1.1.2.2 1999/06/18 13:08:26 hohndel Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * $PI: i740_driver.c,v 1.22 1999/02/18 20:50:59 martin Exp martin $
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
#include "xf86cursor.h"

/*
 * For PCI probing, etc.
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
#include "i740.h"
#include "i740_reg.h"
#include "i740_macros.h"

/* For fabs() and ceil() */
#include <math.h>

typedef struct {
    vgaHWRec std;               /* good old IBM VGA */
    unsigned char DisplayControl;
    unsigned char PixelPipeCfg0;
    unsigned char PixelPipeCfg1;
    unsigned char PixelPipeCfg2;
    unsigned char VideoClk2_M;
    unsigned char VideoClk2_N;
    unsigned char VideoClk2_MN_MSBs;
    unsigned char VideoClk2_DivisorSel;
    unsigned char PLLControl;
    unsigned char AddressMapping;
    unsigned char IOControl;
    unsigned char BitBLTControl;
    unsigned char ExtVertTotal;
    unsigned char ExtVertDispEnd;
    unsigned char ExtVertSyncStart;
    unsigned char ExtVertBlankStart;
    unsigned char ExtHorizTotal;
    unsigned char ExtHorizBlank;
    unsigned char ExtOffset;
    unsigned char InterlaceControl;
    unsigned int  LMI_FIFO_Watermark;
} vgaI740Rec, *vgaI740Ptr;

/* Globals that are allocated in this module. */
unsigned char *I740MMIOBase = NULL;
int I740Chipset = -1;
int I740LinearAddr = 0;
int I740MMIOAddr = 0;
pciTagRec I740PciTag;
int I740CursorStart = 0;
int I740HasSGRAM = TRUE;

/*
 * Forward definitions for the functions that make up the driver.  See
 * the definitions of these functions for the real scoop.
 */
static Bool     I740Probe();
static char *   I740Ident();
static Bool     I740ClockSelect();
static void     I740EnterLeave();
static Bool     I740Init();
static int	I740ValidMode();
static void *   I740Save();
static void     I740Restore();
static void     I740Adjust();
static void     I740SaveScreen();
static void	I740FbInit();
static void	I740DisplayPowerManagementSet();

/* 
 * The i740 driver does not support banked access since only AGP (and
 * a few PCI) cards are available.
 */
#if 0
/*
 * These are the bank select functions.  There are defined in i740_bank.s
 */
static void     I740SetRead();
static void     I740SetWrite();
static void     I740SetReadWrite();
#endif

/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data 
 * that defines how the driver operates.
 */
vgaVideoChipRec I740 = {
    /* 
     * Function pointers
     */
    I740Probe,
    I740Ident,
    I740EnterLeave,
    I740Init,
    I740ValidMode,
    I740Save,
    I740Restore,
    I740Adjust,
    I740SaveScreen,
    (void (*)())NoopDDA,	/* I740GetMode, */
    I740FbInit,
    (void (*)())NoopDDA,	/* I740SetRead, */
    (void (*)())NoopDDA,	/* I740SetWrite, */
    (void (*)())NoopDDA,	/* I740SetReadWrite, */
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
    FALSE,
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
    TRUE,
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
    TRUE,
    /*
     * This is TRUE if the driver has support for 24bpp for the detected
     * configuration.
     */
    TRUE,
    /*
     * This is TRUE if the driver has support for 32bpp for the detected
     * configuration.
     */
    TRUE,
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
#define new ((vgaI740Ptr)vgaNewVideoState)

/*
 * If your chipset uses non-standard I/O ports, you need to define an
 * array of ports, and an integer containing the array size.  The
 * generic VGA ports are defined in vgaHW.c.
 */
static unsigned I740_ExtPorts[] = { XRX, XRX+1, MRX, MRX+1 };
static int Num_I740_ExtPorts = (sizeof(I740_ExtPorts)/
				sizeof(I740_ExtPorts[0]));


/*
 * I740Ident --
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
I740Ident(n)
    int n;
{
    static char *chipsets[] = {
	"i740", "i740_pci"
    };

    if (n + 1 > sizeof(chipsets) / sizeof(char *))
	return(NULL);
    else
	return(chipsets[n]);
}

/*
 * I740ClockSelect --
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
I740ClockSelect(no)
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
	save1 = inb(MSR_R);
	/* Any extended registers would go here */
	break;
    case CLK_REG_RESTORE:
	/*
	 * Here all the previously saved registers are restored.
	 */
	outb(MSR_W, save1);
	/* Any extended registers would go here */
	break;
    default:
	/* 
	 * These are the generic two low-order bits of the clock select 
	 */
	temp = inb(MSR_R);
	outb(MSR_W, (temp & 0xF3) | ((no << 2) & 0x0C));
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
 * I740Probe --
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
I740Probe()
{
    pciConfigPtr pcr;
    pciConfigPtr I740pcr = NULL;
    int i;
    int temp;

    /*
     * Set up I/O ports to be used by this card.  Only do the second
     * xf86AddIOPorts() if there are non-standard ports for this
     * chipset.
     */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_I740_ExtPorts, I740_ExtPorts);

    /*
     * First we attempt to figure out if one of the supported chipsets
     * is present.
     */
    if (vga256InfoRec.chipset) {
	char *chipset;
	for (i = 0; (chipset = I740Ident(i)); i++) {
	    if (!StrCaseCmp(vga256InfoRec.chipset, chipset)) {
		break;
	    }
	}

	if (!chipset)
	    return(FALSE);
    }

    /*
     * Start with PCI probing, this should get us quite far already.
     */

    I740Chipset = -1;
    i = 0;
    if (vgaPCIInfo && vgaPCIInfo->AllCards) {
	while (pcr = vgaPCIInfo->AllCards[i++]) {
	    if ((pcr->_vendor == PCI_VENDOR_INTEL) &&
	        (pcr->_command & PCI_CMD_IO_ENABLE) &&
		(pcr->_command & PCI_CMD_MEM_ENABLE)) {
		int id = pcr->_device;

		if (vga256InfoRec.chipID) {
		    ErrorF("%s %s: i740 chipset override, using ChipID "
			   "value (0x%04X) instead of PCI value (0x%04X)\n",
			   XCONFIG_GIVEN,
			   vga256InfoRec.name,
			   vga256InfoRec.chipID,
			   pcr->_device);
		    id = vga256InfoRec.chipID;
		}

		switch (id) {
		case PCI_CHIP_I740_AGP:
		    vga256InfoRec.chipset = I740Ident(0);
		    I740Chipset = id;
		    I740LinearAddr = pcr->_base0 & 0xff000000; /* 16M window */
		    I740MMIOAddr = pcr->_base1 & 0xfff80000; /* 512K window */
		    break;
		}

		if (I740Chipset != -1) {
		    I740pcr = pcr; /* Save it for later use. */
		}
	    } else if ((pcr->_vendor == PCI_VENDOR_REAL3D) &&
	    		(pcr->_command & PCI_CMD_IO_ENABLE) &&
			(pcr->_command & PCI_CMD_MEM_ENABLE)) {
		int id = pcr->_device;

		if (vga256InfoRec.chipID) {
		    ErrorF("%s %s: i740 chipset override, using ChipID "
			   "value (0x%04X) instead of PCI value (0x%04X)\n",
			   XCONFIG_GIVEN,
			   vga256InfoRec.name,
			   vga256InfoRec.chipID,
			   pcr->_device);
		    id = vga256InfoRec.chipID;
		}

		switch (id) {
		case PCI_CHIP_I740_PCI:
		    vga256InfoRec.chipset = I740Ident(1);
		    I740Chipset = id;
		    I740LinearAddr = pcr->_base0 & 0xff000000; /* 16M window */
		    I740MMIOAddr = pcr->_base1 & 0xfff80000; /* 512K window */
		    break;
		}

		if (I740Chipset != -1) {
		    I740pcr = pcr; /* Save it for later use. */
		}
	    }
	}
    } else {
	return(FALSE);
    }

    if (I740Chipset == -1) {
	if (vga256InfoRec.chipset)
	    ErrorF("%s %s: i740: Unknown chipset\n",
		   XCONFIG_PROBED, vga256InfoRec.name);
	return(FALSE);
    }

    /*
     * Okay, it's an i740.
     */

    I740PciTag = pcibusTag(I740pcr->_bus, I740pcr->_cardnum, I740pcr->_func);

    /* Override the linear address, if user specified one. */
    if (vga256InfoRec.MemBase) {
	if (vga256InfoRec.MemBase != I740LinearAddr) {
	    ErrorF("%s %s: i740 linear address override, using 0x%08X "
		   "instead of probed value (0x%08X)\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.MemBase,
		   I740LinearAddr);
	}
	I740LinearAddr = vga256InfoRec.MemBase;
    }

    /* Override the MMIO address, if user specified one. */
    if (vga256InfoRec.IObase) {
	if (vga256InfoRec.IObase != I740MMIOAddr) {
	    ErrorF("%s %s: i740 linear address override, using 0x%08X "
		   "instead of probed value (0x%08X)\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.IObase,
		   I740MMIOAddr);
	}
	I740MMIOAddr = vga256InfoRec.IObase;
    }

    /* Enable I/O Ports, etc. */
    I740EnterLeave(ENTER);

    /*
     * If the user has specified the amount of memory in the
     * XF86Config file, we respect that setting.
     */
    if (!vga256InfoRec.videoRam) {
	/*
	 * Otherwise, do whatever chipset-specific things are
	 * necessary to figure out how much memory (in kBytes) is
	 * available.
	 */
	outb(XRX, DRAM_ROW_TYPE);
	if ((inb(XRX+1) & DRAM_ROW_1) == DRAM_ROW_1_SDRAM)
	    outb(XRX, DRAM_ROW_BNDRY_1);
	else
	    outb(XRX, DRAM_ROW_BNDRY_0);
	vga256InfoRec.videoRam = inb(XRX+1)*1024;
    }

    /*
     * Last we fill in the remaining data structures.  We specify the
     * chipset name, using the Ident() function and an appropriate
     * index.  We set a boolean for whether or not this driver
     * supports banking for the Monochrome server.  And we set up a
     * list of all the option flags that this driver can make use of.
     */
    vga256InfoRec.bankedMono = FALSE;

#ifdef XFreeXDGA
    vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

    /* Allowed options: */
    OFLG_SET(OPTION_LINEAR, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_NOACCEL, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_SW_CURSOR, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_HW_CURSOR, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_MMIO, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_6_BIT, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_8_BIT, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_SDRAM, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_SGRAM, &I740.ChipOptionFlags);
    OFLG_SET(OPTION_SLOW_DRAM, &I740.ChipOptionFlags);

    /* Make sure users don't set mutually exclusive options!! */
    if (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"sw_cursor\" and \"hw_cursor\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"dac_6_bit\" and \"dac_8_bit\" are incompatible.\n");
    if (OFLG_ISSET(OPTION_SDRAM, &vga256InfoRec.options) &&
	OFLG_ISSET(OPTION_SGRAM, &vga256InfoRec.options))
	FatalError(
	    "\nOptions \"sdram\" and \"sgram\" are incompatible.\n");

    /*
     * Turn on 8 bit DAC mode, if 6 bit mode is not set or if the screen
     * depth is greater than 8bpp.
     */
    if (!OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options) ||
	vgaBitsPerPixel > 8)
	OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);

    /* Turn off DAC_6_BIT if screen depth is greater than 8bpp */
    if (OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options) &&
	vgaBitsPerPixel > 8) {
	OFLG_CLR(OPTION_DAC_6_BIT, &vga256InfoRec.options);
	if (xf86Verbose) {
	    ErrorF("%s %s: %s: Cannot use \"dac_6_bit\" in %d bpp mode\n",
		   XCONFIG_GIVEN,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
		   vgaBitsPerPixel);
	}
    }

    /* 
     * If your driver uses a programmable clockchip, you have to set
     * this option to avoid clock probing etc.
     */
    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

#ifdef DPMSExtension
    vga256InfoRec.DPMSSet = I740DisplayPowerManagementSet;
#endif

    /*
     * If the user specified a ramdac speed in the XF86Config file, we
     * respect that setting.
     */
    outb(XRX, DRAM_ROW_CNTL_LO); temp = inb(XRX+1);
    I740HasSGRAM = !((temp & DRAM_RAS_TIMING) || (temp & DRAM_RAS_PRECHARGE));
    if (OFLG_ISSET(OPTION_SGRAM, &vga256InfoRec.options))
	I740HasSGRAM = TRUE;  /* Override default */
    else if (OFLG_ISSET(OPTION_SDRAM, &vga256InfoRec.options))
	I740HasSGRAM = FALSE; /* Override default */

    /*
     * The max RAMDAC dot clock is 220MHz, but the max pixel pipeline
     * frequency is 203MHz so we'll use the 203MHz value.
     */

    if (vga256InfoRec.dacSpeeds[0]) {
	vga256InfoRec.maxClock = vga256InfoRec.dacSpeeds[0];
    } else {
	if (I740HasSGRAM) {
	    /* SGRAM based cards */

	    switch (I740Chipset) {
	    case PCI_CHIP_I740_AGP:
	    case PCI_CHIP_I740_PCI:
		switch (vgaBitsPerPixel) {
		case 8:  vga256InfoRec.maxClock = 203000; break;
		case 16: vga256InfoRec.maxClock = 163000; break;
		case 24: vga256InfoRec.maxClock = 136000; break;
		case 32: vga256InfoRec.maxClock =  86000; break;
		}
		break;
	    }
	} else {
	    /* SDRAM based cards */

	    switch (I740Chipset) {
	    case PCI_CHIP_I740_AGP:
	    case PCI_CHIP_I740_PCI:
		switch (vgaBitsPerPixel) {
		case 8:  vga256InfoRec.maxClock = 203000; break;
		case 16: vga256InfoRec.maxClock = 163000; break;
		case 24: vga256InfoRec.maxClock = 128000; break;
		case 32: vga256InfoRec.maxClock =  86000; break;
		}
		break;
	    }
	}
    }

    return(TRUE);
}

/*
 * I740EnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down.  Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device.  This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
I740EnterLeave(enter)
    Bool enter;
{
    unsigned char temp;

#ifdef XFreeXDGA
    if ((vga256InfoRec.directMode & XF86DGADirectGraphics) && !enter) {
        if (XAACursorInfoRec.Flags & USE_HARDWARE_CURSOR) 
	    XAACursorInfoRec.HideCursor();
	return;
    }
#endif

    if (enter) {
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);

	/* 
	 * This is a global.  The CRTC base address depends on
	 * whether the VGA is functioning in color or mono mode.
	 * This is just a convenient place to initialize this
	 * variable.
	 */
	vgaIOBase = (inb(MSR_R) & IO_ADDR_SELECT) ? CGA_BASE : MDA_BASE;

	/*
	 * Here we deal with register-level access locks.  This
	 * is a generic VGA protection; most SVGA chipsets have
	 * similar register locks for their extended registers
	 * as well.
	 */
	/* Unprotect CRTC[0-7] */
	outb(vgaIOBase + 4, VERT_SYNC_END);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0x7F);
    } else {
	/*
	 * Here undo what was done above.
	 */

	/* Protect CRTC[0-7] */
	outb(vgaIOBase + 4, VERT_SYNC_END);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp | 0x80);

	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

/*
 * I740CalcFIFO --
 *
 * Calculate burst length and FIFO watermark.
 */

static unsigned int
I740CalcFIFO(freq)
    double freq;
{
    /*
     * Would like to calculate these values automatically, but a generic
     * algorithm does not seem possible.  Note: These FIFO water mark
     * values were tested on several cards and seem to eliminate the
     * all of the snow and vertical banding, but fine adjustments will
     * probably be required for other cards.
     */

    unsigned int wm = 0x18120000;

    switch (vgaBitsPerPixel) {
    case 8:
	if (I740HasSGRAM) {
	    if      (freq > 200) wm = 0x18120000;
	    else if (freq > 175) wm = 0x16110000;
	    else if (freq > 135) wm = 0x120E0000;
	    else                 wm = 0x100D0000;
	} else {
	    if      (freq > 200) wm = 0x18120000;
	    else if (freq > 175) wm = 0x16110000;
	    else if (freq > 135) wm = 0x120E0000;
	    else                 wm = 0x100D0000;
	}
	break;
    case 16:
	if (I740HasSGRAM) {
	    if      (freq > 140) wm = 0x2C1D0000;
	    else if (freq > 120) wm = 0x2C180000;
	    else if (freq > 100) wm = 0x24160000;
	    else if (freq >  90) wm = 0x18120000;
	    else if (freq >  50) wm = 0x16110000;
	    else if (freq >  32) wm = 0x13100000;
	    else                 wm = 0x120E0000;
	} else {
	    if      (freq > 160) wm = 0x28200000;
	    else if (freq > 140) wm = 0x2A1E0000;
	    else if (freq > 130) wm = 0x2B1A0000;
	    else if (freq > 120) wm = 0x2C180000;
	    else if (freq > 100) wm = 0x24180000;
	    else if (freq >  90) wm = 0x18120000;
	    else if (freq >  50) wm = 0x16110000;
	    else if (freq >  32) wm = 0x13100000;
	    else                 wm = 0x120E0000;
	}
	break;
    case 24:
	if (I740HasSGRAM) {
	    if      (freq > 130) wm = 0x31200000;
	    else if (freq > 120) wm = 0x2E200000;
	    else if (freq > 100) wm = 0x2C1D0000;
	    else if (freq >  80) wm = 0x25180000;
	    else if (freq >  64) wm = 0x24160000;
	    else if (freq >  49) wm = 0x18120000;
	    else if (freq >  32) wm = 0x16110000;
	    else                 wm = 0x13100000;
	} else {
	    if      (freq > 120) wm = 0x311F0000;
	    else if (freq > 100) wm = 0x2C1D0000;
	    else if (freq >  80) wm = 0x25180000;
	    else if (freq >  64) wm = 0x24160000;
	    else if (freq >  49) wm = 0x18120000;
	    else if (freq >  32) wm = 0x16110000;
	    else                 wm = 0x13100000;
	}
	break;
    case 32:
	if (I740HasSGRAM) {
	    if      (freq >  80) wm = 0x2A200000;
	    else if (freq >  60) wm = 0x281A0000;
	    else if (freq >  49) wm = 0x25180000;
	    else if (freq >  32) wm = 0x18120000;
	    else                 wm = 0x16110000;
	} else {
	    if      (freq >  80) wm = 0x29200000;
	    else if (freq >  60) wm = 0x281A0000;
	    else if (freq >  49) wm = 0x25180000;
	    else if (freq >  32) wm = 0x18120000;
	    else                 wm = 0x16110000;
	}
	break;
    }

    return wm;
}

/*
 * I740CalcVCLK --
 *
 * Determine the closest clock frequency to the one requested.
 */

#define MAX_VCO_FREQ 450.0
#define TARGET_MAX_N 30
#define REF_FREQ 66.66666666667

#define CALC_VCLK(m,n,p,d) \
    (double)m / ((double)n * (1 << p)) * (4 << (d << 1)) * REF_FREQ

static void
I740CalcVCLK(freq)
    double freq;
{
    int m, n, p, d;
    double f_out;
    double f_err;
    double f_vco;
    int m_best = 0, n_best = 0, p_best = 0, d_best = 0;
    double f_target = freq;
    double err_max = 0.005;
    double err_target = 0.001;
    double err_best = 999999.0;

    p_best = p = log(MAX_VCO_FREQ/f_target)/log((double)2);
    d_best = d = 0;

    f_vco = f_target * (1 << p);

    n = 2;
    do {
	n++;
	m = f_vco / (REF_FREQ / (double)n) / (double)4.0 + 0.5;
	if (m < 3) m = 3;
	f_out = CALC_VCLK(m,n,p,d);
	f_err = 1.0 - (f_target/f_out);
	if (fabs(f_err) < err_max) {
	    m_best = m;
	    n_best = n;
	    err_best = f_err;
	}
    } while ((fabs(f_err) >= err_target) &&
	     ((n <= TARGET_MAX_N) || (fabs(err_best) > err_max)));

    if (fabs(f_err) < err_target) {
	m_best = m;
        n_best = n;
    }

    new->VideoClk2_M          = (m_best-2) & 0xFF;
    new->VideoClk2_N          = (n_best-2) & 0xFF;
    new->VideoClk2_MN_MSBs    = ((((n_best-2) >> 4) & VCO_N_MSBS) |
				 (((m_best-2) >> 8) & VCO_M_MSBS));
    new->VideoClk2_DivisorSel = ((p_best << 4) |
				 (d_best ? 4 : 0) |
				 REF_DIV_1);

#ifdef DEBUG
    ErrorF("Setting dot clock to %.6lf MHz "
	   "[ %02X %02X %02X ] "
	   "[ %d %d %d %d ]\n",
	   CALC_VCLK(m_best,n_best,p_best,d_best),
	   new->VideoClk2_M,
	   new->VideoClk2_N,
	   new->VideoClk2_DivisorSel,
	   m_best, n_best, p_best, d_best);
#endif
}

/*
 * I740Restore --
 *
 * This function restores a video mode.  It basically writes out all of
 * the registers that have previously been saved in the vgaI740Rec data 
 * structure.
 *
 * Note that "Restore" is a little bit incorrect.  This function is also
 * used when the server enters/changes video modes.  The mode definitions 
 * have previously been initialized by the Init() function, below.
 */
static void 
I740Restore(restore)
    vgaI740Ptr restore;
{
    unsigned char temp;
    unsigned int  itemp;

    vgaProtect(TRUE); /* Blank the screen */

    /*
     * Whatever code is needed to get things back to bank zero should be
     * placed here.  Things should be in the same state as when the
     * Save/Init was done.
     */

    outb(MRX, ACQ_CNTL_2); temp = inb(MRX+1);
    if ((temp & FRAME_CAP_MODE) == SINGLE_CAP_MODE) {
	outb(MRX, COL_KEY_CNTL_1); temp = inb(MRX+1);
	temp |= BLANK_DISP_OVERLAY; /* Disable the overlay */
	outb(MRX, COL_KEY_CNTL_1); outb(MRX+1, temp);
    } else {
	temp &= ~FRAME_CAP_MODE;
	outb(MRX, ACQ_CNTL_2); outb(MRX+1, temp);
    }
    usleep(50000);

    /* Turn off DRAM Refresh */
    outb(XRX, DRAM_EXT_CNTL); outb(XRX+1, DRAM_REFRESH_DISABLE);

    usleep(1000); /* Wait 1 ms */

    /* Write the M, N and P values */
    outb(XRX, VCLK2_VCO_M);       outb(XRX+1, restore->VideoClk2_M);
    outb(XRX, VCLK2_VCO_N);       outb(XRX+1, restore->VideoClk2_N);
    outb(XRX, VCLK2_VCO_MN_MSBS); outb(XRX+1, restore->VideoClk2_MN_MSBs);
    outb(XRX, VCLK2_VCO_DIV_SEL); outb(XRX+1, restore->VideoClk2_DivisorSel);

    /*
     * Turn on 8 bit dac mode, if requested.  This is needed to make
     * sure that vgaHWRestore writes the values into the DAC properly.
     * The problem occurs if 8 bit dac mode is requested and the HW is
     * in 6 bit dac mode.  If this happens, all the values are
     * automatically shifted left twice by the HW and incorrect colors
     * will be displayed on the screen.  The only time this can happen
     * is at server startup time and when switching back from a VT.
     */
    outb(XRX, PIXPIPE_CONFIG_0); temp = inb(XRX+1);
    temp &= 0x7F; /* Save all but the 8 bit dac mode bit */
    temp |= (restore->PixelPipeCfg0 & DAC_8_BIT);
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, temp);

    /*
     * This function handles restoring the generic VGA registers.
     */
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

    outb(vgaIOBase + 4, EXT_VERT_TOTAL);
    outb(vgaIOBase + 5, restore->ExtVertTotal);

    outb(vgaIOBase + 4, EXT_VERT_DISPLAY);
    outb(vgaIOBase + 5, restore->ExtVertDispEnd);

    outb(vgaIOBase + 4, EXT_VERT_SYNC_START);
    outb(vgaIOBase + 5, restore->ExtVertSyncStart);

    outb(vgaIOBase + 4, EXT_VERT_BLANK_START);
    outb(vgaIOBase + 5, restore->ExtVertBlankStart);

    outb(vgaIOBase + 4, EXT_HORIZ_TOTAL);
    outb(vgaIOBase + 5, restore->ExtHorizTotal);

    outb(vgaIOBase + 4, EXT_HORIZ_BLANK);
    outb(vgaIOBase + 5, restore->ExtHorizBlank);

    outb(vgaIOBase + 4, EXT_OFFSET);
    outb(vgaIOBase + 5, restore->ExtOffset);

    outb(vgaIOBase + 4, INTERLACE_CNTL); temp = inb(vgaIOBase + 5);
    temp &= ~INTERLACE_ENABLE;
    temp |= restore->InterlaceControl;
    outb(vgaIOBase + 4, INTERLACE_CNTL); outb(vgaIOBase + 5, temp);

    outb(XRX, ADDRESS_MAPPING); temp = inb(XRX+1);
    temp &= 0xE0; /* Save reserved bits 7:5 */
    temp |= restore->AddressMapping;
    outb(XRX, ADDRESS_MAPPING); outb(XRX+1, temp);

    outb(XRX, BITBLT_CNTL); temp = inb(XRX+1);
    temp &= ~COLEXP_MODE;
    temp |= restore->BitBLTControl;
    outb(XRX, BITBLT_CNTL); outb(XRX+1, temp);

    outb(XRX, DISPLAY_CNTL); temp = inb(XRX+1);
    temp &= ~(VGA_WRAP_MODE | GUI_MODE);
    temp |= restore->DisplayControl;
    outb(XRX, DISPLAY_CNTL); outb(XRX+1, temp);

    outb(XRX, PIXPIPE_CONFIG_0); temp = inb(XRX+1);
    temp &= 0x64; /* Save reserved bits 6:5,2 */
    temp |= restore->PixelPipeCfg0;
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, temp);

    outb(XRX, PIXPIPE_CONFIG_2); temp = inb(XRX+1);
    temp &= 0xF3; /* Save reserved bits 7:4,1:0 */
    temp |= restore->PixelPipeCfg2;
    outb(XRX, PIXPIPE_CONFIG_2); outb(XRX+1, temp);

    outb(XRX, PLL_CNTL); temp = inb(XRX+1);
    temp &= ~PLL_MEMCLK_SEL;
#if 1
    temp = restore->PLLControl; /* To fix the 2.3X BIOS problem */
#else
    temp |= restore->PLLControl;
#endif
    outb(XRX, PLL_CNTL); outb(XRX+1, temp);

    outb(XRX, PIXPIPE_CONFIG_1); temp = inb(XRX+1);
    temp &= ~DISPLAY_COLOR_MODE;
    temp |= restore->PixelPipeCfg1;
    outb(XRX, PIXPIPE_CONFIG_1); outb(XRX+1, temp);

    itemp = INREG(FWATER_BLC);
    itemp &= ~(LMI_BURST_LENGTH | LMI_FIFO_WATERMARK);
    itemp |= restore->LMI_FIFO_Watermark;
    OUTREG(FWATER_BLC, itemp);

    /* Turn on DRAM Refresh */
    outb(XRX, DRAM_EXT_CNTL); outb(XRX+1, DRAM_REFRESH_60HZ);

    outb(MRX, COL_KEY_CNTL_1); temp = inb(MRX+1);
    temp &= ~BLANK_DISP_OVERLAY; /* Re-enable the overlay */
    outb(MRX, COL_KEY_CNTL_1); outb(MRX+1, temp);

    if (!(restore->std.Attribute[0x10] & 0x01)) {
	/*
	 * Call vgaHWRestore again to restore the fonts correctly only
	 * when switching back to text mode.  Wait 50 milliseconds for
	 * the DRAM to turn back on before writing the data back.
	 */
	usleep(50000);
	vgaHWRestore((vgaHWPtr)restore);
    }

    vgaProtect(FALSE); /* Turn on screen */

    outb(XRX, IO_CTNL); temp = inb(XRX+1);
    temp &= ~(EXTENDED_ATTR_CNTL | EXTENDED_CRTC_CNTL);
    temp |= restore->IOControl;
    outb(XRX, IO_CTNL); outb(XRX+1, temp);
}

/*
 * I740Save --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaI740Rec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void *
I740Save(save)
    vgaI740Ptr save;
{
    /*
     * Whatever code is needed to get back to bank zero goes here.
     */

    /*
     * This function will handle creating the data structure and filling
     * in the generic VGA portion.
     */
    save = (vgaI740Ptr)vgaHWSave((vgaHWPtr)save, sizeof(vgaI740Rec));

    /*
     * The port I/O code necessary to read in the extended registers 
     * into the fields of the vgaI740Rec structure goes here.
     */

    outb(XRX, IO_CTNL);           save->IOControl = inb(XRX+1);
    outb(XRX, ADDRESS_MAPPING);   save->AddressMapping = inb(XRX+1);
    outb(XRX, BITBLT_CNTL);       save->BitBLTControl = inb(XRX+1);
    outb(XRX, VCLK2_VCO_M);       save->VideoClk2_M = inb(XRX+1);
    outb(XRX, VCLK2_VCO_N);       save->VideoClk2_N = inb(XRX+1);
    outb(XRX, VCLK2_VCO_MN_MSBS); save->VideoClk2_MN_MSBs = inb(XRX+1);
    outb(XRX, VCLK2_VCO_DIV_SEL); save->VideoClk2_DivisorSel = inb(XRX+1);
    outb(XRX, PLL_CNTL);          save->PLLControl = inb(XRX+1);

    outb(vgaIOBase + 4, EXT_VERT_TOTAL);
    save->ExtVertTotal = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_VERT_DISPLAY);
    save->ExtVertDispEnd = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_VERT_SYNC_START);
    save->ExtVertSyncStart = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_VERT_BLANK_START);
    save->ExtVertBlankStart = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_HORIZ_TOTAL);
    save->ExtHorizTotal = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_HORIZ_BLANK);
    save->ExtHorizBlank = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, EXT_OFFSET);
    save->ExtOffset = inb(vgaIOBase + 5);

    outb(vgaIOBase + 4, INTERLACE_CNTL);
    save->InterlaceControl = inb(vgaIOBase + 5);

    outb(XRX, PIXPIPE_CONFIG_0); save->PixelPipeCfg0 = inb(XRX+1);
    outb(XRX, PIXPIPE_CONFIG_1); save->PixelPipeCfg1 = inb(XRX+1);
    outb(XRX, PIXPIPE_CONFIG_2); save->PixelPipeCfg2 = inb(XRX+1);
    outb(XRX, DISPLAY_CNTL);     save->DisplayControl = inb(XRX+1);

    save->LMI_FIFO_Watermark = INREG(FWATER_BLC);

    return ((void *) save);
}

/*
 * I740Init --
 *
 * This is the most important function (after the Probe) function.  This
 * function fills in the vgaI740Rec with all of the register values needed
 * to enable either a 256-color mode (for the color server) or a 16-color
 * mode (for the monochrome server).
 *
 * The 'mode' parameter describes the video mode.  The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.  The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
static Bool
I740Init(mode)
    DisplayModePtr mode;
{
    double dclk = vga256InfoRec.clock[mode->Clock]/1000.0;

    /*
     * This will allocate the datastructure and initialize all of the
     * generic VGA registers.
     */
    if (!vgaHWInit(mode,sizeof(vgaI740Rec)))
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

    /* Make sure we only change 3:0 */
    switch (vgaBitsPerPixel) {
    case 8:
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 3;
	new->ExtOffset      = vga256InfoRec.displayWidth >> 11;
	new->PixelPipeCfg1 = DISPLAY_8BPP_MODE;
	new->BitBLTControl = COLEXP_8BPP;
	break;
    case 16:
	if (xf86weight.green == 5) {
	    new->PixelPipeCfg1 = DISPLAY_15BPP_MODE;
	} else {
	    new->PixelPipeCfg1 = DISPLAY_16BPP_MODE;
	}
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 2;
	new->ExtOffset      = vga256InfoRec.displayWidth >> 10;
	new->BitBLTControl = COLEXP_16BPP;
	break;
    case 24:
	new->std.CRTC[0x13] = (vga256InfoRec.displayWidth * 3) >> 3;
	new->ExtOffset      = (vga256InfoRec.displayWidth * 3)>> 11;
	new->PixelPipeCfg1 = DISPLAY_24BPP_MODE;
	new->BitBLTControl = COLEXP_24BPP;
	break;
    case 32:
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 1;
	new->ExtOffset      = vga256InfoRec.displayWidth >> 9;
	new->PixelPipeCfg1 = DISPLAY_32BPP_MODE;
	new->BitBLTControl = COLEXP_RESERVED; /* Not implemented on i740 */
	break;
    default:
	break;
    }

    /* Turn on 8 bit dac if requested */
    if (OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options))
	new->PixelPipeCfg0 = DAC_8_BIT;
    else
	new->PixelPipeCfg0 = DAC_6_BIT;

    /* Turn on gamma correction */
    if (vgaBitsPerPixel > 8) {
	extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
	int i;
	for (i = 0; i < 256; i++) {
	    new->std.DAC[i*3+0] = xf86rGammaMap[i];
	    new->std.DAC[i*3+1] = xf86gGammaMap[i];
	    new->std.DAC[i*3+2] = xf86bGammaMap[i];
	}
    }
    new->PixelPipeCfg2 = DISPLAY_GAMMA_ENABLE | OVERLAY_GAMMA_ENABLE;

    /* Turn on Extended VGA Interpretation */
    new->IOControl = EXTENDED_CRTC_CNTL;

    /* Turn on linear and page mapping */
    new->AddressMapping = LINEAR_MODE_ENABLE | PAGE_MAPPING_ENABLE;

    /* Turn on GUI mode */
    new->DisplayControl = HIRES_MODE;

    /* Set the MCLK freq */
    if (OFLG_ISSET(OPTION_SLOW_DRAM, &vga256InfoRec.options))
	new->PLLControl = PLL_MEMCLK__66667KHZ; /*  66 MHz */
    else
	new->PLLControl = PLL_MEMCLK_100000KHZ; /* 100 MHz -- use as default */

    /* Calculate the extended CRTC regs */
    new->ExtVertTotal = (mode->CrtcVTotal - 2) >> 8;
    new->ExtVertDispEnd = (mode->CrtcVDisplay - 1) >> 8;
    new->ExtVertSyncStart = mode->CrtcVSyncStart >> 8;
    new->ExtVertBlankStart = mode->CrtcVSyncStart >> 8;
    new->ExtHorizTotal = ((mode->CrtcHTotal >> 3) - 5) >> 8;
    new->ExtHorizBlank = ((mode->CrtcHSyncEnd >> 3) & 0x40) >> 6;

    /* Turn on interlaced mode if necessary */
    if (mode->Flags & V_INTERLACE)
	new->InterlaceControl = INTERLACE_ENABLE;
    else
	new->InterlaceControl = INTERLACE_DISABLE;

    /*
     * Set the overscan color to 0.
     * NOTE: This only affects >8bpp mode.
     */
    new->std.Attribute[0x11] = 0;

    /*
     * Calculate the VCLK that most closely matches the requested dot
     * clock.
     */
    I740CalcVCLK(dclk);

    /* Since we program the clocks ourselves, always use VCLK2. */
    new->std.MiscOutReg |= 0x0C;

    /* Calculate the FIFO Watermark and Burst Length. */
    new->LMI_FIFO_Watermark = I740CalcFIFO(dclk);

    return(TRUE);
}

/*
 * I740Adjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.  This is used to implement the
 * virtual window.
 */
static void 
I740Adjust(x, y)
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

    switch (vgaBitsPerPixel) {
    case  8:	
	break;
    case 16:
	Base *= 2;
	break;
    case 24:
	/*
	 * The last bit does not seem to have any effect on the start
	 * address register in 24bpp mode, so...
	 */
	Base &= 0xFFFFFFFE; /* ...ignore the last bit. */
	Base *= 3;
	break;
    case 32:
	Base *= 4;
	break;
    }

    outb(vgaIOBase + 4, START_ADDR_LO);
    outb(vgaIOBase + 5, Base & 0x000000FF);

    outb(vgaIOBase + 4, START_ADDR_HI);
    outb(vgaIOBase + 5, (Base & 0x0000FF00) >>  8);

    outb(vgaIOBase + 4, EXT_START_ADDR_HI);
    outb(vgaIOBase + 5, (Base & 0x3FC00000) >> 22);

    outb(vgaIOBase + 4, EXT_START_ADDR);
    outb(vgaIOBase + 5, ((Base & 0x003F0000) >> 16) | EXT_START_ADDR_ENABLE);
}

/*
 * I740SaveScreen --
 *
 * This function gets called before and after a synchronous reset is
 * performed on the SVGA chipset during a mode-changing operation.  Some
 * chipsets will reset registers that should not be changed during this.
 * If your function is one of these, then you can use this function to
 * save and restore the registers.
 *
 * Most chipsets do not require this function, and instead put
 * 'vgaHWSaveScreen' in the vgaVideoChipRec structure.
 */
static void
I740SaveScreen(mode)
    int mode;
{
    if (mode == SS_START) {
	/*
	 * Save an registers that will be destroyed by the reset
	 * into static variables.
	 */

	/*
	 * Start sequencer reset.
	 */
	outw(SRX, 0x0100);
    } else {
	/*
	 * End sequencer reset.
	 */
	outw(SRX, 0x0300);

	/*
	 * Now restore those registers.
	 */
    }
}

/*
 * I740FbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 * For most drivers, this function is not required, and 'NoopDDA' is put
 * in the vgaVideoChipRec structure.
 */
static Bool
I740ScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    ScreenPtr pScreen;
    pointer pbits;
    int xsize, ysize, dpix, dpiy, width;
{
    pScreen->StoreColors = I740StoreColors;
    return TRUE;
}

/*
 * I740FbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 * For most drivers, this function is not required, and 'NoopDDA' is put
 * in the vgaVideoChipRec structure.
 */
static void
I740FbInit()
{
    /* Hook out screen init so that we can substitute our own StoreColors */
    vgaSetScreenInitHook(I740ScreenInit);

    /* Print out some useful information about the system. */
    if (xf86Verbose) {
	ErrorF("%s %s: ",
	       (vga256InfoRec.chipset ? XCONFIG_PROBED : XCONFIG_GIVEN),
	       vga256InfoRec.name);
	switch (I740Chipset) {
	case PCI_CHIP_I740_AGP:
	    ErrorF("Intel i740 (AGP)");
	    break;
	case PCI_CHIP_I740_PCI:
	    ErrorF("Intel i740 (PCI)");
	    break;
	}
	ErrorF(" chip\n");

	ErrorF("%s %s: %s: Memory Type: %s\n",
	       ((OFLG_ISSET(OPTION_SGRAM, &vga256InfoRec.options) ||
		 OFLG_ISSET(OPTION_SDRAM, &vga256InfoRec.options))
		? XCONFIG_GIVEN : XCONFIG_PROBED),
	       vga256InfoRec.name,
	       vga256InfoRec.chipset,
	       (I740HasSGRAM ? "SGRAM" : "SDRAM"));
    }

    /* Map IO registers to virtual address space. */
    I740MMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
				 (pointer)(I740MMIOAddr), 0x80000);
    if (!I740MMIOBase)
	FatalError("i740: Can't memory map IO registers\n");
    if (xf86Verbose)
	ErrorF("%s %s: %s: MMIO registers at 0x%08lX\n",
	       (vga256InfoRec.IObase ? XCONFIG_GIVEN : XCONFIG_PROBED),
	       vga256InfoRec.name,
	       vga256InfoRec.chipset,
	       I740MMIOAddr);

    /* Use linear by default. */
    I740.ChipLinearBase = I740LinearAddr;
    I740.ChipUseLinearAddressing = TRUE;

    /*
     * Setup the size of the linear frame buffer memory aperature.
     * NOTE: this must be a power of 2 for the vga support code to
     * operate properly.
     */
    switch(I740Chipset) {
    case PCI_CHIP_I740_AGP:
    case PCI_CHIP_I740_PCI:
	I740.ChipLinearSize = 16*1024*1024; /* exactly 16MB */
	break;
    }

    if (xf86Verbose)
	ErrorF("%s %s: %s: Linear framebuffer at 0x%08lX\n",
	       (vga256InfoRec.MemBase ? XCONFIG_GIVEN : XCONFIG_PROBED),
	       vga256InfoRec.name,
	       vga256InfoRec.chipset,
	       I740.ChipLinearBase);

    /* Use the first 4KB after the framebuffer for the cursor image */
    I740CursorStart = (vga256InfoRec.virtualY *
		       vga256InfoRec.displayWidth *
		       vgaBytesPerPixel + 0x0FFF) & ~0x0FFF;

    /* HW cursor must fit into the first 4MB of the frame buffer */
    if (I740CursorStart >= 4*1024*1024) {
	I740CursorStart = 0;
	ErrorF("%s %s: %s: Warning: "
	       "Cannot fit hardware cursor into frame buffer\n",
	       (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) ?
		XCONFIG_GIVEN : XCONFIG_PROBED),
	       vga256InfoRec.name,
	       vga256InfoRec.chipset);
    }

    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
	I740CursorStart) {

	I740CursorInit();

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

    if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
	I740AccelInit();
    } else {
	if (xf86Verbose)
	    ErrorF("%s %s: %s: Acceleration disabled\n",
		   (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) ?
		    XCONFIG_GIVEN : XCONFIG_PROBED),
		   vga256InfoRec.name,
		   vga256InfoRec.chipset);
    }
}

static int
I740ValidMode(mode, verbose, flag)
    DisplayModePtr mode;
    Bool verbose;
    int flag;
{
    /*
     * Code to check if a mode is suitable for the selected chipset.
     * In most cases this can just return MODE_OK.
     */
    if (mode->Flags & V_INTERLACE) {
	if (verbose) {
	    ErrorF("%s %s: %s: Removing interlaced mode \"%s\"\n",
		   XCONFIG_PROBED,
		   vga256InfoRec.name,
		   vga256InfoRec.chipset,
		   mode->name);
	}
 	return(MODE_BAD);
    }

    return(MODE_OK);
}

/*
 * I740DisplayPowerManagementSet --
 *
 * Sets VESA Display Power Management Signaling (DPMS) Mode.  */
#ifdef DPMSExtension
static void
I740DisplayPowerManagementSet(PowerManagementMode)
    int PowerManagementMode;
{
    unsigned char SEQ01;
    int DPMSSyncSelect;

    if (!xf86VTSema)
	return;

    switch (PowerManagementMode) {
    case DPMSModeOn:
	/* Screen: On; HSync: On, VSync: On */
	SEQ01 = 0x00;
	DPMSSyncSelect = HSYNC_ON | VSYNC_ON;
	break;
    case DPMSModeStandby:
	/* Screen: Off; HSync: Off, VSync: On */
	SEQ01 = 0x20;
	DPMSSyncSelect = HSYNC_OFF | VSYNC_ON;
	break;
    case DPMSModeSuspend:
	/* Screen: Off; HSync: On, VSync: Off */
	SEQ01 = 0x20;
	DPMSSyncSelect = HSYNC_ON | VSYNC_OFF;
	break;
    case DPMSModeOff:
	/* Screen: Off; HSync: Off, VSync: Off */
	SEQ01 = 0x20;
	DPMSSyncSelect = HSYNC_OFF | VSYNC_OFF;
	break;
    }

    /* Turn the screen on/off */
    outb(SRX, 0x01);
    SEQ01 |= inb(SRX+1) & ~0x20;
    outb(SRX+1, SEQ01);

    /* Set the DPMS mode */
    outb(XRX, DPMS_SYNC_SELECT);
    outb(XRX+1, DPMSSyncSelect);
}
#endif
