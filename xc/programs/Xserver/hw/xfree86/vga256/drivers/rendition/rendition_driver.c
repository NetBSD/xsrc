/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/rendition_driver.c,v 1.1.2.8 1999/06/23 12:37:23 hohndel Exp $ */
/*
 * Rendition driver v0.2
 *   implements support for Rendition Verite 1000/2x00 
 *
 * Authors:
 *   Marc Langenbach (mlangen@studcs.uni-sb.de)
 *   Tim Rowley (tor@cs.brown.edu)
 */

/*
 * #include's
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "hwcursor.h"

#include "vos.h"
#include "vtypes.h"
#include "vramdac.h"
#include "vmodes.h"
#include "vboard.h"



/*
 * extern data
 */

extern vgaPCIInformation *vgaPCIInfo;
extern unsigned char vga_pal[];



/*
 * function prototypes
 */

static Bool  RENDITIONProbe();
static char *RENDITIONIdent();
static void  RENDITIONEnterLeave();
static Bool  RENDITIONInit();
static Bool  RENDITIONValidMode();
static void *RENDITIONSave();
static void  RENDITIONRestore();
static void  RENDITIONAdjust();
static void  RENDITIONFbInit();
static int   RENDITIONLinearOffset();
static int   RENDITIONPitchAdjust();



/*
 * global data
 */

struct v_board_t BOARD;
#define NOVERITE 0

#define RENDITIONNumExtPorts 0xff
static unsigned RENDITIONExtPorts[RENDITIONNumExtPorts];

typedef struct {
  vgaHWRec std;
} vgaRenditionRec, *vgaRenditionPtr;


/*
 * This data structure defines the driver itself.
 */
vgaVideoChipRec RENDITION={
  /* 
   * function pointers
   */
  RENDITIONProbe,
  RENDITIONIdent,
  RENDITIONEnterLeave,
  RENDITIONInit,
  RENDITIONValidMode,
  RENDITIONSave,
  RENDITIONRestore,
  RENDITIONAdjust,
  vgaHWSaveScreen,
  (void (*)())NoopDDA, /* GetMode */
  RENDITIONFbInit,
  (void (*)())NoopDDA, /* SetRead */
  (void (*)())NoopDDA, /* SetWrite */
  (void (*)())NoopDDA, /* SetReadWrite */
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
   * seperate read and write bank registers. Almost all chipsets
   * support two banks, and two banks are almost always faster
   */
  FALSE,
  /*
   * The chipset requires vertical timing numbers to be divided
   * by two for interlaced modes
   */
  VGA_DIVIDE_VERT,
  /*
   * This is a dummy initialization for the set of option flags
   * that this driver supports. It gets filled in properly in the
   * probe function, if the probe succeeds (assuming the driver
   * supports any such flags).
   */
  {0,},
  /*
   * This determines the multiple to which the virtual width of
   * the display must be rounded for the 256-color server. 
   */
  8,
  /*
   * If the driver includes support for a linear-mapped frame buffer
   * for the detected configuratio this should be set to TRUE in the
   * Probe or FbInit function. 
   */
  TRUE,
  /*
   * This is the physical base address of the linear-mapped frame
   * buffer (when used). Set it to 0 when not in use.
   */
  0,
  /*
   * This is the size of the linear-mapped frame buffer (when used).
   * Set it to 0 when not in use.
   */
  0,
  /*
   * This is TRUE if the driver has support for the given depth for 
   * the detected configuration. It must be set in the Probe function.
   * It most cases it should be FALSE.
   */
  TRUE, /* 16bpp */
  FALSE,/* 24bpp */
  TRUE, /* 32bpp */
  /*
   * This is a pointer to a list of builtin driver modes.
   * This is rarely used, and in must cases, set it to NULL
   */
  NULL,
  /*
   * This is a factor that can be used to scale the raw clocks
   * to pixel clocks. This is rarely used, and in most cases, set
   * it to 1.
   */
  1,    /* ChipClockMulFactor */
  1     /* ChipClockDivFactor */
};  



/*
 * functions
 */

/*
 * RENDITIONIdent --
 *
 * Returns the string name for supported chipset 'n'. 
 */
static char *
RENDITIONIdent(n)
int n;
{
    static char *chipsets[]={"V1000", "V2x00"};

    if (n+1 > sizeof(chipsets)/sizeof(char *))
        return NULL;
    else
        return chipsets[n];
}



/*
 * RENDITIONProbe --
 *
 * This is the function that makes a yes/no decision about whether or not
 * a chipset supported by this driver is present or not. 
 */
static Bool
RENDITIONProbe()
{
    pciConfigPtr pcr=NULL;
    int c;

    /* First we attempt to figure out if one of the supported chipsets
     * is present. This code fragment is from the mga driver. */
    c=0;
    BOARD.chip=NOVERITE;
    if (vgaPCIInfo && vgaPCIInfo->AllCards) {
        while (pcr=vgaPCIInfo->AllCards[c++]) {
#ifdef DEBUG
            ErrorF( "RENDITION: vendor 0x%x chip 0x%x\n", 
                pcr->_vendor, pcr->_device);
#endif
            if ((pcr->_vendor == PCI_VENDOR_RENDITION) &&
                /* Extra checks to find active card */
                (pcr->_command & PCI_CMD_IO_ENABLE) &&
                (pcr->_command & PCI_CMD_MEM_ENABLE)){

                switch(pcr->_device) {
                    case PCI_CHIP_V1000:
                        BOARD.chip=V1000_DEVICE;
                        vga256InfoRec.chipset=RENDITIONIdent(0);
                        break;
                    case PCI_CHIP_V2x00:
                        BOARD.chip=V2000_DEVICE;
                        vga256InfoRec.chipset=RENDITIONIdent(1);
                        break;
                }
                if (BOARD.chip != NOVERITE)
                    break;
            }
        }
    } 
    else 
        return FALSE;

    if (BOARD.chip == NOVERITE) {
#if 0
        if (vga256InfoRec.chipset)
            ErrorF("%s %s: RENDITION: unknown chipset\n",
                XCONFIG_PROBED, vga256InfoRec.name);
#endif
        return FALSE;
    }

    /* 
     * Aha, it'a a Rendition! 
     */

    /* information that is needed to access the board */
    BOARD.mem_base=(vu8 *)(pcr->_base0&0xff000000);
    BOARD.io_base=(vu32)(pcr->_base1&0xff00);
    BOARD.mmio_base=(vu32)pcr->_base2;

    /* map memory to virtual address space */
    BOARD.vmem_base=(vu8 *)
#if defined(__alpha__)
        xf86MapVidMemSparse(
#else
        xf86MapVidMem(
#endif
            vga256InfoRec.scrnIndex, LINEAR_REGION,
            (pointer)(BOARD.mem_base), 16*1024*1024);

    if (0 != BOARD.mmio_base) 
        BOARD.vmmio_base=(vu32)
#if defined(__alpha__)
            xf86MapVidMemSparse(
#else
            xf86MapVidMem(
#endif
                vga256InfoRec.scrnIndex, MMIO_REGION,
                (pointer)(BOARD.mmio_base), 0x100);

#ifdef DEBUG
    ErrorF( "RENDITION: %s found, MEM@0x%x IO@0x%x MMIO@0x%x\n",
        RENDITIONIdent(BOARD.chip==V1000_DEVICE ? 0 : 1), 
        BOARD.mem_base, BOARD.io_base, BOARD.mmio_base);
    ErrorF( "RENDITION: After mapping: MEM@0x%x IO@0x%x MMIO@0x%x\n",
        BOARD.vmem_base, BOARD.io_base, BOARD.vmmio_base);
#endif

    /* set up I/O ports to be used by this card */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
    if (0 == BOARD.mmio_base) {
        /* OK, that might be a little greedy ... */
        for (c=0; c<RENDITIONNumExtPorts; c++) 
            RENDITIONExtPorts[c]=BOARD.io_base+c;
        xf86AddIOPorts(vga256InfoRec.scrnIndex, RENDITIONNumExtPorts, 
            RENDITIONExtPorts);
    }

    RENDITIONEnterLeave(ENTER); 

    /* count memory */
    RENDITION.ChipLinearBase=(unsigned long)BOARD.mem_base;
    RENDITION.ChipLinearSize=v_getmemorysize(&BOARD);

/*
    RENDITION.ChipLinearSize=4*1024*1024;
*/

    /* use some specified information */
    if (!vga256InfoRec.videoRam)
        vga256InfoRec.videoRam=RENDITION.ChipLinearSize/1024;

    if(vga256InfoRec.dacSpeeds[0])
        vga256InfoRec.maxClock=vga256InfoRec.dacSpeeds[0];
    else {
      if (BOARD.chip==V1000_DEVICE){
        vga256InfoRec.maxClock=135000;
      }
      else vga256InfoRec.maxClock=170000;
    }

    vga256InfoRec.bankedMono=FALSE;

    OFLG_SET(OPTION_NOACCEL, &RENDITION.ChipOptionFlags);
    OFLG_SET(OPTION_SYNC_ON_GREEN, &RENDITION.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_8_BIT, &RENDITION.ChipOptionFlags);
    OFLG_SET(OPTION_OVERCLOCK_MEM, &RENDITION.ChipOptionFlags);

    OFLG_SET(OPTION_SW_CURSOR, &RENDITION.ChipOptionFlags);
    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

  /*vgaSetLinearOffsetHook(RENDITIONLinearOffset);
    vgaSetPitchAdjustHook(RENDITIONPitchAdjust);*/
 
    return TRUE;
}



/*
 * RENDITIONLinearOffset --
 *
 * This function computes the byte offset into the linear frame buffer where
 * the frame buffer data should actually begin.
 */
static int
RENDITIONLinearOffset()
{
#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONLinearOffset() called\n");
#endif

    /* this function should be filled with something <ml> */
    return 0;
}



/*
 * RENDITIONFbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 */
static void
RENDITIONFbInit()
{
#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONFbInit() called\n");
#endif

    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
      if (BOARD.chip==V1000_DEVICE){
        RENDITIONHwCursorInit();
        ErrorF("%s %s: Using hardware cursor\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
      }
      else{
	/* Disabled for the moment due to bugs in software */
	ErrorF("%s %s: Hardware cursor on v2x00 not implemented in this release\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
        ErrorF("%s %s: Disabling hardware cursor\n",
            XCONFIG_PROBED, vga256InfoRec.name);
      }
    }
    else
        ErrorF("%s %s: Disabling hardware cursor\n",
            XCONFIG_GIVEN, vga256InfoRec.name);
}



/*
 * RENDITIONInit --
 *
 * The 'mode' parameter describes the video mode. The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode. The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
static Bool
RENDITIONInit(mode)
DisplayModePtr mode;
{
    struct v_modeinfo_t modeinfo;

#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONInit() called\n");
#endif

    /* is this really needed <ml> */
    if (!vgaHWInit(mode, sizeof(vgaRenditionRec)))
        return FALSE;

    /* construct a modeinfo for the v_setmode function */
    modeinfo.clock=vga256InfoRec.clock[mode->Clock];
    modeinfo.hdisplay=mode->HDisplay;
    modeinfo.hsyncstart=mode->HSyncStart;
    modeinfo.hsyncend=mode->HSyncEnd;
    modeinfo.htotal=mode->HTotal;
    modeinfo.hskew=mode->HSkew;
    modeinfo.vdisplay=mode->VDisplay;
    modeinfo.vsyncstart=mode->VSyncStart;
    modeinfo.vsyncend=mode->VSyncEnd;
    modeinfo.vtotal=mode->VTotal;

    modeinfo.screenwidth=mode->HDisplay;
    modeinfo.virtualwidth=vga256InfoRec.virtualX&0xfff8;

	if ((mode->Flags&(V_PHSYNC|V_NHSYNC)) && (mode->Flags&(V_PVSYNC|V_NVSYNC))) {
        modeinfo.hsynchi=(mode->Flags&V_PHSYNC == V_PHSYNC);
        modeinfo.vsynchi=(mode->Flags&V_PVSYNC == V_PVSYNC);
	}
	else
	{
	    int VDisplay=mode->VDisplay;
	    if (mode->Flags & V_DBLSCAN)
	        VDisplay*=2;
	    if (VDisplay < 400) {
		    /* +hsync -vsync */
            modeinfo.hsynchi=1;
            modeinfo.vsynchi=0;
        }
	    else if (VDisplay < 480) {
		    /* -hsync +vsync */
            modeinfo.hsynchi=0;
            modeinfo.vsynchi=1;
        }
	    else if (VDisplay < 768) {
		    /* -hsync -vsync */
            modeinfo.hsynchi=0;
            modeinfo.vsynchi=0;
        }
	    else {
		    /* +hsync +vsync */
            modeinfo.hsynchi=1;
            modeinfo.vsynchi=1;
        }
	}
	

    switch (vgaBitsPerPixel) {
        case 8:
            modeinfo.bitsperpixel=8;
            modeinfo.pixelformat=V_PIXFMT_8I; 
/*
            v_setpalette(&BOARD, 0, 255, vga256InfoRec.
*/
            break;
        case 16:
            modeinfo.bitsperpixel=16;
            if (vga256InfoRec.weight.green == 5)
                /* on a V1000, this looks too 'red/magenta' <ml> */
                modeinfo.pixelformat=V_PIXFMT_1555;
            else
                modeinfo.pixelformat=V_PIXFMT_565;
            break;
        case 32:
            modeinfo.bitsperpixel=32;
            modeinfo.pixelformat=V_PIXFMT_8888;
            break;
    }

    modeinfo.fifosize=128;

    v_setmode(&BOARD, &modeinfo);

    return TRUE;
}



/*
 * RENDITIONRestore --
 *
 * This function restores a video mode. It basically writes out all of
 * the registers that have previously been saved in the vgaRENDITIONRec data 
 * structure.
 */
static void 
RENDITIONRestore(restore)
vgaHWPtr restore;
{
    int c;

#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONRestore() called\n");
#endif

    v_setmode(&BOARD, &(BOARD.mode));

    /* do we need this? <ml> */
    vgaHWRestore((vgaHWPtr)restore);
}



/*
 * RENDITIONSave --
 *
 * This function saves the video state. It reads all of the SVGA registers
 * into the vgaRENDITIONRec data structure.
 */
static void *
RENDITIONSave(save)
vgaRenditionPtr save;
{
    int c;

#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONSave() called\n");
#endif

    /* needed or not? <ml> */
    save=(vgaRenditionPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaRenditionPtr));
}



/*
 * RENDITIONEnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down. Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device. This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
RENDITIONEnterLeave(enter)
Bool enter;
{
    unsigned char misc_ctrl;
    unsigned char temp;

#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONEnterLeave(%s) called\n", 
        (enter ? "enter" : "leave"));
#endif

#ifdef XFreeXDGA
    if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter) {
        return;
    }  
#endif 

    if (enter) {
        xf86EnableIOPorts(vga256InfoRec.scrnIndex);
        if (BOARD.mmio_base) {
            xf86MapDisplay(vga256InfoRec.scrnIndex, MMIO_REGION);
        }

        vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

        /* Unprotect CRTC[0-7] */
        outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, temp & 0x7F);
    }
    else {
        v_textmode(&BOARD);

        /* Protect CRTC[0-7] */
        outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);

        if (BOARD.mmio_base) {
            xf86UnMapDisplay(vga256InfoRec.scrnIndex, MMIO_REGION);
        }

        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}



/*
 * RENDITIONAdjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.
 */
static void 
RENDITIONAdjust(x, y)
int x, y;
{
    int offset;

#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONAdjust(%d, %d) called\n", x, y);
#endif

    /* this calculation has to be revised! <ml> */
    offset=(y*BOARD.mode.virtualwidth+x)*(BOARD.mode.bitsperpixel>>3);

    v_setframebase(&BOARD, offset);
}



/*
 * RENDITIONValidMode -- 
 *
 * Checks if a mode is suitable for the selected chipset.
 */
static Bool
RENDITIONValidMode(mode,verbose,flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
    int lace = 1 + ((mode->Flags & V_INTERLACE) != 0);

    if ((mode->CrtcHDisplay <= 2048) &&
        (mode->CrtcHSyncStart <= 4096) && 
        (mode->CrtcHSyncEnd <= 4096) && 
        (mode->CrtcHTotal <= 4096) &&
        (mode->CrtcVDisplay <= 2048 * lace) &&
        (mode->CrtcVSyncStart <= 4096 * lace) &&
        (mode->CrtcVSyncEnd <= 4096 * lace) &&
        (mode->CrtcVTotal <= 4096 * lace))
    {
        return MODE_OK;
    }
    else {
        return MODE_BAD;
    }
}



/*
 * RENDITIONPitchAdjust --
 *
 * This function adjusts the display width (pitch) once the virtual
 * width is known.  It returns the display width.
 */
static int 
RENDITIONPitchAdjust()
{
    BOARD.mode.virtualwidth=vga256InfoRec.virtualX&0xfff8;

    return BOARD.mode.virtualwidth; 
}
