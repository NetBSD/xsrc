/* $XConsortium: apm_driver.c /main/5 1996/10/25 10:27:55 kaleb $ */




/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/apm/apm_driver.c,v 3.7.2.6 1998/02/15 23:32:06 robin Exp $ */

/*
  TODO (also see apm_accel.c)

  Add 24 bpp support

  New code for max dotclock?
    Hercules says: Do not let the video dot clock * bytes per pixel
    go above the available memory bandwidth which is around 200MB/s.
    Then how come the manual says it is OK to use a 144MHz vclk in 16
    bit for 1280x1024? That is something like 290MB/s...? Hmm...
    It can't be double indexed mode either, since that doesn't exist 
    for anything but 8 bit...
*/

/* 
   Created by Kent Hamilton for Xfree86 from source from Alliance

   Modified 1997-06 by Henrik Harmsen (hch@cd.chalmers.se, 
                                       Henrik.Harmsen@erv.ericsson.se)
     - Added support for AT3D
     - Acceleration added for 8,16,32bpp: (for AT3D and AT24)
       - Filled rectangles
       - Screen-screen bitblts
       - Host-screen color expansion bitblts for text
     - DPMS support
     - Enabled hardware cursor code (also in 8bpp)
     - Set to programmable VCLK clock
     - Set MCLK to 57.3 MHz on AT3D.
     - Various bugfixes and cleanups
   
   Modified 1997-07-06 by Henrik Harmsen
     - Fixed bug that made the HW cursor screw up on VT switches
     - Probably fixed bug that screwed up the screen when using
       screen-screen bitblts. This forced me to put an ApmSync() at 
       the end of ApmSubsequentScreenToScreenCopy() which makes
       me unhappy... But: Better it works than not...

   Modified 1998-02-08 by Henrik Harmsen
     - Added DGA support.
     - Removed accel for AT24, didn't work anyways according to reports...

   Modified 1998-02-14 by Henrik Harmsen
     - Added accel support for AT24 again. Got hardware, worked fine.
     - Added accel support for AP6422. 
     - Fixed clock register calculation for AP6422 and AT24.
     - DPMS support for AT24 & AP6422.
     - 2% faster text accel for AT24/AT3D :-)

*/

#include <math.h>
#include "X.h"
#include "input.h"
#include "screenint.h"
#include "dix.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "vga.h"
#include "vgaPCI.h"
#include "vga256.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "apm.h"


/* Driver data structures. */
typedef struct {
  /*
   * This structure defines all of the register-level information
   * that must be stored to define a video mode for this chipset.
   * The 'vgaHWRec' member must be first, and contains all of the
   * standard VGA register information, as well as saved text and
   * font data.
   */
  vgaHWRec std;               /* good old IBM VGA */
  /* 
   * Any other registers or other data that the new chipset needs
   * to be saved should be defined here.  The Init/Save/Restore
   * functions will manipulate theses fields.  Examples of things
   * that would go here are registers that contain bank select
   * registers, or extended clock select bits, or extensions to 
   * the timing registers.  Use 'unsigned char' as the type for
   * these registers.
   */
  unsigned char SR1B;
  unsigned char SR1C;
  unsigned char CR19;
  unsigned char CR1A;
  unsigned char CR1B;
  unsigned char CR1C;
  unsigned char CR1D;
  unsigned char CR1E;
  unsigned char XR80;
  unsigned char XRC0;
  unsigned long XRE8;
  unsigned long XREC;
  unsigned long XRF0;
  unsigned long XRF4;
  unsigned long XR140;
  unsigned short XR144;
  unsigned long XR148;
  unsigned short XR14C;
} vgaApmRec, *vgaApmPtr;


/* Driver functions */
static char* ApmIdent(int n);
static Bool  ApmProbe(void);
static void  ApmFbInit(void);
static void  ApmEnterLeave(Bool enter);
static void  ApmRestore(vgaApmPtr restore);
static void* ApmSave(vgaApmPtr save);
static Bool  ApmInit(DisplayModePtr mode);
static void  ApmAdjust(int x, int y);
static int   ApmValidMode(DisplayModePtr mode, Bool verbose, int flag);
#ifdef DPMSExtension
static void  ApmDisplayPowerManagementSet(int PowerManagementMode);
#endif

/* Helper functions */
static unsigned comp_lmn(unsigned clock);

/* Bank select functions. These are defined in apm_bank.s */
void ApmSetRead();
void ApmSetWrite();
void ApmSetReadWrite();

/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data 
 * that defines how the driver operates.
 */
vgaVideoChipRec APM = {
  /* 
   * Function pointers
   */
  ApmProbe,
  ApmIdent,
  ApmEnterLeave,
  ApmInit,
  ApmValidMode,
  (void*(*)())ApmSave,
  (void(*)())ApmRestore,
  ApmAdjust,
  vgaHWSaveScreen,
  (void (*)())NoopDDA,
  ApmFbInit,
  ApmSetRead,
  ApmSetWrite,
  ApmSetReadWrite,
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
   * separate read and write bank registers.
   */
  FALSE,
  /*
   * If the chipset requires vertical timing numbers to be divided
   * by two for interlaced modes, set this to VGA_DIVIDE_VERT.
   */
  VGA_DIVIDE_VERT,
  /*
   * This is a dummy initialization for the set of option flags
   * that this driver supports.  It gets filled in properly in the
   * probe function, if the probe succeeds (assuming the driver
   * supports any such flags).
   */
  {{0,}},
  /*
   * This determines the multiple to which the virtual width of
   * the display must be rounded for the 256-color server.  This
   * will normally be 8, but may be 4 or 16 for some servers.
   */
  8,
  /*
   * If the driver includes support for a linear-mapped frame buffer
   * for the detected configuration this should be set to TRUE in the
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
   * This is TRUE if the driver has support for 32bpp for the detected
   * configuration.
   */
  FALSE,
  FALSE,
  /*
   * This is a pointer to a list of builtin driver modes.
   * This is rarely used, and in must cases, set it to NULL
   */
  NULL,
  /*
   * This is a factor that can be used to scale the raw clocks
   * to pixel clocks.  This is rarely used, and in most cases, set
   * it to 1.
   */
  1,       /* ClockMulFactor */
  1        /* ClockDivFactor */
};



/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */
#define new ((vgaApmPtr)vgaNewVideoState)

/*
 * If your chipset uses non-standard I/O ports, you need to define an
 * array of ports, and an integer containing the array size.  The
 * generic VGA ports are defined in vgaHW.c.
 */
/* This will allow access to all I/O ports */
static unsigned Apm_ExtPorts[] = { 0x400 };
static int Num_Apm_ExtPorts = (sizeof(Apm_ExtPorts)/sizeof(Apm_ExtPorts[0]));

#define VESA	0
#define PCI	1

static int apmBus;
int apmChip;
static int apmDisplayableMemory;
static int apmDPMS;
static int apmAccelSupported;
u32 apm_xbase;

#define AP6422  0
#define AT24    1
#define AT3D    2

static SymTabRec chipsets[] = {
  { AP6422,  "AP6422"},
  { AT24,    "AT24" },
  { AT3D,    "AT3D" },
  { -1,		"" },
};




/*
 * ApmIdent --
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
ApmIdent(int n)
{
  if (chipsets[n].token < 0)
    return NULL;
  else
    return chipsets[n].name;
}



/*
 * ApmProbe --
 *
 * This is the function that makes a yes/no decision about whether or not
 * a chipset supported by this driver is present or not.  The server will
 * call each driver's probe function in sequence, until one returns TRUE
 * or they all fail.
 *
 */
static Bool
ApmProbe(void)
{

  xf86ClearIOPortList(vga256InfoRec.scrnIndex);
  xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
  xf86AddIOPorts(vga256InfoRec.scrnIndex, 
                 Num_Apm_ExtPorts, Apm_ExtPorts);

  /*
   * First we attempt to figure out if one of the supported chipsets
   * is present.
   */

  apmDPMS = FALSE;
  apmAccelSupported = FALSE;

  if (vga256InfoRec.chipset)
  {
    /*
     * This is the easy case.  The user has specified the
     * chipset in the XF86Config file.  All we need to do here
     * is a string comparison against each of the supported
     * names available from the Ident() function.  If this
     * driver supports more than one chipset, there would be
     * nested conditionals here (see the Trident and WD drivers
     * for examples).
     */
    apmChip = xf86StringToToken(chipsets, vga256InfoRec.chipset);
    if (apmChip >= 0)
      ApmEnterLeave(ENTER);
    else
      return FALSE;

    if (apmChip >= AP6422)
    {
      apmDPMS = TRUE;
      apmAccelSupported = TRUE;
    }
  }
  else
  {
    /*
     * OK.  We have to actually test the hardware.  The
     * EnterLeave() function (described below) unlocks access
     * to registers that may be locked, and for OSs that require
     * it, enables I/O access.  So we do this before we probe,
     * even though we don't know for sure that this chipset
     * is present.
     */
    int     i;
    char    id_ap6420[] = "Pro6420";
    char    id_ap6422[] = "Pro6422";
    char    id_at24[]   = "Pro6424";
    char    id_at3d[]   = "ProAT3D"; /* Yeah, the manual could have been 
                                        correct... */
    char    idstring[]  = "       ";

    ApmEnterLeave(ENTER);
    for (i = 0; i < 7; i++)
      idstring[i] = rdinx(0x3c4, 0x11 + i);
    if (!memcmp(id_ap6420, idstring, 7))
    {
      apmChip = AP6422;
      apmDPMS = TRUE;
      apmAccelSupported = TRUE;
    }
    else if (!memcmp(id_ap6422, idstring, 7))
    {
      apmChip = AP6422;
      apmDPMS = TRUE;
      apmAccelSupported = TRUE;
    }
    else if (!memcmp(id_at24, idstring, 7))
    {
      apmChip = AT24;
      apmDPMS = TRUE;
      apmAccelSupported = TRUE;
    }
    else if (!memcmp(id_at3d, idstring, 7))
    {
      apmChip = AT3D;
      apmDPMS = TRUE;
      apmAccelSupported = TRUE;
    }
    else
    {
      ApmEnterLeave(LEAVE);
      return(FALSE);
    }
    vga256InfoRec.chipset = ApmIdent(apmChip);
  }


#ifdef DPMSExtension
  if (apmDPMS)
    vga256InfoRec.DPMSSet = ApmDisplayPowerManagementSet;
#endif



  if (RDXB_IOP(0xca) & 1)
    apmBus = PCI;
  else
    apmBus = VESA;


  switch(apmChip)
  {
    /* These values come from the Manual for AT24 and AT3D 
       in the overview of various modes. I've taken the largest
       number for the different modes. Alliance wouldn't 
       tell me what the maximum frequency was, so...
     */
    case AT24:
         switch(vgaBitsPerPixel)
         {
           case 8:
                vga256InfoRec.maxClock = 160000;
                break;
           case 15:
           case 16:
                vga256InfoRec.maxClock = 144000;
                break;
           case 24:
                vga256InfoRec.maxClock = 75000; /* Hmm. */
                break;
           case 32:
                vga256InfoRec.maxClock = 94500;
                break;
           default:
                return FALSE;
         }
         break;
    case AT3D:
         switch(vgaBitsPerPixel)
         {
           case 8:
                vga256InfoRec.maxClock = 175500;
                break;
           case 15:
           case 16:
                vga256InfoRec.maxClock = 144000;
                break;
           case 24:
                vga256InfoRec.maxClock = 75000; /* Hmm. */
              break;
           case 32:
                vga256InfoRec.maxClock = 94500;
                break;
           default:
                return FALSE;
         }
         break;
    default:
         vga256InfoRec.maxClock = 135000;
         break;
  }


  /*
   * If the user has specified the amount of memory in the XF86Config
   * file, we respect that setting.
   */
  if (!vga256InfoRec.videoRam)
  {
    /*
     * Otherwise, do whatever chipset-specific things are 
     * necessary to figure out how much memory (in kBytes) is 
     * available.
     */
    vga256InfoRec.videoRam = rdinx(0x3c4, 0x20) * 64;
  }

  /* Use always use linear addressing */
  APM.ChipUseLinearAddressing = TRUE;

  if (vga256InfoRec.MemBase != 0)
    APM.ChipLinearBase = vga256InfoRec.MemBase;
  else
    if (apmBus == PCI)
      APM.ChipLinearBase = RDXB_IOP(0x193) << 24;
    else
      /* VESA local bus. */
      /* Pray that 2048MB works. */
      APM.ChipLinearBase = 0x80000000;

  APM.ChipLinearSize = vga256InfoRec.videoRam * 1024;
  APM.ChipHas16bpp = TRUE;
  APM.ChipHas32bpp = TRUE;

  vga256InfoRec.videoRam -= 34; /* We're going to use the last 34 kilobytes 
                                   for memory mapped registers, host->screen 
                                   bitblts and storage for the hardware 
                                   cursor */
 
  /*
   * Last we fill in the remaining data structures.  We specify
   * the chipset name, using the Ident() function and an appropriate
   * index.  We set a boolean for whether or not this driver supports
   * banking for the Monochrome server.  And we set up a list of all
   * the option flags that this driver can make use of.
   */

  vga256InfoRec.bankedMono = FALSE;
  OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
  OFLG_SET(OPTION_NOACCEL, &APM.ChipOptionFlags);
  OFLG_SET(OPTION_SW_CURSOR, &APM.ChipOptionFlags);

#ifdef XFreeXDGA
  vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

  return TRUE;
}

/*
 * ApmFbInit --
 *      enable speedups for the chips that support it
 */
static void
ApmFbInit(void)
{
  int offscreen_available;

  if (xf86Verbose && APM.ChipUseLinearAddressing)
    ErrorF("%s %s: %s: Using linear framebuffer at 0x%08X (%s)\n",
           XCONFIG_PROBED, vga256InfoRec.name,
           vga256InfoRec.chipset, APM.ChipLinearBase,
           apmBus == PCI ? "PCI bus" : "VL bus");

  if (apmAccelSupported && !OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
    ApmAccelInit();

  if (apmAccelSupported && !OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) 
  {
    apmCursorWidth = 64;
    apmCursorHeight = 64;
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Init = ApmCursorInit;
    vgaHWCursor.Restore = ApmRestoreCursor;
    vgaHWCursor.Warp = ApmWarpCursor;
    vgaHWCursor.QueryBestSize = ApmQueryBestSize;
    if (xf86Verbose)
      ErrorF("%s %s: %s: Using hardware cursor\n",
             XCONFIG_PROBED, vga256InfoRec.name,
             vga256InfoRec.chipset);
  }
}


/*
 * ApmEnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down.  Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device.  This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
ApmEnterLeave(Bool enter)
{
  /*
   * The value of the lock register is saved at the first
   * "Enter" call, restored at a "Leave". This reduces the
   * risk of messing up the registers of another chipset.
   */
  static int enterCalled = FALSE;
  static int savedSR10;
  unsigned char temp;

#ifdef XFreeXDGA
  if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
    return;
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

    /*
     * Here we deal with register-level access locks.  This
     * is a generic VGA protection; most SVGA chipsets have
     * similar register locks for their extended registers
     * as well.
     */
    /* Unprotect CRTC[0-7] */
    outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, temp & 0x7F);
    if (enterCalled == FALSE) {
      savedSR10 = rdinx(0x3C4, 0x10);
      apm_xbase = (rdinx(0x3c4, 0x1f) << 8)
        | rdinx(0x3c4, 0x1e);
      enterCalled = TRUE;
    }
    outw(0x3C4, 0x1210);
  }
  else
  {
    /*
     * Here undo what was done above.
     */

    /* Protect CRTC[0-7] */
    outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);

    wrinx(0x3C4, 0x10, savedSR10);

    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }
}

/*
 * ApmRestore --
 *
 * This function restores a video mode.  It basically writes out all of
 * the registers that have previously been saved in the vgaApmRec data 
 * structure.
 *
 * Note that "Restore" is a little bit incorrect.  This function is also
 * used when the server enters/changes video modes.  The mode definitions 
 * have previously been initialized by the Init() function, below.
 */

static void 
ApmRestore(vgaApmPtr restore)
{
  vgaProtect(TRUE);

  /*
   * Whatever code is needed to get things back to bank zero should be
   * placed here.  Things should be in the same state as when the
   * Save/Init was done.
   */

  /* Set aperture index to 0. */
  WRXW_IOP(0xC0, 0);

  /*
   * Write the extended registers first
   */
  wrinx(0x3C4, 0x1b, restore->SR1B);
  wrinx(0x3C4, 0x1c, restore->SR1C);

  apmMMIO_Init = FALSE;

  /* Hardware cursor registers. */
  WRXL_IOP(0x140, restore->XR140);
  WRXW_IOP(0x144, restore->XR144);
  WRXL_IOP(0x148, restore->XR148);
  WRXW_IOP(0x14C, restore->XR14C);

  wrinx(vgaIOBase + 4, 0x19, restore->CR19); /* vgaIOBase == 3d0 */
  wrinx(vgaIOBase + 4, 0x1a, restore->CR1A);
  wrinx(vgaIOBase + 4, 0x1b, restore->CR1B);
  wrinx(vgaIOBase + 4, 0x1c, restore->CR1C);
  wrinx(vgaIOBase + 4, 0x1d, restore->CR1D);
  wrinx(vgaIOBase + 4, 0x1e, restore->CR1E);

  /* RAMDAC registers. */
  WRXL_IOP(0xe8, restore->XRE8);
  WRXL_IOP(0xec, restore->XREC & ~(1 << 7));
  WRXL_IOP(0xec, restore->XREC | (1 << 7)); /* Do a PLL resync */

  WRXB_IOP(0x80, restore->XR80);

  /*
   * This function handles restoring the generic VGA registers.
   */
  vgaHWRestore((vgaHWPtr)restore);

  vgaProtect(FALSE);

}

/*
 * ApmSave --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaApmRec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void *
ApmSave(vgaApmPtr save)
{
  /*
   * Whatever code is needed to get back to bank zero goes here.
   */

  /* Set aperture index to 0. */
  WRXW_IOP(0xC0, 0);

  /*
   * This function will handle creating the data structure and filling
   * in the generic VGA portion.
   */
  save = (vgaApmPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaApmRec));

  save->SR1B = rdinx(0x3C4, 0x1b);
  save->SR1C = rdinx(0x3C4, 0x1c);

  /* Hardware cursor registers. */
  save->XR140 = RDXL_IOP(0x140);
  save->XR144 = RDXW_IOP(0x144);
  save->XR148 = RDXL_IOP(0x148);
  save->XR14C = RDXW_IOP(0x14C);

  save->CR19 = rdinx(vgaIOBase + 4,  0x19);
  save->CR1A = rdinx(vgaIOBase + 4,  0x1A);
  save->CR1B = rdinx(vgaIOBase + 4,  0x1B);
  save->CR1C = rdinx(vgaIOBase + 4,  0x1C);
  save->CR1D = rdinx(vgaIOBase + 4,  0x1D);
  save->CR1E = rdinx(vgaIOBase + 4,  0x1E);

  /* RAMDAC registers. */
  save->XRE8 = RDXL_IOP(0xe8);
  save->XREC = RDXL_IOP(0xec);

  save->XR80 = RDXB_IOP(0x80);

  return ((void *) save);
}

/*
 * ApmInit --
 *
 * This is the most important function (after the Probe) function.  This
 * function fills in the vgaApmRec with all of the register values needed
 * to enable either a 256-color mode (for the color server) or a 16-color
 * mode (for the monochrome server).
 *
 * The 'mode' parameter describes the video mode.  The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.  The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
static Bool
ApmInit(DisplayModePtr mode)
{
  /*
   * This will allocate the datastructure and initialize all of the
   * generic VGA registers.
   */
  if (!vgaHWInit(mode,sizeof(vgaApmRec)))
    return(FALSE);

  /*
   * Here all of the other fields of 'new' get filled in, to
   * handle the SVGA extended registers.  It is also allowable
   * to override generic registers whenever necessary.
   *
   */

  /*
   * The APM chips have a scale factor of 8 for the 
   * scanline offset. There are four extended bit in addition
   * to the 8 VGA bits.
   */
  {
    int offset;
    offset = (vga256InfoRec.displayWidth *
              vga256InfoRec.bitsPerPixel / 8)	>> 3;
    new->std.CRTC[0x13] = offset;
    /* Bit 8 resides at CR1C bits 7:4. */
    new->CR1C = (offset & 0xf00) >> 4;
  }

  /* Set pixel depth. */
  switch(vga256InfoRec.bitsPerPixel)
  {
    case 8:
         new->XR80 = 0x02;
         break;
    case 16:
         new->XR80 = 0x0d;
         break;
    case 32:
         new->XR80 = 0x0f;
         break;
    default:
         FatalError("Unsupported bit depth %d\n", vga256InfoRec.bitsPerPixel);
         break;
  }

  /*
   * Enable VESA Super VGA memory organisation.
   * Also enable Linear Addressing.
   */
  if (APM.ChipUseLinearAddressing) {
    u8 size;
    new->SR1B = 0x00; /* For some reason it doesn't work to enable memory mapped registers here,
                         so that is done in apm_accel.c:CheckMMIO_Init() */
    switch (APM.ChipLinearSize)
    {
      case 0x100000:
           size = 0x00;
           break;
      case 0x200000:
           size = 0x02;
           break;
      case 0x400000:
           size = 0x04;
           break;
      case 0x600000:
           size = 0x06;
           break;
      default:
           ErrorF("Cannot find aperture size for configured amount of video ram\n");
           return FALSE;
    }
    new->SR1C = size | 0x29; /* 2 means simultaneous access to video ram */
    /* new->SR1C = size | 0x09; */
  }
  else {
    /* Banking; Map aperture at 0xA0000. */
    new->SR1B = 0;
    new->SR1C = 0;
  }
  /* Set banking register to zero. */
  new->XRC0 = 0;

  /* Handle the CRTC overflow bits. */
  {
    unsigned char val;
    /* Vertical Overflow. */
    val = 0;
    if ((mode->CrtcVTotal - 2) & 0x400)
      val |= 0x01;
    if ((mode->CrtcVDisplay - 1) & 0x400)
      val |= 0x02;
    /* VBlankStart is equal to VSyncStart + 1. */
    if (mode->CrtcVSyncStart & 0x400)
      val |= 0x04;
    /* VRetraceStart is equal to VSyncStart + 1. */
    if (mode->CrtcVSyncStart & 0x400)
      val |= 0x08;
    new->CR1A = val;

    /* Horizontal Overflow. */
    val = 0;
    if ((mode->CrtcHTotal / 8 - 5) & 0x100)
      val |= 1;
    if ((mode->CrtcHDisplay / 8 - 1) & 0x100)
      val |= 2;
    /* HBlankStart is equal to HSyncStart - 1. */
    if ((mode->CrtcHSyncStart / 8 - 1) & 0x100)
      val |= 4;
    /* HRetraceStart is equal to HSyncStart. */
    if ((mode->CrtcHSyncStart / 8) & 0x100)
      val |= 8;
    new->CR1B = val;
  }
  new->CR1E = 1;          /* disable autoreset feature */

  /*
   * A special case - when using an external clock-setting program,
   * this function must not change bits associated with the clock
   * selection.  This condition can be checked by the condition:
   *
   *	if (new->std.NoClock >= 0)
   *		initialize clock-select bits.
   */

  if (new->std.NoClock >= 0) {
    /* Program clock select. */
    new->XREC = comp_lmn(vga256InfoRec.clock[mode->Clock]);
    if (!new->XREC)
      return FALSE;
    new->std.MiscOutReg |= 0xc;
  }

  /* Set up the RAMDAC registers. */

  if (vgaBitsPerPixel > 8)
    /* Get rid of white border. */
    new->std.Attribute[0x11] = 0x00;


  if (apmChip >= AT3D)
    new->XRE8 = 0x071f01e8; /* Enable 58MHz MCLK (actually 57.3 MHz) 
                               This is what is used in the Windows drivers.
                               The BIOS sets it to 50MHz. */
  else
    new->XRE8 = RDXL_IOP(0xe8); /* No change */

  /*
   * Hardware cursor registers.
   * Generally the SVGA server will take care of enabling the
   * cursor after a mode switch.
   */

  return TRUE;
}

/*
 * ApmAdjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.  This is used to implement the
 * virtual window.
 */
static void 
ApmAdjust(int x, int y)
{
  int Base = ((y * vga256InfoRec.displayWidth + x)
              * (vgaBitsPerPixel / 8)) >> 2;

  /*
   * These are the generic starting address registers.
   */
  outw(vgaIOBase + 4, (Base & 0x00FF00) | 0x0C);
  outw(vgaIOBase + 4, ((Base & 0x00FF) << 8) | 0x0D);

  /*
   * Here the high-order bits are masked and shifted, and put into
   * the appropriate extended registers.
   */
  modinx(vgaIOBase + 4, 0x1c, 0x0f, (Base & 0x0f0000) >> 16);

}

/*
 * ApmValidMode --
 *
 */
static int
ApmValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
  /* Check for CRTC timing bits overflow. */
  if (mode->VTotal > 2047) {
    if (verbose)
      ErrorF("%s %s: %s: Vertical mode timing overflow (%d)\n",
             XCONFIG_PROBED, vga256InfoRec.name,
             vga256InfoRec.chipset, mode->VTotal);
    return MODE_BAD;
  }

  return MODE_OK;
}


#ifdef DPMSExtension
/*
 * DPMS Control registers
 *
 */

static void 
ApmDisplayPowerManagementSet(int PowerManagementMode)
{
  unsigned char dpmsreg, tmp;

  dpmsreg = 0;
  if (!xf86VTSema) return;
  switch (PowerManagementMode) {
    case DPMSModeOn:
         /* Screen: On; HSync: On, VSync: On */
         dpmsreg = 0x00;
         break;
    case DPMSModeStandby:
         /* Screen: Off; HSync: Off, VSync: On */
         dpmsreg = 0x01;
         break;
    case DPMSModeSuspend:
         /* Screen: Off; HSync: On, VSync: Off */
         dpmsreg = 0x02;
         break;
    case DPMSModeOff:
         /* Screen: Off; HSync: Off, VSync: Off */
         dpmsreg = 0x03;
         break;
  }
  tmp = RDXB_IOP(0xD0);
  tmp = (tmp & 0xfc) | dpmsreg;
  WRXB_IOP(0xD0, tmp);
}
#endif

#define WITHIN(v,c1,c2) (((v) >= (c1)) && ((v) <= (c2)))

static unsigned
comp_lmn(unsigned clock)
{
  int     n, m, l, f;
  double  fvco;
  double  fout;
  double  fmax;
  double  fref;
  double  fvco_goal;
  double  k, c;
  FILE* fp;  

  if (apmChip >= AT3D)
    fmax = 400000.0;
  else
    fmax = 250000.0;

  fref = 14318.0;

  for (m = 1; m <= 5; m++)
  {
    for (l = 3; l >= 0; l--)
    {
      for (n = 8; n <= 127; n++)
      {
        fout = ((double)(n + 1) * fref)/((double)(m + 1) * (1 << l));
        fvco_goal = (double)clock * (double)(1 << l);
        fvco = fout * (double)(1 << l);
        if (!WITHIN(fvco, 0.995*fvco_goal, 1.005*fvco_goal))
          continue;
        if (!WITHIN(fvco, 125000.0, fmax))
          continue;
        if (!WITHIN(fvco / (double)(n+1), 300.0, 300000.0))
          continue;
        if (!WITHIN(fref / (double)(m+1), 300.0, 300000.0))
          continue;

        /* The following formula was empirically derived by
           matching a number of fvco values with acceptable
           values of f.

           (fvco can be 125MHz - 400MHz on AT3D)
           (fvco can be 125MHz - 250MHz on AT24/AP6422)

           The table that was measured up follows:

           AT3D

           fvco       f
           (125)     (x-7) guess
           200       5-7
           219       4-7
           253       3-6
           289       2-5
           320       0-4
           (400)     (0-x) guess

           AT24

           fvco       f
           126       7   
           200       5-7 
           211       4-7

           AP6422

           fvco       f
           126       7   
           169       5-7
           200       4-5 
           211       4-5

           From this, a function "f = k * fvco + c" was derived.

           For AT3D, this table was measured with MCLK == 50MHz.
           The driver has since been set to use MCLK == 57.3MHz for,
           but I don't think that makes a difference here.
         */

        if (apmChip >= AT24)
        {
          k = 7.0 / (175.0 - 380.0);
          c = -k * 380.0;
          f = (int)(k * fvco/1000.0 + c + 0.5);
          if (f > 7) f = 7;
          if (f < 0) f = 0;
        }

        if (apmChip < AT24) /* i.e AP6422 */
        {
          c = (211.0*6.0-169.0*4.5)/(211.0-169.0);
          k = (4.5-c)/211.0;
          f = (int)(k * fvco/1000.0 + c + 0.5);
          if (f > 7) f = 7;
          if (f < 0) f = 0;
        }

#if 0
        fp = fopen("/tmp/f","r");
        if (!fp)
        {
          ErrorF("Cannot open /tmp/f\n");
          return 0;
        }
        fscanf(fp,"%d",&f);
        fclose(fp);
        ErrorF("%6.2f\t%6.2f\t%d\t%d\t%d\t%.2f\t%d\n", (double)clock, fout,
          n, m, l, fvco, f);
#endif

        return (n << 16) | (m << 8) | (l << 2) | (f << 4);
      }
    }
  }
  ErrorF("%s %s: %s: Cannot find register values for clock %6.2f MHz. "
         "Please use a (slightly) different clock.\n", 
         XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.chipset, 
         (double)clock / 1000.0);
  return 0;
}


