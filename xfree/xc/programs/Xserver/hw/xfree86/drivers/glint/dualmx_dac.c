/*
 * Copyright 1997,1998 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Original Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *                    Dirk Hohndel,   <hohndel@suse.de>
 *	              Stefan Dirsch,  <sndirsch@suse.de>
 *	              Helmut Fahrion, <hf@suse.de>
 *
 * Modified version of tx_dac.c to support Dual MX rasterizers by 
 *   Jens Owen <jens@precisioninsight.com>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/dualmx_dac.c,v 1.4 2000/05/10 18:55:28 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "IBM.h"
#include "TI.h"
#include "glint_regs.h"
#include "glint.h"

#define DEBUG
#ifdef DEBUG
#define DUMP(name,field) do {                                             \
    value = GLINT_READ_REG(field);                                        \
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\t\t%s(primary): 0x%lX\n", name, value); \
    value = GLINT_SECONDARY_READ_REG(field);                              \
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\t\t%s(secondary): 0x%lX\n", name, value); \
} while (0)

#define TIDUMP(name,field) do {                                           \
    value = glintInTIIndReg(pScrn,field);                                 \
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\t\t%s: 0x%lX\n", name, value); \
} while (0)

void
GLINTDumpRegs(ScrnInfoPtr pScrn) {
    GLINTPtr pGlint = GLINTPTR(pScrn);
    unsigned long value;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\tGAMMA/MX regs:\n");
    DUMP("ResetStatus",ResetStatus);
    DUMP("IntEnable",IntEnable);
    DUMP("IntFlags",IntFlags);
    DUMP("InFIFOSpace",InFIFOSpace);
    DUMP("OutFIFOWords",OutFIFOWords);
    DUMP("DMAAddress",DMAAddress);
    DUMP("DMACount",DMACount);
    DUMP("ErrorFlags",ErrorFlags);
    DUMP("VClkCtl",VClkCtl);
    DUMP("TestRegister",TestRegister);
    DUMP("Aperture0",Aperture0);
    DUMP("Aperture1",Aperture1);
    DUMP("DMAControl",DMAControl);
    DUMP("FIFODis",FIFODis);
    DUMP("LBMemoryCtl",LBMemoryCtl);
    DUMP("LBMemoryEDO",LBMemoryEDO);
    DUMP("FBMemoryCtl",FBMemoryCtl);
    DUMP("FBModeSel",FBModeSel);
    DUMP("FBGCWrMask",FBGCWrMask);
    DUMP("FBGCColorLower",FBGCColorLower);
    DUMP("FBTXMemCtl",FBTXMemCtl);
    DUMP("FBWrMaskk",FBWrMaskk);
    DUMP("FBGCColorUpper",FBGCColorUpper);
    DUMP("OutputFIFO",OutputFIFO);
    DUMP("VTGHLimit",VTGHLimit);
    DUMP("VTGHSyncStart",VTGHSyncStart);
    DUMP("VTGHSyncEnd",VTGHSyncEnd);
    DUMP("VTGHBlankEnd",VTGHBlankEnd);
    DUMP("VTGVLimit",VTGVLimit);
    DUMP("VTGVSyncStart",VTGVSyncStart);
    DUMP("VTGVSyncEnd",VTGVSyncEnd);
    DUMP("VTGVBlankEnd",VTGVBlankEnd);
    DUMP("VTGHGateStart",VTGHGateStart);
    DUMP("VTGHGateEnd",VTGHGateEnd);
    DUMP("VTGVGateStart",VTGVGateStart);
    DUMP("VTGVGateEnd",VTGVGateEnd);
    DUMP("VTGPolarity",VTGPolarity);
    DUMP("VTGFrameRowAddr",VTGFrameRowAddr);
    DUMP("VTGVLineNumber",VTGVLineNumber);
    DUMP("VTGSerialClk",VTGSerialClk);
    DUMP("VTGModeCtl",VTGModeCtl);
    DUMP("GInFIFOSpace",GInFIFOSpace);
    DUMP("GDMAAddress",GDMAAddress);
    DUMP("GDMAControl",GDMAControl);
    DUMP("GOutDMA",GOutDMA);
    DUMP("GOutDMACount",GOutDMACount);
    DUMP("GResetStatus",GResetStatus);
    DUMP("GIntEnable",GIntEnable);
    DUMP("GIntFlags",GIntFlags);
    DUMP("GErrorFlags",GErrorFlags);
    DUMP("GTestRegister",GTestRegister);
    DUMP("GFIFODis",GFIFODis);
    DUMP("GChipConfig",GChipConfig);
    DUMP("GCSRAperture",GCSRAperture);
    DUMP("GPageTableAddr",GPageTableAddr);
    DUMP("GPageTableLength",GPageTableLength);
    DUMP("GDelayTimer",GDelayTimer);
    DUMP("GCommandMode",GCommandMode);
    DUMP("GCommandIntEnable",GCommandIntEnable);
    DUMP("GCommandIntFlags",GCommandIntFlags);
    DUMP("GCommandErrorFlags",GCommandErrorFlags);
    DUMP("GCommandStatus",GCommandStatus);
    DUMP("GCommandFaultingAddr",GCommandFaultingAddr);
    DUMP("GVertexFaultingAddr",GVertexFaultingAddr);
    DUMP("GWriteFaultingAddr",GWriteFaultingAddr);
    DUMP("GFeedbackSelectCount",GFeedbackSelectCount);
    DUMP("GGammaProcessorMode",GGammaProcessorMode);
    DUMP("GVGAShadow",GVGAShadow);
    DUMP("GMultGLINTAperture",GMultGLINTAperture);
    DUMP("GMultGLINT1",GMultGLINT1);
    DUMP("GMultGLINT2",GMultGLINT2);
    DUMP("StartXDom",StartXDom);
    DUMP("dXDom",dXDom);
    DUMP("StartXSub",StartXSub);
    DUMP("dXSub",dXSub);
    DUMP("StartY",StartY);
    DUMP("dY",dY);
    DUMP("GLINTCount",GLINTCount);
    DUMP("Render",Render);
    DUMP("ContinueNewLine",ContinueNewLine);
    DUMP("ContinueNewDom",ContinueNewDom);
    DUMP("ContinueNewSub",ContinueNewSub);
    DUMP("Continue",Continue);
    DUMP("FlushSpan",FlushSpan);
    DUMP("BitMaskPattern",BitMaskPattern);
    DUMP("PointTable0",PointTable0);
    DUMP("PointTable1",PointTable1);
    DUMP("PointTable2",PointTable2);
    DUMP("PointTable3",PointTable3);
    DUMP("RasterizerMode",RasterizerMode);
    DUMP("YLimits",YLimits);
    DUMP("ScanLineOwnership",ScanLineOwnership);
    DUMP("WaitForCompletion",WaitForCompletion);
    DUMP("PixelSize",PixelSize);
    DUMP("XLimits",XLimits);
    DUMP("RectangleOrigin",RectangleOrigin);
    DUMP("RectangleSize",RectangleSize);
    DUMP("PackedDataLimits",PackedDataLimits);
    DUMP("ScissorMode",ScissorMode);
    DUMP("ScissorMinXY",ScissorMinXY);
    DUMP("ScissorMaxXY",ScissorMaxXY);
    DUMP("ScreenSize",ScreenSize);
    DUMP("AreaStippleMode",AreaStippleMode);
    DUMP("LineStippleMode",LineStippleMode);
    DUMP("LoadLineStippleCounters",LoadLineStippleCounters);
    DUMP("UpdateLineStippleCounters",UpdateLineStippleCounters);
    DUMP("SaveLineStippleState",SaveLineStippleState);
    DUMP("WindowOrigin",WindowOrigin);
    DUMP("AreaStipplePattern0",AreaStipplePattern0);
    DUMP("AreaStipplePattern1",AreaStipplePattern1);
    DUMP("AreaStipplePattern2",AreaStipplePattern2);
    DUMP("AreaStipplePattern3",AreaStipplePattern3);
    DUMP("AreaStipplePattern4",AreaStipplePattern4);
    DUMP("AreaStipplePattern5",AreaStipplePattern5);
    DUMP("AreaStipplePattern6",AreaStipplePattern6);
    DUMP("AreaStipplePattern7",AreaStipplePattern7);
    DUMP("TextureAddressMode",TextureAddressMode);
    DUMP("TextureReadMode",TextureReadMode);
    DUMP("TextureFormat",TextureFormat);
    DUMP("TextureCacheControl",TextureCacheControl);
    DUMP("GLINTBorderColor",GLINTBorderColor);
    DUMP("TexelLUTIndex",TexelLUTIndex);
    DUMP("TexelLUTData",TexelLUTData);
    DUMP("Texel0",Texel0);
    DUMP("Texel1",Texel1);
    DUMP("Texel2",Texel2);
    DUMP("Texel3",Texel3);
    DUMP("Texel4",Texel4);
    DUMP("Texel5",Texel5);
    DUMP("Texel6",Texel6);
    DUMP("Texel7",Texel7);
    DUMP("Interp0",Interp0);
    DUMP("Interp1",Interp1);
    DUMP("Interp2",Interp2);
    DUMP("Interp3",Interp3);
    DUMP("Interp4",Interp4);
    DUMP("TextureFilter",TextureFilter);
    DUMP("TexelLUTMode",TexelLUTMode);
    DUMP("TextureColorMode",TextureColorMode);
    DUMP("TextureEnvColor",TextureEnvColor);
    DUMP("FogMode",FogMode);
    DUMP("FogColor",FogColor);
    DUMP("FStart",FStart);
    DUMP("dFdx",dFdx);
    DUMP("dFdyDom",dFdyDom);
    DUMP("KsStart",KsStart);
    DUMP("dKsdx",dKsdx);
    DUMP("dKsdyDom",dKsdyDom);
    DUMP("KdStart", KdStart	);
    DUMP("dKdStart", dKdStart);
    DUMP("dKddyDom", dKddyDom);
    DUMP("RStart",	RStart	);
    DUMP("dRdx", dRdx);
    DUMP("dRdyDom",	dRdyDom	);
    DUMP("GStart",	GStart	);
    DUMP("dGdx",	dGdx	);
    DUMP("dGdyDom",	dGdyDom	);
    DUMP("BStart",	BStart	);
    DUMP("dBdx",	dBdx	);
    DUMP("dBdyDom",	dBdyDom	);
    DUMP("AStart",	AStart	);
    DUMP("dAdx",	dAdx	);
    DUMP("dAdyDom",	dAdyDom	);
    DUMP("ColorDDAMode",	ColorDDAMode	);
    DUMP("ConstantColor",	ConstantColor	);
    DUMP("GLINTColor", GLINTColor);
    DUMP("AlphaTestMode",	AlphaTestMode	);
    DUMP("AntialiasMode", AntialiasMode);
    DUMP("AlphaBlendMode", AlphaBlendMode);
    DUMP("DitherMode",	DitherMode	);
    DUMP("FBSoftwareWriteMask", FBSoftwareWriteMask);
    DUMP("LogicalOpMode",	LogicalOpMode	);
    DUMP("FBWriteData", FBWriteData);
    DUMP("RouterMode", RouterMode);
    DUMP("LBReadMode", LBReadMode);
    DUMP("LBReadFormat", LBReadFormat);
    DUMP("LBSourceOffset", LBSourceOffset);
    DUMP("LBStencil", LBStencil);
    DUMP("LBDepth",		LBDepth		);
    DUMP("LBWindowBase", LBWindowBase);
    DUMP("LBWriteMode", LBWriteMode);
    DUMP("LBWriteFormat", LBWriteFormat);
    DUMP("TextureData",	TextureData	);
    DUMP("TextureDownloadOffset", TextureDownloadOffset);
    DUMP("GLINTWindow",	GLINTWindow	);
    DUMP("StencilMode", StencilMode);
    DUMP("StencilData", StencilData);
    DUMP("GLINTStencil",	GLINTStencil	);
    DUMP("DepthMode", DepthMode);
    DUMP("GLINTDepth",	GLINTDepth	);
    DUMP("ZStartU",	ZStartU	);
    DUMP("ZStartL",	ZStartL	);
    DUMP("dZdxU", dZdxU);
    DUMP("dZdxL", dZdxL);
    DUMP("dZdyDomU",dZdyDomU	);
    DUMP("dZdyDomL", dZdyDomL);
    DUMP("FastClearDepth", FastClearDepth);
    DUMP("FBReadMode", FBReadMode);
    DUMP("FBSourceOffset",	FBSourceOffset	);
    DUMP("FBPixelOffset", FBPixelOffset);
    DUMP("FBColor",	FBColor	);
    DUMP("FBData", FBData);
    DUMP("FBSourceData", FBSourceData			);
    DUMP("FBWindowBase", FBWindowBase		);
    DUMP("FBWriteMode", FBWriteMode	);
    DUMP("FBHardwareWriteMask", FBHardwareWriteMask	);
    DUMP("FBBlockColor", FBBlockColor	);
    DUMP("FBReadPixel", FBReadPixel);
    DUMP("PatternRamMode",	PatternRamMode		);
    DUMP("PatternRamData0",	PatternRamData0	);
    DUMP("PatternRamData1", PatternRamData1);
    DUMP("PatternRamData2",	PatternRamData2		);
    DUMP("PatternRamData3",	PatternRamData3	);
    DUMP("PatternRamData4", PatternRamData4);
    DUMP("PatternRamData5",	PatternRamData5				);
    DUMP("PatternRamData6",	PatternRamData6			);
    DUMP("PatternRamData7",	PatternRamData7		);
    DUMP("FilterMode",	FilterMode	);
    DUMP("StatisticMode", StatisticMode);
    DUMP("MinRegion",	MinRegion	);
    DUMP("MaxRegion",	MaxRegion	);
    DUMP("ResetPickResult",	ResetPickResult	);
    DUMP("MitHitRegion",	MitHitRegion	);
    DUMP("MaxHitRegion",	MaxHitRegion	);
    DUMP("PickResult",	PickResult	);
    DUMP("GlintSync", GlintSync);
    DUMP("FBBlockColorU",	FBBlockColorU		);
    DUMP("FBBlockColorL",	FBBlockColorL	);
    DUMP("SuspendUntilFrameBlank",	SuspendUntilFrameBlank	);
    DUMP("FBSourceBase",	FBSourceBase	);
    DUMP("FBSourceDelta", FBSourceDelta);
    DUMP("Config",	Config	);
    DUMP("YUVMode",      YUVMode      );
    DUMP("DrawTriangle", DrawTriangle);
    DUMP("RepeatTriangle", RepeatTriangle);
    DUMP("DrawLine01", DrawLine01);
    DUMP("DrawLine10", DrawLine10			);
    DUMP("RepeatLine", RepeatLine		);
    DUMP("BroadcastMask", BroadcastMask	);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\tTI3030 direct regs:\n");
#define TI_WRITE_ADDR            0x4000  
#define TI_RAMDAC_DATA           0x4008 
#define TI_PIXEL_MASK            0x4010 
#define TI_READ_ADDR             0x4018  
#define TI_CURS_COLOR_WRITE_ADDR 0x4020 
#define TI_CURS_COLOR_DATA       0x4028 
#define TI_CURS_COLOR_READ_ADDR  0x4038 
#define TI_DIRECT_CURS_CTRL      0x4048 
#define TI_INDEX_DATA            0x4050 
#define TI_CURS_RAM_DATA         0x4058 
#define TI_CURS_X_LOW            0x4060 
#define TI_CURS_X_HIGH           0x4068 
#define TI_CURS_Y_LOW            0x4070 
#define TI_CURS_Y_HIGH           0x4078 
    DUMP("TI_WRITE_ADDR", TI_WRITE_ADDR);
    DUMP("TI_RAMDAC_DATA", TI_RAMDAC_DATA);
    DUMP("TI_PIXEL_MASK", TI_PIXEL_MASK);
    DUMP("TI_READ_ADDR", TI_READ_ADDR);
    DUMP("TI_CURS_COLOR_WRITE_ADDR", TI_CURS_COLOR_WRITE_ADDR);
    DUMP("TI_CURS_COLOR_DATA", TI_CURS_COLOR_DATA);
    DUMP("TI_CURS_COLOR_READ_ADDR", TI_CURS_COLOR_READ_ADDR);
    DUMP("TI_DIRECT_CURS_CTRL", TI_DIRECT_CURS_CTRL);
    DUMP("TI_INDEX_DATA", TI_INDEX_DATA);
    DUMP("TI_CURS_RAM_DATA", TI_CURS_RAM_DATA);
    DUMP("TI_CURS_X_LOW", TI_CURS_X_LOW);
    DUMP("TI_CURS_X_HIGH", TI_CURS_X_HIGH);
    DUMP("TI_CURS_Y_LOW", TI_CURS_Y_LOW);
    DUMP("TI_CURS_Y_HIGH", TI_CURS_Y_HIGH);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\tTI3030 indirect regs:\n");
    TIDUMP("TIDAC_rev",TIDAC_rev);
    TIDUMP("TIDAC_ind_curs_ctrl",TIDAC_ind_curs_ctrl);
    TIDUMP("TIDAC_byte_router_ctrl",TIDAC_byte_router_ctrl);
    TIDUMP("TIDAC_latch_ctrl",TIDAC_latch_ctrl);
    TIDUMP("TIDAC_true_color_ctrl",TIDAC_true_color_ctrl);
    TIDUMP("TIDAC_multiplex_ctrl",TIDAC_multiplex_ctrl);
    TIDUMP("TIDAC_clock_select",TIDAC_clock_select);
    TIDUMP("TIDAC_palette_page",TIDAC_palette_page);
    TIDUMP("TIDAC_general_ctrl",TIDAC_general_ctrl);
    TIDUMP("TIDAC_misc_ctrl",TIDAC_misc_ctrl);
    TIDUMP("TIDAC_pll_addr",TIDAC_pll_addr);
    TIDUMP("TIDAC_pll_pixel_data",TIDAC_pll_pixel_data);
    TIDUMP("TIDAC_pll_memory_data",TIDAC_pll_memory_data);
    TIDUMP("TIDAC_pll_loop_data",TIDAC_pll_loop_data);
    TIDUMP("TIDAC_key_over_low",TIDAC_key_over_low);
    TIDUMP("TIDAC_key_over_high",TIDAC_key_over_high);
    TIDUMP("TIDAC_key_red_low",TIDAC_key_red_low);
    TIDUMP("TIDAC_key_red_high",TIDAC_key_red_high);
    TIDUMP("TIDAC_key_green_low",TIDAC_key_green_low);
    TIDUMP("TIDAC_key_green_high",TIDAC_key_green_high);
    TIDUMP("TIDAC_key_blue_low",TIDAC_key_blue_low);
    TIDUMP("TIDAC_key_blue_high",TIDAC_key_blue_high);
    TIDUMP("TIDAC_key_ctrl",TIDAC_key_ctrl);
    TIDUMP("TIDAC_clock_ctrl",TIDAC_clock_ctrl);
    TIDUMP("TIDAC_sense_test",TIDAC_sense_test);
    TIDUMP("TIDAC_test_mode_data",TIDAC_test_mode_data);
    TIDUMP("TIDAC_crc_remain_lsb",TIDAC_crc_remain_lsb);
    TIDUMP("TIDAC_crc_remain_msb",TIDAC_crc_remain_msb);
    TIDUMP("TIDAC_crc_bit_select",TIDAC_crc_bit_select);
    TIDUMP("TIDAC_id",TIDAC_id);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\tTI3030 PLL regs:\n");
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x00);
    TIDUMP("Pixel N",TIDAC_pll_pixel_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x01);
    TIDUMP("Pixel M",TIDAC_pll_pixel_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x02);
    TIDUMP("Pixel P",TIDAC_pll_pixel_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x00);
    TIDUMP("Memory N",TIDAC_pll_memory_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x04);
    TIDUMP("Memory M",TIDAC_pll_memory_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x08);
    TIDUMP("Memory P",TIDAC_pll_memory_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x00);
    TIDUMP("Loop N",TIDAC_pll_loop_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x10);
    TIDUMP("Loop M",TIDAC_pll_loop_data);
    glintOutTIIndReg(pScrn, TIDAC_pll_addr, 0, 0x20);
    TIDUMP("Loop P",TIDAC_pll_loop_data);
}
#endif

static int
Shiftbpp(ScrnInfoPtr pScrn, int value)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int logbytesperaccess;

    if ( (pGlint->RamDac->RamDacType == (IBM640_RAMDAC)) ||
         (pGlint->RamDac->RamDacType == (TI3030_RAMDAC)) )
    	logbytesperaccess = 4;
    else
    	logbytesperaccess = 3;
	
    switch (pScrn->bitsPerPixel) {
    case 8:
	value >>= logbytesperaccess;
	pGlint->BppShift = logbytesperaccess;
	break;
    case 16:
	if (pGlint->DoubleBuffer) {
	    value >>= (logbytesperaccess-2);
	    pGlint->BppShift = logbytesperaccess-2;
	} else {
	    value >>= (logbytesperaccess-1);
	    pGlint->BppShift = logbytesperaccess-1;
	}
	break;
    case 24:
	value *= 3;
	value >>= logbytesperaccess;
	pGlint->BppShift = logbytesperaccess;
	break;
    case 32:
	value >>= (logbytesperaccess-2);
	pGlint->BppShift = logbytesperaccess-2;
	break;
    }
    return (value);
}

Bool
DualMXInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    GLINTRegPtr pReg = &pGlint->ModeReg;
    RamDacHWRecPtr pTI = RAMDACHWPTR(pScrn);
    RamDacRegRecPtr ramdacReg = &pTI->ModeReg;
    CARD32 temp1, temp2, temp3, temp4;

    pReg->glintRegs[Aperture0 >> 3] = 0;
    pReg->glintRegs[Aperture1 >> 3] = 0;

    if (pGlint->UsePCIRetry) {
	pReg->glintRegs[DFIFODis >> 3] = GLINT_READ_REG(DFIFODis) | 0x01;
    	if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA)
	    pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x01;
	else
	    pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x03;
    } else {
	pReg->glintRegs[DFIFODis >> 3] = GLINT_READ_REG(DFIFODis) & 0xFFFFFFFE;
	pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x01;
    }

    temp1 = mode->CrtcHSyncStart - mode->CrtcHDisplay;
    temp2 = mode->CrtcVSyncStart - mode->CrtcVDisplay;
    temp3 = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;
    temp4 = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;

    pReg->glintRegs[VTGHLimit >> 3] = Shiftbpp(pScrn,mode->CrtcHTotal);
    pReg->glintRegs[VTGHSyncEnd >> 3] = Shiftbpp(pScrn, temp1 + temp3);
    pReg->glintRegs[VTGHSyncStart >> 3] = Shiftbpp(pScrn, temp1);
    pReg->glintRegs[VTGHBlankEnd >> 3] = Shiftbpp(pScrn, mode->CrtcHTotal -
							mode->CrtcHDisplay);

    pReg->glintRegs[VTGVLimit >> 3] = mode->CrtcVTotal;
    pReg->glintRegs[VTGVSyncEnd >> 3] = temp2 + temp4;
    pReg->glintRegs[VTGVSyncStart >> 3] = temp2;
    pReg->glintRegs[VTGVBlankEnd >> 3] = mode->CrtcVTotal - mode->CrtcVDisplay;

#if 1 /* We force them high */
    pReg->glintRegs[VTGPolarity >> 3] = 0xBA;
#else
    pReg->glintRegs[VTGPolarity >> 3] = (((mode->Flags & V_PHSYNC) ? 0:2)<<2) |
			     ((mode->Flags & V_PVSYNC) ? 0 : 2) | (0xb0);
#endif

    pReg->glintRegs[VClkCtl >> 3] = 0;
    pReg->glintRegs[VTGVGateStart >> 3] = pReg->glintRegs[VTGVBlankEnd>>3] - 1; 
    pReg->glintRegs[VTGVGateEnd >> 3] = pReg->glintRegs[VTGVBlankEnd>>3];
    /*
     * tell DAC to use the ICD chip clock 0 as ref clock 
     * and set up some more video timing generator registers
     */
    pReg->glintRegs[VTGHGateStart >> 3] = pReg->glintRegs[VTGHBlankEnd>>3] - 1;
    pReg->glintRegs[VTGHGateEnd >> 3] = pReg->glintRegs[VTGHLimit>>3] - 1;
    pReg->glintRegs[VTGSerialClk >> 3] = 0x0002;
    pReg->glintRegs[FBModeSel >> 3] = 0x907;
    pReg->glintRegs[VTGModeCtl >> 3] = 0x04;

    /*
     * Setup memory control registers for FB and LB
     */
    pReg->glintRegs[FBMemoryCtl >> 3] = 0x00000800;
    pReg->glintRegs[LBMemoryEDO >> 3] = GLINT_READ_REG(LBMemoryEDO);
    pReg->glintRegs[LBMemoryEDO >> 3] &= ~(LBEDOMask |
					   LBEDOBankSizeMask |
					   LBTwoPageDetectorMask);
    pReg->glintRegs[LBMemoryEDO >> 3] |= (LBEDOEnabled |
					  LBEDOBankSize4M |
					  LBTwoPageDetector);
    pReg->glintRegs[LBMemoryCtl >> 3] = GLINT_READ_REG(LBMemoryCtl);
    pReg->glintRegs[LBMemoryCtl >> 3] &= ~(LBNumBanksMask |
					   LBPageSizeMask |
					   LBRASCASLowMask |
					   LBRASPrechargeMask |
					   LBCASLowMask |
					   LBPageModeMask |
					   LBRefreshCountMask);
    pReg->glintRegs[LBMemoryCtl >> 3] |= (LBNumBanks2 |
					  LBPageSize1024 |
					  LBRASCASLow2 |
					  LBRASPrecharge2 |
					  LBCASLow1 |
					  LBPageModeEnabled |
					  (0x20 << LBRefreshCountShift));
    pReg->glintRegs[GCSRAperture >> 3] = GCSRSecondaryGLINTMapEn;

    /* 
     * Setup HW 
     * 
     * Note: The order of discovery for the MX devices is dependent
     * on which way the resource allocation code decides to scan the
     * bus.  This setup assumes the first MX found owns the even
     * scanlines.  Should the implementation change an scan the bus
     * in the opposite direction, then simple invert the indices for
     * MXPciInfo below.  If this is setup wrong, the bug will appear
     * as incorrect scanline interleaving when software rendering.
     */
    pReg->glintRegs[GMultGLINTAperture >> 3] = pGlint->realMXWidth;
    pReg->glintRegs[GMultGLINT1 >> 3] = 
				pGlint->MXPciInfo[0]->memBase[2] & 0xFF800000;
    pReg->glintRegs[GMultGLINT2 >> 3] = 
				pGlint->MXPciInfo[1]->memBase[2] & 0xFF800000;

    /* Copy info to secondary regs */
    pReg->glintSecondRegs[Aperture0>>3]     = pReg->glintRegs[Aperture0>>3];
    pReg->glintSecondRegs[Aperture1>>3]     = pReg->glintRegs[Aperture1>>3];

    pReg->glintSecondRegs[DFIFODis>>3]      = pReg->glintRegs[DFIFODis>>3];
    pReg->glintSecondRegs[FIFODis>>3]       = pReg->glintRegs[FIFODis>>3];
    pReg->glintSecondRegs[VTGHLimit>>3]     = pReg->glintRegs[VTGHLimit>>3];
    pReg->glintSecondRegs[VTGHSyncEnd>>3]   = pReg->glintRegs[VTGHSyncEnd>>3];
    pReg->glintSecondRegs[VTGHSyncStart>>3] = pReg->glintRegs[VTGHSyncStart>>3];
    pReg->glintSecondRegs[VTGHBlankEnd>>3]  = pReg->glintRegs[VTGHBlankEnd>>3];
    pReg->glintSecondRegs[VTGVLimit>>3]     = pReg->glintRegs[VTGVLimit>>3];
    pReg->glintSecondRegs[VTGVSyncEnd>>3]   = pReg->glintRegs[VTGVSyncEnd>>3];
    pReg->glintSecondRegs[VTGVSyncStart>>3] = pReg->glintRegs[VTGVSyncStart>>3];
    pReg->glintSecondRegs[VTGVBlankEnd>>3]  = pReg->glintRegs[VTGVBlankEnd>>3];
    pReg->glintSecondRegs[VTGPolarity>>3]   = pReg->glintRegs[VTGPolarity>>3];
    pReg->glintSecondRegs[VClkCtl>>3]       = pReg->glintRegs[VClkCtl>>3];
    pReg->glintSecondRegs[VTGVGateStart>>3] = pReg->glintRegs[VTGVGateStart>>3];
    pReg->glintSecondRegs[VTGVGateEnd>>3]   = pReg->glintRegs[VTGVGateEnd>>3];
    pReg->glintSecondRegs[VTGSerialClk>>3]  = pReg->glintRegs[VTGSerialClk>>3];
    pReg->glintSecondRegs[VTGHGateStart>>3] = pReg->glintRegs[VTGHGateStart>>3];
    pReg->glintSecondRegs[VTGHGateEnd>>3]   = pReg->glintRegs[VTGHGateEnd>>3];
    pReg->glintSecondRegs[FBModeSel>>3]     = pReg->glintRegs[FBModeSel>>3];
    pReg->glintSecondRegs[VTGModeCtl>>3]    = pReg->glintRegs[VTGModeCtl>>3];
    pReg->glintSecondRegs[FBMemoryCtl>>3]   = pReg->glintRegs[FBMemoryCtl>>3];
    pReg->glintSecondRegs[LBMemoryEDO>>3]   = pReg->glintRegs[LBMemoryEDO>>3];
    pReg->glintSecondRegs[LBMemoryCtl>>3]   = pReg->glintRegs[LBMemoryCtl>>3];
    pReg->glintSecondRegs[GCSRAperture>>3]  = pReg->glintRegs[GCSRAperture>>3];
    pReg->glintSecondRegs[GMultGLINTAperture>>3] = 
					pReg->glintRegs[GMultGLINTAperture>>3];
    pReg->glintSecondRegs[GMultGLINT1>>3]   = pReg->glintRegs[GMultGLINT1>>3];
    pReg->glintSecondRegs[GMultGLINT2>>3]   = pReg->glintRegs[GMultGLINT2>>3];

    switch (pGlint->RamDac->RamDacType) {
    case TI3030_RAMDAC:
    case TI3026_RAMDAC:
	{
	    /* Get the programmable clock values */
	    unsigned long m=0,n=0,p=0;
	    unsigned long clock;
	    int count;
	    unsigned long q, status, VCO;

	    clock = TIramdacCalculateMNPForClock(pGlint->RefClock, 
		mode->Clock, 1, pGlint->MinClock, pGlint->MaxClock, &m, &n, &p);

	    ramdacReg->DacRegs[TIDAC_PIXEL_N] = ((n & 0x3f) | 0xc0);
	    ramdacReg->DacRegs[TIDAC_PIXEL_M] =  (m & 0x3f)        ;
	    ramdacReg->DacRegs[TIDAC_PIXEL_P] = ((p & 0x03) | 0xb0);
	    ramdacReg->DacRegs[TIDAC_PIXEL_VALID] = TRUE;

    	    if (pGlint->RamDac->RamDacType == (TI3026_RAMDAC))
                n = 65 - ((64 << 2) / pScrn->bitsPerPixel);
	    else
                n = 65 - ((128 << 2) / pScrn->bitsPerPixel);
	    m = 61;
	    p = 0;
	    for (q = 0; q < 8; q++) {
		if (q > 0) p = 3;
		for ( ; p < 4; p++) {
		    VCO = ((clock * (q + 1) * (65 - m)) / (65 - n)) << (p + 1);
		    if (VCO >= 110000) { break; }
		}
		if (VCO >= 110000) { break; }
	    }
	    ramdacReg->DacRegs[TIDAC_clock_ctrl] = (q | 0x38);

	    ramdacReg->DacRegs[TIDAC_LOOP_N] = ((n & 0x3f) | 0xc0);
	    ramdacReg->DacRegs[TIDAC_LOOP_M] =  (m & 0x3f)        ;
	    ramdacReg->DacRegs[TIDAC_LOOP_P] = ((p & 0x03) | 0xf0);
	    ramdacReg->DacRegs[TIDAC_LOOP_VALID] = TRUE;
	}
	break;
    }

    /* Now use helper routines to setup bpp for this driver */
    (*pGlint->RamDac->SetBpp)(pScrn, ramdacReg);

    return(TRUE);
}

void
DualMXSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    glintReg->glintRegs[Aperture0 >> 3]  = GLINT_READ_REG(Aperture0);
    glintReg->glintRegs[Aperture1 >> 3]  = GLINT_READ_REG(Aperture1);

    glintReg->glintRegs[DFIFODis >> 3]  = GLINT_READ_REG(DFIFODis);
    glintReg->glintRegs[FIFODis >> 3]  = GLINT_READ_REG(FIFODis);

    glintReg->glintRegs[VClkCtl >> 3] = GLINT_READ_REG(VClkCtl);
    glintReg->glintRegs[VTGPolarity >> 3] = GLINT_READ_REG(VTGPolarity);
    glintReg->glintRegs[VTGHLimit >> 3] = GLINT_READ_REG(VTGHLimit);
    glintReg->glintRegs[VTGHBlankEnd >> 3] = GLINT_READ_REG(VTGHBlankEnd);
    glintReg->glintRegs[VTGHSyncStart >> 3] = GLINT_READ_REG(VTGHSyncStart);
    glintReg->glintRegs[VTGHSyncEnd >> 3] = GLINT_READ_REG(VTGHSyncEnd);
    glintReg->glintRegs[VTGVLimit >> 3] = GLINT_READ_REG(VTGVLimit);
    glintReg->glintRegs[VTGVBlankEnd >> 3] = GLINT_READ_REG(VTGVBlankEnd);
    glintReg->glintRegs[VTGVSyncStart >> 3] = GLINT_READ_REG(VTGVSyncStart);
    glintReg->glintRegs[VTGVSyncEnd >> 3] = GLINT_READ_REG(VTGVSyncEnd);
    glintReg->glintRegs[VTGVGateStart >> 3] = GLINT_READ_REG(VTGVGateStart);
    glintReg->glintRegs[VTGVGateEnd >> 3] = GLINT_READ_REG(VTGVGateEnd);
    glintReg->glintRegs[VTGSerialClk >> 3] = GLINT_READ_REG(VTGSerialClk);
    glintReg->glintRegs[FBModeSel >> 3] = GLINT_READ_REG(FBModeSel);
    glintReg->glintRegs[VTGModeCtl >> 3] = GLINT_READ_REG(VTGModeCtl);
    glintReg->glintRegs[VTGHGateStart >> 3] = GLINT_READ_REG(VTGHGateStart);
    glintReg->glintRegs[VTGHGateEnd >> 3] = GLINT_READ_REG(VTGHGateEnd);
    glintReg->glintRegs[FBMemoryCtl >> 3] = GLINT_READ_REG(FBMemoryCtl);
    glintReg->glintRegs[LBMemoryEDO >> 3] = GLINT_READ_REG(LBMemoryEDO);
    glintReg->glintRegs[LBMemoryCtl >> 3] = GLINT_READ_REG(LBMemoryCtl);
    glintReg->glintRegs[GCSRAperture >> 3] = GLINT_READ_REG(GCSRAperture);
    glintReg->glintRegs[GMultGLINTAperture>>3] = 
					GLINT_READ_REG(GMultGLINTAperture);
    glintReg->glintRegs[GMultGLINT1>>3] = GLINT_READ_REG(GMultGLINT1);
    glintReg->glintRegs[GMultGLINT2>>3] = GLINT_READ_REG(GMultGLINT2);

    glintReg->glintSecondRegs[Aperture0 >> 3] = 
	GLINT_SECONDARY_READ_REG(Aperture0);
    glintReg->glintSecondRegs[Aperture1 >> 3] = 
	GLINT_SECONDARY_READ_REG(Aperture1);

    glintReg->glintSecondRegs[DFIFODis >> 3] = 
	GLINT_SECONDARY_READ_REG(DFIFODis);
    glintReg->glintSecondRegs[FIFODis >> 3] = 
	GLINT_SECONDARY_READ_REG(FIFODis);

    glintReg->glintSecondRegs[VClkCtl >> 3] = 
	GLINT_SECONDARY_READ_REG(VClkCtl);
    glintReg->glintSecondRegs[VTGPolarity >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGPolarity);
    glintReg->glintSecondRegs[VTGHLimit >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHLimit);
    glintReg->glintSecondRegs[VTGHBlankEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHBlankEnd);
    glintReg->glintSecondRegs[VTGHSyncStart >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHSyncStart);
    glintReg->glintSecondRegs[VTGHSyncEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHSyncEnd);
    glintReg->glintSecondRegs[VTGVLimit >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVLimit);
    glintReg->glintSecondRegs[VTGVBlankEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVBlankEnd);
    glintReg->glintSecondRegs[VTGVSyncStart >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVSyncStart);
    glintReg->glintSecondRegs[VTGVSyncEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVSyncEnd);
    glintReg->glintSecondRegs[VTGVGateStart >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVGateStart);
    glintReg->glintSecondRegs[VTGVGateEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGVGateEnd);
    glintReg->glintSecondRegs[VTGSerialClk >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGSerialClk);
    glintReg->glintSecondRegs[FBModeSel >> 3] = 
	GLINT_SECONDARY_READ_REG(FBModeSel);
    glintReg->glintSecondRegs[VTGModeCtl >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGModeCtl);
    glintReg->glintSecondRegs[VTGHGateStart >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHGateStart);
    glintReg->glintSecondRegs[VTGHGateEnd >> 3] = 
	GLINT_SECONDARY_READ_REG(VTGHGateEnd);
    glintReg->glintSecondRegs[FBMemoryCtl >> 3] = 
	GLINT_SECONDARY_READ_REG(FBMemoryCtl);
    glintReg->glintSecondRegs[LBMemoryEDO >> 3] = 
	GLINT_SECONDARY_READ_REG(LBMemoryEDO);
    glintReg->glintSecondRegs[LBMemoryCtl >> 3] = 
	GLINT_SECONDARY_READ_REG(LBMemoryCtl);
    glintReg->glintSecondRegs[GCSRAperture >> 3] = 
	GLINT_SECONDARY_READ_REG(GCSRAperture);
    glintReg->glintSecondRegs[GMultGLINTAperture>>3] = 
	GLINT_SECONDARY_READ_REG(GMultGLINTAperture);
    glintReg->glintSecondRegs[GMultGLINT1>>3] = 
	GLINT_SECONDARY_READ_REG(GMultGLINT1);
    glintReg->glintSecondRegs[GMultGLINT2>>3] = 
	GLINT_SECONDARY_READ_REG(GMultGLINT2);
}

void
DualMXRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

#if 0
    GLINT_SLOW_WRITE_REG(0, ResetStatus);
    while(GLINT_READ_REG(ResetStatus) != 0) {
	xf86MsgVerb(X_INFO, 2, "Resetting Engine - Please Wait.\n");
    };
#endif

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture0 >> 3], Aperture0);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture1 >> 3], Aperture1);

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[DFIFODis >> 3], DFIFODis);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FIFODis >> 3], FIFODis);

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGPolarity >> 3], VTGPolarity);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGSerialClk >> 3], VTGSerialClk);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGModeCtl >> 3], VTGModeCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHLimit >> 3], VTGHLimit);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHSyncStart >> 3],VTGHSyncStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHSyncEnd >> 3], VTGHSyncEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHBlankEnd >> 3], VTGHBlankEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVLimit >> 3], VTGVLimit);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVSyncStart >> 3],VTGVSyncStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVSyncEnd >> 3], VTGVSyncEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVBlankEnd >> 3], VTGVBlankEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVGateStart >> 3],VTGVGateStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVGateEnd >> 3], VTGVGateEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FBModeSel >> 3], FBModeSel);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHGateStart >> 3],VTGHGateStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHGateEnd >> 3], VTGHGateEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FBMemoryCtl >> 3], FBMemoryCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[LBMemoryEDO >> 3], LBMemoryEDO);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[LBMemoryCtl >> 3], LBMemoryCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[GCSRAperture >> 3], GCSRAperture);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[GMultGLINTAperture >> 3], 
							GMultGLINTAperture);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[GMultGLINT1 >> 3], GMultGLINT1);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[GMultGLINT2 >> 3], GMultGLINT2);

    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[Aperture0 >> 3], Aperture0);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[Aperture1 >> 3], Aperture1);

    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[DFIFODis >> 3], DFIFODis);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[FIFODis >> 3], FIFODis);

    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGPolarity >> 3], VTGPolarity);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGSerialClk >> 3], VTGSerialClk);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGModeCtl >> 3], VTGModeCtl);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHLimit >> 3], VTGHLimit);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHSyncStart >> 3],VTGHSyncStart);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHSyncEnd >> 3], VTGHSyncEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHBlankEnd >> 3], VTGHBlankEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVLimit >> 3], VTGVLimit);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVSyncStart >> 3],VTGVSyncStart);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVSyncEnd >> 3], VTGVSyncEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVBlankEnd >> 3], VTGVBlankEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVGateStart >> 3],VTGVGateStart);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGVGateEnd >> 3], VTGVGateEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[FBModeSel >> 3], FBModeSel);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHGateStart >> 3],VTGHGateStart);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[VTGHGateEnd >> 3], VTGHGateEnd);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[FBMemoryCtl >> 3], FBMemoryCtl);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[LBMemoryEDO >> 3], LBMemoryEDO);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[LBMemoryCtl >> 3], LBMemoryCtl);
    GLINT_SECONDARY_SLOW_WRITE_REG(
    	glintReg->glintSecondRegs[GCSRAperture >> 3], GCSRAperture);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[GMultGLINTAperture >>3], GMultGLINTAperture); 
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[GMultGLINT1 >> 3], GMultGLINT1);
    GLINT_SECONDARY_SLOW_WRITE_REG(
	glintReg->glintSecondRegs[GMultGLINT2 >> 3], GMultGLINT2);
}
