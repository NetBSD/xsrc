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

/* Hacked together from mga driver and 3.3.4 NVIDIA driver by Jarno Paananen
   <jpaana@s2.org> */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nv_include.h"

#include "nvreg.h"
#include "nvvga.h"

/*
 * Override VGA I/O routines.
 */
static void NVWriteCrtc(vgaHWPtr pVga, CARD8 index, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PCIO, pVga->IOBase + VGA_CRTC_INDEX_OFFSET, index);
    VGA_WR08(pNv->riva.PCIO, pVga->IOBase + VGA_CRTC_DATA_OFFSET,  value);
}
static CARD8 NVReadCrtc(vgaHWPtr pVga, CARD8 index)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PCIO, pVga->IOBase + VGA_CRTC_INDEX_OFFSET, index);
    return (VGA_RD08(pNv->riva.PCIO, pVga->IOBase + VGA_CRTC_DATA_OFFSET));
}
static void NVWriteGr(vgaHWPtr pVga, CARD8 index, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PVIO, VGA_GRAPH_INDEX, index);
    VGA_WR08(pNv->riva.PVIO, VGA_GRAPH_DATA,  value);
}
static CARD8 NVReadGr(vgaHWPtr pVga, CARD8 index)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PVIO, VGA_GRAPH_INDEX, index);
    return (VGA_RD08(pNv->riva.PVIO, VGA_GRAPH_DATA));
}
static void NVWriteSeq(vgaHWPtr pVga, CARD8 index, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PVIO, VGA_SEQ_INDEX, index);
    VGA_WR08(pNv->riva.PVIO, VGA_SEQ_DATA,  value);
}
static CARD8 NVReadSeq(vgaHWPtr pVga, CARD8 index)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PVIO, VGA_SEQ_INDEX, index);
    return (VGA_RD08(pNv->riva.PVIO, VGA_SEQ_DATA));
}
static void NVWriteAttr(vgaHWPtr pVga, CARD8 index, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    volatile CARD8 tmp;

    tmp = VGA_RD08(pNv->riva.PCIO, pVga->IOBase + VGA_IN_STAT_1_OFFSET);
    if (pVga->paletteEnabled)
        index &= ~0x20;
    else
        index |= 0x20;
    VGA_WR08(pNv->riva.PCIO, VGA_ATTR_INDEX,  index);
    VGA_WR08(pNv->riva.PCIO, VGA_ATTR_DATA_W, value);
}
static CARD8 NVReadAttr(vgaHWPtr pVga, CARD8 index)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    volatile CARD8 tmp;

    tmp = VGA_RD08(pNv->riva.PCIO, pVga->IOBase + VGA_IN_STAT_1_OFFSET);
    if (pVga->paletteEnabled)
        index &= ~0x20;
    else
        index |= 0x20;
    VGA_WR08(pNv->riva.PCIO, VGA_ATTR_INDEX, index);
    return (VGA_RD08(pNv->riva.PCIO, VGA_ATTR_DATA_R));
}
static void NVWriteMiscOut(vgaHWPtr pVga, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PVIO, VGA_MISC_OUT_W, value);
}
static CARD8 NVReadMiscOut(vgaHWPtr pVga)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    return (VGA_RD08(pNv->riva.PVIO, VGA_MISC_OUT_R));
}
static void NVEnablePalette(vgaHWPtr pVga)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    volatile CARD8 tmp;

    tmp = VGA_RD08(pNv->riva.PCIO, pVga->IOBase + VGA_IN_STAT_1_OFFSET);
    VGA_WR08(pNv->riva.PCIO, VGA_ATTR_INDEX, 0x00);
    pVga->paletteEnabled = TRUE;
}
static void NVDisablePalette(vgaHWPtr pVga)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    volatile CARD8 tmp;

    tmp = VGA_RD08(pNv->riva.PCIO, pVga->IOBase + VGA_IN_STAT_1_OFFSET);
    VGA_WR08(pNv->riva.PCIO, VGA_ATTR_INDEX, 0x20);
    pVga->paletteEnabled = FALSE;
}
static void NVWriteDacMask(vgaHWPtr pVga, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PDIO, VGA_DAC_MASK, value);
}
static CARD8 NVReadDacMask(vgaHWPtr pVga)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    return (VGA_RD08(pNv->riva.PDIO, VGA_DAC_MASK));
}
static void NVWriteDacReadAddr(vgaHWPtr pVga, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PDIO, VGA_DAC_READ_ADDR, value);
}
static void NVWriteDacWriteAddr(vgaHWPtr pVga, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PDIO, VGA_DAC_WRITE_ADDR, value);
}
static void NVWriteDacData(vgaHWPtr pVga, CARD8 value)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    VGA_WR08(pNv->riva.PDIO, VGA_DAC_DATA, value);
}
static CARD8 NVReadDacData(vgaHWPtr pVga)
{
    NVPtr pNv = (NVPtr)pVga->MMIOBase;
    return (VGA_RD08(pNv->riva.PDIO, VGA_DAC_DATA));
}


static void
NVCommonSetup(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    vgaHWPtr pVga = VGAHWPTR(pScrn);
    CARD32 regBase = pNv->IOAddress;
    int mmioFlags;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVCommonSetup\n"));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- Regbase %x\n", regBase));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- riva %x\n", &pNv->riva));

    pNv->PreInit = NVXRamdacInit;
    pNv->Save = NVXDACSave;
    pNv->Restore = NVXDACRestore;
    pNv->ModeInit = NVXDACInit;

    pNv->Dac.LoadPalette = NVXDACLoadPalette;

    /*
     * Override VGA I/O routines.
     */
    pVga->writeCrtc         = NVWriteCrtc;
    pVga->readCrtc          = NVReadCrtc;
    pVga->writeGr           = NVWriteGr;
    pVga->readGr            = NVReadGr;
    pVga->writeAttr         = NVWriteAttr;
    pVga->readAttr          = NVReadAttr;
    pVga->writeSeq          = NVWriteSeq;
    pVga->readSeq           = NVReadSeq;
    pVga->writeMiscOut      = NVWriteMiscOut;
    pVga->readMiscOut       = NVReadMiscOut;
    pVga->enablePalette     = NVEnablePalette;
    pVga->disablePalette    = NVDisablePalette;
    pVga->writeDacMask      = NVWriteDacMask;
    pVga->readDacMask       = NVReadDacMask;
    pVga->writeDacWriteAddr = NVWriteDacWriteAddr;
    pVga->writeDacReadAddr  = NVWriteDacReadAddr;
    pVga->writeDacData      = NVWriteDacData;
    pVga->readDacData       = NVReadDacData;
    /*
     * Note: There are different pointers to the CRTC/AR and GR/SEQ registers.
     * Bastardize the intended uses of these to make it work.
     */
    pVga->MMIOBase   = (CARD8 *)pNv;
    pVga->MMIOOffset = 0;

    /*
     * No IRQ in use.
     */
    pNv->riva.EnableIRQ = 0;
    /*
     * Map remaining registers. This MUST be done in the OS specific driver code.
     */
    pNv->riva.IO      = VGA_IOBASE_COLOR;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- IO %x\n", pNv->riva.IO));

    mmioFlags = VIDMEM_MMIO | VIDMEM_READSIDEEFFECT;

    pNv->riva.PRAMDAC = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00680000, 0x00003000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PRAMDAC %x\n", pNv->riva.PRAMDAC));
    pNv->riva.PFB     = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00100000, 0x00001000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PFB %x\n", pNv->riva.PFB));
    pNv->riva.PFIFO   = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00002000, 0x00002000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PFIFO %x\n", pNv->riva.PFIFO));
    pNv->riva.PGRAPH  = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00400000, 0x00002000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PGRAPH %x\n", pNv->riva.PGRAPH));
    pNv->riva.PEXTDEV = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00101000, 0x00001000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PEXTDEV %x\n", pNv->riva.PEXTDEV));
    pNv->riva.PTIMER  = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00009000, 0x00001000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PTIMER %x\n", pNv->riva.PTIMER));
    pNv->riva.PMC     = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00000000, 0x00009000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PMC %x\n", pNv->riva.PMC));
    pNv->riva.FIFO    = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                      regBase+0x00800000, 0x00010000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- FIFO %x\n", pNv->riva.FIFO));

    /*
     * These registers are read/write as 8 bit values.  Probably have to map
     * sparse on alpha.
     */
    pNv->riva.PCIO = (U008 *)xf86MapPciMem(pScrn->scrnIndex, mmioFlags,
                                           pNv->PciTag, regBase+0x00601000,
                                           0x00001000);
    pNv->riva.PDIO = (U008 *)xf86MapPciMem(pScrn->scrnIndex, mmioFlags,
                                           pNv->PciTag, regBase+0x00681000,
                                           0x00001000);
    pNv->riva.PVIO = (U008 *)xf86MapPciMem(pScrn->scrnIndex, mmioFlags,
                                           pNv->PciTag, regBase+0x000C0000,
                                           0x00001000);
    
    RivaGetConfig(pNv);

    pNv->Dac.maxPixelClock = pNv->riva.MaxVClockFreqKHz;

    vgaHWUnlock(VGAHWPTR(pScrn));
    pNv->riva.LockUnlock(&pNv->riva, 0);
}

void
NVX1Setup(ScrnInfoPtr pScrn)
{
}

void
NVX3Setup(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    CARD32 frameBase = pNv->FbAddress;
    int mmioFlags;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVX3Setup\n"));

    /*
     * Record chip architecture based in PCI probe.
     */
    pNv->riva.Architecture = 3;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    mmioFlags = VIDMEM_MMIO | VIDMEM_READSIDEEFFECT;
    pNv->riva.PRAMIN = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     frameBase+0x00C00000, 0x00008000);

    NVCommonSetup(pScrn);
}

void
NVX4Setup(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    CARD32 regBase = pNv->IOAddress;
    int mmioFlags;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVX4Setup\n"));

    pNv->riva.Architecture = 4;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    mmioFlags = VIDMEM_MMIO | VIDMEM_READSIDEEFFECT;
    pNv->riva.PRAMIN = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00710000, 0x00010000);
    pNv->riva.PCRTC  = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00600000, 0x00001000);

    NVCommonSetup(pScrn);
}
void
NVX10Setup(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    CARD32 regBase = pNv->IOAddress;
    int mmioFlags;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVX10Setup\n"));
    pNv->riva.Architecture = 0x10;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    mmioFlags = VIDMEM_MMIO | VIDMEM_READSIDEEFFECT;
    pNv->riva.PRAMIN = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00710000, 0x00010000);
    pNv->riva.PCRTC  = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00600000, 0x00001000);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "- PCRTC %x\n", pNv->riva.PCRTC));
    NVCommonSetup(pScrn);
}

void
NVX20Setup(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    CARD32 regBase = pNv->IOAddress;
    int mmioFlags;
    int flags;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVX20Setup\n"));

    pNv->riva.Architecture = 0x20;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS sp
ecific driver code.
     */
    mmioFlags = VIDMEM_MMIO | VIDMEM_READSIDEEFFECT;
    pNv->riva.PRAMIN = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00710000, 0x00010000);
    pNv->riva.PCRTC  = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, pNv->PciTag,
                                     regBase+0x00600000, 0x00001000);

    NVCommonSetup(pScrn);
    
    if (pNv->Chipset == NV_CHIP_XBOX)
    {
	pNv->riva.PMC[0x200/4] |= 0x10001100;
	flags = pNv->riva.PMC[0x200/4];
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "EnableFlags: 0x%0x\n",flags));
    }
}

