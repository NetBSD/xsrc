/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tdfx/vb_driver.c,v 1.1.2.10 2000/01/24 21:07:02 dawes Exp $ */
/*
   Voodoo Banshee driver version 1.0.2

   Author: Daryll Strauss

   Copyright: 1998,1999
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
#include "miline.h"

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

#include "vb.h"

#define VB2XCUTOFF 135000

VBRec VBinfo;

/* Print a driver identifying message. */
static char *VBIdentify(int flags);

/* Identify if there is any hardware present that I know how to drive. */
static Bool VBProbe();

/* Enter from a virtual terminal */
static void VBEnterLeave(Bool Enter);

/* Allow mode switching */
static Bool VBInit(DisplayModePtr mode);

/* Allow moving the viewport */
static void VBAdjust(int x, int y);

/* Check if a mode is valid on the hardware */
static Bool VBValidMode(DisplayModePtr mode, Bool verbose, int flags);

#ifdef DPMSExtension
/* Switch to various Display Power Management System levels */
static void VBDisplayPowerManagementSet(int PowerManagermentMode);
#endif

static void * VBSave(void *savein);
static void VBRestore(void *restorein);
static void VBFbInit();
extern VBIdle();

#ifdef VB_PCI_IO
/* vgaHW* Replacements */
extern void VBvgaHWRestore(vgaHWPtr restore);
extern void *VBvgaHWSave(vgaHWPtr save, int size);
extern Bool VBvgaHWInit(DisplayModePtr mode, int size);
#else
#define VBvgaHWRestore vgaHWRestore
#define VBvgaHWSave vgaHWSave
#define VBvgaHWInit vgaHWInit
#endif

vgaVideoChipRec TDFX = {
  VBProbe,
  VBIdentify,
  VBEnterLeave,
  VBInit,
  VBValidMode,
  VBSave,
  VBRestore,
  VBAdjust,
  (void(*)())NoopDDA, /* SaveScreen */
  (void(*)())NoopDDA, /* GetMode */
  VBFbInit,
  (void(*)())NoopDDA, /* SetRead */
  (void(*)())NoopDDA, /* SetWrite */
  (void(*)())NoopDDA, /* SetReadWrite */
  0x10000, /* size of the mapped memory window */
  0x10000, /* size of the video memory bank */
  16, /* bits to shift addresses */
  0xFFFF, /* mask to determine address */
  0x0000, 0x10000, /* range of addresses with a bank read */
  0x0000, 0x10000, /* range of addresses with a bank write */
  FALSE, /* supports single bank */
  VGA_NO_DIVIDE_VERT, /* divide vert timings when interlaced */
  {0,}, /* option flags */
  8, /* multiple for width rounding */
  TRUE, /* has linear framebuffer */
  0, /* physical address */
  0, /* size */
  TRUE, /* supports 16bpp */
  TRUE, /* supports 24bpp */
  TRUE, /* supports 32bpp */
  NULL, /* builtin modes */
  1, /* clock mult factor */
  1 /* clock div factor */
};

#define VERSION 1
#define VB_NAME "VB"
#define VB_DRIVER_NAME "vb"
#define VB_DRIVER_VERSION 0x00000102

static char *
VBIdentify(int n) {
  char *VBChipsets[] = {"Voodoo Banshee", "Voodoo3"};

  VBTRACE("VBIdentify start\n");
  if (n+1>sizeof(VBChipsets)/sizeof(char*)) return 0;
  return VBChipsets[n];
}

static void
PrintRegisters(vgaVBPtr regs)
{
#ifdef TRACE
  static int vgaIOBase=0;
  VBPtr pVB;
  int i;

  pVB = VBPTR();
#if 0
  ErrorF("VGA Registers\n");
#ifdef VB_PCI_IO
  ErrorF("Using PCI I/O Registers\n");
#endif
  ErrorF("MiscOutReg = %x versus %x\n", inb(VGA_REG(0x3cc)), regs->std.MiscOutReg);
  ErrorF("Noclock is %d\n", regs->std.NoClock);
  for (i=0; i<25; i++) {
    outb(VGA_REG(0x3D4), i);
    ErrorF("CRTC[%d]=%d versus %d\n", i, inb(VGA_REG(0x3D5)), regs->std.CRTC[i]);
  }
  if (!vgaIOBase)
    vgaIOBase = (inb(VGA_REG(0x3cc)) & 0x1) ? 0x3D0 : 0x3B0;
  for (i=0; i<21; i++) {
    inb(VGA_REG(vgaIOBase+0xA));
    outb(VGA_REG(0x3C0), i);
    ErrorF("Attribute[%d]=%d versus %d\n", i, inb(VGA_REG(0x3C1)), regs->std.Attribute[i]);
  }
  inb(VGA_REG(vgaIOBase+0xA));
  outb(VGA_REG(0x3C0), BIT(5));
  for (i=0; i<9; i++) {
    outb(VGA_REG(0x3CE), i);
    ErrorF("Graphics[%d]=%d versus %d\n", i, inb(VGA_REG(0x3CF)), regs->std.Graphics[i]);
  }
  for (i=0; i<5; i++) {
    outb(VGA_REG(0x3C4), i);
    ErrorF("Sequencer[%d]=%d versus %d\n", i, inb(VGA_REG(0x3C5)), regs->std.Sequencer[i]);
  }
#endif
#if 1
  ErrorF("Banshee Registers\n");
  ErrorF("VidCfg = %x versus %x\n",  inl(pVB->IOBase+VIDPROCCFG), regs->vidcfg);
  ErrorF("DACmode = %x versus %x\n", inl(pVB->IOBase+DACMODE), regs->dacmode);
  ErrorF("Vgainit0 = %x versus %x\n", inl(pVB->IOBase+VGAINIT0), regs->vgainit0);
  ErrorF("Vgainit1 = %x versus %x\n", inl(pVB->IOBase+VGAINIT1), regs->vgainit1);
  ErrorF("DramInit0 = %x\n", inl(pVB->IOBase+DRAMINIT0));
  ErrorF("DramInit1 = %x\n", inl(pVB->IOBase+DRAMINIT1));
  ErrorF("VidPLL = %x versus %x\n", inl(pVB->IOBase+PLLCTRL0), regs->vidpll);
  ErrorF("screensize = %x versus %x\n", inl(pVB->IOBase+VIDSCREENSIZE), regs->screensize);
  ErrorF("stride = %x versus %x\n", inl(pVB->IOBase+VIDDESKTOPOVERLAYSTRIDE), regs->stride);
  ErrorF("startaddr = %x versus %x\n", inl(pVB->IOBase+VIDDESKTOPSTARTADDR), regs->startaddr);
  ErrorF("Input Status 0 = %x\n", inb(pVB->IOBase+0xc2));
  ErrorF("Input Status 1 = %x\n", inb(pVB->IOBase+0xda));
  ErrorF("2D Status = %x\n", inl(pVB->IOBase+SST_2D_OFFSET));
  ErrorF("3D Status = %x\n", inl(pVB->IOBase+SST_3D_OFFSET));
#endif
#endif
}
  
static int
VBCountRam() {
  VBPtr pVB = VBPTR();
  int memSize;
  int memType=-1; /* SDRAM or SGRAM */

  VBTRACE("VBCountRam start\n");
  memSize=0;
  if (pVB->IOBase) {
    CARD32 
      partSize,                 /* size of SGRAM chips in Mbits */
      nChips,                   /* # chips of SDRAM/SGRAM */
      dramInit0_strap,    
      dramInit1_strap,    
      dramInit1,
      miscInit1;

    /* determine memory type: SDRAM or SGRAM */
    memType = MEM_TYPE_SGRAM;
    dramInit1_strap = inl(pVB->IOBase+DRAMINIT1);
    dramInit1_strap &= SST_MCTL_TYPE_SDRAM;
    if (dramInit1_strap) memType = MEM_TYPE_SDRAM;

    /* set memory interface delay values and enable refresh */
    /* these apply to all RAM vendors */
    dramInit1 = 0x0;
    dramInit1 |= 2<<SST_SGRAM_OFLOP_DEL_ADJ_SHIFT;
    dramInit1 |= SST_SGRAM_CLK_NODELAY;
    dramInit1 |= SST_DRAM_REFRESH_EN;
    dramInit1 |= (0x18 << SST_DRAM_REFRESH_VALUE_SHIFT) & SST_DRAM_REFRESH_VALUE;  
    dramInit1 &= ~SST_MCTL_TYPE_SDRAM;
    dramInit1 |= dramInit1_strap;
    outl(pVB->IOBase+DRAMINIT1, dramInit1);

    /* determine memory size from strapping pins (dramInit0 and dramInit1) */
    dramInit0_strap = inl(pVB->IOBase+DRAMINIT0);

    dramInit0_strap &= SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS;

    if ( memType == MEM_TYPE_SDRAM ) {
      memSize = 16;
    } else {
      nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;
    
      if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_8MBIT )  {
	partSize = 8;
      } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_16MBIT) {
	partSize = 16;
      } else {
	ErrorF("Invalid sgram type = 0x%x",
	       (dramInit0_strap & SST_SGRAM_TYPE) << SST_SGRAM_TYPE_SHIFT );
	return 0;
      }
      memSize = (nChips * partSize) / 8;      /* in MBytes */
    }
    VBTRACEREG("dramInit0 = %x dramInit1 = %x\n", dramInit0_strap, dramInit1_strap);
    VBTRACEREG("MemConfig %d chips %d size %d total\n", nChips, partSize, memSize);

    /*
      disable block writes for SDRAM
    */
    miscInit1 = inl(pVB->IOBase+MISCINIT1);
    if ( memType == MEM_TYPE_SDRAM ) {
      miscInit1 |= SST_DISABLE_2D_BLOCK_WRITE;
    }
    miscInit1|=1;
    outl(pVB->IOBase+MISCINIT1, miscInit1);
  }

  if (memType == MEM_TYPE_SGRAM) {
    ErrorF("Found %d MB SGRAM\n", memSize);
  } else {
    ErrorF("Found %d MB SDRAM\n", memSize);
  }
  /* return # of KBytes of board memory */
  return memSize*1024;
}

/*
 * Map the framebuffer and MMIO memory.
 */

static Bool
VBMapMem()
{
  CARD32 save;
  VBPtr pVB;
  int i;
  pciTagRec pciTag;


  VBTRACE("VBMapMem start\n");
  pVB = VBPTR();

  /*
   * Disable memory and I/O before mapping the MMIO area.  This avoids
   * the MMIO area being read during the mapping (which happens on
   * some SVR4 versions), which will cause a lockup.
   */
  pciTag = pcibusTag(pVB->PciConfig->_bus, pVB->PciConfig->_cardnum, 
		     pVB->PciConfig->_func);

  save = pciReadLong(pciTag, PCI_CMD_STAT_REG);
  pciWriteLong(pciTag, PCI_CMD_STAT_REG,
       save & ~(PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE));

  /*
   * Map IO registers to virtual address space
   */ 
#if !defined(__alpha__)
  pVB->IOMap = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
			      (pointer)pVB->IOAddress, VBIOMAPSIZE);
#else
  /*
   * For Alpha, we need to map SPARSE memory, since we need
   * byte/short access.
   */
  pVB->IOMap = xf86MapVidMemSparse(vga256InfoRec.scrnIndex, MMIO_REGION,
				    (pointer)pVB->IOAddress, VBIOMAPSIZE);
#endif
  if (pVB->IOMap == NULL)
    return FALSE;

#if defined(SVR4)
  /*
   * For some SVR4 versions, a 32-bit read is done for the first
   * location in each page when the page is first mapped.  If this
   * is done while memory and I/O are enabled, the result will be
   * a lockup, so make sure each page is mapped here while it is safe
   * to do so.
   */
  for (i=0; i<VBIOMAPSIZE; i+=0x1000) {
    CARD32 val;
    val = *(volatile CARD32 *)(pVB->IOMap+i);
  }
#endif

#ifdef __alpha__
  /*
   * for Alpha, we need to map DENSE memory as well, for
   * setting CPUToScreenColorExpandBase.
   */
  pVB->IOMapDense = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
				  (pointer)pVB->IOAddress, VBIOMAPSIZE);
  if (pVB->IOMapDense == NULL)
    return FALSE;
#endif /* __alpha__ */

  /* Re-enable I/O and memory */
  pciWriteLong(pciTag, PCI_CMD_STAT_REG,
	       save | (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE));

  return TRUE;
}


/*
 * Unmap the framebuffer and MMIO memory.
 */

static Bool
VBUnmapMem()
{
  VBPtr pVB;

  VBTRACE("VBUnmapMem start\n");
  pVB = VBPTR();

  /*
   * Unmap IO registers to virtual address space
   */ 
#ifndef __alpha__
  if (pVB->IOMap)
    xf86UnMapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
		    (pointer)pVB->IOMap, VBIOMAPSIZE);
#else
  if (pVB->IOMap) 
    xf86UnMapVidMemSparse(vga256InfoRec.scrnIndex, MMIO_REGION,
			  (pointer)pVB->IOMap, VBIOMAPSIZE);
#endif
  pVB->IOMap = NULL;

#ifdef __alpha__
  if (pVB->IOMapDense)
    xf86UnMapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
		    (pointer)pVB->IOMapDense, VBIOMAPSIZE);
  pVB->IOMapDense = NULL;
#endif /* __alpha__ */
  return TRUE;
}

static Bool
VBProbe() {
  int i;
  pciConfigPtr pcr=0, vbpcr=0;
  VBPtr pVB;
  
  VBTRACE("VBProbe start\n");
  pVB = VBPTR();
  if (vga256InfoRec.chipset) {
    char *chipset;
    for (i=0; chipset=VBIdentify(i); i++) {
      if (!StrCaseCmp(vga256InfoRec.chipset, chipset)) break;
    }
    if (!chipset) return FALSE;
  }

  pVB->PciConfig=0;
  if (vgaPCIInfo && vgaPCIInfo->AllCards) {
    i=0;
    while (pcr = vgaPCIInfo->AllCards[i++]) {
      if (pcr->_vendor == PCI_VENDOR_3DFX) {
	int id = pcr->_device;

	if (vga256InfoRec.chipID) {
	  ErrorF("%s %s: VB chipset override, using ChipID "
		 "%0x04x instead of 0x%04x\n", XCONFIG_GIVEN,
		 vga256InfoRec.name, vga256InfoRec.chipID,
		 pcr->_device);
	}
	switch (id) {
	case PCI_CHIP_BANSHEE:
	  vga256InfoRec.chipset = VBIdentify(0);
	  pVB->PciConfig=pcr;
	  break;
	case PCI_CHIP_VOODOO3:
	  vga256InfoRec.chipset = VBIdentify(1);
	  pVB->PciConfig=pcr;
	  break;
	}
      }
    }
  } else return FALSE;

  if (!pVB->PciConfig) return FALSE;

  if (!vga256InfoRec.IObase) {
    vga256InfoRec.IObase = pVB->PciConfig->_base2&0xFFFFFFFC;
  }
  pVB->IOBase = vga256InfoRec.IObase;
  pVB->IOAddress = pVB->PciConfig->_base0&0xFFFFFFF0;
  if (!vga256InfoRec.MemBase) {
    vga256InfoRec.MemBase = pVB->PciConfig->_base1&0xFFFFFFF0;
  }
  pVB->FbAddress = TDFX.ChipLinearBase = vga256InfoRec.MemBase;

  VBEnterLeave(ENTER);

  if (!vga256InfoRec.videoRam) {
    vga256InfoRec.videoRam = VBCountRam();
  } else {
    VBCountRam(); /* We have to initialize it */
  }
  pVB->FbMapSize = TDFX.ChipLinearSize = vga256InfoRec.videoRam*1024;

  VBMapMem();

  pVB->MinClock=0;
  if (vga256InfoRec.dacSpeeds[0]) {
    vga256InfoRec.maxClock = pVB->MaxClock = vga256InfoRec.dacSpeeds[0];
  } else {
    switch (pVB->PciConfig->_device) {
    case PCI_CHIP_BANSHEE:
      vga256InfoRec.maxClock = pVB->MaxClock = 270000;
      break;
    case PCI_CHIP_VOODOO3:
      vga256InfoRec.maxClock = pVB->MaxClock = 300000;
      break;
    default:
      /* This should never happen */
      ErrorF("Unknown chipset value\n");
      return FALSE;
    }
  }

  /* We need this early on to initialize */
  pVB->cpp = vga256InfoRec.bitsPerPixel/8;
  if (pVB->cpp<1 || pVB->cpp>4 || pVB->cpp*8!=vga256InfoRec.bitsPerPixel) {
    ErrorF("VB: unsupported depth\n");
    return FALSE;
  }
  pVB->vgaInitDone=FALSE;

  vga256InfoRec.bankedMono = FALSE;
#ifdef XFreeXDGA
  vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

#ifdef DPMSExtension
  vga256InfoRec.DPMSSet = VBDisplayPowerManagementSet;
#endif

  OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
  OFLG_SET(OPTION_SW_CURSOR, &TDFX.ChipOptionFlags);
  OFLG_SET(OPTION_NOACCEL, &TDFX.ChipOptionFlags);
  return TRUE;
}

static void *
VBSave(void *savein)
{
  VBPtr pVB;
  vgaVBPtr save = (vgaVBPtr)savein;

  VBTRACE("VBSave start save=%x\n", save);
  pVB = VBPTR();
  save=VBvgaHWSave(savein, sizeof(vgaVBRec));
#ifdef TRACE
  if (save!=savein) 
    ErrorF("New save is %x\n", save);
#endif
  outb(VGA_REG(0x3d4), 0x1a);
  save->ExtVga[0] = inb(VGA_REG(0x3d5));
  outb(VGA_REG(0x3d4), 0x1b);
  save->ExtVga[1] = inb(VGA_REG(0x3d5));
  save->vgainit0=inl(pVB->IOBase+VGAINIT0);
  /*  save->vgainit1=inl(pVB->IOBase+VGAINIT1); */
  save->vidcfg=inl(pVB->IOBase+VIDPROCCFG);
  save->vidpll=inl(pVB->IOBase+PLLCTRL0);
  save->dacmode=inl(pVB->IOBase+DACMODE);
  save->screensize=inl(pVB->IOBase+VIDSCREENSIZE);
  save->stride=inl(pVB->IOBase+VIDDESKTOPOVERLAYSTRIDE);
  save->cursloc=inl(pVB->IOBase+HWCURPATADDR);
  save->startaddr=inl(pVB->IOBase+VIDDESKTOPSTARTADDR);
  save->clip0min=REF32(SST_2D_OFFSET+SST_2D_CLIP0MIN);
  save->clip0max=REF32(SST_2D_OFFSET+SST_2D_CLIP0MAX);
  save->clip1min=REF32(SST_2D_OFFSET+SST_2D_CLIP1MIN);
  save->clip1max=REF32(SST_2D_OFFSET+SST_2D_CLIP1MAX);
  PrintRegisters(save);
  return save;
}

static void
VBRestore(void *restorein)
{
  VBPtr pVB;
  vgaVBPtr restore = (vgaVBPtr)restorein;

  VBTRACE("VBRestore start restore=%x\n", restore);
  pVB = VBPTR();

  /* This will be nonzero when glide is active */
  if (REF32(0x0080024)) return;

  vgaProtect(TRUE);
  VBvgaHWRestore(restorein);
  outw(VGA_REG(0x3d4), (restore->ExtVga[0]<<8) | 0x1A);
  outw(VGA_REG(0x3d4), (restore->ExtVga[1]<<8) | 0x1B);
  outl(pVB->IOBase+PLLCTRL0, restore->vidpll);
  outl(pVB->IOBase+DACMODE, restore->dacmode);
  outl(pVB->IOBase+VIDDESKTOPOVERLAYSTRIDE, restore->stride);
  outl(pVB->IOBase+HWCURPATADDR, restore->cursloc);
  outl(pVB->IOBase+VIDSCREENSIZE, restore->screensize);
  outl(pVB->IOBase+VIDDESKTOPSTARTADDR, restore->startaddr);
  REF32(SST_2D_OFFSET+SST_2D_CLIP0MIN)=restore->clip0min;
  REF32(SST_2D_OFFSET+SST_2D_CLIP0MAX)=restore->clip0max;
  REF32(SST_2D_OFFSET+SST_2D_CLIP1MIN)=restore->clip1min;
  REF32(SST_2D_OFFSET+SST_2D_CLIP1MAX)=restore->clip1max;
  outl(pVB->IOBase+VGAINIT0, restore->vgainit0);

  outl(pVB->IOBase+VIDPROCCFG, restore->vidcfg);
  vgaProtect(FALSE);
  VBIdle();
}

static void
VBEnterLeave(Bool enter)
{
  static int vgaIOBase=0;
  unsigned char temp;
  VBPtr pVB;

  VBTRACE("VBEnterLeave start %d\n", enter);
#ifdef XFreeXDGA
  if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter) {
    VBHideCursor();
    return;
  }
#endif

  pVB = VBPTR();
  if (enter) {
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    if (!vgaIOBase)
      vgaIOBase = (inb(VGA_REG(0x3cc)) & 0x1) ? 0x3D0 : 0x3B0;
    if (pVB->IOMap) {
      xf86MapDisplay(vga256InfoRec.scrnIndex, MMIO_REGION);
    }

    /* unprotect */
    outb(VGA_REG(vgaIOBase+4), 0x11);
    temp = inb(VGA_REG(vgaIOBase + 5));
    outb(VGA_REG(vgaIOBase+5), temp&0x7f);
  } else {
    outb(VGA_REG(vgaIOBase+4), 0x11);
    temp=inb(VGA_REG(vgaIOBase+5));
    outb(VGA_REG(vgaIOBase+5), (temp&0x7f)|0x80);

    if (pVB->IOMap) {
      xf86UnMapDisplay(vga256InfoRec.scrnIndex, MMIO_REGION);
      if (xf86Exiting && xf86Info.caughtSignal) VBUnmapMem();
    }
    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }
}

static void
VBAdjust(int x, int y)
{
  VBPtr pVB;
  vgaVBPtr vbReg;

  pVB = VBPTR();
  vbReg=(vgaVBPtr)vgaNewVideoState;
  vbReg->startaddr=y*pVB->stride+(x*pVB->cpp);
  VBTRACE("VBAdjustFrame to x=%d y=%d offset=%d\n", x, y, vbReg->startaddr);
  outl(pVB->IOBase+VIDDESKTOPSTARTADDR, vbReg->startaddr);
}

static Bool
VBValidMode(DisplayModePtr mode, Bool verbose, int flags)
{
  VBTRACE("VBValidMode start\n");
  if ((mode->HDisplay>2046) || (mode->VDisplay>1536)) 
    return MODE_BAD;
  /* Banshee doesn't support interlace. Does V3? */
  if (mode->Flags&V_INTERLACE) 
    return MODE_BAD;
  /* In clock doubled mode widths must be divisible by 16 instead of 8 */
  if ((mode->Clock>VB2XCUTOFF) && (mode->HDisplay%16))
    return MODE_BAD;
  return MODE_OK;
}

#ifdef DPMSExtension
static void
VBDisplayPowerManagementSet(int PowerManagementMode)
{
 int dacmode, state;
 VBPtr pVB;

  VBTRACE("VBDisplayPowerManagementSet start. Mode=%d\n", PowerManagementMode);
  if (!xf86VTSema) return;
  pVB = VBPTR();
  dacmode=inl(pVB->IOBase+DACMODE);
  switch (PowerManagementMode) {
  case DPMSModeOn:
    /* Screen: On; HSync: On, VSync: On */
    state=0;
    break;
  case DPMSModeStandby:
    /* Screen: Off; HSync: Off, VSync: On */
    state=BIT(1);
    break;
  case DPMSModeSuspend:
    /* Screen: Off; HSync: On, VSync: Off */
    state=BIT(3);
    break;
  case DPMSModeOff:
    /* Screen: Off; HSync: Off, VSync: Off */
    state=BIT(1)|BIT(3);
    break;
  }
  dacmode&=~(BIT(1)|BIT(3));
  dacmode|=state;
  outl(pVB->IOBase+DACMODE, dacmode);
}
#endif

#define REFFREQ 14318.18

static int
CalcPLL(int freq, int *f_out) {
  int m, n, k, best_m, best_n, best_k, f_cur, best_error;

  best_error=freq;
  best_n=best_m=best_k=0;
  for (n=1; n<256; n++) {
    f_cur=REFFREQ*(n+2);
    if (f_cur<freq) {
      f_cur=f_cur/3;
      if (freq-f_cur<best_error) {
	best_error=freq-f_cur;
	best_n=n;
	best_m=1;
	best_k=0;
	continue;
      }
    }
    for (m=1; m<64; m++) {
      for (k=0; k<4; k++) {
	f_cur=REFFREQ*(n+2)/(m+2)/(1<<k);
	if (abs(f_cur-freq)<best_error) {
	  best_error=abs(f_cur-freq);
	  best_n=n;
	  best_m=m;
	  best_k=k;
	}
      }
    }
  }
  n=best_n;
  m=best_m;
  k=best_k;
  *f_out=REFFREQ*(n+2)/(m+2)/(1<<k);
  return (n<<8)|(m<<2)|k;
}

static Bool
SetupVidPLL(DisplayModePtr mode) {
  VBPtr pVB;
  vgaVBPtr vbReg;
  int freq, f_out;

  VBTRACE("SetupVidPLL start\n");
  pVB = VBPTR();
  vbReg=(vgaVBPtr)vgaNewVideoState;
  freq=vga256InfoRec.clock[vbReg->std.NoClock];
  vbReg->dacmode&=~SST_DAC_MODE_2X;
  vbReg->vidcfg&=~SST_VIDEO_2X_MODE_EN;
  if (freq<pVB->MinClock) {
    ErrorF("Underclocked PLLs\n");
    freq=pVB->MinClock;
  } else if (freq>VB2XCUTOFF) {
    if (freq>pVB->MaxClock) {
      ErrorF("Overclocked PLLs\n");
      freq=pVB->MaxClock;
    }
    vbReg->dacmode|=SST_DAC_MODE_2X;
    vbReg->vidcfg|=SST_VIDEO_2X_MODE_EN;
  }
  vbReg->vidpll=CalcPLL(freq, &f_out);
  VBTRACEREG("Vid PLL freq=%d f_out=%d reg=%x\n", freq, f_out, 
     vbReg->vidpll);
  return TRUE;
}

#if 0
static Bool
SetupMemPLL(int freq) {
  VBPtr pVB;
  vgaVBPtr vbReg;
  int f_out;

  VBTRACE("SetupMemPLL start\n");
  pVB=VBPTR();
  vbReg=(vgaVBPtr)vgaNewVideoState;
  vbReg->mempll=CalcPLL(freq, &f_out);
  outl(pVB->IOBase+PLLCTRL1, vbReg->mempll);
  VBTRACEREG("Mem PLL freq=%d f_out=%d reg=%x\n", freq, f_out, 
     vbReg->mempll);
  return TRUE;
}

static Bool
SetupGfxPLL(int freq) {
  VBPtr pVB;
  vgaVBPtr vbReg;
  int f_out;

  VBTRACE("SetupGfxPLL start\n");
  pVB=VBPTR();
  vbReg=(vgaVBPtr)vgaNewVideoState;
  vbReg->gfxpll=CalcPLL(freq, &f_out);
  outl(pVB->IOBase+PLLCTRL2, vbReg->gfxpll);
  VBTRACEREG("Gfx PLL freq=%d f_out=%d reg=%x\n", freq, f_out, 
     vbReg->gfxpll);
  return TRUE;
}
#endif

static Bool
VBInitVGA()
{
  VBPtr pVB;
  vgaVBPtr vbReg;
  int miscinit1;
  int i, j;

  VBTRACE("VBInitVGA start\n");
  pVB=VBPTR();
  if (!pVB->IOBase) return FALSE;
  pVB->vgaInitDone=TRUE;
  vbReg = (vgaVBPtr)vgaNewVideoState;
  vbReg->vgainit0 = 0;
  vbReg->vgainit0 |= SST_VGA0_EXTENSIONS;
  vbReg->vgainit0 |= SST_WAKEUP_3C3 << SST_VGA0_WAKEUP_SELECT_SHIFT;
  /*  vbReg->vgainit0 |= SST_VGA0_ENABLE_DECODE << SST_VGA0_LEGACY_DECODE_SHIFT; */
  vbReg->vgainit0 |= SST_ENABLE_ALT_READBACK << SST_VGA0_CONFIG_READBACK_SHIFT;
  vbReg->vgainit0 |= BIT(12);

  vbReg->vidcfg = SST_VIDEO_PROCESSOR_EN | SST_CURSOR_X11 | SST_DESKTOP_EN |
    (pVB->cpp-1)<<SST_DESKTOP_PIXEL_FORMAT_SHIFT;
  if (pVB->cpp!=1) vbReg->vidcfg |= SST_DESKTOP_CLUT_BYPASS;

  vbReg->stride = pVB->stride;
    
  vbReg->cursloc = pVB->CursorData;

  vbReg->clip0min = vbReg->clip1min = 0;
  vbReg->clip0max = vbReg->clip1max = pVB->maxClip;

  return TRUE;
}  

static Bool
VBScreenInit(ScreenPtr pScreen, pointer pbits, int xsize, int ysize,
	     int dpix, int dpiy, int width) {
  miSetZeroLineBias(pScreen, OCTANT2 | OCTANT5 | OCTANT7 | OCTANT8);
  return TRUE;
}

static void
VBFbInit()
{
  VBPtr pVB;

  VBTRACE("VBFbInit start\n");
  pVB = VBPTR();

  vgaSetScreenInitHook(VBScreenInit);
  /* Set the stride */
  pVB->stride=vga256InfoRec.displayWidth*pVB->cpp;

  if (pVB->stride>=1024) {
    pVB->CursorData=TDFX.ChipLinearSize-pVB->stride;
  } else {
    pVB->CursorData=TDFX.ChipLinearSize-
      ((1024+pVB->stride-1)/pVB->stride)*pVB->stride;
  }
  if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) &&
      vga256InfoRec.displayWidth < HW_CURSOR_MAX_DISPLAYWIDTH)
    VBHwCursorInit();
  if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
    VBAccelInit();
}

static Bool
VBSetMode(DisplayModePtr mode) {
  VBPtr pVB;
  vgaVBPtr vbReg;
  int i;
  int hbs, hbe, vbs, vbe, hse, wd;
  int hd, hss, ht, vss, vt, vd, vse;

  VBTRACE("VBSetMode start\n");

  pVB = VBPTR();
  if (!pVB->vgaInitDone) VBInitVGA();
  vbReg = (vgaVBPtr)vgaNewVideoState;

  vbReg->ExtVga[0]=0;
  vbReg->ExtVga[1]=0;

  /* Calculate the CRTC values */
  hd = (mode->CrtcHDisplay>>3)-1;
  hss = (mode->CrtcHSyncStart>>3)-1;
  hse = (mode->CrtcHSyncEnd>>3)-1;
  ht = (mode->CrtcHTotal>>3)-1;
  hbs = hd;
  hbe = ht;

  vd = mode->CrtcVDisplay-1;
  vss = mode->CrtcVSyncStart-1;
  vse = mode->CrtcVSyncEnd-1;
  vt = mode->CrtcVTotal-2;
  vbs = vd;
  vbe = vt;

  if (pVB->cpp==4)
    wd = vga256InfoRec.displayWidth>>1;
  else
    wd = vga256InfoRec.displayWidth>>(4-pVB->cpp);

  /* Tell the board we're using a programmable clock */
  vbReg->std.MiscOutReg |= 0xC;

  /* Handle the higher resolution modes */
  vbReg->ExtVga[0] |= (ht&0x100)>>8;
  vbReg->ExtVga[0] |= (hd&0x100)>>6;
  vbReg->ExtVga[0] |= (hbs&0x100)>>4;
  vbReg->ExtVga[0] |= (hbe&0x40)>>1;
  vbReg->ExtVga[0] |= (hss&0x100)>>2;
  vbReg->ExtVga[0] |= (hse&0x20)<<2; 

  vbReg->ExtVga[1] |= (vt&0x400)>>10;
  vbReg->ExtVga[1] |= (vd&0x400)>>8; 
  vbReg->ExtVga[1] |= (vbs&0x400)>>6;
  vbReg->ExtVga[1] |= (vbe&0x400)>>4;

  /* Program the normal CRTC registers */
  vbReg->std.CRTC[0] = ht - 4;
  vbReg->std.CRTC[1] = hd;
  vbReg->std.CRTC[2] = hbs;
  vbReg->std.CRTC[3] = (hbe&0x1F)|0x80;
  vbReg->std.CRTC[4] = hss;
  vbReg->std.CRTC[5] = ((hbe&0x20)<<2) | (hse&0x1F);
  vbReg->std.CRTC[6] = vt&0xFF;
  vbReg->std.CRTC[7] = ((vt&0x100)>>8) | ((vd&0x100)>>7) | ((vss&0x100)>>6) |
    ((vbs&0x100)>>5) | 0x10 | ((vt&0x200)>>4) | ((vd&0x200)>>3) | 
    ((vss&0x200)>>2);
  vbReg->std.CRTC[9] = ((vbs&0x200)>>4)|0x40;
  vbReg->std.CRTC[16] = vss&0xFF;
  vbReg->std.CRTC[17] = (vse&0xF)|0x20;
  vbReg->std.CRTC[18] = vd&0xFF;
  vbReg->std.CRTC[19] = wd&0xFF;
  vbReg->std.CRTC[21] = vbs&0xFF;
  vbReg->std.CRTC[22] = (vbe+1)&0xFF;

  if (!SetupVidPLL(mode)) return FALSE;

  /* Set the screen size */
  if (mode->Flags&V_DBLSCAN) {
    vbReg->std.CRTC[9] |= 0x80;
    vbReg->screensize=mode->HDisplay|(mode->VDisplay<<13);
    vbReg->vidcfg |= SST_HALF_MODE;
  } else {
    vbReg->screensize=mode->HDisplay|(mode->VDisplay<<12);
    vbReg->vidcfg &= ~SST_HALF_MODE;
  }

  VBTRACEREG("cpp=%d Hdisplay=%d Vdisplay=%d stride=%d screensize=%x\n", 
	     pVB->cpp, mode->HDisplay, mode->VDisplay, vbReg->stride, 
	     vbReg->screensize);
  return TRUE;
}

static Bool
VBInit(DisplayModePtr mode)
{
  VBPtr pVB;
  vgaHWPtr hwp;
  Bool dbl;
  vgaVBPtr vbReg;

  VBTRACE("VBModeInit start\n");
  dbl=FALSE;
  /* Check for 2x mode */
  if (vga256InfoRec.clock[mode->Clock]>VB2XCUTOFF) {
    mode->CrtcHDisplay>>=1;
    mode->CrtcHSyncStart>>=1;
    mode->CrtcHSyncEnd>>=1;
    mode->CrtcHTotal>>=1;
    mode->CrtcHSkew>>=1;
    dbl=TRUE;
  }

  /* Setup the mode */
  if (!VBvgaHWInit(mode, sizeof(vgaVBRec))) return FALSE;

  if (!VBSetMode(mode)) return FALSE;

  if (dbl) {
    mode->CrtcHTotal<<=1;
    mode->CrtcHDisplay<<=1;
    mode->CrtcHSyncStart<<=1;
    mode->CrtcHSyncEnd<<=1;
    mode->CrtcHSkew<<=1;
  }    

  return TRUE;
}
