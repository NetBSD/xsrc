/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/mga/mga_warp.c,v 1.5 2000/11/02 19:10:53 dawes Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "mga_bios.h"
#include "mga_reg.h"
#include "mga.h"
#include "mga_macros.h"
#include "mgareg_flags.h"
#include "mga_ucode.h"
#include "mga_dri.h"

Bool mgaConfigureWarp(ScrnInfoPtr pScrn)
{
   MGAPtr pMga = MGAPTR(pScrn);
   int wmisc;

   CHECK_DMA_QUIESCENT(pMga, pScrn);
   WAITFIFO(3);

   switch(pMga->Chipset) {
    case PCI_CHIP_MGAG400:
      OUTREG(MGAREG_WIADDR2, WIA_wmode_suspend);
      OUTREG(MGAREG_WGETMSB, 0x00000E00);
      OUTREG(MGAREG_WVRTXSZ, 0x00001807);
      OUTREG(MGAREG_WACCEPTSEQ, 0x18000000);
      break;
    case PCI_CHIP_MGAG200:
    case PCI_CHIP_MGAG200_PCI:
      OUTREG(MGAREG_WIADDR, WIA_wmode_suspend);
      OUTREG(MGAREG_WGETMSB, 0x1606);
      OUTREG(MGAREG_WVRTXSZ, 7); /* may be set on something else later on */
      break;
    default:
      return FALSE;
   }

   WAITFIFO(1);
   OUTREG(MGAREG_WMISC, WM_wucodecache_enable |
	  		WM_wmaster_enable |
	  		WM_wcacheflush_enable);
   wmisc = INREG(MGAREG_WMISC);
   if (wmisc != (WM_wucodecache_enable | WM_wmaster_enable)) {
      FatalError("[mga] WARP engine wrongly configured (%d != %d)."
		 "  Switch off your PC and try again.\n", wmisc,
		 WM_wmaster_enable | WM_wucodecache_enable);
   }
   return TRUE;
}

static unsigned int mgaG400GetMicrocodeSize(MGAPtr pMGA)
{
#define CODESIZE(which) ((sizeof(which)/256 + 1)*256)
   unsigned int microcode_size = 0;

   microcode_size = CODESIZE(WARP_G400_t2gz) + CODESIZE(WARP_G400_t2gza) +
     CODESIZE(WARP_G400_t2gzaf) + CODESIZE(WARP_G400_t2gzf) +
     CODESIZE(WARP_G400_t2gzs) + CODESIZE(WARP_G400_t2gzsa) +
     CODESIZE(WARP_G400_t2gzsaf) + CODESIZE(WARP_G400_t2gzsf) +
     CODESIZE(WARP_G400_tgz) + CODESIZE(WARP_G400_tgza) +
     CODESIZE(WARP_G400_tgzaf) + CODESIZE(WARP_G400_tgzf) +
     CODESIZE(WARP_G400_tgzs) + CODESIZE(WARP_G400_tgzsa) +
     CODESIZE(WARP_G400_tgzsaf) + CODESIZE(WARP_G400_tgzsf);
   microcode_size = ((microcode_size + 4096 - 1) / 4096) * 4096;
   return(microcode_size);
#undef CODESIZE
}

static unsigned int mgaG200GetMicrocodeSize(MGAPtr pMGA)
{
#define CODESIZE(which) ((sizeof(which)/256 + 1)*256)
   unsigned int microcode_size;

   microcode_size = CODESIZE(WARP_G200_tgz) +
     CODESIZE(WARP_G200_tgza) + CODESIZE(WARP_G200_tgzaf) +
     CODESIZE(WARP_G200_tgzf) + CODESIZE(WARP_G200_tgzs) +
     CODESIZE(WARP_G200_tgzsa) + CODESIZE(WARP_G200_tgzsaf) +
     CODESIZE(WARP_G200_tgzsf);
   microcode_size = ((microcode_size + 4096 - 1) / 4096) * 4096;
   return(microcode_size);
#undef CODESIZE
}

static unsigned int mgaG400InstallMicrocode(MGAPtr pMGA, int agp_offset)
{
#define mgaWarpInstallCode(which, where) {\
      pMGADRIServer->WarpIndex[where].installed = 1; \
      pMGADRIServer->WarpIndex[where].phys_addr = pcbase; \
      pMGADRIServer->WarpIndex[where].size = sizeof(WARP_G400_ ## which); \
      memcpy(vcbase, WARP_G400_ ## which, sizeof(WARP_G400_ ## which)); \
      pcbase += (sizeof(WARP_G400_ ## which) / 256 + 1) * 256; \
      vcbase += (sizeof(WARP_G400_ ## which) / 256 + 1) * 256; \
    }
   MGADRIServerPrivatePtr pMGADRIServer = pMGA->DRIServerInfo;
   CARD8 *vcbase = pMGADRIServer->agp_map + agp_offset;
   unsigned long pcbase = (unsigned long)pMGADRIServer->agpBase + agp_offset;
   unsigned int microcode_size = 0;

   memset(pMGADRIServer->WarpIndex, 0,
	  sizeof(drmMgaWarpIndex) * MGA_MAX_WARP_PIPES);

   microcode_size = mgaG400GetMicrocodeSize(pMGA);
   mgaWarpInstallCode(tgz,     MGA_WARP_TGZ);
   mgaWarpInstallCode(tgzf,    MGA_WARP_TGZF);
   mgaWarpInstallCode(tgza,    MGA_WARP_TGZA);
   mgaWarpInstallCode(tgzaf,   MGA_WARP_TGZAF);
   mgaWarpInstallCode(tgzs,    MGA_WARP_TGZS);
   mgaWarpInstallCode(tgzsf,   MGA_WARP_TGZSF);
   mgaWarpInstallCode(tgzsa,   MGA_WARP_TGZSA);
   mgaWarpInstallCode(tgzsaf,  MGA_WARP_TGZSAF);
   mgaWarpInstallCode(t2gz,    MGA_WARP_T2GZ);
   mgaWarpInstallCode(t2gzf,   MGA_WARP_T2GZF);
   mgaWarpInstallCode(t2gza,   MGA_WARP_T2GZA);
   mgaWarpInstallCode(t2gzaf,  MGA_WARP_T2GZAF);
   mgaWarpInstallCode(t2gzs,   MGA_WARP_T2GZS);
   mgaWarpInstallCode(t2gzsf,  MGA_WARP_T2GZSF);
   mgaWarpInstallCode(t2gzsa,  MGA_WARP_T2GZSA);
   mgaWarpInstallCode(t2gzsaf, MGA_WARP_T2GZSAF);
#undef mgaWarpInstallCode
   return microcode_size;
}

static unsigned int mgaG200InstallMicrocode(MGAPtr pMGA, int agp_offset)
{
#define mgaWarpInstallCode(which,where) {\
      pMGADRIServer->WarpIndex[where].installed = 1; \
      pMGADRIServer->WarpIndex[where].phys_addr  = pcbase; \
      pMGADRIServer->WarpIndex[where].size  = sizeof(WARP_G200_ ## which); \
      memcpy(vcbase, WARP_G200_ ## which, sizeof(WARP_G200_ ## which)); \
      pcbase += (sizeof(WARP_G200_ ## which) / 256 + 1) * 256; \
      vcbase += (sizeof(WARP_G200_ ## which) / 256 + 1) * 256; \
    }
   MGADRIServerPrivatePtr pMGADRIServer = pMGA->DRIServerInfo;
   CARD8 *vcbase = pMGADRIServer->agp_map + agp_offset;
   unsigned long pcbase = (unsigned long)pMGADRIServer->agpBase + agp_offset;
   unsigned int microcode_size = 0;

   memset(pMGADRIServer->WarpIndex, 0,
	  sizeof(drmMgaWarpIndex) * MGA_MAX_WARP_PIPES);

   microcode_size = mgaG400GetMicrocodeSize(pMGA);
   mgaWarpInstallCode(tgz,    MGA_WARP_TGZ);
   mgaWarpInstallCode(tgza,   MGA_WARP_TGZA);
   mgaWarpInstallCode(tgzaf,  MGA_WARP_TGZAF);
   mgaWarpInstallCode(tgzf,   MGA_WARP_TGZF);
   mgaWarpInstallCode(tgzs,   MGA_WARP_TGZS);
   mgaWarpInstallCode(tgzsa,  MGA_WARP_TGZSA);
   mgaWarpInstallCode(tgzsaf, MGA_WARP_TGZSAF);
   mgaWarpInstallCode(tgzsf,  MGA_WARP_TGZSF);
#undef mgaWarpInstallCode
   return microcode_size;
}

unsigned int mgaGetMicrocodeSize(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMGA = MGAPTR(pScrn);
   int retval;

   switch(pMGA->Chipset) {
    case PCI_CHIP_MGAG400:
      retval = mgaG400GetMicrocodeSize(pMGA);
      break;
    case PCI_CHIP_MGAG200:
    case PCI_CHIP_MGAG200_PCI:
      retval = mgaG200GetMicrocodeSize(pMGA);
      break;
    default:
      retval = 0;
   }
   return(retval);
}

unsigned int mgaInstallMicrocode(ScreenPtr pScreen, int agp_offset)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMGA = MGAPTR(pScrn);
   int retval;

   switch(pMGA->Chipset) {
    case PCI_CHIP_MGAG400:
      retval = mgaG400InstallMicrocode(pMGA, agp_offset);
      break;
    case PCI_CHIP_MGAG200:
    case PCI_CHIP_MGAG200_PCI:
      retval = mgaG200InstallMicrocode(pMGA, agp_offset);
      break;
    default:
      retval = 0;
   }

   if (retval)
      xf86DrvMsg(pScrn->scrnIndex, X_INFO, "WARP Microcode Loaded\n");
   return retval;
}
