/*
 * Basic hardware and memory detection
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:  	Thomas Winischhofer <thomas@winischhofer.net>
 *
 * Ideas and methods for old series based on code by Can-Ru Yeou, XGI Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86.h"
#include "fb.h"
#include "xf86_OSproc.h"
#include "xorgVersion.h"

#include "xf86cmap.h"

#include "xgi.h"
#include "xgi_regs.h"
#include "xgi_dac.h"
#include "xgi_driver.h"
/* #include "valid_mode.h" */

#define _XF86DGA_SERVER_
#include <X11/extensions/xf86dgaproto.h>

#include "globals.h"
#ifdef HAVE_XEXTPROTO_71
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#endif


#include "vb_def.h"
extern  int  FbDevExist;

static Bool bAccessVGAPCIInfo(PXGI_HW_DEVICE_INFO pHwDevInfo, ULONG ulOffset,
    ULONG ulSet, CARD32 *pulValue);
static Bool bAccessNBridgePCIInfo(PXGI_HW_DEVICE_INFO pHwDevInfo,
    ULONG ulOffset, ULONG ulSet, ULONG *pulValue);
static Bool XGI_IsXG21(ScrnInfoPtr pScrn);

static void XGI_InitHwDevInfo(ScrnInfoPtr pScrn);

/* Jong 10/16/2007; merge code */
static void
xgiXG2X_Setup(ScrnInfoPtr pScrn)
{

/*********************************************************************
 * Setup
 * Decide the following item of execution data:
 *
 * pXGI->BusWidth
 * pXGI->videoRam (with KB unit)
 * pXGI->CursorOffset (with Byte Unit)
 * pXGI->cmdQueueSize (with Byte Unit)
 * pXGI->cmdQueueSizeMask (with Byte Unit)
 * pXGI->cmdQueueOffset (with Byte Unit)
 * pXGI->cmdQueueLen = 0 ; // init value
 * pXGI->cmdQueueLenMin = 0x200 ; // init value
 * pXGI->cmdQueueLenMax = pXGI->cmdQueueSize -  pXGI->cmdQueueLenMin ;
 *********************************************************************/

    XGIPtr        pXGI = XGIPTR(pScrn);
    unsigned int  ulMemConfig = 0;
    unsigned long ulMemSize   = 0;
    unsigned long ulDramType  = 0;
    char *dramTypeStr ;
    unsigned long ulTemp ;

    /* DumpDDIName("xgiXG2X_Setup()\n") ; */

    inXGIIDXREG(XGICR, 0x48, ulTemp) ;
    if(ulTemp & (1<<0)) /* GPIOH, CR48 D[0] read */
    {
        dramTypeStr = "DDRII DRAM" ;
    }
    else
    {
        dramTypeStr = "DDR DRAM" ;
    }


    pXGI->MemClock = XG40Mclk(pXGI);

    /*********************************************************************************************************
     * SR14 DRAM Size Register
     *     Default value: XXh
     *     D[7:4]    Memory size per channel {BChMemSize}
     *         0011: 8 MB
     *         0100: 16 MB
     *         0101: 32 MB
     *         0110: 64 MB
     *         0111: 128 MB
     *         1000: 256MB
     *        others: reserved
     *     D[3:2]    Number of  dram channels [1:0] {BChNum}
     *         00: uni-channel
     *         01: reserved
     *         10: dual-channel.
     *         11: quad-channel
     *     D1  Data width per channel selection {BDataWidth}
     *         0: 32-bits
     *         1: 64-bits
     *     D0  Dram channel mapping {BReverseChMapping}
     *         0: Normal mapping
     *         1: Reversal mapping
     *             Dual-channel: Logical channel A/B to physical channel B/A
     *             Quad-channel: Logical  channel A/B/C/D to physical channel C/D/A/B
     *
     *********************************************************************************************************/

    outXGIIDXREG(XGISR, 0x5, 0x86) ;
    inXGIIDXREG(XGISR, 0x14, ulMemConfig) ;
    inXGIIDXREG(XGISR, 0x3A, ulDramType) ;

    PDEBUG(ErrorF("xg40_Setup(): ulMemConfig = %02X\n",ulMemConfig)) ;
    PDEBUG(ErrorF("xg40_Setup(): ulDramType = %02X\n",ulDramType)) ;

    pXGI->BusWidth = (ulMemConfig & (1<<1) )?64:32 ;

    switch(ulMemConfig>>4)
    {
    case 8:
        ulMemSize = 256*1024 ;
        break ;
    case 7:
        ulMemSize = 128*1024 ;
        break ;
    case 6:
        ulMemSize = 64*1024 ;
        break ;
    case 5:
        ulMemSize = 32*1024 ;
        break ;
    case 4:
        ulMemSize = 16*1024 ;
        break ;
    case 3:
        ulMemSize = 8*1024 ;
        break ;
    default:
        ulMemSize = 8*1024 ;
    }

    if (pXGI->Chipset == PCI_CHIP_XGIXG40) {
	const unsigned revision =
#ifdef XSERVER_LIBPCIACCESS
	    pXGI->PciInfo->revision
#else
	    pciReadLong(pXGI->PciTag, 0x08) & 0x0FF
#endif
	    ;

	/* Revision 2 cards encode the memory config bits slightly differently
	 * from revision 1 cards.
	 */
        if (revision == 2) {
            switch((ulMemConfig>>2)&0x1)
            {
            case 0:
                /* Uni channel */
                ulMemSize *= 1 ;
       	        break ;
            case 1:
                /* Dual channel */
                ulMemSize *= 2 ;
    	        break ;
            }
        }
        else
        {
            switch((ulMemConfig>>2)&0x3)
            {
            case 2:
                /* Dual channel */
                ulMemSize *= 2 ;
        	break ;
            case 3:
                /* Quad channel */
                ulMemSize *= 4 ;
    	        break ;
           }
        }
    }

    pScrn->videoRam = ulMemSize ;

    /*********************************************************************************************************
     * SR15 DRAM Address Mapping Register
     * Default value: XXh
     *     D7  Channel  interleaving configuration { BChConfig }
     *         0: Divide the whole memory  into 2/4 equal-sized regions , each mapped to one channel
     *         1: Divide the whole memory into 2 regions according to BTilingSize[1:0] . The low-address region
     *         will be channel-interleaved as per BFineGranSize; the high-address region will be channel-
     *         interleaved  as per BCoarseGranSize[1:0]
     *     D[6:5]    Memory size of tile-mapped region {BTilingSize}
     *         00: 4 MB
     *         01: 8 MB
     *         10: 16 MB
     *         11: 32 MB
     *         The following bits are effective only when D7=1
     *     D4  Channel-interleaving granularity for tile-mapped region {BFineGranSize}
     *         0:  64 B
     *         1:  128 B
     *     D[3:2] Channel-interleaving granularity for linearly mapped region {BCoarseGranSize}
     *         00: 1KB
     *         01: 2KB
     *         10: 4KB
     *         11: 1MB
     *     D[1:0] reserved
     *********************************************************************************************************/

    /* Accelerator parameter Initialization */
    if(( pXGI->Chipset == PCI_CHIP_XGIXG20 )||( pXGI->Chipset == PCI_CHIP_XGIXG21 )||( pXGI->Chipset == PCI_CHIP_XGIXG27 ))
    {
        pXGI->cmdQueueSize = VOLARI_CQSIZEXG20;
    /* XgiMode = XG20_Mode ; */
        PDEBUG(ErrorF(" ---XG20_Mode \n"));
    }


    pXGI->cmdQueueSizeMask = pXGI->cmdQueueSize - 1 ;
    pXGI->pCQ_shareWritePort = &(pXGI->cmdQueue_shareWP_only2D);


    /*
     If FbDevExist, XFree86 driver use the 8MB only. The rest
     frame buffer is used by other AP.
     */

    if( FbDevExist && (pXGI->Chipset != PCI_CHIP_XGIXG20 ) && (pXGI->Chipset != PCI_CHIP_XGIXG21 ) && (pXGI->Chipset != PCI_CHIP_XGIXG27 ) )
    {
        if( pScrn->videoRam < 8*1024 )
        {
            pXGI->cmdQueueOffset = 4*1024*1024 - pXGI->cmdQueueSize ;
        }
        else if( pScrn->videoRam < 16*1024 )
        {
            pXGI->cmdQueueOffset = 8*1024*1024 - pXGI->cmdQueueSize ;
        }
        else
        {
            pXGI->cmdQueueOffset = 13*1024*1024 - pXGI->cmdQueueSize ;
        }
    }
    else
    {
        pXGI->cmdQueueOffset = (pScrn->videoRam)*1024 - pXGI->cmdQueueSize ;
    }

    pXGI->CursorOffset = pXGI->cmdQueueOffset - 64*1024 ;
    PDEBUG4(ErrorF("pScrn->videoRam = %08lX pXGI->cmdQueueSize = %08lX\n",
			pScrn->videoRam, pXGI->cmdQueueSize)) ;
    PDEBUG4(ErrorF("pXGI->cmdQueueOffset = %08lX pXGI->CursorOffset = %08lX\n",
		pXGI->cmdQueueOffset, pXGI->CursorOffset)) ;

    pXGI->cmdQueueLen = 0 ;
    pXGI->cmdQueueLenMin = 0x200 ;
    pXGI->cmdQueueLenMax = pXGI->cmdQueueSize -  pXGI->cmdQueueLenMin ;

    /*****************************************************************
     * Dual Chip support put here                                    *
     *****************************************************************/
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected DRAM type : %s\n", dramTypeStr);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected memory clock : %3.3fMHz\n",
            pXGI->MemClock/1000.0);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected VRAM bus width is %d\n", pXGI->BusWidth);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected Cmd Queue size is %d KB\n", pXGI->cmdQueueSize / 1024);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected Cmd Queue Offset is %d\n", pXGI->cmdQueueOffset ) ;
    XGI_InitHwDevInfo(pScrn);
}

static void
xgiXG40_Setup(ScrnInfoPtr pScrn)
{
    static const char *const dramChannelStr[5] = {
	"<invalid>", "Single", "Dual", "<invalid>", "Quad"
    };

    static const char *const dramTypeStr[4] = {
	"DDR SDRAM",
	"DDR2 SDRAM",
	"DDR2x SDRAM",
	""
    };

/*********************************************************************
 * Setup
 * Decide the following item of execution data:
 *
 * pXGI->BusWidth
 * pXGI->videoRam (with KB unit)
 * pXGI->CursorOffset (with Byte Unit)
 * pXGI->cmdQueueSize (with Byte Unit)
 * pXGI->cmdQueueSizeMask (with Byte Unit)
 * pXGI->cmdQueueOffset (with Byte Unit)
 * pXGI->cmdQueueLen = 0 ; // init value
 * pXGI->cmdQueueLenMin = 0x200 ; // init value
 * pXGI->cmdQueueLenMax = pXGI->cmdQueueSize -  pXGI->cmdQueueLenMin ;
 *********************************************************************/

    XGIPtr        pXGI = XGIPTR(pScrn);
    unsigned int  ulMemConfig = 0;
    unsigned mem_per_channel;
    unsigned mem_channels = 1;
    unsigned long ulDramType  = 0;

    PDEBUG4(ErrorF("xgiXG40_Setup()\n")) ;

    pXGI->MemClock = XG40Mclk(pXGI);

    /* SR14 DRAM Size Register
     *     Default value: XXh
     *     D[7:4]    Memory size per channel {BChMemSize}
     *         0011: 8 MB
     *         0100: 16 MB
     *         0101: 32 MB
     *         0110: 64 MB
     *         0111: 128 MB
     *         1000: 256MB
     *        others: reserved
     *     D[3:2]    Number of  dram channels [1:0] {BChNum}
     *         00: uni-channel
     *         01: reserved
     *         10: dual-channel
     *         11: quad-channel
     *     D1  Data width per channel selection {BDataWidth}
     *         0: 32-bits
     *         1: 64-bits
     *     D0  Dram channel mapping {BReverseChMapping}
     *         0: Normal mapping
     *         1: Reversal mapping
     *             Dual-channel: Logical channel A/B to physical channel B/A
     *             Quad-channel: Logical channel A/B/C/D to physical channel
     *                           C/D/A/B
     */

    outXGIIDXREG(XGISR, 0x5, 0x86) ;
    inXGIIDXREG(XGISR, 0x14, ulMemConfig) ;

    /* FIXME: Is this correct?  The SiS driver detects this differently
     * FIXME: for XG20.
     */
    inXGIIDXREG(XGISR, 0x3A, ulDramType) ;

    PDEBUG(ErrorF("xg40_Setup(): ulMemConfig = %02X\n",ulMemConfig)) ;
    PDEBUG(ErrorF("xg40_Setup(): ulDramType = %02X\n",ulDramType)) ;

    /* FIXME: Is this correct?  The SiS driver detects this differently
     * FIXME: for XG20.
     */
    pXGI->BusWidth = (ulMemConfig & 0x02) ? 64 : 32;

    mem_per_channel = ((ulMemConfig >> 4) >= 3)
        ? (1 << (ulMemConfig >> 4)) * 1024
        : 8 * 1024;


    /* All XG20 family chips are single channel, so only test the channel
     * count field on XG40 family chips.
     */
    if (pXGI->Chipset == PCI_CHIP_XGIXG40) {
        /* Check the PCI revision field.  For whatever reason, rev. 2 XG40
         * chips encode the DRAM channel count differently than other
         * revisions.
         */
        if (pXGI->ChipRev == 2) {
            switch ((ulMemConfig >> 2) & 0x1) {
            case 1:
                /* Dual channel */
                mem_channels = 2;
                break ;
            }
        }
        else {
            switch ((ulMemConfig >> 2) & 0x3) {
            case 2:
                /* Dual channel */
                mem_channels = 2;
                break ;
            case 3:
                /* Quad channel */
                mem_channels = 4;
                break ;
            }
        }
    }

    pScrn->videoRam = mem_per_channel * mem_channels;

    /* SR15 DRAM Address Mapping Register
     * Default value: XXh
     *     D7  Channel  interleaving configuration { BChConfig }
     *         0: Divide the whole memory  into 2/4 equal-sized regions , each mapped to one channel
     *         1: Divide the whole memory into 2 regions according to BTilingSize[1:0] . The low-address region
     *         will be channel-interleaved as per BFineGranSize; the high-address region will be channel-
     *         interleaved  as per BCoarseGranSize[1:0]
     *     D[6:5]    Memory size of tile-mapped region {BTilingSize}
     *         00: 4 MB
     *         01: 8 MB
     *         10: 16 MB
     *         11: 32 MB
     *         The following bits are effective only when D7=1
     *     D4  Channel-interleaving granularity for tile-mapped region {BFineGranSize}
     *         0:  64 B
     *         1:  128 B
     *     D[3:2] Channel-interleaving granularity for linearly mapped region {BCoarseGranSize}
     *         00: 1KB
     *         01: 2KB
     *         10: 4KB
     *         11: 1MB
     *     D[1:0] reserved
     */

    /* Accelerator parameter Initialization */
    
    pXGI->cmdQueueSize = ((pXGI->Chipset == PCI_CHIP_XGIXG20)||(pXGI->Chipset == PCI_CHIP_XGIXG21||(pXGI->Chipset == PCI_CHIP_XGIXG27)))
	? VOLARI_CQSIZEXG20 : VOLARI_CQSIZE;
    pXGI->cmdQueueSizeMask = pXGI->cmdQueueSize - 1 ;
    pXGI->pCQ_shareWritePort = &(pXGI->cmdQueue_shareWP_only2D);


    /* If FbDevExist, X.org driver uses 8MB only. The rest of the framebuffer
     * is used by the fbdev driver.
     */
    if (FbDevExist) {
	/* FIXME: Is it even possible to have less than 8Mb of video memory?
	 */
        if (pScrn->videoRam < 8*1024) {
            pXGI->cmdQueueOffset = 4*1024*1024 - pXGI->cmdQueueSize;
        }
	else if (pScrn->videoRam < 16*1024) {
	    pXGI->cmdQueueOffset = 8*1024*1024 - pXGI->cmdQueueSize;
	}
        else {
            pXGI->cmdQueueOffset = 13*1024*1024 - pXGI->cmdQueueSize;
        }
    }
    else {
        pXGI->cmdQueueOffset = (pScrn->videoRam)*1024 - pXGI->cmdQueueSize;
    }

    pXGI->CursorOffset = pXGI->cmdQueueOffset - 64*1024;
    PDEBUG4(ErrorF("pScrn->videoRam = %08lX pXGI->cmdQueueSize = %08lX\n",
			pScrn->videoRam, pXGI->cmdQueueSize)) ;
    PDEBUG4(ErrorF("pXGI->cmdQueueOffset = %08lX pXGI->CursorOffset = %08lX\n",
		pXGI->cmdQueueOffset, pXGI->CursorOffset)) ;

    pXGI->cmdQueueLen = 0 ;
    pXGI->cmdQueueLenMin = 0x200 ;
    pXGI->cmdQueueLenMax = pXGI->cmdQueueSize -  pXGI->cmdQueueLenMin ;

    /* Dual Chip support put here
     */
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Detected DRAM type : %s channel %s\n", 
	       dramChannelStr[mem_channels],
	       dramTypeStr[(ulDramType & 0x02) >> 1]);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected memory clock : %3.3fMHz\n",
            pXGI->MemClock/1000.0);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected VRAM bus width is %d\n", pXGI->BusWidth);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected Cmd Queue size is %d KB\n", pXGI->cmdQueueSize / 1024);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
            "Detected Cmd Queue Offset is %d\n", pXGI->cmdQueueOffset ) ;
    XGI_InitHwDevInfo(pScrn);
}

void
XGISetup(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    pXGI->Flags = 0;
    pXGI->VBFlags = 0;

	/* Jong 10/16/2007; merge code */
    switch (pXGI->Chipset) {
		case PCI_CHIP_XGIXG20:
		case PCI_CHIP_XGIXG21:
		case PCI_CHIP_XGIXG27:
			xgiXG2X_Setup(pScrn);
			break;

		case PCI_CHIP_XGIXG40:
		default:
			xgiXG40_Setup(pScrn);
			break;
    }
}

/* Jong 01/07/2008; Force to disable 2D engine by SR3A[6]=1 */
Bool ForceToDisable2DEngine(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI ;
	Bool   bReturn=FALSE;
    CARD8  bForce;

    pXGI = XGIPTR(pScrn); 

	if(pXGI->Chipset == PCI_CHIP_XGIXG21)
	{
	    inXGIIDXREG(XGISR, 0x3A, bForce) ;
		bForce &= 0x40;

		if(bForce == 0)
			bReturn=FALSE;
		else
			bReturn=TRUE;
	}
	else
	{
		bReturn=FALSE;
	}

	return(bReturn);
}

Bool
XGI_IsXG21(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    Bool is_XG21 = FALSE;

    if (pXGI->Chipset == PCI_CHIP_XGIXG20) {
		int temp;

        orXGIIDXREG(XGICR, Index_CR_GPIO_Reg3, GPIOG_EN);
        inXGIIDXREG(XGICR, Index_CR_GPIO_Reg1, temp);

		is_XG21 = ((temp & GPIOG_READ) != 0);
    }
    
    return is_XG21;
}

void
XGI_InitHwDevInfo(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    PXGI_HW_DEVICE_INFO pHwDevInfo = &pXGI->xgi_HwDevExt;
    int i;

    pHwDevInfo->pDevice = pXGI ;
    pHwDevInfo->pjVirtualRomBase = pXGI->BIOS ;
    pHwDevInfo->pjCustomizedROMImage = NULL ;
    pHwDevInfo->pjVideoMemoryAddress = (UCHAR*)(pXGI->FbBase) ;
    PDEBUG(ErrorF("pXGI->FbBase = 0x%08lx\n",(ULONG)(pXGI->FbBase))) ;
    PDEBUG(ErrorF("pHwDevInfo->pjVideoMemoryAddress = 0x%08lx\n",(ULONG)(pHwDevInfo->pjVideoMemoryAddress))) ;
    pHwDevInfo->ulVideoMemorySize = pXGI->FbMapSize ;
    pHwDevInfo->pjIOAddress = pXGI->RelIO + 0x30 ;

    switch (pXGI->Chipset) {
    case PCI_CHIP_XGIXG40:
        pHwDevInfo->jChipType = XG40 ;
        break ;
    case PCI_CHIP_XGIXG20:
        pHwDevInfo->jChipType = XGI_IsXG21(pScrn)?XG21:XG20 ;
        break ;
    case PCI_CHIP_XGIXG27:
    pHwDevInfo->jChipType = XG27;
        break;
    case PCI_CHIP_XGIXG21:
	pHwDevInfo->jChipType = XG21;
	break;
    default:
        pHwDevInfo->jChipType = XG40 ;
        break ;
    }

    pHwDevInfo->jChipRevision = pXGI->ChipRev;
    pHwDevInfo->ujVBChipID = VB_CHIP_UNKNOWN ;
    pHwDevInfo->ulExternalChip = 0 ;

    pHwDevInfo->ulCRT2LCDType = LCD_1024x768 ;
    pHwDevInfo->bIntegratedMMEnabled = FALSE ;
    pHwDevInfo->bSkipDramSizing = TRUE ;

    pHwDevInfo->pSR = pXGI->SRList ;
    pHwDevInfo->pCR = pXGI->CRList ;
    pHwDevInfo->pQueryVGAConfigSpace = (PXGI_QUERYSPACE) bAccessVGAPCIInfo;

    for( i = 0 ; i < ExtRegSize ; i++ ){
        pHwDevInfo->pSR[i].jIdx = 0xFF ;
        pHwDevInfo->pSR[i].jVal = 0xFF ;
        pHwDevInfo->pCR[i].jIdx = 0xFF ;
        pHwDevInfo->pCR[i].jVal = 0xFF ;
    }

    for( i = 0 ; i < VBIOS_VER_MAX_LENGTH ; i++ ){
        pHwDevInfo -> szVBIOSVer[i] = '\0' ;
    }


    XGINew_InitVBIOSData(pHwDevInfo, pXGI->XGI_Pr);
    PDEBUG(ErrorF("XGINew_InitVBIOSData(pHwDevInfo) done\n")) ;

    ErrorF("XGI_InitVBIOSData  VBType = %x\n", pXGI->XGI_Pr->VBType);
    XGI_New_GetVBType(pXGI->XGI_Pr, pHwDevInfo); //yilin
    ErrorF("XGI_New_GetVBType  VBType = %x\n", pXGI->XGI_Pr->VBType);

 //   pHwDevInfo->ujVBChipID = VB_CHIP_301 ; //yilin
    if( pXGI->XGI_Pr->VBType & (VB_XGI301 | VB_XGI301B | VB_XGI301C))
    {
    PDEBUG(ErrorF("VB chip = 301 \n")) ;
	 pHwDevInfo->ujVBChipID = VB_CHIP_301 ;
    }
    else if( pXGI->VBFlags & ( VB_XGI302B| VB_XGI302LV ))
    {
        pHwDevInfo->ujVBChipID = VB_CHIP_302 ;
    }
/*
    else if (pXGI->VBFlags & VB_LVDS) {
        pHwDevInfo->ulExternalChip |= 0x01 ;
    }
*/ //yilin


    PDEBUG(ErrorF("pHwDevInfo->jChipType = %08lX done\n",pHwDevInfo->jChipType)) ;
}

Bool
bAccessVGAPCIInfo(PXGI_HW_DEVICE_INFO pHwDevInfo, ULONG ulOffset, ULONG ulSet, CARD32 *pulValue)
{
    XGIPtr pXGI ;
#ifdef XSERVER_LIBPCIACCESS
    int err;
#else
    PCITAG pciDev;
#endif

    if (!pHwDevInfo || !pulValue) {
        return FALSE;
    }

    pXGI = (XGIPtr)pHwDevInfo->pDevice ;
#ifdef XSERVER_LIBPCIACCESS
    if (ulSet) {
	err = pci_device_cfg_write_u32(pXGI->PciInfo, *pulValue,
				       ulOffset & ~3);
    } else {
	err = pci_device_cfg_read_u32(pXGI->PciInfo, (uint32_t *)pulValue,
				       ulOffset & ~3);
    }

    return (err == 0);
#else
    pciDev = pXGI->PciTag ;

    if (ulSet) {
        pciWriteLong(pciDev, ulOffset&0xFFFFFFFc, *pulValue);
    } else {
        *pulValue = pciReadLong(pciDev, ulOffset&0xFFFFFFFc);
    }

    return TRUE ;
#endif
}
