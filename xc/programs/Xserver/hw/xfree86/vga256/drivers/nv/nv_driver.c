/* $XConsortium: nv_driver.c /main/3 1996/10/28 05:13:37 kaleb $ */
/*
 * Copyright 1996-1997  David J. McKay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv_driver.c,v 3.5.2.13 2000/11/01 01:52:46 dawes Exp $ */

#include <math.h>
#include <stdlib.h>


#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "vgaPCI.h"
/*
 * If the driver makes use of XF86Config 'Option' flags, the following will be
 * required
 */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"


#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/* Little hack to declare all the base pointers */
#define extern
#include "nvreg.h"
#undef extern

#include "nvvga.h"

static char *NVIdent(int n);
static Bool NVProbe(void);

vgaVideoChipRec NV =
{
  NVProbe,
  NVIdent,
  NULL, /* EnterLeave */
  NULL, /* Init */
  NULL, /* ValidMode */
  NULL, /* Save */
  NULL, /* Restore */
  NULL, /* Adjust */
  NULL, /* SaveScreen */
  NULL, /* GetMode */
  NULL, /* FbInit */
  (void (*)(int))NoopDDA, /* We always assume linear memory */
  (void (*)(int))NoopDDA,
  (void (*)(int))NoopDDA,
  0x10000,/* This is the size of the mapped memory window, usually 64k    */
  0x10000,/* This is the size of a video memory bank for this chipset     */
  16,     /* Number of bits to shift addressto determine the bank number  */
  0xFFFF, /* Bitmask used to determine the address within a specific bank */
  0x00000, 0x10000,   /* Bottom and top addresses for reads inside a bank */
  0x00000, 0x10000,   /* Same for writes */
  FALSE, /* True if chipset seperate read and write bank registers */
  VGA_NO_DIVIDE_VERT,  /* VGA_DIVIDE_VERT if vertical timing numbers
                          to be divided by two for interlaced modes */
  {0,},  /* Option flags */
  8,     /* Multiple to which the virtual width rounded */
  TRUE,  /* Support linear-mapped frame buffer */
  0,     /* Physical base address of the linear-mapped frame buffer */
  0  ,   /* Size  of the linear-mapped frame buffer */
  TRUE,  /* 16 bpp */
  FALSE, /* 24 bpp */
  FALSE, /* 32 bpp */
  NULL,  /* Pointer to a list of builtin driver modes */
  1,      /* Scale factor used to scale the raw clocks to pixel clocks */
  1
};

static NVChipType chipis=NV1;

NVChipType GetChipType(void)
{
  return chipis;
}

typedef Bool (*NVProbeFuncType)(vgaVideoChipRec *rec,void *base0,void *base1);

Bool NV1Probe(vgaVideoChipRec *rec,void *base0,void *base1);
Bool NV3Probe(vgaVideoChipRec *rec,void *base0,void *base1);
Bool NV4Probe(vgaVideoChipRec *rec,void *base0,void *base1);
Bool NV10Probe(vgaVideoChipRec *rec,void *base0,void *base1);

static NVProbeFuncType NVProbeFuncList[NumNVChips]= {
  NV1Probe,
  NV3Probe,
  NV4Probe,
  NV10Probe
};


typedef struct {
  char *name;
  NVChipType type;
  int vendor;
  int device;
}NVProbeInfo;

static NVProbeInfo probeList[]={
  { "NV1",NV1,PCI_VENDOR_NVIDIA,PCI_CHIP_DAC64},
  { "STG2000",NV1,PCI_VENDOR_SGS,PCI_CHIP_STG1764},
  { "RIVA 128",NV3,PCI_VENDOR_NVIDIA_SGS,PCI_CHIP_RIVA128},
  { "RIVA TNT",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_TNT},
  { "RIVA TNT2",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_TNT2},
  { "RIVA ULTRA TNT2",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_UTNT2},
  { "RIVA VANTA",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_VTNT2},
  { "RIVA ULTRA VANTA",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_UVTNT2},
  { "RIVA INTEGRATED",NV4,PCI_VENDOR_NVIDIA,PCI_CHIP_ITNT2},
  { "GeForce 256",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE256},
  { "GeForce DDR",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCEDDR},
  { "Quadro",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_QUADRO},
  { "GeForce2 GTS",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE2GTS},
  { "GeForce2 GTS (rev1)",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE2GTS_1},
  { "GeForce2 Ultra",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE2ULTRA},
  { "Quadro 2 Pro",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_QUADRO2PRO},
  { "GeForce2 MX",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE2MX},
  { "GeForce2 MX DDR",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_GEFORCE2MXDDR},
  { "Quadro 2 MXR",NV10,PCI_VENDOR_NVIDIA,PCI_CHIP_QUADRO2MXR}
};


#define NUM_PROBE_ENTRIES (sizeof(probeList)/sizeof(NVProbeInfo))

static char *NVIdent(int n)
{
  if( (n<0) || (n>=NUM_PROBE_ENTRIES)) return NULL;

  return probeList[n].name;
}


static Bool NVProbe(void)
{
  int i;
  void *base0=NULL,*base1=NULL;
  Bool ret;
  int idx=0,found=0;
  pciConfigPtr pcrp,*pciList;
  int noaccelSet;

  /* first things first; if a chipset is given, then we check if it is
     one we support, otherwise we silently go away... */
  if (vga256InfoRec.chipset) {
    char *chipset;
    for (i = 0; (chipset = NVIdent(i)); i++) {
      if (!StrCaseCmp(vga256InfoRec.chipset, chipset))
        break;
    }
    if (!chipset)
      return FALSE;
  }
  if (vgaPCIInfo && vgaPCIInfo->AllCards) {
    pciList=vgaPCIInfo->AllCards;
    while((pcrp=pciList[idx++]) && (!found)) {
      for(i=0;i<NUM_PROBE_ENTRIES && !found;i++) {
        if((pcrp->_vendor==probeList[i].vendor) &&
           (pcrp->_command & PCI_CMD_IO_ENABLE) &&
           (pcrp->_command & PCI_CMD_MEM_ENABLE) &&
           ( ((pcrp->_device & 0xFFFB)==probeList[i].device) ||
             (vga256InfoRec.chipset &&
             !StrCaseCmp(vga256InfoRec.chipset,probeList[i].name)))) {
          base0=(void*) (pcrp->_base0 & 0xFF800000);
          base1=(void*) (pcrp->_base1 & 0xFF800000);
          chipis=probeList[i].type;
          vga256InfoRec.chipset=probeList[i].name;
          found=1;
        }
      }
    }
  }
  if(!found) return FALSE;

  /* Now force programmable clock */
  OFLG_SET(CLOCK_OPTION_PROGRAMABLE,&vga256InfoRec.clockOptions);

  vga256InfoRec.clocks = 0;
#ifdef XFreeXDGA
  vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
  vga256InfoRec.bankedMono = FALSE;


  return NVProbeFuncList[GetChipType()](&NV,base0,base1);
}

