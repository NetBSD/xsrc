/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3v/s3v_misc.c,v 1.1.2.11 1999/07/30 11:21:40 hohndel Exp $ */

/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */

/* 
 * Various functions used in the virge driver. 
 * Right now, this only contains the PCI probing function.
 * 
 * Created 18/03/97 by Sebastien Marineau
 * Revision: 
 * [0.2] 08/02/98: Rewrite to use the VGA PCI information instead of re-probing
 *       the PCI bus.
 *
 * [0.1] 18/03/97: Added PCI probe function, taken from accel/s3_virge server.
 *       Not sure if the code used to adjust the PCI base address is 
 *       still needed for the ViRGE chipsets.
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

#include "regs3v.h"
#include "s3v_driver.h"

extern vgaPCIInformation *vgaPCIInfo;
extern SymTabRec s3vChipTable[];
extern S3VPRIV s3vPriv;


/*
 * s3vGetPCIInfo -- probe for PCI information
 */

S3PCIInformation *
s3vGetPCIInfo()
{
   static S3PCIInformation info = {0, };
   pciConfigPtr pcrp = NULL;
   Bool found = FALSE;
   int i = 0;


   if (vgaPCIInfo && vgaPCIInfo->AllCards) {
      while (pcrp = vgaPCIInfo->AllCards[i]) {
         if ((pcrp->_vendor == PCI_S3_VENDOR_ID) && 
             (pcrp->_command & PCI_CMD_IO_ENABLE) &&
	     (pcrp->_command & PCI_CMD_MEM_ENABLE)) {
	    int ChipId = pcrp->_device;
	    if (vga256InfoRec.chipID) {
	      ErrorF("%s %s: S3 chipset override, using chip_id = 0x%04x instead of 0x%04x\n",
		  XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.chipID, ChipId);
	      ChipId = vga256InfoRec.chipID;
	    }
	    found = TRUE;

	    switch (ChipId) {
	    case PCI_ViRGE:
	       info.ChipType = S3_ViRGE;
	       break;
	    case PCI_ViRGE_VX:
	       info.ChipType = S3_ViRGE_VX;
	       break;
	    case PCI_ViRGE_DXGX:
	       info.ChipType = S3_ViRGE_DXGX;
	       break;
	    case PCI_ViRGE_GX2:
	       info.ChipType = S3_ViRGE_GX2;
	       break;
	    case PCI_ViRGE_MX:
	       info.ChipType = S3_ViRGE_MX;
	       break;
	    case PCI_ViRGE_MXP:
	       info.ChipType = S3_ViRGE_MXP;
	       break;
           case PCI_TRIO_3D:
              info.ChipType = S3_TRIO_3D;
              break;
           case PCI_TRIO_3D_2X:
              info.ChipType = S3_TRIO_3D_2X;
              break;
#if 0 /* not yet */
           case PCI_CHIP_SAVAGE3D:
              info.ChipType = S3_SAVAGE_3D;
              break;
           case PCI_CHIP_SAVAGE3D_M:
              info.ChipType = S3_SAVAGE_3D_M;
              break;
#endif
	    default:
	       info.ChipType = S3_UNKNOWN;
	       info.DevID = pcrp->_device;
	       break;
	    }
	    info.ChipRev = pcrp->_rev_id;
	    info.MemBase = pcrp->_base0 & 0xFF800000;
	    break;
         }
      i++;
      }
   }
   else 
      return (FALSE);

   /* for new mmio we have to ensure that the PCI base address is
    * 64MB aligned and that there are no address collitions within 64MB.
    * S3 868/968 only pretend to need 32MB and thus fool
    * the BIOS PCI auto configuration :-(  */

   if (info.ChipType == S3_ViRGE) {
      unsigned long base0;
      char *probed;
      char map_64m[64];
      int j;

      if (vga256InfoRec.MemBase == 0) {
	 base0  = info.MemBase;
	 probed = XCONFIG_PROBED;
      }
      else {
	 base0  = vga256InfoRec.MemBase;
	 probed = XCONFIG_GIVEN;
      }

      /* map allocated 64MB blocks */
      for (j=0; j<64; j++) map_64m[j] = 0;
      map_64m[63] = 1;  /* don't use the last 64MB area */
      for (j=0; (pcrp = vgaPCIInfo->AllCards[j]); j++) {
	 if (i != j) {
	    map_64m[ (pcrp->_base0 >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base1 >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base2 >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base3 >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base4 >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base5 >> 26) & 0x3f] = 1;
	 }
      }

      /* check for 64MB alignment and free space */
      if ((base0 & 0x3ffffff) ||
	  map_64m[(base0 >> 26) & 0x3f] ||
	  map_64m[((base0+0x3ffffff) >> 26) & 0x3f]) {
	 for (j=63; j>=16 && map_64m[j]; j--);
	 info.MemBase = ((unsigned long)j) << 26;
	 ErrorF("%s %s: S3V: PCI base address not correctly aligned or address conflict\n",
		probed, vga256InfoRec.name);
	 ErrorF("\t\tbase address changed from 0x%08lx to 0x%08lx\n",
		base0, info.MemBase);
         xf86writepci(vga256InfoRec.scrnIndex, vgaPCIInfo->AllCards[i]->_bus, 
		    vgaPCIInfo->AllCards[i]->_cardnum,
		    vgaPCIInfo->AllCards[i]->_func, 
		    PCI_MAP_REG_START, ~0L,
		    info.MemBase | PCI_MAP_MEMORY | PCI_MAP_MEMORY_TYPE_32BIT);
      }
   }
   else {
	/* Don't do this check for other chipsets. */
   }

   if (found && xf86Verbose) {
      if (info.ChipType != S3_UNKNOWN) {
	 ErrorF("%s %s: S3V: %s rev %x, Linear FB @ 0x%08lx\n", XCONFIG_PROBED,
		vga256InfoRec.name,xf86TokenToString(s3vChipTable, info.ChipType), 
		info.ChipRev, info.MemBase);
      }
   }

   if (found)
      return &info;
   else
      return NULL;
}



