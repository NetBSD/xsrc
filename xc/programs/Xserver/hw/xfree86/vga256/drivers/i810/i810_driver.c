/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810_driver.c,v 1.1.2.2 1999/11/18 19:06:17 hohndel Exp $ */
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
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 *   Based on i740 driver by
 *      Kevin E. Martin <kevin@precisioninsight.com>
 *
 * $PI$
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
#include "i810.h"
#include "i810_reg.h"

/* For fabs() and ceil() */
#include <math.h>

typedef struct {
   vgaHWRec std;               /* good old IBM VGA */
   unsigned char DisplayControl;
   unsigned char PixelPipeCfg0;
   unsigned char PixelPipeCfg1;
   unsigned char PixelPipeCfg2;
   unsigned short VideoClk2_M;
   unsigned short VideoClk2_N;
   unsigned char VideoClk2_DivisorSel;
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
   unsigned int  LprbTail;
   unsigned int  LprbHead;
   unsigned int  LprbStart;
   unsigned int  LprbLen;
   unsigned short IntrHwStatMask;
   unsigned short IntrEnabled;
   unsigned short IntrIdentity;
   unsigned short IntrMask;
   unsigned short ErrorMask;
} vgaI810Rec, *vgaI810Ptr;

/* Globals that are allocated in this module. */
unsigned char *I810MMIOBase = NULL;
int I810Chipset = -1;
int I810LinearAddr = 0;
int I810MMIOAddr = 0;

unsigned long I810CursorPhysical = 0;
int I810CursorOffset = 0;

I810RingBuffer I810LpRing;


pciTagRec I810PciTag;


static I810MemRange I810FrameBuffer;
I810MemRange I810Cursor;
I810MemRange I810Mprotect;




#ifndef I810_DEBUG
int I810_DEBUG = (0
/*     		  | DEBUG_ALWAYS_SYNC  */
  		  | DEBUG_VERBOSE_ACCEL 
/*  		  | DEBUG_VERBOSE_SYNC */
/*  		  | DEBUG_VERBOSE_VGA */
/*  		  | DEBUG_VERBOSE_RING    */
/*  		  | DEBUG_VERBOSE_OUTREG  */
/*  		  | DEBUG_VERBOSE_MEMORY */
		  );
#endif


/*
 * Forward definitions for the functions that make up the driver.  See
 * the definitions of these functions for the real scoop.
 */
static Bool     I810Probe();
static char *   I810Ident();
static void     I810EnterLeave();
static Bool     I810Init();
static int	I810ValidMode();
static void *   I810Save();
static void     I810Restore();
static void     I810Adjust();
static void     I810SaveScreen();
static void	I810FbInit();
static void	I810DisplayPowerManagementSet();
static int      I810PitchAdjust(void);


/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data 
 * that defines how the driver operates.
 */
vgaVideoChipRec I810 = {
    /* 
     * Function pointers
     */
    I810Probe,
    I810Ident,
    I810EnterLeave,
    I810Init,
    I810ValidMode,
    I810Save,
    I810Restore,
    I810Adjust,
    I810SaveScreen,
    (void (*)())NoopDDA,	/* I810GetMode, */
    I810FbInit,
    (void (*)())NoopDDA,	/* I810SetRead, */
    (void (*)())NoopDDA,	/* I810SetWrite, */
    (void (*)())NoopDDA,	/* I810SetReadWrite, */

    0x10000,			/* banked mode stuff */
    0x10000,			/*  */
    16,				/*  */
    0xFFFF,			/*  */
    0x00000, 0x10000,		/*  */
    0x00000, 0x10000,		/*  */
    FALSE,			/* banked mode stuff */
    VGA_NO_DIVIDE_VERT,
    {0,},
    8,				/* scanline padding - replace pitchadjust? */
    TRUE,			/* support lfb */
    0,
    0,
    TRUE,			/* support 16bpp */
    TRUE,			/* support 24bpp */
    FALSE,			/* support 32bpp */
    NULL,
    1,
    1
};

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */
#define new ((vgaI810Ptr)vgaNewVideoState)




/*
 * I810Ident --
 *
 * Returns the string name for supported chipset 'n'. 
 */
static char *
I810Ident(n)
    int n;
{
    static char *chipsets[] = {
	"i810", "i810-dc100", "i810e"
    };

    if (n + 1 > sizeof(chipsets) / sizeof(char *))
	return(NULL);
    else
	return(chipsets[n]);
}



/*
 * I810Probe --
 *
 * This is the function that makes a yes/no decision about whether or not
 * a chipset supported by this driver is present or not.  The server will
 * call each driver's probe function in sequence, until one returns TRUE
 * or they all fail.
 */
static Bool
I810Probe()
{
   pciConfigPtr pcr;
   pciConfigPtr I810pcr = NULL;
   pciConfigPtr I810BridgePcr = NULL;
   int temp;
   int i;

   /*
    * Set up I/O ports to be used by this card.  Only do the second
    * xf86AddIOPorts() if there are non-standard ports for this
    * chipset.
    */
   xf86ClearIOPortList(vga256InfoRec.scrnIndex);
   xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

   /*
    * First we attempt to figure out if one of the supported chipsets
    * is present.
    */
   if (vga256InfoRec.chipset) {
      char *chipset;
      for (i = 0; (chipset = I810Ident(i)); i++) {
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
   i = 0;
   I810Chipset = -1;
   if (vgaPCIInfo && vgaPCIInfo->AllCards) {
      while (pcr = vgaPCIInfo->AllCards[i++]) {
	 if (pcr->_vendor == PCI_VENDOR_INTEL) {
	    int id = pcr->_device;

	    if (vga256InfoRec.chipID) {
	       ErrorF("%s %s: i810 chipset override, using ChipID "
		      "value (0x%04X) instead of PCI value (0x%04X)\n",
		      XCONFIG_GIVEN,
		      vga256InfoRec.name,
		      vga256InfoRec.chipID,
		      pcr->_device);
	       id = vga256InfoRec.chipID;
	    }

	    switch (id) {
	    case PCI_CHIP_I810:
	       vga256InfoRec.chipset = I810Ident(0);
	       I810Chipset = id;
	       I810LinearAddr = pcr->_base0 & 0xfe000000; /* 32/64M */
	       I810MMIOAddr = pcr->_base1 & 0xfff80000; /* 512K window */
	       I810pcr = pcr; 
	       break;

	    case PCI_CHIP_I810_DC100:
	       vga256InfoRec.chipset = I810Ident(1);
	       I810Chipset = id;
	       I810LinearAddr = pcr->_base0 & 0xfe000000; /* 32/64M */
	       I810MMIOAddr = pcr->_base1 & 0xfff80000; /* 512K window */
	       I810pcr = pcr; 
	       break;

	    case PCI_CHIP_I810_E:
	       vga256InfoRec.chipset = I810Ident(2);
	       I810Chipset = id;
	       I810LinearAddr = pcr->_base0 & 0xfe000000; /* 32/64M */
	       I810MMIOAddr = pcr->_base1 & 0xfff80000; /* 512K window */
	       I810pcr = pcr; 
	       break;

	    case PCI_CHIP_I810_BRIDGE:
	    case PCI_CHIP_I810_DC100_BRIDGE:
	    case PCI_CHIP_I810_E_BRIDGE:
	       I810BridgePcr = pcr;
	       break;
	    }
	 }
      }
   } else 
      return(FALSE);
   

   if (I810Chipset == -1) {
      if (vga256InfoRec.chipset)
	 ErrorF("%s %s: i810: Unknown chipset\n",
		XCONFIG_PROBED, vga256InfoRec.name);
      return(FALSE);
   }

   /*
    * Okay, it's an i810.
    */
   I810PciTag = pcibusTag(I810pcr->_bus, I810pcr->_cardnum, I810pcr->_func);

   /* Override the linear address, if user specified one. */
   if (vga256InfoRec.MemBase) {
      if (vga256InfoRec.MemBase != I810LinearAddr) {
	 ErrorF("%s %s: i810 linear address override, using 0x%08X "
		"instead of probed value (0x%08X)\n",
		XCONFIG_GIVEN,
		vga256InfoRec.name,
		vga256InfoRec.MemBase,
		I810LinearAddr);
      }
      I810LinearAddr = vga256InfoRec.MemBase;
   }

   /* Override the MMIO address, if user specified one. */
   if (vga256InfoRec.IObase) {
      if (vga256InfoRec.IObase != I810MMIOAddr) {
	 ErrorF("%s %s: i810 linear address override, using 0x%08X "
		"instead of probed value (0x%08X)\n",
		XCONFIG_GIVEN,
		vga256InfoRec.name,
		vga256InfoRec.IObase,
		I810MMIOAddr);
      }
      I810MMIOAddr = vga256InfoRec.IObase;
   }

   /* Enable I/O Ports, etc. */
   I810EnterLeave(ENTER);

   /* Look for kernel GTT support, else try to fall back.  Currently
    * ignores any dedicated video ram on the 810-dc100, as this is
    * much slower than using system ram.  The main use of the dcache
    * is as a zbuffer in a (forthcoming) 3d capable server.
    */
   if (!I810CharacterizeSystemRam( I810BridgePcr )) {
      I810EnterLeave(LEAVE);
      return FALSE;
   }

   if (!vga256InfoRec.videoRam) {
      FatalError("\nCouldn't allocate physical memory for frame buffer.\n");
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
   OFLG_SET(OPTION_LINEAR, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_NOACCEL, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_SW_CURSOR, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_HW_CURSOR, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_MMIO, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_DAC_6_BIT, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_DAC_8_BIT, &I810.ChipOptionFlags);
   OFLG_SET(OPTION_SLOW_DRAM, &I810.ChipOptionFlags);

   /* Make sure users don't set mutually exclusive options!! */
   if (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
       OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
      FatalError(
	 "\nOptions \"sw_cursor\" and \"hw_cursor\" are incompatible.\n");
   if (OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options) &&
       OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options))
      FatalError(
	 "\nOptions \"dac_6_bit\" and \"dac_8_bit\" are incompatible.\n");

   /* Turn on 8 bit DAC mode, if 6 bit mode is not set or if the screen
    * depth is greater than 8bpp.
    */
   if (!OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options) ||
       vgaBitsPerPixel > 8)
      OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);

   /* Turn off DAC_6_BIT if screen depth is greater than 8bpp 
    */
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
   vga256InfoRec.DPMSSet = I810DisplayPowerManagementSet;
#endif

   /*
    * For the i740, the max RAMDAC dot clock is 220MHz, but the max
    * pixel pipeline frequency is 203MHz so we'll use the 203MHz
    * value.  
    *
    * The i810 datasheet on intel's site has a max ramdac listed as
    * 230 mHz.  I'm sticking with the 203 figure until I get some more
    * information.
    */
   if (vga256InfoRec.dacSpeeds[0]) {
      vga256InfoRec.maxClock = vga256InfoRec.dacSpeeds[0];
   } else {
      switch (I810Chipset) {
      case PCI_CHIP_I810:
      case PCI_CHIP_I810_DC100:
      case PCI_CHIP_I810_E:
	 switch (vgaBitsPerPixel) {
	 case 8:  vga256InfoRec.maxClock = 203000; break;
	 case 16: vga256InfoRec.maxClock = 163000; break;
	 case 24: vga256InfoRec.maxClock = 128000; break;
	 case 32: vga256InfoRec.maxClock =  86000; break;
	 }
	 break;
      }

   }


   /* Undocumented bug in blit engine which is seen when screen pitch
    * is not a multiple of 64 bytes.  
    */
   vgaSetPitchAdjustHook(I810PitchAdjust);

   return(TRUE);
}



static int I810PitchAdjust(void) 
{
   int vx = vga256InfoRec.virtualX;
   int bytesperpixel = vgaBitsPerPixel >> 3;

   while ((vx*bytesperpixel) & 63)
      vx++;

   if (vx != vga256InfoRec.virtualX)
      ErrorF("%s %s: %s: Display width padded to %d bytes (%d pixels).\n",
	     XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.chipset,
	     vx*bytesperpixel, vx);

   return vx;
}

/*
 * I810EnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down.  Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device.  This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
I810EnterLeave(enter)
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
 * I810CalcVCLK --
 *
 * Determine the closest clock frequency to the one requested.
 */
#define MAX_VCO_FREQ 600.0
#define TARGET_MAX_N 30
#define REF_FREQ 24.0

#define CALC_VCLK(m,n,p) \
    (double)m / ((double)n * (1 << p)) * 4 * REF_FREQ

static void
I810CalcVCLK( double freq )
{
    int m, n, p;
    double f_out, f_best;
    double f_err;
    double f_vco;
    int m_best = 0, n_best = 0, p_best = 0;
    double f_target = freq;
    double err_max = 0.005;
    double err_target = 0.001;
    double err_best = 999999.0;

    p_best = p = log(MAX_VCO_FREQ/f_target)/log((double)2);
    f_vco = f_target * (1 << p);

    n = 2;
    do {
	n++;
	m = f_vco / (REF_FREQ / (double)n) / (double)4.0 + 0.5;
	if (m < 3) m = 3;
	f_out = CALC_VCLK(m,n,p);
	f_err = 1.0 - (f_target/f_out);
	if (fabs(f_err) < err_max) {
	    m_best = m;
	    n_best = n;
	    f_best = f_out;
	    err_best = f_err;
	}
    } while ((fabs(f_err) >= err_target) &&
	     ((n <= TARGET_MAX_N) || (fabs(err_best) > err_max)));

    if (fabs(f_err) < err_target) {
	m_best = m;
        n_best = n;
    }

    new->VideoClk2_M          = (m_best-2) & 0x3FF;
    new->VideoClk2_N          = (n_best-2) & 0x3FF;
    new->VideoClk2_DivisorSel = (p_best << 4);

    ErrorF("%s %s: Setting dot clock to %.1lf MHz "
	   "[ 0x%x 0x%x 0x%x ] "
	   "[ %d %d %d ]\n",
	   XCONFIG_PROBED, vga256InfoRec.name, 
	   CALC_VCLK(m_best,n_best,p_best),
	   new->VideoClk2_M,
	   new->VideoClk2_N,
	   new->VideoClk2_DivisorSel,
	   m_best, n_best, p_best);
}

/*
 * I810Restore --
 *
 * This function restores a video mode.  It basically writes out all of
 * the registers that have previously been saved in the vgaI810Rec data 
 * structure.
 *
 * Note that "Restore" is a little bit incorrect.  This function is also
 * used when the server enters/changes video modes.  The mode definitions 
 * have previously been initialized by the Init() function, below.
 */
static void 
I810Restore(restore)
    vgaI810Ptr restore;
{
    unsigned char temp;
    unsigned short stemp;
    unsigned int  itemp;
    int i;
   
    if (I810FrameBufferLocked) 
       I810UnlockFrameBuffer();    

    vgaProtect(TRUE); /* Blank the screen */

    /*
     * Whatever code is needed to get things back to bank zero should be
     * placed here.  Things should be in the same state as when the
     * Save/Init was done.
     */

    usleep(50000);

    /* Turn off DRAM Refresh */
    temp = INREG8( DRAM_ROW_CNTL_HI );
    temp &= ~DRAM_REFRESH_RATE;
    temp |= DRAM_REFRESH_DISABLE;
    OUTREG8( DRAM_ROW_CNTL_HI, temp );

    usleep(1000); /* Wait 1 ms */

    /* Write the M, N and P values */
    OUTREG16( VCLK2_VCO_M, restore->VideoClk2_M);
    OUTREG16( VCLK2_VCO_N, restore->VideoClk2_N);
    OUTREG8( VCLK2_VCO_DIV_SEL, restore->VideoClk2_DivisorSel);

    /*
     * Turn on 8 bit dac mode, if requested.  This is needed to make
     * sure that vgaHWRestore writes the values into the DAC properly.
     * The problem occurs if 8 bit dac mode is requested and the HW is
     * in 6 bit dac mode.  If this happens, all the values are
     * automatically shifted left twice by the HW and incorrect colors
     * will be displayed on the screen.  The only time this can happen
     * is at server startup time and when switching back from a VT.
     */
    temp = INREG8(PIXPIPE_CONFIG_0); 
    temp &= 0x7F; /* Save all but the 8 bit dac mode bit */
    temp |= (restore->PixelPipeCfg0 & DAC_8_BIT);
    OUTREG8( PIXPIPE_CONFIG_0, temp );

    /*
     * This function handles restoring the generic VGA registers.
     */
    vgaHWRestore((vgaHWPtr)restore);

    /*
     * Code to restore any SVGA registers that have been saved/modified
     * goes here.  Note that it is allowable, and often correct, to 
     * only modify certain bits in a register by a read/modify/write cycle.
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

    outb(GRX, ADDRESS_MAPPING); temp = inb(GRX+1);
    temp &= 0xE0; /* Save reserved bits 7:5 */
    temp |= restore->AddressMapping;
    outb(GRX, ADDRESS_MAPPING); outb(GRX+1, temp);


    /* Turn on DRAM Refresh */
    temp = INREG8( DRAM_ROW_CNTL_HI );
    temp &= ~DRAM_REFRESH_RATE;
    temp |= DRAM_REFRESH_60HZ;
    OUTREG8( DRAM_ROW_CNTL_HI, temp );

    temp = INREG8( BITBLT_CNTL );
    temp &= ~COLEXP_MODE;
    temp |= restore->BitBLTControl;
    OUTREG8( BITBLT_CNTL, temp );

    temp = INREG8( DISPLAY_CNTL );
    temp &= ~(VGA_WRAP_MODE | GUI_MODE);
    temp |= restore->DisplayControl;
    OUTREG8( DISPLAY_CNTL, temp );

    temp = INREG8( PIXPIPE_CONFIG_0 );
    temp &= 0x64; /* Save reserved bits 6:5,2 */
    temp |= restore->PixelPipeCfg0;
    OUTREG8( PIXPIPE_CONFIG_0, temp );

    temp = INREG8( PIXPIPE_CONFIG_2 );
    temp &= 0xF3; /* Save reserved bits 7:4,1:0 */
    temp |= restore->PixelPipeCfg2;
    OUTREG8( PIXPIPE_CONFIG_2, temp );

    temp = INREG8( PIXPIPE_CONFIG_1 );
    temp &= ~DISPLAY_COLOR_MODE;
    temp |= restore->PixelPipeCfg1;
    OUTREG8( PIXPIPE_CONFIG_1, temp );

    stemp = INREG16(HWSTAM);
    stemp &= INTR_RESERVED;
    stemp |= restore->IntrHwStatMask;
    OUTREG16(HWSTAM, stemp);

    stemp = INREG16(IER);
    stemp &= INTR_RESERVED;
    stemp |= restore->IntrEnabled;
    OUTREG16(IER, stemp);

    stemp = INREG16(IMR);
    stemp &= INTR_RESERVED;
    stemp |= restore->IntrMask;
    OUTREG16(IMR, stemp);

    OUTREG16(EIR, 0);

    stemp = INREG16(EMR);
    stemp &= ERROR_RESERVED;
    stemp |= restore->ErrorMask;
    OUTREG16(EMR, stemp);


    itemp = INREG(FWATER_BLC);
    itemp &= ~(LM_BURST_LENGTH | LM_FIFO_WATERMARK | 
	       MM_BURST_LENGTH | MM_FIFO_WATERMARK );
    itemp |= restore->LMI_FIFO_Watermark;
    OUTREG(FWATER_BLC, itemp);

    /* First disable the ring buffer (Need to wait for empty first?, if so
     * should probably do it before entering this section)
     */
    itemp = INREG(LP_RING + RING_LEN);
    itemp &= ~RING_VALID_MASK;
    OUTREG(LP_RING + RING_LEN, itemp );

    /* Set up the low priority ring buffer.
     */
    OUTREG(LP_RING + RING_TAIL, 0 );
    OUTREG(LP_RING + RING_HEAD, 0 );

    itemp = INREG(LP_RING + RING_START);
    itemp &= ~(START_ADDR);
    itemp |= restore->LprbStart;
    OUTREG(LP_RING + RING_START, itemp );

    itemp = INREG(LP_RING + RING_LEN);
    itemp &= ~(RING_NR_PAGES | RING_REPORT_MASK | RING_VALID_MASK);
    itemp |= restore->LprbLen;
    OUTREG(LP_RING + RING_LEN, itemp );

    /* Reset our shadow variables.
     */
    I810LpRing.head = 0;
    I810LpRing.tail = 0;      
    I810LpRing.space = 0;

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

    outb(vgaIOBase + 4, IO_CTNL); temp = inb(vgaIOBase + 5);
    temp &= ~(EXTENDED_ATTR_CNTL | EXTENDED_CRTC_CNTL);
    temp |= restore->IOControl;
    outb(vgaIOBase + 4, IO_CTNL); outb(vgaIOBase + 5, temp);
}


/* Famous last words
 */
void 
I810PrintErrorState()
{
   fprintf(stderr, "pgetbl_ctl: 0x%x pgetbl_err: 0x%x\n", 
	   INREG(PGETBL_CTL),
	   INREG(PGE_ERR));

   fprintf(stderr, "ipeir: %x iphdr: %x\n", 
	   INREG(IPEIR),
	   INREG(IPEHR));

   fprintf(stderr, "LP ring tail: %x head: %x len: %x start %x\n",
	   INREG(LP_RING + RING_TAIL),
	   INREG(LP_RING + RING_HEAD) & HEAD_ADDR,
	   INREG(LP_RING + RING_LEN),
	   INREG(LP_RING + RING_START));

   fprintf(stderr, "eir: %x esr: %x emr: %x\n",
	   INREG16(EIR),
	   INREG16(ESR),
	   INREG16(EMR));

   fprintf(stderr, "instdone: %x instpm: %x\n",
	   INREG16(INST_DONE),
	   INREG8(INST_PM));

   fprintf(stderr, "memmode: %x instps: %x\n",
	   INREG(MEMMODE),
	   INREG(INST_PS));

   fprintf(stderr, "hwstam: %x ier: %x imr: %x iir: %x\n",
	   INREG16(HWSTAM),
	   INREG16(IER),
	   INREG16(IMR),
	   INREG16(IIR));
}


/*
 * I810Save --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaI810Rec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void *
I810Save(save)
    vgaI810Ptr save;
{
    int i;

    /*
     * This function will handle creating the data structure and filling
     * in the generic VGA portion.
     */
    save = (vgaI810Ptr)vgaHWSave((vgaHWPtr)save, sizeof(vgaI810Rec));

    /*
     * The port I/O code necessary to read in the extended registers 
     * into the fields of the vgaI810Rec structure goes here.
     */
    outb(vgaIOBase + 4, IO_CTNL); save->IOControl = inb(vgaIOBase + 5);
    outb(GRX, ADDRESS_MAPPING);   save->AddressMapping = inb(GRX+1);

    save->BitBLTControl = INREG8( BITBLT_CNTL );
    save->VideoClk2_M = INREG16( VCLK2_VCO_M );
    save->VideoClk2_N = INREG16( VCLK2_VCO_N );
    save->VideoClk2_DivisorSel = INREG8( VCLK2_VCO_DIV_SEL );

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


    save->IntrHwStatMask = INREG16(HWSTAM);
    save->IntrEnabled = INREG16(IER);
    save->IntrIdentity = INREG16(IIR);
    save->IntrMask = INREG16(IMR);

    save->ErrorMask = INREG16(EMR);

    save->PixelPipeCfg0 = INREG8( PIXPIPE_CONFIG_0 );
    save->PixelPipeCfg1 = INREG8( PIXPIPE_CONFIG_1 );
    save->PixelPipeCfg2 = INREG8( PIXPIPE_CONFIG_2 );
    save->DisplayControl = INREG8( DISPLAY_CNTL );
    save->LMI_FIFO_Watermark = INREG( FWATER_BLC );

    save->LprbTail = INREG(LP_RING + RING_TAIL);
    save->LprbHead = INREG(LP_RING + RING_HEAD);
    save->LprbStart = INREG(LP_RING + RING_START);
    save->LprbLen = INREG(LP_RING + RING_LEN);

    if ((save->LprbTail & TAIL_ADDR) != (save->LprbHead & HEAD_ADDR) &&
	save->LprbLen & RING_VALID) {
       I810PrintErrorState();
       FatalError( "Active ring not flushed\n");
    }

    return ((void *) save);
}

/*
 * I810Init --
 *
 * This is the most important function (after the Probe) function.  This
 * function fills in the vgaI810Rec with all of the register values needed
 * to enable either a 256-color mode (for the color server) or a 16-color
 * mode (for the monochrome server).
 *
 * The 'mode' parameter describes the video mode.  The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.  The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 *
 * This is called after I810FBInit(), I hope..
 */
static Bool
I810Init(mode)
    DisplayModePtr mode;
{
    double dclk = vga256InfoRec.clock[mode->Clock]/1000.0;
    static int first_time = 1;
    int i;

    if (first_time) {
       first_time = 0;
       I810AccelFinishInit();
    }

    /*
     * This will allocate the datastructure and initialize all of the
     * generic VGA registers.
     */
    if (!vgaHWInit(mode,sizeof(vgaI810Rec)))
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
	new->BitBLTControl = COLEXP_RESERVED; /* Not implemented on i810 */
	break;
    default:
	break;
    }

    /* Turn on 8 bit dac if requested */
    if (OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options))
	new->PixelPipeCfg0 = DAC_8_BIT;
    else
	new->PixelPipeCfg0 = DAC_6_BIT;

    /* Turn on gamma correction 
     */
    if (vgaBitsPerPixel > 8) {
	extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
	int i;
	for (i = 0; i < 256; i++) {
	    new->std.DAC[i*3+0] = xf86rGammaMap[i];
	    new->std.DAC[i*3+1] = xf86gGammaMap[i];
	    new->std.DAC[i*3+2] = xf86bGammaMap[i];
	}
    }

    new->PixelPipeCfg2 = ( 0
			   | DISPLAY_GAMMA_ENABLE 
			   | OVERLAY_GAMMA_ENABLE
			   ) ;


    /* Turn on Extended VGA Interpretation 
     */
    new->IOControl = EXTENDED_CRTC_CNTL;

    /* Turn on linear mapping, and the GTT. 
     */
    new->AddressMapping = ( LINEAR_MODE_ENABLE | 
			    GTT_MEM_MAP_ENABLE);
    

    /* Turn on GUI mode */
    new->DisplayControl = HIRES_MODE;

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
    I810CalcVCLK(dclk);

    /* Since we program the clocks ourselves, always use VCLK2. */
    new->std.MiscOutReg |= 0x0C;

    /* Calculate the FIFO Watermark and Burst Length. */
    new->LMI_FIFO_Watermark = I810CalcWatermark( dclk, I810LmFreqSel );

    new->IntrHwStatMask = ~INTR_RESERVED;
    new->IntrEnabled = 0x0000;
    new->IntrIdentity = 0x0000;
    new->IntrMask = ~INTR_RESERVED;	/* unmask all interrupts */

    new->ErrorMask = 0;

    /* Setup the ring buffer */
    new->LprbTail = 0;
    new->LprbHead = 0;
    new->LprbStart = I810LpRing.mem.Start;

    if (new->LprbStart) 
       new->LprbLen = (I810LpRing.mem.Size-4096) | RING_NO_REPORT | RING_VALID;
    else
       new->LprbLen = RING_INVALID;

    return(TRUE);
}

/*
 * I810Adjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.  This is used to implement the
 * virtual window.
 */
static void 
I810Adjust(x, y)
    int x, y;
{
    int Base = (y * vga256InfoRec.displayWidth + 
		I810FrameBuffer.Start +
		x) >> 2;

    I810CursorOffset = 0;

    if (I810_DEBUG&DEBUG_VERBOSE_VGA) {
       fprintf(stderr, "I810adjust %d %d\n", x, y);
       fprintf(stderr, "base: %x", Base);
    }

    switch (vgaBitsPerPixel) {
    case  8:	
	break;
    case 16:
	Base *= 2;
	break;
    case 24:
	/* KW: Need to do 16-pixel alignment for i810, otherwise you
	 * get bad watermark problems.  Need to fixup the mouse
	 * pointer positioning to take this into account.  
	 */
        I810CursorOffset = (Base & 0x3) * 4;
        Base &= ~0x3; 
	Base *= 3;
	break;
    case 32:
	Base *= 4;
	break;
    }

    if (I810_DEBUG&DEBUG_VERBOSE_VGA) 
       fprintf(stderr, " cursor offset: %d\n", I810CursorOffset);

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
 * I810SaveScreen --
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
I810SaveScreen(mode)
    int mode;
{
    if (mode == SS_START) {
	outw(SRX, 0x0100);
    } else {
	outw(SRX, 0x0300);
    }
}

/*
 * I810FbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 * For most drivers, this function is not required, and 'NoopDDA' is put
 * in the vgaVideoChipRec structure.
 */
static Bool
I810ScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    ScreenPtr pScreen;
    pointer pbits;
    int xsize, ysize, dpix, dpiy, width;
{
    pScreen->StoreColors = I810StoreColors;
    return TRUE;
}

/*
 * I810FbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 * For most drivers, this function is not required, and 'NoopDDA' is put
 * in the vgaVideoChipRec structure.
 */
static void
I810FbInit()
{
   /* Hook out screen init so that we can substitute our own StoreColors 
   */
   vgaSetScreenInitHook(I810ScreenInit);

   /* Print out some useful information about the system. */
   if (xf86Verbose) {
      ErrorF("%s %s: ",
	     (vga256InfoRec.chipset ? XCONFIG_PROBED : XCONFIG_GIVEN),
	     vga256InfoRec.name);
      switch (I810Chipset) {
      case PCI_CHIP_I810:
	 ErrorF("Intel i810\n");
	 break;
      case PCI_CHIP_I810_DC100:
	 ErrorF("Intel i810-dc100\n");
	 break;
      case PCI_CHIP_I810_E:
	 ErrorF("Intel i810e\n");
	 break;
      }
   }

   /* Map IO registers to virtual address space. */
   I810MMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
				(pointer)(I810MMIOAddr), 0x80000);
   if (!I810MMIOBase)
      FatalError("i810: Can't memory map IO registers\n");
   if (xf86Verbose)
      ErrorF("%s %s: %s: MMIO registers at 0x%08lX\n",
	     (vga256InfoRec.IObase ? XCONFIG_GIVEN : XCONFIG_PROBED),
	     vga256InfoRec.name,
	     vga256InfoRec.chipset,
	     I810MMIOAddr);

   /* Use linear by default. */
   I810.ChipLinearBase = I810LinearAddr;
   I810.ChipUseLinearAddressing = TRUE;
   I810.ChipLinearSize = 8 * 1024 * 1024; 


   if (xf86Verbose)
      ErrorF("%s %s: %s: Linear framebuffer at 0x%08lX\n",
	     (vga256InfoRec.MemBase ? XCONFIG_GIVEN : XCONFIG_PROBED),
	     vga256InfoRec.name,
	     vga256InfoRec.chipset,
	     I810.ChipLinearBase);

   /* Allocate the framebuffer.
    */
   if (!I810AllocLow( &I810FrameBuffer, 
		      I810DisplayPtr,
		      (vga256InfoRec.virtualY *
		       vga256InfoRec.displayWidth *
		       vgaBytesPerPixel) ))
      FatalError("Failed to allocate space for framebuffer");
   
   if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
      fprintf(stderr, "framebuffer %x to %x\n", I810FrameBuffer.Start,
	      I810FrameBuffer.End);

   I810Mprotect = I810FrameBuffer;

   if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) 
   {
      if (!I810AllocHigh( &I810Cursor, &I810SysMem, 4096 )) {
	 ErrorF("%s %s: %s: Warning: "
		"Cannot allocate memory in framebuffer for cursor image\n",
		(OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) ?
		 XCONFIG_GIVEN : XCONFIG_PROBED),
		vga256InfoRec.name,
		vga256InfoRec.chipset);
      }
      else {
	 /* Translate to a physical system memory address - this is the
	  * only thing for which the hardware will not use the GTT...  
	  */
	 I810CursorPhysical = I810LocalToPhysical( I810Cursor.Start ); 

	 if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
	    fprintf(stderr, "cursor local %x phys %x\n", 
		    I810Cursor.Start, I810CursorPhysical);

	 if (I810CursorPhysical) 
	    I810CursorInit(); 
      }
   }

   if (xf86Verbose)
      ErrorF("%s %s: %s: Using %s cursor\n",
	     (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) ?
	      XCONFIG_GIVEN : XCONFIG_PROBED),
	     vga256InfoRec.name,
	     vga256InfoRec.chipset,
	     (I810CursorPhysical ? "hardware" : "software"));


   memset( &I810LpRing, 0, sizeof( I810LpRing ) );

   /* Must be power of two for tail_mask to work.
    */
   if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
      if (I810AllocHigh( &I810LpRing.mem, &I810SysMem, 4096 )) {

	 if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
	    fprintf(stderr, "ring buffer at local %x\n", I810LpRing.mem.Start);

	 I810LpRing.base_reg = LP_RING;
	 I810LpRing.tail_mask = I810LpRing.mem.Size - 1;
	 I810LpRing.virtual_start = 0;
	 I810LpRing.head = 0;
	 I810LpRing.tail = 0;      
	 I810LpRing.space = 0;		 
	 I810AccelInit();
      } else { 
	 ErrorF("%s %s: %s: Warning: "
		"Cannot allocate memory in framebuffer for ring buffer\n",
		(OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) ?
		 XCONFIG_GIVEN : XCONFIG_PROBED),
		vga256InfoRec.name,
		vga256InfoRec.chipset);
      }
   }
   else if (xf86Verbose)
      ErrorF("%s %s: %s: Acceleration disabled\n",
	     (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) ?
	      XCONFIG_GIVEN : XCONFIG_PROBED),
	     vga256InfoRec.name,
	     vga256InfoRec.chipset);
}

static int
I810ValidMode(mode, verbose, flag)
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
 * I810DisplayPowerManagementSet --
 *
 * Sets VESA Display Power Management Signaling (DPMS) Mode.  
 */
#ifdef DPMSExtension
static void
I810DisplayPowerManagementSet(PowerManagementMode)
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
    OUTREG8( DPMS_SYNC_SELECT, DPMSSyncSelect );
}
#endif
