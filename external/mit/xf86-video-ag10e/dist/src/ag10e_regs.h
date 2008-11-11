/*
 * Fujitsu AG-10e framebuffer - hardware registers.
 *
 * Copyright (C) 2007 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6_regs.h,v 1.1 2000/05/23 04:47:43 dawes Exp $ */

#ifndef AG10E_REGS_H
#define AG10E_REGS_H

#include "compiler.h"

#define pGlint pAG10E
#define GLINTPTR(p)    ((AG10EPtr)((p)->driverPrivate))
#define IOBase regs
#define FbBase fb
#define FbMapSize vidmem

#undef MMIO_OUT32
#define MMIO_OUT32(base, offset, val) \
	xf86WriteMmio32Be(base, offset, (CARD32)(val))

#undef MMIO_IN32
#define MMIO_IN32(base, offset) \
	xf86ReadMmio32Be(base, offset)

#include "glint_regs.h"

/* AlphaTestMode */
#define AlphaTestModeDisable          0x00000000
#define AlphaTestModeEnable           0x00000001
#define AT_Never                      0x00000000
#define AT_Less                       0x00000002
#define AT_Equal                      0x00000004
#define AT_LessEqual                  0x00000006
#define AT_Greater                    0x00000008
#define AT_NotEqual                   0x0000000a
#define AT_GreaterEqual               0x0000000c
#define AT_Always                     0x0000000e
#define AT_CompareMask                0x0000000e
#define AT_RefValueMask               0x00000ff0

/* AlphaBlendMode */
#define AlphaBlendModeDisable         0x00000000
#define AlphaBlendModeEnable          0x00000001
#define AB_Src_Zero                   0x00000000
#define AB_Src_One                    0x00000002
#define AB_Src_DstColor               0x00000004
#define AB_Src_OneMinusDstColor       0x00000006
#define AB_Src_SrcAlpha               0x00000008
#define AB_Src_OneMinusSrcAlpha       0x0000000a
#define AB_Src_DstAlpha               0x0000000c
#define AB_Src_OneMinusDstAlpha       0x0000000e
#define AB_Src_SrcAlphaSaturate       0x00000010
#define AB_SrcBlendMask               0x0000001e
#define AB_Dst_Zero                   0x00000000
#define AB_Dst_One                    0x00000020
#define AB_Dst_SrcColor               0x00000040
#define AB_Dst_OneMinusSrcColor       0x00000060
#define AB_Dst_SrcAlpha               0x00000080
#define AB_Dst_OneMinusSrcAlpha       0x000000a0
#define AB_Dst_DstAlpha               0x000000c0
#define AB_Dst_OneMinusDstAlpha       0x000000e0
#define AB_DstBlendMask               0x000000e0
#define AB_ColorFmt_8888              0x00000000
#define AB_ColorFmt_5555              0x00000100
#define AB_ColorFmt_4444              0x00000200
#define AB_ColorFmt_4444Front         0x00000300
#define AB_ColorFmt_4444Back          0x00000400
#define AB_ColorFmt_332Front          0x00000500
#define AB_ColorFmt_332Back           0x00000600
#define AB_ColorFmt_121Front          0x00000700
#define AB_ColorFmt_121Back           0x00000800
#define AB_ColorFmt_555Back           0x00000d00
#define AB_ColorFmt_CI8               0x00000e00
#define AB_ColorFmt_CI4               0x00000f00
#define AB_AlphaBufferPresent         0x00000000
#define AB_NoAlphaBufferPresent       0x00001000
#define AB_ColorOrder_BGR             0x00000000
#define AB_ColorOrder_RGB             0x00002000
#define AB_OpenGLType                 0x00000000
#define AB_QuickDraw3DType            0x00004000
#define AB_AlphaDst_FBData            0x00000000
#define AB_AlphaDst_FBSourceData      0x00008000
#define AB_ColorConversionScale       0x00000000
#define AB_ColorConversionShift       0x00010000
#define AB_AlphaConversionScale       0x00000000
#define AB_AlphaConversionShift       0x00020000

/* TextureAddressMode */
#define TextureAddressModeDisable     0x00000000
#define TextureAddressModeEnable      0x00000001
#define TAM_SWrap_Clamp               0x00000000
#define TAM_SWrap_Repeat              0x00000002
#define TAM_SWrap_Mirror              0x00000004
#define TAM_SWrap_Mask                0x00000006
#define TAM_TWrap_Clamp               0x00000000
#define TAM_TWrap_Repeat              0x00000008
#define TAM_TWrap_Mirror              0x00000010
#define TAM_TWrap_Mask                0x00000018
#define TAM_Operation_2D              0x00000000
#define TAM_Operation_3D              0x00000020
#define TAM_InhibitDDAInit            0x00000040
#define TAM_LODDisable                0x00000000
#define TAM_LODEnable                 0x00000080
#define TAM_DY_Disable                0x00000000
#define TAM_DY_Enable                 0x00000100
#define TAM_WidthMask                 0x00001e00
#define TAM_HeightMask                0x0001e000
#define TAM_TexMapType_1D             0x00000000
#define TAM_TexMapType_2D             0x00020000
#define TAM_TexMapType_Mask           0x00020000

/* TextureReadMode */
#define TextureReadModeDisable        0x00000000
#define TextureReadModeEnable         0x00000001
#define TRM_WidthMask                 0x0000001e
#define TRM_HeightMask                0x000001e0
#define TRM_Depth1                    0x00000000
#define TRM_Depth2                    0x00000200
#define TRM_Depth4                    0x00000400
#define TRM_Depth8                    0x00000600
#define TRM_Depth16                   0x00000800
#define TRM_Depth32                   0x00000a00
#define TRM_DepthMask                 0x00000e00
#define TRM_Border                    0x00001000
#define TRM_Patch                     0x00002000
#define TRM_Mag_Nearest               0x00000000
#define TRM_Mag_Linear                0x00004000
#define TRM_Mag_Mask                  0x00004000
#define TRM_Min_Nearest               0x00000000
#define TRM_Min_Linear                0x00008000
#define TRM_Min_NearestMMNearest      0x00010000
#define TRM_Min_NearestMMLinear       0x00018000
#define TRM_Min_LinearMMNearest       0x00020000
#define TRM_Min_LinearMMLinear        0x00028000
#define TRM_Min_Mask                  0x00038000
#define TRM_UWrap_Clamp               0x00000000
#define TRM_UWrap_Repeat              0x00040000
#define TRM_UWrap_Mirror              0x00080000
#define TRM_UWrap_Mask                0x000c0000
#define TRM_VWrap_Clamp               0x00000000
#define TRM_VWrap_Repeat              0x00100000
#define TRM_VWrap_Mirror              0x00200000
#define TRM_VWrap_Mask                0x00300000
#define TRM_TexMapType_1D             0x00000000
#define TRM_TexMapType_2D             0x00400000
#define TRM_TexMapType_Mask           0x00400000
#define TRM_MipMapDisable             0x00000000
#define TRM_MipMapEnable              0x00800000
#define TRM_PrimaryCacheDisable       0x00000000
#define TRM_PrimaryCacheEnable        0x01000000
#define TRM_FBSourceAddr_None         0x00000000
#define TRM_FBSourceAddr_Index        0x02000000
#define TRM_FBSourceAddr_Coord        0x04000000
#define TRM_BorderClamp               0x08000000

/* TextureColorMode */
#define TextureColorModeDisable       0x00000000
#define TextureColorModeEnable        0x00000001
#define TCM_Modulate                  0x00000000
#define TCM_Decal                     0x00000002
#define TCM_Blend                     0x00000004
#define TCM_Replace                   0x00000006
#define TCM_ApplicationMask           0x0000000e
#define TCM_OpenGLType                0x00000000
#define TCM_QuickDraw3DType           0x00000010
#define TCM_KdDDA_Disable             0x00000000
#define TCM_KdDDA_Enable              0x00000020
#define TCM_KsDDA_Disable             0x00000000
#define TCM_KsDDA_Enable              0x00000040
#define TCM_BaseFormat_Alpha          0x00000000
#define TCM_BaseFormat_Lum            0x00000080
#define TCM_BaseFormat_LumAlpha       0x00000100
#define TCM_BaseFormat_Intensity      0x00000180
#define TCM_BaseFormat_RGB            0x00000200
#define TCM_BaseFormat_RGBA           0x00000280
#define TCM_BaseFormatMask            0x00000380
#define TCM_LoadMode_None             0x00000000
#define TCM_LoadMode_Ks               0x00000400
#define TCM_LoadMode_Kd               0x00000800

/* TextureCacheControl */
#define TCC_Invalidate                0x00000001
#define TCC_Disable                   0x00000000
#define TCC_Enable                    0x00000002

/* TextureFilterMode */
#define TextureFilterModeDisable      0x00000000
#define TextureFilterModeEnable       0x00000001
#define TFM_AlphaMapEnable            0x00000002
#define TFM_AlphaMapSense             0x00000004

/* TextureFormat */
#define TF_LittleEndian               0x00000000
#define TF_BigEndian                  0x00000001
#define TF_16Bit_565                  0x00000000
#define TF_16Bit_555                  0x00000002
#define TF_ColorOrder_BGR             0x00000000
#define TF_ColorOrder_RGB             0x00000004
#define TF_Compnents_1                0x00000000
#define TF_Compnents_2                0x00000008
#define TF_Compnents_3                0x00000010
#define TF_Compnents_4                0x00000018
#define TF_CompnentsMask              0x00000018
#define TF_OutputFmt_Texel            0x00000000
#define TF_OutputFmt_Color            0x00000020
#define TF_OutputFmt_BitMask          0x00000040
#define TF_OutputFmtMask              0x00000060
#define TF_MirrorEnable               0x00000080
#define TF_InvertEnable               0x00000100
#define TF_ByteSwapEnable             0x00000200
#define TF_LUTOffsetMask              0x0003fc00
#define TF_OneCompFmt_Lum             0x00000000
#define TF_OneCompFmt_Alpha           0x00040000
#define TF_OneCompFmt_Intensity        0x00080000
#define TF_OneCompFmt_Mask            0x000c0000

#endif /* AG10E_REGS_H */
