/*
 * Copyright (C) 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


/* Instructions, enums and structures for IVB.
 *
 * This file has been generated, do not hand edit.
 */

#ifndef GFX7_PACK_H
#define GFX7_PACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#ifndef __gen_validate_value
#define __gen_validate_value(x)
#endif

#ifndef __intel_field_functions
#define __intel_field_functions

#ifdef NDEBUG
#define NDEBUG_UNUSED __attribute__((unused))
#else
#define NDEBUG_UNUSED
#endif

union __intel_value {
   float f;
   uint32_t dw;
};

static inline __attribute__((always_inline)) uint64_t
__gen_mbo(uint32_t start, uint32_t end)
{
   return (~0ull >> (64 - (end - start + 1))) << start;
}

static inline __attribute__((always_inline)) uint64_t
__gen_uint(uint64_t v, uint32_t start, NDEBUG_UNUSED uint32_t end)
{
   __gen_validate_value(v);

#ifndef NDEBUG
   const int width = end - start + 1;
   if (width < 64) {
      const uint64_t max = (1ull << width) - 1;
      assert(v <= max);
   }
#endif

   return v << start;
}

static inline __attribute__((always_inline)) uint64_t
__gen_sint(int64_t v, uint32_t start, uint32_t end)
{
   const int width = end - start + 1;

   __gen_validate_value(v);

#ifndef NDEBUG
   if (width < 64) {
      const int64_t max = (1ll << (width - 1)) - 1;
      const int64_t min = -(1ll << (width - 1));
      assert(min <= v && v <= max);
   }
#endif

   const uint64_t mask = ~0ull >> (64 - width);

   return (v & mask) << start;
}

static inline __attribute__((always_inline)) uint64_t
__gen_offset(uint64_t v, NDEBUG_UNUSED uint32_t start, NDEBUG_UNUSED uint32_t end)
{
   __gen_validate_value(v);
#ifndef NDEBUG
   uint64_t mask = (~0ull >> (64 - (end - start + 1))) << start;

   assert((v & ~mask) == 0);
#endif

   return v;
}

static inline __attribute__((always_inline)) uint64_t
__gen_address(__gen_user_data *data, void *location,
              __gen_address_type address, uint32_t delta,
              __attribute__((unused)) uint32_t start, uint32_t end)
{
   uint64_t addr_u64 = __gen_combine_address(data, location, address, delta);
   if (end == 31) {
      return addr_u64;
   } else if (end < 63) {
      const unsigned shift = 63 - end;
      return (addr_u64 << shift) >> shift;
   } else {
      return addr_u64;
   }
}

static inline __attribute__((always_inline)) uint32_t
__gen_float(float v)
{
   __gen_validate_value(v);
   return ((union __intel_value) { .f = (v) }).dw;
}

static inline __attribute__((always_inline)) uint64_t
__gen_sfixed(float v, uint32_t start, uint32_t end, uint32_t fract_bits)
{
   __gen_validate_value(v);

   const float factor = (1 << fract_bits);

#ifndef NDEBUG
   const float max = ((1 << (end - start)) - 1) / factor;
   const float min = -(1 << (end - start)) / factor;
   assert(min <= v && v <= max);
#endif

   const int64_t int_val = llroundf(v * factor);
   const uint64_t mask = ~0ull >> (64 - (end - start + 1));

   return (int_val & mask) << start;
}

static inline __attribute__((always_inline)) uint64_t
__gen_ufixed(float v, uint32_t start, NDEBUG_UNUSED uint32_t end, uint32_t fract_bits)
{
   __gen_validate_value(v);

   const float factor = (1 << fract_bits);

#ifndef NDEBUG
   const float max = ((1 << (end - start + 1)) - 1) / factor;
   const float min = 0.0f;
   assert(min <= v && v <= max);
#endif

   const uint64_t uint_val = llroundf(v * factor);

   return uint_val << start;
}

#ifndef __gen_address_type
#error #define __gen_address_type before including this file
#endif

#ifndef __gen_user_data
#error #define __gen_combine_address before including this file
#endif

#undef NDEBUG_UNUSED

#endif


enum GFX7_3D_Color_Buffer_Blend_Factor {
   BLENDFACTOR_ONE                      =      1,
   BLENDFACTOR_SRC_COLOR                =      2,
   BLENDFACTOR_SRC_ALPHA                =      3,
   BLENDFACTOR_DST_ALPHA                =      4,
   BLENDFACTOR_DST_COLOR                =      5,
   BLENDFACTOR_SRC_ALPHA_SATURATE       =      6,
   BLENDFACTOR_CONST_COLOR              =      7,
   BLENDFACTOR_CONST_ALPHA              =      8,
   BLENDFACTOR_SRC1_COLOR               =      9,
   BLENDFACTOR_SRC1_ALPHA               =     10,
   BLENDFACTOR_ZERO                     =     17,
   BLENDFACTOR_INV_SRC_COLOR            =     18,
   BLENDFACTOR_INV_SRC_ALPHA            =     19,
   BLENDFACTOR_INV_DST_ALPHA            =     20,
   BLENDFACTOR_INV_DST_COLOR            =     21,
   BLENDFACTOR_INV_CONST_COLOR          =     23,
   BLENDFACTOR_INV_CONST_ALPHA          =     24,
   BLENDFACTOR_INV_SRC1_COLOR           =     25,
   BLENDFACTOR_INV_SRC1_ALPHA           =     26,
};

enum GFX7_3D_Color_Buffer_Blend_Function {
   BLENDFUNCTION_ADD                    =      0,
   BLENDFUNCTION_SUBTRACT               =      1,
   BLENDFUNCTION_REVERSE_SUBTRACT       =      2,
   BLENDFUNCTION_MIN                    =      3,
   BLENDFUNCTION_MAX                    =      4,
};

enum GFX7_3D_Compare_Function {
   COMPAREFUNCTION_ALWAYS               =      0,
   COMPAREFUNCTION_NEVER                =      1,
   COMPAREFUNCTION_LESS                 =      2,
   COMPAREFUNCTION_EQUAL                =      3,
   COMPAREFUNCTION_LEQUAL               =      4,
   COMPAREFUNCTION_GREATER              =      5,
   COMPAREFUNCTION_NOTEQUAL             =      6,
   COMPAREFUNCTION_GEQUAL               =      7,
};

enum GFX7_3D_Logic_Op_Function {
   LOGICOP_CLEAR                        =      0,
   LOGICOP_NOR                          =      1,
   LOGICOP_AND_INVERTED                 =      2,
   LOGICOP_COPY_INVERTED                =      3,
   LOGICOP_AND_REVERSE                  =      4,
   LOGICOP_INVERT                       =      5,
   LOGICOP_XOR                          =      6,
   LOGICOP_NAND                         =      7,
   LOGICOP_AND                          =      8,
   LOGICOP_EQUIV                        =      9,
   LOGICOP_NOOP                         =     10,
   LOGICOP_OR_INVERTED                  =     11,
   LOGICOP_COPY                         =     12,
   LOGICOP_OR_REVERSE                   =     13,
   LOGICOP_OR                           =     14,
   LOGICOP_SET                          =     15,
};

enum GFX7_3D_Prim_Topo_Type {
   _3DPRIM_POINTLIST                    =      1,
   _3DPRIM_LINELIST                     =      2,
   _3DPRIM_LINESTRIP                    =      3,
   _3DPRIM_TRILIST                      =      4,
   _3DPRIM_TRISTRIP                     =      5,
   _3DPRIM_TRIFAN                       =      6,
   _3DPRIM_QUADLIST                     =      7,
   _3DPRIM_QUADSTRIP                    =      8,
   _3DPRIM_LINELIST_ADJ                 =      9,
   _3DPRIM_LINESTRIP_ADJ                =     10,
   _3DPRIM_TRILIST_ADJ                  =     11,
   _3DPRIM_TRISTRIP_ADJ                 =     12,
   _3DPRIM_TRISTRIP_REVERSE             =     13,
   _3DPRIM_POLYGON                      =     14,
   _3DPRIM_RECTLIST                     =     15,
   _3DPRIM_LINELOOP                     =     16,
   _3DPRIM_POINTLIST_BF                 =     17,
   _3DPRIM_LINESTRIP_CONT               =     18,
   _3DPRIM_LINESTRIP_BF                 =     19,
   _3DPRIM_LINESTRIP_CONT_BF            =     20,
   _3DPRIM_TRIFAN_NOSTIPPLE             =     22,
   _3DPRIM_PATCHLIST_1                  =     32,
   _3DPRIM_PATCHLIST_2                  =     33,
   _3DPRIM_PATCHLIST_3                  =     34,
   _3DPRIM_PATCHLIST_4                  =     35,
   _3DPRIM_PATCHLIST_5                  =     36,
   _3DPRIM_PATCHLIST_6                  =     37,
   _3DPRIM_PATCHLIST_7                  =     38,
   _3DPRIM_PATCHLIST_8                  =     39,
   _3DPRIM_PATCHLIST_9                  =     40,
   _3DPRIM_PATCHLIST_10                 =     41,
   _3DPRIM_PATCHLIST_11                 =     42,
   _3DPRIM_PATCHLIST_12                 =     43,
   _3DPRIM_PATCHLIST_13                 =     44,
   _3DPRIM_PATCHLIST_14                 =     45,
   _3DPRIM_PATCHLIST_15                 =     46,
   _3DPRIM_PATCHLIST_16                 =     47,
   _3DPRIM_PATCHLIST_17                 =     48,
   _3DPRIM_PATCHLIST_18                 =     49,
   _3DPRIM_PATCHLIST_19                 =     50,
   _3DPRIM_PATCHLIST_20                 =     51,
   _3DPRIM_PATCHLIST_21                 =     52,
   _3DPRIM_PATCHLIST_22                 =     53,
   _3DPRIM_PATCHLIST_23                 =     54,
   _3DPRIM_PATCHLIST_24                 =     55,
   _3DPRIM_PATCHLIST_25                 =     56,
   _3DPRIM_PATCHLIST_26                 =     57,
   _3DPRIM_PATCHLIST_27                 =     58,
   _3DPRIM_PATCHLIST_28                 =     59,
   _3DPRIM_PATCHLIST_29                 =     60,
   _3DPRIM_PATCHLIST_30                 =     61,
   _3DPRIM_PATCHLIST_31                 =     62,
   _3DPRIM_PATCHLIST_32                 =     63,
};

enum GFX7_3D_Stencil_Operation {
   STENCILOP_KEEP                       =      0,
   STENCILOP_ZERO                       =      1,
   STENCILOP_REPLACE                    =      2,
   STENCILOP_INCRSAT                    =      3,
   STENCILOP_DECRSAT                    =      4,
   STENCILOP_INCR                       =      5,
   STENCILOP_DECR                       =      6,
   STENCILOP_INVERT                     =      7,
};

enum GFX7_3D_Vertex_Component_Control {
   VFCOMP_NOSTORE                       =      0,
   VFCOMP_STORE_SRC                     =      1,
   VFCOMP_STORE_0                       =      2,
   VFCOMP_STORE_1_FP                    =      3,
   VFCOMP_STORE_1_INT                   =      4,
   VFCOMP_STORE_VID                     =      5,
   VFCOMP_STORE_IID                     =      6,
   VFCOMP_STORE_PID                     =      7,
};

enum GFX7_TextureCoordinateMode {
   TCM_WRAP                             =      0,
   TCM_MIRROR                           =      1,
   TCM_CLAMP                            =      2,
   TCM_CUBE                             =      3,
   TCM_CLAMP_BORDER                     =      4,
   TCM_MIRROR_ONCE                      =      5,
};

#define GFX7_3DSTATE_CONSTANT_BODY_length      6
struct GFX7_3DSTATE_CONSTANT_BODY {
   uint32_t                             ReadLength[4];
   uint32_t                             MOCS;
   __gen_address_type                   Buffer[4];
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ReadLength[0], 0, 15) |
      __gen_uint(values->ReadLength[1], 16, 31);

   dw[1] =
      __gen_uint(values->ReadLength[2], 0, 15) |
      __gen_uint(values->ReadLength[3], 16, 31);

   const uint32_t v2 =
      __gen_uint(values->MOCS, 0, 4);
   dw[2] = __gen_address(data, &dw[2], values->Buffer[0], v2, 5, 31);

   dw[3] = __gen_address(data, &dw[3], values->Buffer[1], 0, 5, 31);

   dw[4] = __gen_address(data, &dw[4], values->Buffer[2], 0, 5, 31);

   dw[5] = __gen_address(data, &dw[5], values->Buffer[3], 0, 5, 31);
}

#define GFX7_BINDING_TABLE_STATE_length        1
struct GFX7_BINDING_TABLE_STATE {
   uint64_t                             SurfaceStatePointer;
};

static inline __attribute__((always_inline)) void
GFX7_BINDING_TABLE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_BINDING_TABLE_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->SurfaceStatePointer, 5, 31);
}

#define GFX7_BLEND_STATE_ENTRY_length          2
struct GFX7_BLEND_STATE_ENTRY {
   enum GFX7_3D_Color_Buffer_Blend_Factor DestinationBlendFactor;
   enum GFX7_3D_Color_Buffer_Blend_Factor SourceBlendFactor;
   enum GFX7_3D_Color_Buffer_Blend_Function ColorBlendFunction;
   enum GFX7_3D_Color_Buffer_Blend_Factor DestinationAlphaBlendFactor;
   enum GFX7_3D_Color_Buffer_Blend_Factor SourceAlphaBlendFactor;
   enum GFX7_3D_Color_Buffer_Blend_Function AlphaBlendFunction;
   bool                                 IndependentAlphaBlendEnable;
   bool                                 ColorBufferBlendEnable;
   bool                                 PostBlendColorClampEnable;
   bool                                 PreBlendColorClampEnable;
   uint32_t                             ColorClampRange;
#define COLORCLAMP_UNORM                         0
#define COLORCLAMP_SNORM                         1
#define COLORCLAMP_RTFORMAT                      2
   uint32_t                             YDitherOffset;
   uint32_t                             XDitherOffset;
   bool                                 ColorDitherEnable;
   enum GFX7_3D_Compare_Function        AlphaTestFunction;
   bool                                 AlphaTestEnable;
   enum GFX7_3D_Logic_Op_Function       LogicOpFunction;
   bool                                 LogicOpEnable;
   bool                                 WriteDisableBlue;
   bool                                 WriteDisableGreen;
   bool                                 WriteDisableRed;
   bool                                 WriteDisableAlpha;
   bool                                 AlphaToCoverageDitherEnable;
   bool                                 AlphaToOneEnable;
   bool                                 AlphaToCoverageEnable;
};

static inline __attribute__((always_inline)) void
GFX7_BLEND_STATE_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_BLEND_STATE_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DestinationBlendFactor, 0, 4) |
      __gen_uint(values->SourceBlendFactor, 5, 9) |
      __gen_uint(values->ColorBlendFunction, 11, 13) |
      __gen_uint(values->DestinationAlphaBlendFactor, 15, 19) |
      __gen_uint(values->SourceAlphaBlendFactor, 20, 24) |
      __gen_uint(values->AlphaBlendFunction, 26, 28) |
      __gen_uint(values->IndependentAlphaBlendEnable, 30, 30) |
      __gen_uint(values->ColorBufferBlendEnable, 31, 31);

   dw[1] =
      __gen_uint(values->PostBlendColorClampEnable, 0, 0) |
      __gen_uint(values->PreBlendColorClampEnable, 1, 1) |
      __gen_uint(values->ColorClampRange, 2, 3) |
      __gen_uint(values->YDitherOffset, 8, 9) |
      __gen_uint(values->XDitherOffset, 10, 11) |
      __gen_uint(values->ColorDitherEnable, 12, 12) |
      __gen_uint(values->AlphaTestFunction, 13, 15) |
      __gen_uint(values->AlphaTestEnable, 16, 16) |
      __gen_uint(values->LogicOpFunction, 18, 21) |
      __gen_uint(values->LogicOpEnable, 22, 22) |
      __gen_uint(values->WriteDisableBlue, 24, 24) |
      __gen_uint(values->WriteDisableGreen, 25, 25) |
      __gen_uint(values->WriteDisableRed, 26, 26) |
      __gen_uint(values->WriteDisableAlpha, 27, 27) |
      __gen_uint(values->AlphaToCoverageDitherEnable, 29, 29) |
      __gen_uint(values->AlphaToOneEnable, 30, 30) |
      __gen_uint(values->AlphaToCoverageEnable, 31, 31);
}

#define GFX7_BLEND_STATE_length                0
struct GFX7_BLEND_STATE {
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_BLEND_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX7_BLEND_STATE * restrict values)
{
}

#define GFX7_CC_VIEWPORT_length                2
struct GFX7_CC_VIEWPORT {
   float                                MinimumDepth;
   float                                MaximumDepth;
};

static inline __attribute__((always_inline)) void
GFX7_CC_VIEWPORT_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX7_CC_VIEWPORT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_float(values->MinimumDepth);

   dw[1] =
      __gen_float(values->MaximumDepth);
}

#define GFX7_COLOR_CALC_STATE_length           6
struct GFX7_COLOR_CALC_STATE {
   uint32_t                             AlphaTestFormat;
#define ALPHATEST_UNORM8                         0
#define ALPHATEST_FLOAT32                        1
   bool                                 RoundDisableFunctionDisable;
   uint32_t                             BackfaceStencilReferenceValue;
   uint32_t                             StencilReferenceValue;
   uint32_t                             AlphaReferenceValueAsUNORM8;
   float                                AlphaReferenceValueAsFLOAT32;
   float                                BlendConstantColorRed;
   float                                BlendConstantColorGreen;
   float                                BlendConstantColorBlue;
   float                                BlendConstantColorAlpha;
};

static inline __attribute__((always_inline)) void
GFX7_COLOR_CALC_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_COLOR_CALC_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->AlphaTestFormat, 0, 0) |
      __gen_uint(values->RoundDisableFunctionDisable, 15, 15) |
      __gen_uint(values->BackfaceStencilReferenceValue, 16, 23) |
      __gen_uint(values->StencilReferenceValue, 24, 31);

   dw[1] =
      __gen_uint(values->AlphaReferenceValueAsUNORM8, 0, 31) |
      __gen_float(values->AlphaReferenceValueAsFLOAT32);

   dw[2] =
      __gen_float(values->BlendConstantColorRed);

   dw[3] =
      __gen_float(values->BlendConstantColorGreen);

   dw[4] =
      __gen_float(values->BlendConstantColorBlue);

   dw[5] =
      __gen_float(values->BlendConstantColorAlpha);
}

#define GFX7_DEPTH_STENCIL_STATE_length        3
struct GFX7_DEPTH_STENCIL_STATE {
   enum GFX7_3D_Stencil_Operation       BackfaceStencilPassDepthPassOp;
   enum GFX7_3D_Stencil_Operation       BackfaceStencilPassDepthFailOp;
   enum GFX7_3D_Stencil_Operation       BackfaceStencilFailOp;
   enum GFX7_3D_Compare_Function        BackfaceStencilTestFunction;
   bool                                 DoubleSidedStencilEnable;
   bool                                 StencilBufferWriteEnable;
   enum GFX7_3D_Stencil_Operation       StencilPassDepthPassOp;
   enum GFX7_3D_Stencil_Operation       StencilPassDepthFailOp;
   enum GFX7_3D_Stencil_Operation       StencilFailOp;
   enum GFX7_3D_Compare_Function        StencilTestFunction;
   bool                                 StencilTestEnable;
   uint32_t                             BackfaceStencilWriteMask;
   uint32_t                             BackfaceStencilTestMask;
   uint32_t                             StencilWriteMask;
   uint32_t                             StencilTestMask;
   bool                                 DepthBufferWriteEnable;
   enum GFX7_3D_Compare_Function        DepthTestFunction;
   bool                                 DepthTestEnable;
};

static inline __attribute__((always_inline)) void
GFX7_DEPTH_STENCIL_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_DEPTH_STENCIL_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BackfaceStencilPassDepthPassOp, 3, 5) |
      __gen_uint(values->BackfaceStencilPassDepthFailOp, 6, 8) |
      __gen_uint(values->BackfaceStencilFailOp, 9, 11) |
      __gen_uint(values->BackfaceStencilTestFunction, 12, 14) |
      __gen_uint(values->DoubleSidedStencilEnable, 15, 15) |
      __gen_uint(values->StencilBufferWriteEnable, 18, 18) |
      __gen_uint(values->StencilPassDepthPassOp, 19, 21) |
      __gen_uint(values->StencilPassDepthFailOp, 22, 24) |
      __gen_uint(values->StencilFailOp, 25, 27) |
      __gen_uint(values->StencilTestFunction, 28, 30) |
      __gen_uint(values->StencilTestEnable, 31, 31);

   dw[1] =
      __gen_uint(values->BackfaceStencilWriteMask, 0, 7) |
      __gen_uint(values->BackfaceStencilTestMask, 8, 15) |
      __gen_uint(values->StencilWriteMask, 16, 23) |
      __gen_uint(values->StencilTestMask, 24, 31);

   dw[2] =
      __gen_uint(values->DepthBufferWriteEnable, 26, 26) |
      __gen_uint(values->DepthTestFunction, 27, 29) |
      __gen_uint(values->DepthTestEnable, 31, 31);
}

#define GFX7_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT_length      2
struct GFX7_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT {
   bool                                 MBErrorConcealmentPSliceWeightPredictionDisable;
   bool                                 MBErrorConcealmentPSliceMotionVectorsOverrideDisable;
   bool                                 MBErrorConcealmentPSliceReferenceIndexOverrideDisable;
   bool                                 MBErrorConcealmentBSpatialWeightPredictionDisable;
   bool                                 MBErrorConcealmentBSpatialMotionVectorsOverrideDisable;
   bool                                 MBErrorConcealmentBSpatialReferenceIndexOverrideDisable;
   uint32_t                             MBErrorConcealmentBSpatialPredictionMode;
   bool                                 MBHeaderErrorHandling;
   bool                                 EntropyErrorHandling;
   bool                                 MPRErrorHandling;
   bool                                 BSDPrematureCompleteErrorHandling;
   uint32_t                             ConcealmentPictureID;
   bool                                 MBErrorConcealmentBTemporalWeightPredictionDisable;
   bool                                 MBErrorConcealmentBTemporalMotionVectorsOverrideEnable;
   bool                                 MBErrorConcealmentBTemporalReferenceIndexOverrideEnable;
   uint32_t                             MBErrorConcealmentBTemporalPredictionMode;
   bool                                 InitCurrentMBNumber;
   uint32_t                             ConcealmentMethod;
   uint32_t                             FirstMBBitOffset;
   bool                                 LastSlice;
   bool                                 EmulationPreventionBytePresent;
   bool                                 FixPrevMBSkipped;
   uint32_t                             FirstMBByteOffsetofSliceDataorSliceHeader;
};

static inline __attribute__((always_inline)) void
GFX7_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT_pack(__attribute__((unused)) __gen_user_data *data,
                                                         __attribute__((unused)) void * restrict dst,
                                                         __attribute__((unused)) const struct GFX7_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MBErrorConcealmentPSliceWeightPredictionDisable, 0, 0) |
      __gen_uint(values->MBErrorConcealmentPSliceMotionVectorsOverrideDisable, 1, 1) |
      __gen_uint(values->MBErrorConcealmentPSliceReferenceIndexOverrideDisable, 2, 2) |
      __gen_uint(values->MBErrorConcealmentBSpatialWeightPredictionDisable, 3, 3) |
      __gen_uint(values->MBErrorConcealmentBSpatialMotionVectorsOverrideDisable, 4, 4) |
      __gen_uint(values->MBErrorConcealmentBSpatialReferenceIndexOverrideDisable, 5, 5) |
      __gen_uint(values->MBErrorConcealmentBSpatialPredictionMode, 6, 7) |
      __gen_uint(values->MBHeaderErrorHandling, 8, 8) |
      __gen_uint(values->EntropyErrorHandling, 10, 10) |
      __gen_uint(values->MPRErrorHandling, 12, 12) |
      __gen_uint(values->BSDPrematureCompleteErrorHandling, 14, 14) |
      __gen_uint(values->ConcealmentPictureID, 16, 21) |
      __gen_uint(values->MBErrorConcealmentBTemporalWeightPredictionDisable, 24, 24) |
      __gen_uint(values->MBErrorConcealmentBTemporalMotionVectorsOverrideEnable, 25, 25) |
      __gen_uint(values->MBErrorConcealmentBTemporalReferenceIndexOverrideEnable, 26, 26) |
      __gen_uint(values->MBErrorConcealmentBTemporalPredictionMode, 27, 28) |
      __gen_uint(values->InitCurrentMBNumber, 30, 30) |
      __gen_uint(values->ConcealmentMethod, 31, 31);

   dw[1] =
      __gen_uint(values->FirstMBBitOffset, 0, 2) |
      __gen_uint(values->LastSlice, 3, 3) |
      __gen_uint(values->EmulationPreventionBytePresent, 4, 4) |
      __gen_uint(values->FixPrevMBSkipped, 7, 7) |
      __gen_uint(values->FirstMBByteOffsetofSliceDataorSliceHeader, 16, 31);
}

#define GFX7_INTERFACE_DESCRIPTOR_DATA_length      8
struct GFX7_INTERFACE_DESCRIPTOR_DATA {
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 MaskStackExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             ThreadPriority;
#define NormalPriority                           0
#define HighPriority                             1
   bool                                 SingleProgramFlow;
   uint32_t                             SamplerCount;
#define Nosamplersused                           0
#define Between1and4samplersused                 1
#define Between5and8samplersused                 2
#define Between9and12samplersused                3
#define Between13and16samplersused               4
   uint64_t                             SamplerStatePointer;
   uint32_t                             BindingTableEntryCount;
   uint64_t                             BindingTablePointer;
   uint32_t                             ConstantURBEntryReadOffset;
   uint32_t                             ConstantURBEntryReadLength;
   uint32_t                             NumberofThreadsinGPGPUThreadGroup;
   uint32_t                             SharedLocalMemorySize;
   bool                                 BarrierEnable;
   uint32_t                             RoundingMode;
#define RTNE                                     0
#define RU                                       1
#define RD                                       2
#define RTZ                                      3
};

static inline __attribute__((always_inline)) void
GFX7_INTERFACE_DESCRIPTOR_DATA_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX7_INTERFACE_DESCRIPTOR_DATA * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->KernelStartPointer, 6, 31);

   dw[1] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadPriority, 17, 17) |
      __gen_uint(values->SingleProgramFlow, 18, 18);

   dw[2] =
      __gen_uint(values->SamplerCount, 2, 4) |
      __gen_offset(values->SamplerStatePointer, 5, 31);

   dw[3] =
      __gen_uint(values->BindingTableEntryCount, 0, 4) |
      __gen_offset(values->BindingTablePointer, 5, 15);

   dw[4] =
      __gen_uint(values->ConstantURBEntryReadOffset, 0, 15) |
      __gen_uint(values->ConstantURBEntryReadLength, 16, 31);

   dw[5] =
      __gen_uint(values->NumberofThreadsinGPGPUThreadGroup, 0, 7) |
      __gen_uint(values->SharedLocalMemorySize, 16, 20) |
      __gen_uint(values->BarrierEnable, 21, 21) |
      __gen_uint(values->RoundingMode, 22, 23);

   dw[6] = 0;

   dw[7] = 0;
}

#define GFX7_MEMORY_OBJECT_CONTROL_STATE_length      1
struct GFX7_MEMORY_OBJECT_CONTROL_STATE {
   uint32_t                             L3CacheabilityControlL3CC;
   uint32_t                             LLCCacheabilityControlLLCCC;
   uint32_t                             GraphicsDataTypeGFDT;
};

static inline __attribute__((always_inline)) void
GFX7_MEMORY_OBJECT_CONTROL_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX7_MEMORY_OBJECT_CONTROL_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->L3CacheabilityControlL3CC, 0, 0) |
      __gen_uint(values->LLCCacheabilityControlLLCCC, 1, 1) |
      __gen_uint(values->GraphicsDataTypeGFDT, 2, 2);
}

#define GFX7_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION_length      2
struct GFX7_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION {
   uint32_t                             FirstMBBitOffset;
   bool                                 LastMB;
   bool                                 LastPicSlice;
   uint32_t                             MBCount;
   uint32_t                             SliceVerticalPosition;
   uint32_t                             SliceHorizontalPosition;
   uint32_t                             QuantizerScaleCode;
};

static inline __attribute__((always_inline)) void
GFX7_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION_pack(__attribute__((unused)) __gen_user_data *data,
                                                       __attribute__((unused)) void * restrict dst,
                                                       __attribute__((unused)) const struct GFX7_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->FirstMBBitOffset, 0, 2) |
      __gen_uint(values->LastMB, 3, 3) |
      __gen_uint(values->LastPicSlice, 5, 5) |
      __gen_uint(values->MBCount, 8, 14) |
      __gen_uint(values->SliceVerticalPosition, 16, 22) |
      __gen_uint(values->SliceHorizontalPosition, 24, 30);

   dw[1] =
      __gen_uint(values->QuantizerScaleCode, 24, 28);
}

#define GFX7_PALETTE_ENTRY_length              1
struct GFX7_PALETTE_ENTRY {
   uint32_t                             Blue;
   uint32_t                             Green;
   uint32_t                             Red;
   uint32_t                             Alpha;
};

static inline __attribute__((always_inline)) void
GFX7_PALETTE_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_PALETTE_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->Blue, 0, 7) |
      __gen_uint(values->Green, 8, 15) |
      __gen_uint(values->Red, 16, 23) |
      __gen_uint(values->Alpha, 24, 31);
}

#define GFX7_RENDER_SURFACE_STATE_length       8
struct GFX7_RENDER_SURFACE_STATE {
   bool                                 CubeFaceEnablePositiveZ;
   bool                                 CubeFaceEnableNegativeZ;
   bool                                 CubeFaceEnablePositiveY;
   bool                                 CubeFaceEnableNegativeY;
   bool                                 CubeFaceEnablePositiveX;
   bool                                 CubeFaceEnableNegativeX;
   uint32_t                             MediaBoundaryPixelMode;
#define NORMAL_MODE                              0
#define PROGRESSIVE_FRAME                        2
#define INTERLACED_FRAME                         3
   uint32_t                             RenderCacheReadWriteMode;
   uint32_t                             SurfaceArraySpacing;
#define ARYSPC_FULL                              0
#define ARYSPC_LOD0                              1
   uint32_t                             VerticalLineStrideOffset;
   uint32_t                             VerticalLineStride;
   uint32_t                             TileWalk;
#define TILEWALK_XMAJOR                          0
#define TILEWALK_YMAJOR                          1
   bool                                 TiledSurface;
   uint32_t                             SurfaceHorizontalAlignment;
#define HALIGN_4                                 0
#define HALIGN_8                                 1
   uint32_t                             SurfaceVerticalAlignment;
#define VALIGN_2                                 0
#define VALIGN_4                                 1
   uint32_t                             SurfaceFormat;
   bool                                 SurfaceArray;
   uint32_t                             SurfaceType;
#define SURFTYPE_1D                              0
#define SURFTYPE_2D                              1
#define SURFTYPE_3D                              2
#define SURFTYPE_CUBE                            3
#define SURFTYPE_BUFFER                          4
#define SURFTYPE_STRBUF                          5
#define SURFTYPE_NULL                            7
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             Width;
   uint32_t                             Height;
   uint32_t                             SurfacePitch;
   uint32_t                             Depth;
   uint32_t                             MultisamplePositionPaletteIndex;
   uint32_t                             StrbufMinimumArrayElement;
   uint32_t                             NumberofMultisamples;
#define MULTISAMPLECOUNT_1                       0
#define MULTISAMPLECOUNT_4                       2
#define MULTISAMPLECOUNT_8                       3
   uint32_t                             MultisampledSurfaceStorageFormat;
#define MSFMT_MSS                                0
#define MSFMT_DEPTH_STENCIL                      1
   uint32_t                             RenderTargetViewExtent;
   uint32_t                             MinimumArrayElement;
   uint32_t                             RenderTargetRotation;
#define RTROTATE_0DEG                            0
#define RTROTATE_90DEG                           1
#define RTROTATE_270DEG                          3
   uint32_t                             MIPCountLOD;
   uint32_t                             SurfaceMinLOD;
   uint32_t                             MOCS;
   uint32_t                             YOffset;
   uint32_t                             XOffset;
   bool                                 MCSEnable;
   uint32_t                             YOffsetforUVPlane;
   bool                                 AppendCounterEnable;
   uint32_t                             AuxiliarySurfacePitch;
   __gen_address_type                   AppendCounterAddress;
   __gen_address_type                   AuxiliarySurfaceBaseAddress;
   uint32_t                             XOffsetforUVPlane;
   uint32_t                             ReservedMBZ;
   float                                ResourceMinLOD;
   uint32_t                             AlphaClearColor;
#define CC_ZERO                                  0
#define CC_ONE                                   1
   uint32_t                             BlueClearColor;
#define CC_ZERO                                  0
#define CC_ONE                                   1
   uint32_t                             GreenClearColor;
#define CC_ZERO                                  0
#define CC_ONE                                   1
   uint32_t                             RedClearColor;
#define CC_ZERO                                  0
#define CC_ONE                                   1
};

static inline __attribute__((always_inline)) void
GFX7_RENDER_SURFACE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_RENDER_SURFACE_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->CubeFaceEnablePositiveZ, 0, 0) |
      __gen_uint(values->CubeFaceEnableNegativeZ, 1, 1) |
      __gen_uint(values->CubeFaceEnablePositiveY, 2, 2) |
      __gen_uint(values->CubeFaceEnableNegativeY, 3, 3) |
      __gen_uint(values->CubeFaceEnablePositiveX, 4, 4) |
      __gen_uint(values->CubeFaceEnableNegativeX, 5, 5) |
      __gen_uint(values->MediaBoundaryPixelMode, 6, 7) |
      __gen_uint(values->RenderCacheReadWriteMode, 8, 8) |
      __gen_uint(values->SurfaceArraySpacing, 10, 10) |
      __gen_uint(values->VerticalLineStrideOffset, 11, 11) |
      __gen_uint(values->VerticalLineStride, 12, 12) |
      __gen_uint(values->TileWalk, 13, 13) |
      __gen_uint(values->TiledSurface, 14, 14) |
      __gen_uint(values->SurfaceHorizontalAlignment, 15, 15) |
      __gen_uint(values->SurfaceVerticalAlignment, 16, 17) |
      __gen_uint(values->SurfaceFormat, 18, 26) |
      __gen_uint(values->SurfaceArray, 28, 28) |
      __gen_uint(values->SurfaceType, 29, 31);

   dw[1] = __gen_address(data, &dw[1], values->SurfaceBaseAddress, 0, 0, 31);

   dw[2] =
      __gen_uint(values->Width, 0, 13) |
      __gen_uint(values->Height, 16, 29);

   dw[3] =
      __gen_uint(values->SurfacePitch, 0, 17) |
      __gen_uint(values->Depth, 21, 31);

   dw[4] =
      __gen_uint(values->MultisamplePositionPaletteIndex, 0, 2) |
      __gen_uint(values->StrbufMinimumArrayElement, 0, 26) |
      __gen_uint(values->NumberofMultisamples, 3, 5) |
      __gen_uint(values->MultisampledSurfaceStorageFormat, 6, 6) |
      __gen_uint(values->RenderTargetViewExtent, 7, 17) |
      __gen_uint(values->MinimumArrayElement, 18, 28) |
      __gen_uint(values->RenderTargetRotation, 29, 30);

   dw[5] =
      __gen_uint(values->MIPCountLOD, 0, 3) |
      __gen_uint(values->SurfaceMinLOD, 4, 7) |
      __gen_uint(values->MOCS, 16, 19) |
      __gen_uint(values->YOffset, 20, 23) |
      __gen_uint(values->XOffset, 25, 31);

   const uint32_t v6 =
      __gen_uint(values->MCSEnable, 0, 0) |
      __gen_uint(values->YOffsetforUVPlane, 0, 13) |
      __gen_uint(values->AppendCounterEnable, 1, 1) |
      __gen_uint(values->AuxiliarySurfacePitch, 3, 11) |
      __gen_uint(values->XOffsetforUVPlane, 16, 29) |
      __gen_uint(values->ReservedMBZ, 30, 31);
   dw[6] = __gen_address(data, &dw[6], values->AuxiliarySurfaceBaseAddress, v6, 12, 31);

   dw[7] =
      __gen_ufixed(values->ResourceMinLOD, 0, 11, 8) |
      __gen_uint(values->AlphaClearColor, 28, 28) |
      __gen_uint(values->BlueClearColor, 29, 29) |
      __gen_uint(values->GreenClearColor, 30, 30) |
      __gen_uint(values->RedClearColor, 31, 31);
}

#define GFX7_SAMPLER_BORDER_COLOR_STATE_length      4
struct GFX7_SAMPLER_BORDER_COLOR_STATE {
   uint32_t                             BorderColorUnormRed;
   float                                BorderColorFloatRed;
   uint32_t                             BorderColorUnormGreen;
   uint32_t                             BorderColorUnormBlue;
   uint32_t                             BorderColorUnormAlpha;
   float                                BorderColorFloatGreen;
   float                                BorderColorFloatBlue;
   float                                BorderColorFloatAlpha;
};

static inline __attribute__((always_inline)) void
GFX7_SAMPLER_BORDER_COLOR_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX7_SAMPLER_BORDER_COLOR_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BorderColorUnormRed, 0, 7) |
      __gen_float(values->BorderColorFloatRed) |
      __gen_uint(values->BorderColorUnormGreen, 8, 15) |
      __gen_uint(values->BorderColorUnormBlue, 16, 23) |
      __gen_uint(values->BorderColorUnormAlpha, 24, 31);

   dw[1] =
      __gen_float(values->BorderColorFloatGreen);

   dw[2] =
      __gen_float(values->BorderColorFloatBlue);

   dw[3] =
      __gen_float(values->BorderColorFloatAlpha);
}

#define GFX7_SAMPLER_STATE_length              4
struct GFX7_SAMPLER_STATE {
   uint32_t                             AnisotropicAlgorithm;
#define LEGACY                                   0
#define EWAApproximation                         1
   float                                TextureLODBias;
   uint32_t                             MinModeFilter;
#define MAPFILTER_NEAREST                        0
#define MAPFILTER_LINEAR                         1
#define MAPFILTER_ANISOTROPIC                    2
#define MAPFILTER_MONO                           6
   uint32_t                             MagModeFilter;
#define MAPFILTER_NEAREST                        0
#define MAPFILTER_LINEAR                         1
#define MAPFILTER_ANISOTROPIC                    2
#define MAPFILTER_MONO                           6
   uint32_t                             MipModeFilter;
#define MIPFILTER_NONE                           0
#define MIPFILTER_NEAREST                        1
#define MIPFILTER_LINEAR                         3
   float                                BaseMipLevel;
   uint32_t                             LODPreClampEnable;
#define CLAMP_ENABLE_OGL                         1
   uint32_t                             TextureBorderColorMode;
#define DX10OGL                                  0
#define DX9                                      1
   bool                                 SamplerDisable;
   uint32_t                             CubeSurfaceControlMode;
#define PROGRAMMED                               0
#define OVERRIDE                                 1
   uint32_t                             ShadowFunction;
#define PREFILTEROP_ALWAYS                       0
#define PREFILTEROP_NEVER                        1
#define PREFILTEROP_LESS                         2
#define PREFILTEROP_EQUAL                        3
#define PREFILTEROP_LEQUAL                       4
#define PREFILTEROP_GREATER                      5
#define PREFILTEROP_NOTEQUAL                     6
#define PREFILTEROP_GEQUAL                       7
   float                                MaxLOD;
   float                                MinLOD;
   uint64_t                             BorderColorPointer;
   enum GFX7_TextureCoordinateMode      TCZAddressControlMode;
   enum GFX7_TextureCoordinateMode      TCYAddressControlMode;
   enum GFX7_TextureCoordinateMode      TCXAddressControlMode;
   bool                                 NonnormalizedCoordinateEnable;
   uint32_t                             TrilinearFilterQuality;
#define FULL                                     0
#define MED                                      2
#define LOW                                      3
   bool                                 RAddressMinFilterRoundingEnable;
   bool                                 RAddressMagFilterRoundingEnable;
   bool                                 VAddressMinFilterRoundingEnable;
   bool                                 VAddressMagFilterRoundingEnable;
   bool                                 UAddressMinFilterRoundingEnable;
   bool                                 UAddressMagFilterRoundingEnable;
   uint32_t                             MaximumAnisotropy;
#define RATIO21                                  0
#define RATIO41                                  1
#define RATIO61                                  2
#define RATIO81                                  3
#define RATIO101                                 4
#define RATIO121                                 5
#define RATIO141                                 6
#define RATIO161                                 7
   uint32_t                             ChromaKeyMode;
#define KEYFILTER_KILL_ON_ANY_MATCH              0
#define KEYFILTER_REPLACE_BLACK                  1
   uint32_t                             ChromaKeyIndex;
   bool                                 ChromaKeyEnable;
};

static inline __attribute__((always_inline)) void
GFX7_SAMPLER_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_SAMPLER_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->AnisotropicAlgorithm, 0, 0) |
      __gen_sfixed(values->TextureLODBias, 1, 13, 8) |
      __gen_uint(values->MinModeFilter, 14, 16) |
      __gen_uint(values->MagModeFilter, 17, 19) |
      __gen_uint(values->MipModeFilter, 20, 21) |
      __gen_ufixed(values->BaseMipLevel, 22, 26, 1) |
      __gen_uint(values->LODPreClampEnable, 28, 28) |
      __gen_uint(values->TextureBorderColorMode, 29, 29) |
      __gen_uint(values->SamplerDisable, 31, 31);

   dw[1] =
      __gen_uint(values->CubeSurfaceControlMode, 0, 0) |
      __gen_uint(values->ShadowFunction, 1, 3) |
      __gen_ufixed(values->MaxLOD, 8, 19, 8) |
      __gen_ufixed(values->MinLOD, 20, 31, 8);

   dw[2] =
      __gen_offset(values->BorderColorPointer, 5, 31);

   dw[3] =
      __gen_uint(values->TCZAddressControlMode, 0, 2) |
      __gen_uint(values->TCYAddressControlMode, 3, 5) |
      __gen_uint(values->TCXAddressControlMode, 6, 8) |
      __gen_uint(values->NonnormalizedCoordinateEnable, 10, 10) |
      __gen_uint(values->TrilinearFilterQuality, 11, 12) |
      __gen_uint(values->RAddressMinFilterRoundingEnable, 13, 13) |
      __gen_uint(values->RAddressMagFilterRoundingEnable, 14, 14) |
      __gen_uint(values->VAddressMinFilterRoundingEnable, 15, 15) |
      __gen_uint(values->VAddressMagFilterRoundingEnable, 16, 16) |
      __gen_uint(values->UAddressMinFilterRoundingEnable, 17, 17) |
      __gen_uint(values->UAddressMagFilterRoundingEnable, 18, 18) |
      __gen_uint(values->MaximumAnisotropy, 19, 21) |
      __gen_uint(values->ChromaKeyMode, 22, 22) |
      __gen_uint(values->ChromaKeyIndex, 23, 24) |
      __gen_uint(values->ChromaKeyEnable, 25, 25);
}

#define GFX7_SCISSOR_RECT_length               2
struct GFX7_SCISSOR_RECT {
   uint32_t                             ScissorRectangleXMin;
   uint32_t                             ScissorRectangleYMin;
   uint32_t                             ScissorRectangleXMax;
   uint32_t                             ScissorRectangleYMax;
};

static inline __attribute__((always_inline)) void
GFX7_SCISSOR_RECT_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_SCISSOR_RECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ScissorRectangleXMin, 0, 15) |
      __gen_uint(values->ScissorRectangleYMin, 16, 31);

   dw[1] =
      __gen_uint(values->ScissorRectangleXMax, 0, 15) |
      __gen_uint(values->ScissorRectangleYMax, 16, 31);
}

#define GFX7_SF_CLIP_VIEWPORT_length          16
struct GFX7_SF_CLIP_VIEWPORT {
   float                                ViewportMatrixElementm00;
   float                                ViewportMatrixElementm11;
   float                                ViewportMatrixElementm22;
   float                                ViewportMatrixElementm30;
   float                                ViewportMatrixElementm31;
   float                                ViewportMatrixElementm32;
   float                                XMinClipGuardband;
   float                                XMaxClipGuardband;
   float                                YMinClipGuardband;
   float                                YMaxClipGuardband;
};

static inline __attribute__((always_inline)) void
GFX7_SF_CLIP_VIEWPORT_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SF_CLIP_VIEWPORT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_float(values->ViewportMatrixElementm00);

   dw[1] =
      __gen_float(values->ViewportMatrixElementm11);

   dw[2] =
      __gen_float(values->ViewportMatrixElementm22);

   dw[3] =
      __gen_float(values->ViewportMatrixElementm30);

   dw[4] =
      __gen_float(values->ViewportMatrixElementm31);

   dw[5] =
      __gen_float(values->ViewportMatrixElementm32);

   dw[6] = 0;

   dw[7] = 0;

   dw[8] =
      __gen_float(values->XMinClipGuardband);

   dw[9] =
      __gen_float(values->XMaxClipGuardband);

   dw[10] =
      __gen_float(values->YMinClipGuardband);

   dw[11] =
      __gen_float(values->YMaxClipGuardband);

   dw[12] = 0;

   dw[13] = 0;

   dw[14] = 0;

   dw[15] = 0;
}

#define GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_length      1
struct GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL {
   uint32_t                             SourceAttribute;
   uint32_t                             SwizzleSelect;
#define INPUTATTR                                0
#define INPUTATTR_FACING                         1
#define INPUTATTR_W                              2
#define INPUTATTR_FACING_W                       3
   uint32_t                             ConstantSource;
#define CONST_0000                               0
#define CONST_0001_FLOAT                         1
#define CONST_1111_FLOAT                         2
#define PRIM_ID                                  3
   uint32_t                             SwizzleControlMode;
   bool                                 ComponentOverrideX;
   bool                                 ComponentOverrideY;
   bool                                 ComponentOverrideZ;
   bool                                 ComponentOverrideW;
};

static inline __attribute__((always_inline)) void
GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SourceAttribute, 0, 4) |
      __gen_uint(values->SwizzleSelect, 6, 7) |
      __gen_uint(values->ConstantSource, 9, 10) |
      __gen_uint(values->SwizzleControlMode, 11, 11) |
      __gen_uint(values->ComponentOverrideX, 12, 12) |
      __gen_uint(values->ComponentOverrideY, 13, 13) |
      __gen_uint(values->ComponentOverrideZ, 14, 14) |
      __gen_uint(values->ComponentOverrideW, 15, 15);
}

#define GFX7_SO_DECL_length                    1
struct GFX7_SO_DECL {
   uint32_t                             ComponentMask;
   uint32_t                             RegisterIndex;
   bool                                 HoleFlag;
   uint32_t                             OutputBufferSlot;
};

static inline __attribute__((always_inline)) void
GFX7_SO_DECL_pack(__attribute__((unused)) __gen_user_data *data,
                  __attribute__((unused)) void * restrict dst,
                  __attribute__((unused)) const struct GFX7_SO_DECL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ComponentMask, 0, 3) |
      __gen_uint(values->RegisterIndex, 4, 9) |
      __gen_uint(values->HoleFlag, 11, 11) |
      __gen_uint(values->OutputBufferSlot, 12, 13);
}

#define GFX7_SO_DECL_ENTRY_length              2
struct GFX7_SO_DECL_ENTRY {
   struct GFX7_SO_DECL                  Stream0Decl;
   struct GFX7_SO_DECL                  Stream1Decl;
   struct GFX7_SO_DECL                  Stream2Decl;
   struct GFX7_SO_DECL                  Stream3Decl;
};

static inline __attribute__((always_inline)) void
GFX7_SO_DECL_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_SO_DECL_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   uint32_t v0_0;
   GFX7_SO_DECL_pack(data, &v0_0, &values->Stream0Decl);

   uint32_t v0_1;
   GFX7_SO_DECL_pack(data, &v0_1, &values->Stream1Decl);

   dw[0] =
      __gen_uint(v0_0, 0, 15) |
      __gen_uint(v0_1, 16, 31);

   uint32_t v1_0;
   GFX7_SO_DECL_pack(data, &v1_0, &values->Stream2Decl);

   uint32_t v1_1;
   GFX7_SO_DECL_pack(data, &v1_1, &values->Stream3Decl);

   dw[1] =
      __gen_uint(v1_0, 0, 15) |
      __gen_uint(v1_1, 16, 31);
}

#define GFX7_VERTEX_BUFFER_STATE_length        4
struct GFX7_VERTEX_BUFFER_STATE {
   uint32_t                             BufferPitch;
   bool                                 VertexFetchInvalidate;
   bool                                 NullVertexBuffer;
   bool                                 AddressModifyEnable;
   uint32_t                             MOCS;
   uint32_t                             BufferAccessType;
#define VERTEXDATA                               0
#define INSTANCEDATA                             1
   uint32_t                             VertexBufferIndex;
   __gen_address_type                   BufferStartingAddress;
   __gen_address_type                   EndAddress;
   uint32_t                             InstanceDataStepRate;
};

static inline __attribute__((always_inline)) void
GFX7_VERTEX_BUFFER_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_VERTEX_BUFFER_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BufferPitch, 0, 11) |
      __gen_uint(values->VertexFetchInvalidate, 12, 12) |
      __gen_uint(values->NullVertexBuffer, 13, 13) |
      __gen_uint(values->AddressModifyEnable, 14, 14) |
      __gen_uint(values->MOCS, 16, 19) |
      __gen_uint(values->BufferAccessType, 20, 20) |
      __gen_uint(values->VertexBufferIndex, 26, 31);

   dw[1] = __gen_address(data, &dw[1], values->BufferStartingAddress, 0, 0, 31);

   dw[2] = __gen_address(data, &dw[2], values->EndAddress, 0, 0, 31);

   dw[3] =
      __gen_uint(values->InstanceDataStepRate, 0, 31);
}

#define GFX7_VERTEX_ELEMENT_STATE_length       2
struct GFX7_VERTEX_ELEMENT_STATE {
   uint32_t                             SourceElementOffset;
   bool                                 EdgeFlagEnable;
   uint32_t                             SourceElementFormat;
   bool                                 Valid;
   uint32_t                             VertexBufferIndex;
   enum GFX7_3D_Vertex_Component_Control Component3Control;
   enum GFX7_3D_Vertex_Component_Control Component2Control;
   enum GFX7_3D_Vertex_Component_Control Component1Control;
   enum GFX7_3D_Vertex_Component_Control Component0Control;
};

static inline __attribute__((always_inline)) void
GFX7_VERTEX_ELEMENT_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_VERTEX_ELEMENT_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SourceElementOffset, 0, 11) |
      __gen_uint(values->EdgeFlagEnable, 15, 15) |
      __gen_uint(values->SourceElementFormat, 16, 24) |
      __gen_uint(values->Valid, 25, 25) |
      __gen_uint(values->VertexBufferIndex, 26, 31);

   dw[1] =
      __gen_uint(values->Component3Control, 16, 18) |
      __gen_uint(values->Component2Control, 20, 22) |
      __gen_uint(values->Component1Control, 24, 26) |
      __gen_uint(values->Component0Control, 28, 30);
}

#define GFX7_3DPRIMITIVE_length                7
#define GFX7_3DPRIMITIVE_length_bias           2
#define GFX7_3DPRIMITIVE_header                 \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      3,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DPRIMITIVE {
   uint32_t                             DWordLength;
   bool                                 PredicateEnable;
   bool                                 IndirectParameterEnable;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   enum GFX7_3D_Prim_Topo_Type          PrimitiveTopologyType;
   uint32_t                             VertexAccessType;
#define SEQUENTIAL                               0
#define RANDOM                                   1
   bool                                 EndOffsetEnable;
   uint32_t                             VertexCountPerInstance;
   uint32_t                             StartVertexLocation;
   uint32_t                             InstanceCount;
   uint32_t                             StartInstanceLocation;
   int32_t                              BaseVertexLocation;
};

static inline __attribute__((always_inline)) void
GFX7_3DPRIMITIVE_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX7_3DPRIMITIVE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->PredicateEnable, 8, 8) |
      __gen_uint(values->IndirectParameterEnable, 10, 10) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->PrimitiveTopologyType, 0, 5) |
      __gen_uint(values->VertexAccessType, 8, 8) |
      __gen_uint(values->EndOffsetEnable, 9, 9);

   dw[2] =
      __gen_uint(values->VertexCountPerInstance, 0, 31);

   dw[3] =
      __gen_uint(values->StartVertexLocation, 0, 31);

   dw[4] =
      __gen_uint(values->InstanceCount, 0, 31);

   dw[5] =
      __gen_uint(values->StartInstanceLocation, 0, 31);

   dw[6] =
      __gen_sint(values->BaseVertexLocation, 0, 31);
}

#define GFX7_3DSTATE_AA_LINE_PARAMETERS_length      3
#define GFX7_3DSTATE_AA_LINE_PARAMETERS_length_bias      2
#define GFX7_3DSTATE_AA_LINE_PARAMETERS_header  \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     10,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_AA_LINE_PARAMETERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   float                                AACoverageSlope;
   float                                AACoverageBias;
   float                                AACoverageEndCapSlope;
   float                                AACoverageEndCapBias;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_AA_LINE_PARAMETERS_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX7_3DSTATE_AA_LINE_PARAMETERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_ufixed(values->AACoverageSlope, 0, 7, 8) |
      __gen_ufixed(values->AACoverageBias, 16, 23, 8);

   dw[2] =
      __gen_ufixed(values->AACoverageEndCapSlope, 0, 7, 8) |
      __gen_ufixed(values->AACoverageEndCapBias, 16, 23, 8);
}

#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS_length      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS_length_bias      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     40,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoDSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoDSBindingTable, 5, 15);
}

#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS_length      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS_length_bias      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     41,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoGSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoGSBindingTable, 5, 15);
}

#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS_length      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS_length_bias      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     39,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoHSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoHSBindingTable, 5, 15);
}

#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS_length      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS_length_bias      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     42,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoPSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoPSBindingTable, 5, 15);
}

#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS_length      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS_length_bias      2
#define GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     38,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoVSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_BINDING_TABLE_POINTERS_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoVSBindingTable, 5, 15);
}

#define GFX7_3DSTATE_BLEND_STATE_POINTERS_length      2
#define GFX7_3DSTATE_BLEND_STATE_POINTERS_length_bias      2
#define GFX7_3DSTATE_BLEND_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     36,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_BLEND_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             BlendStatePointer;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_BLEND_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX7_3DSTATE_BLEND_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_mbo(0, 0) |
      __gen_offset(values->BlendStatePointer, 6, 31);
}

#define GFX7_3DSTATE_CC_STATE_POINTERS_length      2
#define GFX7_3DSTATE_CC_STATE_POINTERS_length_bias      2
#define GFX7_3DSTATE_CC_STATE_POINTERS_header   \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     14,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CC_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             ColorCalcStatePointer;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CC_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX7_3DSTATE_CC_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_mbo(0, 0) |
      __gen_offset(values->ColorCalcStatePointer, 6, 31);
}

#define GFX7_3DSTATE_CHROMA_KEY_length         4
#define GFX7_3DSTATE_CHROMA_KEY_length_bias      2
#define GFX7_3DSTATE_CHROMA_KEY_header          \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CHROMA_KEY {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ChromaKeyTableIndex;
   uint32_t                             ChromaKeyLowValue;
   uint32_t                             ChromaKeyHighValue;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CHROMA_KEY_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX7_3DSTATE_CHROMA_KEY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ChromaKeyTableIndex, 30, 31);

   dw[2] =
      __gen_uint(values->ChromaKeyLowValue, 0, 31);

   dw[3] =
      __gen_uint(values->ChromaKeyHighValue, 0, 31);
}

#define GFX7_3DSTATE_CLEAR_PARAMS_length       3
#define GFX7_3DSTATE_CLEAR_PARAMS_length_bias      2
#define GFX7_3DSTATE_CLEAR_PARAMS_header        \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CLEAR_PARAMS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             DepthClearValue;
   bool                                 DepthClearValueValid;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CLEAR_PARAMS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_3DSTATE_CLEAR_PARAMS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DepthClearValue, 0, 31);

   dw[2] =
      __gen_uint(values->DepthClearValueValid, 0, 0);
}

#define GFX7_3DSTATE_CLIP_length               4
#define GFX7_3DSTATE_CLIP_length_bias          2
#define GFX7_3DSTATE_CLIP_header                \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     18,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CLIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             UserClipDistanceCullTestEnableBitmask;
   bool                                 StatisticsEnable;
   uint32_t                             CullMode;
#define CULLMODE_BOTH                            0
#define CULLMODE_NONE                            1
#define CULLMODE_FRONT                           2
#define CULLMODE_BACK                            3
   bool                                 EarlyCullEnable;
   uint32_t                             VertexSubPixelPrecisionSelect;
   uint32_t                             FrontWinding;
   uint32_t                             TriangleFanProvokingVertexSelect;
#define Vertex0                                  0
#define Vertex1                                  1
#define Vertex2                                  2
   uint32_t                             LineStripListProvokingVertexSelect;
#define Vertex0                                  0
#define Vertex1                                  1
   uint32_t                             TriangleStripListProvokingVertexSelect;
#define Vertex0                                  0
#define Vertex1                                  1
#define Vertex2                                  2
   bool                                 NonPerspectiveBarycentricEnable;
   bool                                 PerspectiveDivideDisable;
   uint32_t                             ClipMode;
#define CLIPMODE_NORMAL                          0
#define CLIPMODE_REJECT_ALL                      3
#define CLIPMODE_ACCEPT_ALL                      4
   uint32_t                             UserClipDistanceClipTestEnableBitmask;
   bool                                 GuardbandClipTestEnable;
   bool                                 ViewportZClipTestEnable;
   bool                                 ViewportXYClipTestEnable;
   uint32_t                             APIMode;
#define APIMODE_OGL                              0
#define APIMODE_D3D                              1
   bool                                 ClipEnable;
   uint32_t                             MaximumVPIndex;
   bool                                 ForceZeroRTAIndexEnable;
   float                                MaximumPointWidth;
   float                                MinimumPointWidth;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CLIP_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_3DSTATE_CLIP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->UserClipDistanceCullTestEnableBitmask, 0, 7) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->CullMode, 16, 17) |
      __gen_uint(values->EarlyCullEnable, 18, 18) |
      __gen_uint(values->VertexSubPixelPrecisionSelect, 19, 19) |
      __gen_uint(values->FrontWinding, 20, 20);

   dw[2] =
      __gen_uint(values->TriangleFanProvokingVertexSelect, 0, 1) |
      __gen_uint(values->LineStripListProvokingVertexSelect, 2, 3) |
      __gen_uint(values->TriangleStripListProvokingVertexSelect, 4, 5) |
      __gen_uint(values->NonPerspectiveBarycentricEnable, 8, 8) |
      __gen_uint(values->PerspectiveDivideDisable, 9, 9) |
      __gen_uint(values->ClipMode, 13, 15) |
      __gen_uint(values->UserClipDistanceClipTestEnableBitmask, 16, 23) |
      __gen_uint(values->GuardbandClipTestEnable, 26, 26) |
      __gen_uint(values->ViewportZClipTestEnable, 27, 27) |
      __gen_uint(values->ViewportXYClipTestEnable, 28, 28) |
      __gen_uint(values->APIMode, 30, 30) |
      __gen_uint(values->ClipEnable, 31, 31);

   dw[3] =
      __gen_uint(values->MaximumVPIndex, 0, 3) |
      __gen_uint(values->ForceZeroRTAIndexEnable, 5, 5) |
      __gen_ufixed(values->MaximumPointWidth, 6, 16, 3) |
      __gen_ufixed(values->MinimumPointWidth, 17, 27, 3);
}

#define GFX7_3DSTATE_CONSTANT_DS_length        7
#define GFX7_3DSTATE_CONSTANT_DS_length_bias      2
#define GFX7_3DSTATE_CONSTANT_DS_header         \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     26,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CONSTANT_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX7_3DSTATE_CONSTANT_BODY    ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_DS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX7_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX7_3DSTATE_CONSTANT_GS_length        7
#define GFX7_3DSTATE_CONSTANT_GS_length_bias      2
#define GFX7_3DSTATE_CONSTANT_GS_header         \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     22,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CONSTANT_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX7_3DSTATE_CONSTANT_BODY    ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_GS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX7_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX7_3DSTATE_CONSTANT_HS_length        7
#define GFX7_3DSTATE_CONSTANT_HS_length_bias      2
#define GFX7_3DSTATE_CONSTANT_HS_header         \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     25,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CONSTANT_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX7_3DSTATE_CONSTANT_BODY    ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_HS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX7_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX7_3DSTATE_CONSTANT_PS_length        7
#define GFX7_3DSTATE_CONSTANT_PS_length_bias      2
#define GFX7_3DSTATE_CONSTANT_PS_header         \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     23,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CONSTANT_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX7_3DSTATE_CONSTANT_BODY    ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_PS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX7_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX7_3DSTATE_CONSTANT_VS_length        7
#define GFX7_3DSTATE_CONSTANT_VS_length_bias      2
#define GFX7_3DSTATE_CONSTANT_VS_header         \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     21,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_CONSTANT_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX7_3DSTATE_CONSTANT_BODY    ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_CONSTANT_VS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_CONSTANT_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX7_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX7_3DSTATE_DEPTH_BUFFER_length       7
#define GFX7_3DSTATE_DEPTH_BUFFER_length_bias      2
#define GFX7_3DSTATE_DEPTH_BUFFER_header        \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =      5,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_DEPTH_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             SurfaceFormat;
#define D32_FLOAT                                1
#define D24_UNORM_X8_UINT                        3
#define D16_UNORM                                5
   bool                                 HierarchicalDepthBufferEnable;
   bool                                 StencilWriteEnable;
   bool                                 DepthWriteEnable;
   uint32_t                             SurfaceType;
#define SURFTYPE_1D                              0
#define SURFTYPE_2D                              1
#define SURFTYPE_3D                              2
#define SURFTYPE_CUBE                            3
#define SURFTYPE_NULL                            7
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             LOD;
   uint32_t                             Width;
   uint32_t                             Height;
   uint32_t                             MOCS;
   uint32_t                             MinimumArrayElement;
   uint32_t                             Depth;
#define SURFTYPE_CUBEmustbezero                  0
   int32_t                              DepthCoordinateOffsetX;
   int32_t                              DepthCoordinateOffsetY;
   uint32_t                             RenderTargetViewExtent;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_DEPTH_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_3DSTATE_DEPTH_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SurfacePitch, 0, 17) |
      __gen_uint(values->SurfaceFormat, 18, 20) |
      __gen_uint(values->HierarchicalDepthBufferEnable, 22, 22) |
      __gen_uint(values->StencilWriteEnable, 27, 27) |
      __gen_uint(values->DepthWriteEnable, 28, 28) |
      __gen_uint(values->SurfaceType, 29, 31);

   dw[2] = __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 31);

   dw[3] =
      __gen_uint(values->LOD, 0, 3) |
      __gen_uint(values->Width, 4, 17) |
      __gen_uint(values->Height, 18, 31);

   dw[4] =
      __gen_uint(values->MOCS, 0, 3) |
      __gen_uint(values->MinimumArrayElement, 10, 20) |
      __gen_uint(values->Depth, 21, 31);

   dw[5] =
      __gen_sint(values->DepthCoordinateOffsetX, 0, 15) |
      __gen_sint(values->DepthCoordinateOffsetY, 16, 31);

   dw[6] =
      __gen_uint(values->RenderTargetViewExtent, 21, 31);
}

#define GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS_length      2
#define GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS_length_bias      2
#define GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     37,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoDEPTH_STENCIL_STATE;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                               __attribute__((unused)) void * restrict dst,
                                               __attribute__((unused)) const struct GFX7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_mbo(0, 0) |
      __gen_offset(values->PointertoDEPTH_STENCIL_STATE, 6, 31);
}

#define GFX7_3DSTATE_DRAWING_RECTANGLE_length      4
#define GFX7_3DSTATE_DRAWING_RECTANGLE_length_bias      2
#define GFX7_3DSTATE_DRAWING_RECTANGLE_header   \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_DRAWING_RECTANGLE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ClippedDrawingRectangleXMin;
   uint32_t                             ClippedDrawingRectangleYMin;
   uint32_t                             ClippedDrawingRectangleXMax;
   uint32_t                             ClippedDrawingRectangleYMax;
   int32_t                              DrawingRectangleOriginX;
   int32_t                              DrawingRectangleOriginY;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_DRAWING_RECTANGLE_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX7_3DSTATE_DRAWING_RECTANGLE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ClippedDrawingRectangleXMin, 0, 15) |
      __gen_uint(values->ClippedDrawingRectangleYMin, 16, 31);

   dw[2] =
      __gen_uint(values->ClippedDrawingRectangleXMax, 0, 15) |
      __gen_uint(values->ClippedDrawingRectangleYMax, 16, 31);

   dw[3] =
      __gen_sint(values->DrawingRectangleOriginX, 0, 15) |
      __gen_sint(values->DrawingRectangleOriginY, 16, 31);
}

#define GFX7_3DSTATE_DS_length                 6
#define GFX7_3DSTATE_DS_length_bias            2
#define GFX7_3DSTATE_DS_header                  \
   .DWordLength                         =      4,  \
   ._3DCommandSubOpcode                 =     29,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   bool                                 VectorMaskEnable;
   uint32_t                             SingleDomainPointDispatch;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             PatchURBEntryReadOffset;
   uint32_t                             PatchURBEntryReadLength;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 Enable;
   bool                                 DSCacheDisable;
   bool                                 ComputeWCoordinateEnable;
   bool                                 StatisticsEnable;
   uint32_t                             MaximumNumberofThreads;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_DS_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->KernelStartPointer, 6, 31);

   dw[2] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleDomainPointDispatch, 31, 31);

   const uint32_t v3 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[3] = __gen_address(data, &dw[3], values->ScratchSpaceBasePointer, v3, 10, 31);

   dw[4] =
      __gen_uint(values->PatchURBEntryReadOffset, 4, 9) |
      __gen_uint(values->PatchURBEntryReadLength, 11, 17) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 20, 24);

   dw[5] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->DSCacheDisable, 1, 1) |
      __gen_uint(values->ComputeWCoordinateEnable, 2, 2) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->MaximumNumberofThreads, 25, 31);
}

#define GFX7_3DSTATE_GS_length                 7
#define GFX7_3DSTATE_GS_length_bias            2
#define GFX7_3DSTATE_GS_header                  \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     17,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 MaskStackExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define alternate                                1
   uint32_t                             ThreadPriority;
#define NormalPriority                           0
#define HighPriority                             1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   bool                                 VectorMaskEnable;
   bool                                 SingleProgramFlow;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   uint32_t                             VertexURBEntryReadOffset;
   bool                                 IncludeVertexHandles;
   uint32_t                             VertexURBEntryReadLength;
   enum GFX7_3D_Prim_Topo_Type          OutputTopology;
   uint32_t                             OutputVertexSize;
   bool                                 Enable;
   bool                                 DiscardAdjacency;
   uint32_t                             ReorderMode;
#define LEADING                                  0
#define TRAILING                                 1
   uint32_t                             Hint;
   bool                                 IncludePrimitiveID;
   uint32_t                             GSInvocationsIncrementValue;
   uint32_t                             StatisticsEnable;
   uint32_t                             DispatchMode;
#define DISPATCH_MODE_SINGLE                     0
#define DISPATCH_MODE_DUAL_INSTANCE              1
#define DISPATCH_MODE_DUAL_OBJECT                2
   uint32_t                             DefaultStreamID;
   uint32_t                             InstanceControl;
   uint32_t                             ControlDataHeaderSize;
   uint32_t                             ControlDataFormat;
#define GSCTL_CUT                                0
#define GSCTL_SID                                1
   uint32_t                             MaximumNumberofThreads;
   uint64_t                             SemaphoreHandle;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_GS_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->KernelStartPointer, 6, 31);

   dw[2] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleProgramFlow, 31, 31);

   const uint32_t v3 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[3] = __gen_address(data, &dw[3], values->ScratchSpaceBasePointer, v3, 10, 31);

   dw[4] =
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 0, 3) |
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->IncludeVertexHandles, 10, 10) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->OutputTopology, 17, 22) |
      __gen_uint(values->OutputVertexSize, 23, 28);

   dw[5] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->DiscardAdjacency, 1, 1) |
      __gen_uint(values->ReorderMode, 2, 2) |
      __gen_uint(values->Hint, 3, 3) |
      __gen_uint(values->IncludePrimitiveID, 4, 4) |
      __gen_uint(values->GSInvocationsIncrementValue, 5, 9) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->DispatchMode, 11, 12) |
      __gen_uint(values->DefaultStreamID, 13, 14) |
      __gen_uint(values->InstanceControl, 15, 19) |
      __gen_uint(values->ControlDataHeaderSize, 20, 23) |
      __gen_uint(values->ControlDataFormat, 24, 24) |
      __gen_uint(values->MaximumNumberofThreads, 25, 31);

   dw[6] =
      __gen_offset(values->SemaphoreHandle, 0, 11);
}

#define GFX7_3DSTATE_HIER_DEPTH_BUFFER_length      3
#define GFX7_3DSTATE_HIER_DEPTH_BUFFER_length_bias      2
#define GFX7_3DSTATE_HIER_DEPTH_BUFFER_header   \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      7,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_HIER_DEPTH_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             MOCS;
   __gen_address_type                   SurfaceBaseAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_HIER_DEPTH_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX7_3DSTATE_HIER_DEPTH_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SurfacePitch, 0, 16) |
      __gen_uint(values->MOCS, 25, 28);

   dw[2] = __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 31);
}

#define GFX7_3DSTATE_HS_length                 7
#define GFX7_3DSTATE_HS_length_bias            2
#define GFX7_3DSTATE_HS_header                  \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     27,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MaximumNumberofThreads;
   bool                                 SoftwareExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define alternate                                1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   uint32_t                             InstanceCount;
   bool                                 StatisticsEnable;
   bool                                 Enable;
   uint64_t                             KernelStartPointer;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 IncludeVertexHandles;
   bool                                 VectorMaskEnable;
   bool                                 SingleProgramFlow;
   uint64_t                             SemaphoreHandle;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_HS_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->MaximumNumberofThreads, 0, 6) |
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29);

   dw[2] =
      __gen_uint(values->InstanceCount, 0, 3) |
      __gen_uint(values->StatisticsEnable, 29, 29) |
      __gen_uint(values->Enable, 31, 31);

   dw[3] =
      __gen_offset(values->KernelStartPointer, 6, 31);

   const uint32_t v4 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[4] = __gen_address(data, &dw[4], values->ScratchSpaceBasePointer, v4, 10, 31);

   dw[5] =
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 19, 23) |
      __gen_uint(values->IncludeVertexHandles, 24, 24) |
      __gen_uint(values->VectorMaskEnable, 26, 26) |
      __gen_uint(values->SingleProgramFlow, 27, 27);

   dw[6] =
      __gen_offset(values->SemaphoreHandle, 0, 11);
}

#define GFX7_3DSTATE_INDEX_BUFFER_length       3
#define GFX7_3DSTATE_INDEX_BUFFER_length_bias      2
#define GFX7_3DSTATE_INDEX_BUFFER_header        \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     10,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_INDEX_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             IndexFormat;
#define INDEX_BYTE                               0
#define INDEX_WORD                               1
#define INDEX_DWORD                              2
   bool                                 CutIndexEnable;
   uint32_t                             MOCS;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   __gen_address_type                   BufferStartingAddress;
   __gen_address_type                   BufferEndingAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_INDEX_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_3DSTATE_INDEX_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->IndexFormat, 8, 9) |
      __gen_uint(values->CutIndexEnable, 10, 10) |
      __gen_uint(values->MOCS, 12, 15) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] = __gen_address(data, &dw[1], values->BufferStartingAddress, 0, 0, 31);

   dw[2] = __gen_address(data, &dw[2], values->BufferEndingAddress, 0, 0, 31);
}

#define GFX7_3DSTATE_LINE_STIPPLE_length       3
#define GFX7_3DSTATE_LINE_STIPPLE_length_bias      2
#define GFX7_3DSTATE_LINE_STIPPLE_header        \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      8,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_LINE_STIPPLE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             LineStipplePattern;
   uint32_t                             CurrentStippleIndex;
   uint32_t                             CurrentRepeatCounter;
   bool                                 ModifyEnableCurrentRepeatCounterCurrentStippleIndex;
   uint32_t                             LineStippleRepeatCount;
   float                                LineStippleInverseRepeatCount;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_LINE_STIPPLE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_3DSTATE_LINE_STIPPLE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->LineStipplePattern, 0, 15) |
      __gen_uint(values->CurrentStippleIndex, 16, 19) |
      __gen_uint(values->CurrentRepeatCounter, 21, 29) |
      __gen_uint(values->ModifyEnableCurrentRepeatCounterCurrentStippleIndex, 31, 31);

   dw[2] =
      __gen_uint(values->LineStippleRepeatCount, 0, 8) |
      __gen_ufixed(values->LineStippleInverseRepeatCount, 15, 31, 16);
}

#define GFX7_3DSTATE_MONOFILTER_SIZE_length      2
#define GFX7_3DSTATE_MONOFILTER_SIZE_length_bias      2
#define GFX7_3DSTATE_MONOFILTER_SIZE_header     \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     17,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_MONOFILTER_SIZE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MonochromeFilterHeight;
   uint32_t                             MonochromeFilterWidth;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_MONOFILTER_SIZE_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_3DSTATE_MONOFILTER_SIZE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->MonochromeFilterHeight, 0, 2) |
      __gen_uint(values->MonochromeFilterWidth, 3, 5);
}

#define GFX7_3DSTATE_MULTISAMPLE_length        4
#define GFX7_3DSTATE_MULTISAMPLE_length_bias      2
#define GFX7_3DSTATE_MULTISAMPLE_header         \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     13,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_MULTISAMPLE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             NumberofMultisamples;
#define NUMSAMPLES_1                             0
#define NUMSAMPLES_4                             2
#define NUMSAMPLES_8                             3
   uint32_t                             PixelLocation;
#define CENTER                                   0
#define UL_CORNER                                1
   float                                Sample0YOffset;
   float                                Sample0XOffset;
   float                                Sample1YOffset;
   float                                Sample1XOffset;
   float                                Sample2YOffset;
   float                                Sample2XOffset;
   float                                Sample3YOffset;
   float                                Sample3XOffset;
   float                                Sample4YOffset;
   float                                Sample4XOffset;
   float                                Sample5YOffset;
   float                                Sample5XOffset;
   float                                Sample6YOffset;
   float                                Sample6XOffset;
   float                                Sample7YOffset;
   float                                Sample7XOffset;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_MULTISAMPLE_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_MULTISAMPLE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->NumberofMultisamples, 1, 3) |
      __gen_uint(values->PixelLocation, 4, 4);

   dw[2] =
      __gen_ufixed(values->Sample0YOffset, 0, 3, 4) |
      __gen_ufixed(values->Sample0XOffset, 4, 7, 4) |
      __gen_ufixed(values->Sample1YOffset, 8, 11, 4) |
      __gen_ufixed(values->Sample1XOffset, 12, 15, 4) |
      __gen_ufixed(values->Sample2YOffset, 16, 19, 4) |
      __gen_ufixed(values->Sample2XOffset, 20, 23, 4) |
      __gen_ufixed(values->Sample3YOffset, 24, 27, 4) |
      __gen_ufixed(values->Sample3XOffset, 28, 31, 4);

   dw[3] =
      __gen_ufixed(values->Sample4YOffset, 0, 3, 4) |
      __gen_ufixed(values->Sample4XOffset, 4, 7, 4) |
      __gen_ufixed(values->Sample5YOffset, 8, 11, 4) |
      __gen_ufixed(values->Sample5XOffset, 12, 15, 4) |
      __gen_ufixed(values->Sample6YOffset, 16, 19, 4) |
      __gen_ufixed(values->Sample6XOffset, 20, 23, 4) |
      __gen_ufixed(values->Sample7YOffset, 24, 27, 4) |
      __gen_ufixed(values->Sample7XOffset, 28, 31, 4);
}

#define GFX7_3DSTATE_POLY_STIPPLE_OFFSET_length      2
#define GFX7_3DSTATE_POLY_STIPPLE_OFFSET_length_bias      2
#define GFX7_3DSTATE_POLY_STIPPLE_OFFSET_header \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =      6,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_POLY_STIPPLE_OFFSET {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PolygonStippleYOffset;
   uint32_t                             PolygonStippleXOffset;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_POLY_STIPPLE_OFFSET_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX7_3DSTATE_POLY_STIPPLE_OFFSET * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->PolygonStippleYOffset, 0, 4) |
      __gen_uint(values->PolygonStippleXOffset, 8, 12);
}

#define GFX7_3DSTATE_POLY_STIPPLE_PATTERN_length     33
#define GFX7_3DSTATE_POLY_STIPPLE_PATTERN_length_bias      2
#define GFX7_3DSTATE_POLY_STIPPLE_PATTERN_header\
   .DWordLength                         =     31,  \
   ._3DCommandSubOpcode                 =      7,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_POLY_STIPPLE_PATTERN {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PatternRow[32];
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_POLY_STIPPLE_PATTERN_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX7_3DSTATE_POLY_STIPPLE_PATTERN * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->PatternRow[0], 0, 31);

   dw[2] =
      __gen_uint(values->PatternRow[1], 0, 31);

   dw[3] =
      __gen_uint(values->PatternRow[2], 0, 31);

   dw[4] =
      __gen_uint(values->PatternRow[3], 0, 31);

   dw[5] =
      __gen_uint(values->PatternRow[4], 0, 31);

   dw[6] =
      __gen_uint(values->PatternRow[5], 0, 31);

   dw[7] =
      __gen_uint(values->PatternRow[6], 0, 31);

   dw[8] =
      __gen_uint(values->PatternRow[7], 0, 31);

   dw[9] =
      __gen_uint(values->PatternRow[8], 0, 31);

   dw[10] =
      __gen_uint(values->PatternRow[9], 0, 31);

   dw[11] =
      __gen_uint(values->PatternRow[10], 0, 31);

   dw[12] =
      __gen_uint(values->PatternRow[11], 0, 31);

   dw[13] =
      __gen_uint(values->PatternRow[12], 0, 31);

   dw[14] =
      __gen_uint(values->PatternRow[13], 0, 31);

   dw[15] =
      __gen_uint(values->PatternRow[14], 0, 31);

   dw[16] =
      __gen_uint(values->PatternRow[15], 0, 31);

   dw[17] =
      __gen_uint(values->PatternRow[16], 0, 31);

   dw[18] =
      __gen_uint(values->PatternRow[17], 0, 31);

   dw[19] =
      __gen_uint(values->PatternRow[18], 0, 31);

   dw[20] =
      __gen_uint(values->PatternRow[19], 0, 31);

   dw[21] =
      __gen_uint(values->PatternRow[20], 0, 31);

   dw[22] =
      __gen_uint(values->PatternRow[21], 0, 31);

   dw[23] =
      __gen_uint(values->PatternRow[22], 0, 31);

   dw[24] =
      __gen_uint(values->PatternRow[23], 0, 31);

   dw[25] =
      __gen_uint(values->PatternRow[24], 0, 31);

   dw[26] =
      __gen_uint(values->PatternRow[25], 0, 31);

   dw[27] =
      __gen_uint(values->PatternRow[26], 0, 31);

   dw[28] =
      __gen_uint(values->PatternRow[27], 0, 31);

   dw[29] =
      __gen_uint(values->PatternRow[28], 0, 31);

   dw[30] =
      __gen_uint(values->PatternRow[29], 0, 31);

   dw[31] =
      __gen_uint(values->PatternRow[30], 0, 31);

   dw[32] =
      __gen_uint(values->PatternRow[31], 0, 31);
}

#define GFX7_3DSTATE_PS_length                 8
#define GFX7_3DSTATE_PS_length_bias            2
#define GFX7_3DSTATE_PS_header                  \
   .DWordLength                         =      6,  \
   ._3DCommandSubOpcode                 =     32,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer0;
   bool                                 SoftwareExceptionEnable;
   bool                                 MaskStackExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             RoundingMode;
#define RTNE                                     0
#define RU                                       1
#define RD                                       2
#define RTZ                                      3
   uint32_t                             FloatingPointMode;
#define IEEE745                                  0
#define Alt                                      1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             DenormalMode;
#define FTZ                                      0
#define RET                                      1
   uint32_t                             SamplerCount;
   bool                                 VectorMaskEnable;
   bool                                 SingleProgramFlow;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   bool                                 _8PixelDispatchEnable;
   bool                                 _16PixelDispatchEnable;
   bool                                 _32PixelDispatchEnable;
   uint32_t                             PositionXYOffsetSelect;
#define POSOFFSET_NONE                           0
#define POSOFFSET_CENTROID                       2
#define POSOFFSET_SAMPLE                         3
   bool                                 RenderTargetResolveEnable;
   bool                                 DualSourceBlendEnable;
   bool                                 RenderTargetFastClearEnable;
   bool                                 oMaskPresenttoRenderTarget;
   bool                                 AttributeEnable;
   bool                                 PushConstantEnable;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData2;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData1;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData0;
   uint64_t                             KernelStartPointer1;
   uint64_t                             KernelStartPointer2;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PS_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->KernelStartPointer0, 6, 31);

   dw[2] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->RoundingMode, 14, 15) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->DenormalMode, 26, 26) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleProgramFlow, 31, 31);

   const uint32_t v3 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[3] = __gen_address(data, &dw[3], values->ScratchSpaceBasePointer, v3, 10, 31);

   dw[4] =
      __gen_uint(values->_8PixelDispatchEnable, 0, 0) |
      __gen_uint(values->_16PixelDispatchEnable, 1, 1) |
      __gen_uint(values->_32PixelDispatchEnable, 2, 2) |
      __gen_uint(values->PositionXYOffsetSelect, 3, 4) |
      __gen_uint(values->RenderTargetResolveEnable, 6, 6) |
      __gen_uint(values->DualSourceBlendEnable, 7, 7) |
      __gen_uint(values->RenderTargetFastClearEnable, 8, 8) |
      __gen_uint(values->oMaskPresenttoRenderTarget, 9, 9) |
      __gen_uint(values->AttributeEnable, 10, 10) |
      __gen_uint(values->PushConstantEnable, 11, 11) |
      __gen_uint(values->MaximumNumberofThreads, 24, 31);

   dw[5] =
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData2, 0, 6) |
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData1, 8, 14) |
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData0, 16, 22);

   dw[6] =
      __gen_offset(values->KernelStartPointer1, 6, 31);

   dw[7] =
      __gen_offset(values->KernelStartPointer2, 6, 31);
}

#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS_length      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS_length_bias      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     20,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
#define _0KB                                     0
   uint32_t                             ConstantBufferOffset;
#define _0KB                                     0
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 4) |
      __gen_uint(values->ConstantBufferOffset, 16, 19);
}

#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS_length      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS_length_bias      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     21,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
#define _0KB                                     0
   uint32_t                             ConstantBufferOffset;
#define _0KB                                     0
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 4) |
      __gen_uint(values->ConstantBufferOffset, 16, 19);
}

#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS_length      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS_length_bias      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     19,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
#define _0KB                                     0
   uint32_t                             ConstantBufferOffset;
#define _0KB                                     0
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 4) |
      __gen_uint(values->ConstantBufferOffset, 16, 19);
}

#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS_length      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS_length_bias      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     22,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
#define _0KB                                     0
   uint32_t                             ConstantBufferOffset;
#define _0KB                                     0
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 4) |
      __gen_uint(values->ConstantBufferOffset, 16, 19);
}

#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS_length      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS_length_bias      2
#define GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     18,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
#define _0KB                                     0
   uint32_t                             ConstantBufferOffset;
#define _0KB                                     0
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_PUSH_CONSTANT_ALLOC_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 4) |
      __gen_uint(values->ConstantBufferOffset, 16, 19);
}

#define GFX7_3DSTATE_SAMPLER_PALETTE_LOAD0_length_bias      2
#define GFX7_3DSTATE_SAMPLER_PALETTE_LOAD0_header\
   ._3DCommandSubOpcode                 =      2,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_PALETTE_LOAD0 {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_PALETTE_LOAD0_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_PALETTE_LOAD0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_3DSTATE_SAMPLER_PALETTE_LOAD1_length_bias      2
#define GFX7_3DSTATE_SAMPLER_PALETTE_LOAD1_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     12,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_PALETTE_LOAD1 {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_PALETTE_LOAD1_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_PALETTE_LOAD1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS_length      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS_length_bias      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     45,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoDSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoDSSamplerState, 5, 31);
}

#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS_length      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS_length_bias      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     46,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoGSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoGSSamplerState, 5, 31);
}

#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS_length      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS_length_bias      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     44,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoHSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoHSSamplerState, 5, 31);
}

#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS_length      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS_length_bias      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     47,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoPSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoPSSamplerState, 5, 31);
}

#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS_length      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS_length_bias      2
#define GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     43,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoVSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLER_STATE_POINTERS_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->PointertoVSSamplerState, 5, 31);
}

#define GFX7_3DSTATE_SAMPLE_MASK_length        2
#define GFX7_3DSTATE_SAMPLE_MASK_length_bias      2
#define GFX7_3DSTATE_SAMPLE_MASK_header         \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     24,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SAMPLE_MASK {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SampleMask;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SAMPLE_MASK_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_3DSTATE_SAMPLE_MASK * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SampleMask, 0, 7);
}

#define GFX7_3DSTATE_SBE_length               14
#define GFX7_3DSTATE_SBE_length_bias           2
#define GFX7_3DSTATE_SBE_header                 \
   .DWordLength                         =     12,  \
   ._3DCommandSubOpcode                 =     31,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SBE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   uint32_t                             PointSpriteTextureCoordinateOrigin;
#define UPPERLEFT                                0
#define LOWERLEFT                                1
   bool                                 AttributeSwizzleEnable;
   uint32_t                             NumberofSFOutputAttributes;
   uint32_t                             AttributeSwizzleControlMode;
#define SWIZ_0_15                                0
#define SWIZ_16_31                               1
   struct GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL Attribute[16];
   uint32_t                             PointSpriteTextureCoordinateEnable;
   uint32_t                             ConstantInterpolationEnable;
   uint32_t                             Attribute0WrapShortestEnables;
   uint32_t                             Attribute1WrapShortestEnables;
   uint32_t                             Attribute2WrapShortestEnables;
   uint32_t                             Attribute3WrapShortestEnables;
   uint32_t                             Attribute4WrapShortestEnables;
   uint32_t                             Attribute5WrapShortestEnables;
   uint32_t                             Attribute6WrapShortestEnables;
   uint32_t                             Attribute7WrapShortestEnables;
   uint32_t                             Attribute8WrapShortestEnables;
   uint32_t                             Attribute9WrapShortestEnables;
   uint32_t                             Attribute10WrapShortestEnables;
   uint32_t                             Attribute11WrapShortestEnables;
   uint32_t                             Attribute12WrapShortestEnables;
   uint32_t                             Attribute13WrapShortestEnables;
   uint32_t                             Attribute14WrapShortestEnables;
   uint32_t                             Attribute15WrapShortestEnables;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SBE_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX7_3DSTATE_SBE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 15) |
      __gen_uint(values->PointSpriteTextureCoordinateOrigin, 20, 20) |
      __gen_uint(values->AttributeSwizzleEnable, 21, 21) |
      __gen_uint(values->NumberofSFOutputAttributes, 22, 27) |
      __gen_uint(values->AttributeSwizzleControlMode, 28, 28);

   uint32_t v2_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v2_0, &values->Attribute[0]);

   uint32_t v2_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v2_1, &values->Attribute[1]);

   dw[2] =
      __gen_uint(v2_0, 0, 15) |
      __gen_uint(v2_1, 16, 31);

   uint32_t v3_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v3_0, &values->Attribute[2]);

   uint32_t v3_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v3_1, &values->Attribute[3]);

   dw[3] =
      __gen_uint(v3_0, 0, 15) |
      __gen_uint(v3_1, 16, 31);

   uint32_t v4_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v4_0, &values->Attribute[4]);

   uint32_t v4_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v4_1, &values->Attribute[5]);

   dw[4] =
      __gen_uint(v4_0, 0, 15) |
      __gen_uint(v4_1, 16, 31);

   uint32_t v5_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v5_0, &values->Attribute[6]);

   uint32_t v5_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v5_1, &values->Attribute[7]);

   dw[5] =
      __gen_uint(v5_0, 0, 15) |
      __gen_uint(v5_1, 16, 31);

   uint32_t v6_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v6_0, &values->Attribute[8]);

   uint32_t v6_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v6_1, &values->Attribute[9]);

   dw[6] =
      __gen_uint(v6_0, 0, 15) |
      __gen_uint(v6_1, 16, 31);

   uint32_t v7_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v7_0, &values->Attribute[10]);

   uint32_t v7_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v7_1, &values->Attribute[11]);

   dw[7] =
      __gen_uint(v7_0, 0, 15) |
      __gen_uint(v7_1, 16, 31);

   uint32_t v8_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v8_0, &values->Attribute[12]);

   uint32_t v8_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v8_1, &values->Attribute[13]);

   dw[8] =
      __gen_uint(v8_0, 0, 15) |
      __gen_uint(v8_1, 16, 31);

   uint32_t v9_0;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v9_0, &values->Attribute[14]);

   uint32_t v9_1;
   GFX7_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v9_1, &values->Attribute[15]);

   dw[9] =
      __gen_uint(v9_0, 0, 15) |
      __gen_uint(v9_1, 16, 31);

   dw[10] =
      __gen_uint(values->PointSpriteTextureCoordinateEnable, 0, 31);

   dw[11] =
      __gen_uint(values->ConstantInterpolationEnable, 0, 31);

   dw[12] =
      __gen_uint(values->Attribute0WrapShortestEnables, 0, 3) |
      __gen_uint(values->Attribute1WrapShortestEnables, 4, 7) |
      __gen_uint(values->Attribute2WrapShortestEnables, 8, 11) |
      __gen_uint(values->Attribute3WrapShortestEnables, 12, 15) |
      __gen_uint(values->Attribute4WrapShortestEnables, 16, 19) |
      __gen_uint(values->Attribute5WrapShortestEnables, 20, 23) |
      __gen_uint(values->Attribute6WrapShortestEnables, 24, 27) |
      __gen_uint(values->Attribute7WrapShortestEnables, 28, 31);

   dw[13] =
      __gen_uint(values->Attribute8WrapShortestEnables, 0, 3) |
      __gen_uint(values->Attribute9WrapShortestEnables, 4, 7) |
      __gen_uint(values->Attribute10WrapShortestEnables, 8, 11) |
      __gen_uint(values->Attribute11WrapShortestEnables, 12, 15) |
      __gen_uint(values->Attribute12WrapShortestEnables, 16, 19) |
      __gen_uint(values->Attribute13WrapShortestEnables, 20, 23) |
      __gen_uint(values->Attribute14WrapShortestEnables, 24, 27) |
      __gen_uint(values->Attribute15WrapShortestEnables, 28, 31);
}

#define GFX7_3DSTATE_SCISSOR_STATE_POINTERS_length      2
#define GFX7_3DSTATE_SCISSOR_STATE_POINTERS_length_bias      2
#define GFX7_3DSTATE_SCISSOR_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     15,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SCISSOR_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             ScissorRectPointer;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SCISSOR_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX7_3DSTATE_SCISSOR_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->ScissorRectPointer, 5, 31);
}

#define GFX7_3DSTATE_SF_length                 7
#define GFX7_3DSTATE_SF_length_bias            2
#define GFX7_3DSTATE_SF_header                  \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =     19,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SF {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             FrontWinding;
   bool                                 ViewportTransformEnable;
   uint32_t                             BackFaceFillMode;
#define FILL_MODE_SOLID                          0
#define FILL_MODE_WIREFRAME                      1
#define FILL_MODE_POINT                          2
   uint32_t                             FrontFaceFillMode;
#define FILL_MODE_SOLID                          0
#define FILL_MODE_WIREFRAME                      1
#define FILL_MODE_POINT                          2
   bool                                 GlobalDepthOffsetEnablePoint;
   bool                                 GlobalDepthOffsetEnableWireframe;
   bool                                 GlobalDepthOffsetEnableSolid;
   bool                                 StatisticsEnable;
   bool                                 LegacyGlobalDepthBiasEnable;
   uint32_t                             DepthBufferSurfaceFormat;
#define D32_FLOAT_S8X24_UINT                     0
#define D32_FLOAT                                1
#define D24_UNORM_S8_UINT                        2
#define D24_UNORM_X8_UINT                        3
#define D16_UNORM                                5
   uint32_t                             MultisampleRasterizationMode;
#define MSRASTMODE_OFF_PIXEL                     0
#define MSRASTMODE_OFF_PATTERN                   1
#define MSRASTMODE_ON_PIXEL                      2
#define MSRASTMODE_ON_PATTERN                    3
   bool                                 ScissorRectangleEnable;
   uint32_t                             LineEndCapAntialiasingRegionWidth;
#define _05pixels                                0
#define _10pixels                                1
#define _20pixels                                2
#define _40pixels                                3
   float                                LineWidth;
   uint32_t                             CullMode;
#define CULLMODE_BOTH                            0
#define CULLMODE_NONE                            1
#define CULLMODE_FRONT                           2
#define CULLMODE_BACK                            3
   bool                                 AntialiasingEnable;
   float                                PointWidth;
   uint32_t                             PointWidthSource;
#define Vertex                                   0
#define State                                    1
   uint32_t                             VertexSubPixelPrecisionSelect;
#define _8Bit                                    0
#define _4Bit                                    1
   uint32_t                             AALineDistanceMode;
#define AALINEDISTANCE_TRUE                      1
   uint32_t                             TriangleFanProvokingVertexSelect;
#define Vertex0                                  0
#define Vertex1                                  1
#define Vertex2                                  2
   uint32_t                             LineStripListProvokingVertexSelect;
   uint32_t                             TriangleStripListProvokingVertexSelect;
#define Vertex0                                  0
#define Vertex1                                  1
#define Vertex2                                  2
   bool                                 LastPixelEnable;
   float                                GlobalDepthOffsetConstant;
   float                                GlobalDepthOffsetScale;
   float                                GlobalDepthOffsetClamp;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SF_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_SF * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->FrontWinding, 0, 0) |
      __gen_uint(values->ViewportTransformEnable, 1, 1) |
      __gen_uint(values->BackFaceFillMode, 3, 4) |
      __gen_uint(values->FrontFaceFillMode, 5, 6) |
      __gen_uint(values->GlobalDepthOffsetEnablePoint, 7, 7) |
      __gen_uint(values->GlobalDepthOffsetEnableWireframe, 8, 8) |
      __gen_uint(values->GlobalDepthOffsetEnableSolid, 9, 9) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->LegacyGlobalDepthBiasEnable, 11, 11) |
      __gen_uint(values->DepthBufferSurfaceFormat, 12, 14);

   dw[2] =
      __gen_uint(values->MultisampleRasterizationMode, 8, 9) |
      __gen_uint(values->ScissorRectangleEnable, 11, 11) |
      __gen_uint(values->LineEndCapAntialiasingRegionWidth, 16, 17) |
      __gen_ufixed(values->LineWidth, 18, 27, 7) |
      __gen_uint(values->CullMode, 29, 30) |
      __gen_uint(values->AntialiasingEnable, 31, 31);

   dw[3] =
      __gen_ufixed(values->PointWidth, 0, 10, 3) |
      __gen_uint(values->PointWidthSource, 11, 11) |
      __gen_uint(values->VertexSubPixelPrecisionSelect, 12, 12) |
      __gen_uint(values->AALineDistanceMode, 14, 14) |
      __gen_uint(values->TriangleFanProvokingVertexSelect, 25, 26) |
      __gen_uint(values->LineStripListProvokingVertexSelect, 27, 28) |
      __gen_uint(values->TriangleStripListProvokingVertexSelect, 29, 30) |
      __gen_uint(values->LastPixelEnable, 31, 31);

   dw[4] =
      __gen_float(values->GlobalDepthOffsetConstant);

   dw[5] =
      __gen_float(values->GlobalDepthOffsetScale);

   dw[6] =
      __gen_float(values->GlobalDepthOffsetClamp);
}

#define GFX7_3DSTATE_SO_BUFFER_length          4
#define GFX7_3DSTATE_SO_BUFFER_length_bias      2
#define GFX7_3DSTATE_SO_BUFFER_header           \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     24,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SO_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             MOCS;
   uint32_t                             SOBufferIndex;
   __gen_address_type                   SurfaceBaseAddress;
   __gen_address_type                   SurfaceEndAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SO_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_3DSTATE_SO_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SurfacePitch, 0, 11) |
      __gen_uint(values->MOCS, 25, 28) |
      __gen_uint(values->SOBufferIndex, 29, 30);

   dw[2] = __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 2, 31);

   dw[3] = __gen_address(data, &dw[3], values->SurfaceEndAddress, 0, 2, 31);
}

#define GFX7_3DSTATE_SO_DECL_LIST_length_bias      2
#define GFX7_3DSTATE_SO_DECL_LIST_header        \
   ._3DCommandSubOpcode                 =     23,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_SO_DECL_LIST {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             StreamtoBufferSelects0;
   uint32_t                             StreamtoBufferSelects1;
   uint32_t                             StreamtoBufferSelects2;
   uint32_t                             StreamtoBufferSelects3;
   uint32_t                             NumEntries0;
   uint32_t                             NumEntries1;
   uint32_t                             NumEntries2;
   uint32_t                             NumEntries3;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_SO_DECL_LIST_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_3DSTATE_SO_DECL_LIST * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->StreamtoBufferSelects0, 0, 3) |
      __gen_uint(values->StreamtoBufferSelects1, 4, 7) |
      __gen_uint(values->StreamtoBufferSelects2, 8, 11) |
      __gen_uint(values->StreamtoBufferSelects3, 12, 15);

   dw[2] =
      __gen_uint(values->NumEntries0, 0, 7) |
      __gen_uint(values->NumEntries1, 8, 15) |
      __gen_uint(values->NumEntries2, 16, 23) |
      __gen_uint(values->NumEntries3, 24, 31);
}

#define GFX7_3DSTATE_STENCIL_BUFFER_length      3
#define GFX7_3DSTATE_STENCIL_BUFFER_length_bias      2
#define GFX7_3DSTATE_STENCIL_BUFFER_header      \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      6,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_STENCIL_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             MOCS;
   __gen_address_type                   SurfaceBaseAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_STENCIL_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX7_3DSTATE_STENCIL_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SurfacePitch, 0, 16) |
      __gen_uint(values->MOCS, 25, 28);

   dw[2] = __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 31);
}

#define GFX7_3DSTATE_STREAMOUT_length          3
#define GFX7_3DSTATE_STREAMOUT_length_bias      2
#define GFX7_3DSTATE_STREAMOUT_header           \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     30,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_STREAMOUT {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 SOBufferEnable0;
   bool                                 SOBufferEnable1;
   bool                                 SOBufferEnable2;
   bool                                 SOBufferEnable3;
   bool                                 SOStatisticsEnable;
   uint32_t                             ReorderMode;
#define LEADING                                  0
#define TRAILING                                 1
   uint32_t                             RenderStreamSelect;
   bool                                 RenderingDisable;
   bool                                 SOFunctionEnable;
   uint32_t                             Stream0VertexReadLength;
   uint32_t                             Stream0VertexReadOffset;
   uint32_t                             Stream1VertexReadLength;
   uint32_t                             Stream1VertexReadOffset;
   uint32_t                             Stream2VertexReadLength;
   uint32_t                             Stream2VertexReadOffset;
   uint32_t                             Stream3VertexReadLength;
   uint32_t                             Stream3VertexReadOffset;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_STREAMOUT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_3DSTATE_STREAMOUT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SOBufferEnable0, 8, 8) |
      __gen_uint(values->SOBufferEnable1, 9, 9) |
      __gen_uint(values->SOBufferEnable2, 10, 10) |
      __gen_uint(values->SOBufferEnable3, 11, 11) |
      __gen_uint(values->SOStatisticsEnable, 25, 25) |
      __gen_uint(values->ReorderMode, 26, 26) |
      __gen_uint(values->RenderStreamSelect, 27, 28) |
      __gen_uint(values->RenderingDisable, 30, 30) |
      __gen_uint(values->SOFunctionEnable, 31, 31);

   dw[2] =
      __gen_uint(values->Stream0VertexReadLength, 0, 4) |
      __gen_uint(values->Stream0VertexReadOffset, 5, 5) |
      __gen_uint(values->Stream1VertexReadLength, 8, 12) |
      __gen_uint(values->Stream1VertexReadOffset, 13, 13) |
      __gen_uint(values->Stream2VertexReadLength, 16, 20) |
      __gen_uint(values->Stream2VertexReadOffset, 21, 21) |
      __gen_uint(values->Stream3VertexReadLength, 24, 28) |
      __gen_uint(values->Stream3VertexReadOffset, 29, 29);
}

#define GFX7_3DSTATE_TE_length                 4
#define GFX7_3DSTATE_TE_length_bias            2
#define GFX7_3DSTATE_TE_header                  \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     28,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_TE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 TEEnable;
   uint32_t                             TEMode;
#define HW_TESS                                  0
#define SW_TESS                                  1
   uint32_t                             TEDomain;
#define QUAD                                     0
#define TRI                                      1
#define ISOLINE                                  2
   uint32_t                             OutputTopology;
#define OUTPUT_POINT                             0
#define OUTPUT_LINE                              1
#define OUTPUT_TRI_CW                            2
#define OUTPUT_TRI_CCW                           3
   uint32_t                             Partitioning;
#define INTEGER                                  0
#define ODD_FRACTIONAL                           1
#define EVEN_FRACTIONAL                          2
   float                                MaximumTessellationFactorOdd;
   float                                MaximumTessellationFactorNotOdd;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_TE_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_TE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->TEEnable, 0, 0) |
      __gen_uint(values->TEMode, 1, 2) |
      __gen_uint(values->TEDomain, 4, 5) |
      __gen_uint(values->OutputTopology, 8, 9) |
      __gen_uint(values->Partitioning, 12, 13);

   dw[2] =
      __gen_float(values->MaximumTessellationFactorOdd);

   dw[3] =
      __gen_float(values->MaximumTessellationFactorNotOdd);
}

#define GFX7_3DSTATE_URB_DS_length             2
#define GFX7_3DSTATE_URB_DS_length_bias        2
#define GFX7_3DSTATE_URB_DS_header              \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     50,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_URB_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             DSNumberofURBEntries;
   uint32_t                             DSURBEntryAllocationSize;
   uint32_t                             DSURBStartingAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_URB_DS_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_3DSTATE_URB_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DSNumberofURBEntries, 0, 15) |
      __gen_uint(values->DSURBEntryAllocationSize, 16, 24) |
      __gen_uint(values->DSURBStartingAddress, 25, 29);
}

#define GFX7_3DSTATE_URB_GS_length             2
#define GFX7_3DSTATE_URB_GS_length_bias        2
#define GFX7_3DSTATE_URB_GS_header              \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     51,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_URB_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             GSNumberofURBEntries;
   uint32_t                             GSURBEntryAllocationSize;
   uint32_t                             GSURBStartingAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_URB_GS_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_3DSTATE_URB_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->GSNumberofURBEntries, 0, 15) |
      __gen_uint(values->GSURBEntryAllocationSize, 16, 24) |
      __gen_uint(values->GSURBStartingAddress, 25, 29);
}

#define GFX7_3DSTATE_URB_HS_length             2
#define GFX7_3DSTATE_URB_HS_length_bias        2
#define GFX7_3DSTATE_URB_HS_header              \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     49,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_URB_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             HSNumberofURBEntries;
   uint32_t                             HSURBEntryAllocationSize;
   uint32_t                             HSURBStartingAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_URB_HS_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_3DSTATE_URB_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->HSNumberofURBEntries, 0, 15) |
      __gen_uint(values->HSURBEntryAllocationSize, 16, 24) |
      __gen_uint(values->HSURBStartingAddress, 25, 29);
}

#define GFX7_3DSTATE_URB_VS_length             2
#define GFX7_3DSTATE_URB_VS_length_bias        2
#define GFX7_3DSTATE_URB_VS_header              \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     48,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_URB_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             VSNumberofURBEntries;
   uint32_t                             VSURBEntryAllocationSize;
   uint32_t                             VSURBStartingAddress;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_URB_VS_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_3DSTATE_URB_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->VSNumberofURBEntries, 0, 15) |
      __gen_uint(values->VSURBEntryAllocationSize, 16, 24) |
      __gen_uint(values->VSURBStartingAddress, 25, 29);
}

#define GFX7_3DSTATE_VERTEX_BUFFERS_length_bias      2
#define GFX7_3DSTATE_VERTEX_BUFFERS_header      \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =      8,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VERTEX_BUFFERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VERTEX_BUFFERS_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX7_3DSTATE_VERTEX_BUFFERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_3DSTATE_VERTEX_ELEMENTS_length_bias      2
#define GFX7_3DSTATE_VERTEX_ELEMENTS_header     \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      9,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VERTEX_ELEMENTS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VERTEX_ELEMENTS_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_3DSTATE_VERTEX_ELEMENTS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_3DSTATE_VF_STATISTICS_length      1
#define GFX7_3DSTATE_VF_STATISTICS_length_bias      1
#define GFX7_3DSTATE_VF_STATISTICS_header       \
   ._3DCommandSubOpcode                 =     11,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      1,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VF_STATISTICS {
   bool                                 StatisticsEnable;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VF_STATISTICS_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_3DSTATE_VF_STATISTICS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->StatisticsEnable, 0, 0) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC_length      2
#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC_length_bias      2
#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     35,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             CCViewportPointer;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_CC * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->CCViewportPointer, 5, 31);
}

#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_length      2
#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_length_bias      2
#define GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     33,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             SFClipViewportPointer;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_pack(__attribute__((unused)) __gen_user_data *data,
                                                  __attribute__((unused)) void * restrict dst,
                                                  __attribute__((unused)) const struct GFX7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->SFClipViewportPointer, 6, 31);
}

#define GFX7_3DSTATE_VS_length                 6
#define GFX7_3DSTATE_VS_length_bias            2
#define GFX7_3DSTATE_VS_header                  \
   .DWordLength                         =      4,  \
   ._3DCommandSubOpcode                 =     16,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   bool                                 VectorMaskEnable;
   bool                                 SingleVertexDispatch;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 Enable;
   bool                                 VertexCacheDisable;
   bool                                 StatisticsEnable;
   uint32_t                             MaximumNumberofThreads;
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_VS_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->KernelStartPointer, 6, 31);

   dw[2] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleVertexDispatch, 31, 31);

   const uint32_t v3 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[3] = __gen_address(data, &dw[3], values->ScratchSpaceBasePointer, v3, 10, 31);

   dw[4] =
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 20, 24);

   dw[5] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->VertexCacheDisable, 1, 1) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->MaximumNumberofThreads, 25, 31);
}

#define GFX7_3DSTATE_WM_length                 3
#define GFX7_3DSTATE_WM_length_bias            2
#define GFX7_3DSTATE_WM_header                  \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     20,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_3DSTATE_WM {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MultisampleRasterizationMode;
#define MSRASTMODE_OFF_PIXEL                     0
#define MSRASTMODE_OFF_PATTERN                   1
#define MSRASTMODE_ON_PIXEL                      2
#define MSRASTMODE_ON_PATTERN                    3
   uint32_t                             PointRasterizationRule;
#define RASTRULE_UPPER_LEFT                      0
#define RASTRULE_UPPER_RIGHT                     1
   bool                                 LineStippleEnable;
   bool                                 PolygonStippleEnable;
   uint32_t                             LineAntialiasingRegionWidth;
#define _05pixels                                0
#define _10pixels                                1
#define _20pixels                                2
#define _40pixels                                3
   uint32_t                             LineEndCapAntialiasingRegionWidth;
   bool                                 PixelShaderUsesInputCoverageMask;
   uint32_t                             BarycentricInterpolationMode;
#define BIM_PERSPECTIVE_PIXEL                    1
#define BIM_PERSPECTIVE_CENTROID                 2
#define BIM_PERSPECTIVE_SAMPLE                   4
#define BIM_LINEAR_PIXEL                         8
#define BIM_LINEAR_CENTROID                      16
#define BIM_LINEAR_SAMPLE                        32
   uint32_t                             PositionZWInterpolationMode;
#define INTERP_PIXEL                             0
#define INTERP_CENTROID                          2
#define INTERP_SAMPLE                            3
   bool                                 PixelShaderUsesSourceW;
   bool                                 PixelShaderUsesSourceDepth;
   uint32_t                             EarlyDepthStencilControl;
#define EDSC_NORMAL                              0
#define EDSC_PSEXEC                              1
#define EDSC_PREPS                               2
   uint32_t                             PixelShaderComputedDepthMode;
#define PSCDEPTH_OFF                             0
#define PSCDEPTH_ON                              1
#define PSCDEPTH_ON_GE                           2
#define PSCDEPTH_ON_LE                           3
   bool                                 PixelShaderKillsPixel;
   bool                                 LegacyDiamondLineRasterization;
   bool                                 HierarchicalDepthBufferResolveEnable;
   bool                                 DepthBufferResolveEnable;
   bool                                 ThreadDispatchEnable;
   bool                                 DepthBufferClear;
   bool                                 StatisticsEnable;
   uint32_t                             MultisampleDispatchMode;
#define MSDISPMODE_PERSAMPLE                     0
#define MSDISPMODE_PERPIXEL                      1
};

static inline __attribute__((always_inline)) void
GFX7_3DSTATE_WM_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_3DSTATE_WM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->MultisampleRasterizationMode, 0, 1) |
      __gen_uint(values->PointRasterizationRule, 2, 2) |
      __gen_uint(values->LineStippleEnable, 3, 3) |
      __gen_uint(values->PolygonStippleEnable, 4, 4) |
      __gen_uint(values->LineAntialiasingRegionWidth, 6, 7) |
      __gen_uint(values->LineEndCapAntialiasingRegionWidth, 8, 9) |
      __gen_uint(values->PixelShaderUsesInputCoverageMask, 10, 10) |
      __gen_uint(values->BarycentricInterpolationMode, 11, 16) |
      __gen_uint(values->PositionZWInterpolationMode, 17, 18) |
      __gen_uint(values->PixelShaderUsesSourceW, 19, 19) |
      __gen_uint(values->PixelShaderUsesSourceDepth, 20, 20) |
      __gen_uint(values->EarlyDepthStencilControl, 21, 22) |
      __gen_uint(values->PixelShaderComputedDepthMode, 23, 24) |
      __gen_uint(values->PixelShaderKillsPixel, 25, 25) |
      __gen_uint(values->LegacyDiamondLineRasterization, 26, 26) |
      __gen_uint(values->HierarchicalDepthBufferResolveEnable, 27, 27) |
      __gen_uint(values->DepthBufferResolveEnable, 28, 28) |
      __gen_uint(values->ThreadDispatchEnable, 29, 29) |
      __gen_uint(values->DepthBufferClear, 30, 30) |
      __gen_uint(values->StatisticsEnable, 31, 31);

   dw[2] =
      __gen_uint(values->MultisampleDispatchMode, 31, 31);
}

#define GFX7_GPGPU_OBJECT_length               8
#define GFX7_GPGPU_OBJECT_length_bias          2
#define GFX7_GPGPU_OBJECT_header                \
   .DWordLength                         =      6,  \
   .SubOpcode                           =      4,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_GPGPU_OBJECT {
   uint32_t                             DWordLength;
   bool                                 PredicateEnable;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             SharedLocalMemoryFixedOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             HalfSliceDestinationSelect;
#define HalfSlice1                               2
#define HalfSlice0                               1
#define EitherHalfSlice                          0
   uint32_t                             EndofThreadGroup;
   uint32_t                             SharedLocalMemoryOffset;
   uint64_t                             IndirectDataStartAddress;
   uint32_t                             ThreadGroupIDX;
   uint32_t                             ThreadGroupIDY;
   uint32_t                             ThreadGroupIDZ;
   uint32_t                             ExecutionMask;
};

static inline __attribute__((always_inline)) void
GFX7_GPGPU_OBJECT_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_GPGPU_OBJECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->PredicateEnable, 8, 8) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 4) |
      __gen_uint(values->SharedLocalMemoryFixedOffset, 7, 7);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->HalfSliceDestinationSelect, 17, 18) |
      __gen_uint(values->EndofThreadGroup, 24, 24) |
      __gen_uint(values->SharedLocalMemoryOffset, 28, 31);

   dw[3] =
      __gen_offset(values->IndirectDataStartAddress, 0, 31);

   dw[4] =
      __gen_uint(values->ThreadGroupIDX, 0, 31);

   dw[5] =
      __gen_uint(values->ThreadGroupIDY, 0, 31);

   dw[6] =
      __gen_uint(values->ThreadGroupIDZ, 0, 31);

   dw[7] =
      __gen_uint(values->ExecutionMask, 0, 31);
}

#define GFX7_GPGPU_WALKER_length              11
#define GFX7_GPGPU_WALKER_length_bias          2
#define GFX7_GPGPU_WALKER_header                \
   .DWordLength                         =      9,  \
   .SubOpcodeA                          =      5,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_GPGPU_WALKER {
   uint32_t                             DWordLength;
   bool                                 PredicateEnable;
   bool                                 IndirectParameterEnable;
   uint32_t                             SubOpcodeA;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             ThreadWidthCounterMaximum;
   uint32_t                             ThreadHeightCounterMaximum;
   uint32_t                             ThreadDepthCounterMaximum;
   uint32_t                             SIMDSize;
#define SIMD8                                    0
#define SIMD16                                   1
#define SIMD32                                   2
   uint32_t                             ThreadGroupIDStartingX;
   uint32_t                             ThreadGroupIDXDimension;
   uint32_t                             ThreadGroupIDStartingY;
   uint32_t                             ThreadGroupIDYDimension;
   uint32_t                             ThreadGroupIDStartingZ;
   uint32_t                             ThreadGroupIDZDimension;
   uint32_t                             RightExecutionMask;
   uint32_t                             BottomExecutionMask;
};

static inline __attribute__((always_inline)) void
GFX7_GPGPU_WALKER_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_GPGPU_WALKER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->PredicateEnable, 8, 8) |
      __gen_uint(values->IndirectParameterEnable, 10, 10) |
      __gen_uint(values->SubOpcodeA, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 4);

   dw[2] =
      __gen_uint(values->ThreadWidthCounterMaximum, 0, 5) |
      __gen_uint(values->ThreadHeightCounterMaximum, 8, 13) |
      __gen_uint(values->ThreadDepthCounterMaximum, 16, 21) |
      __gen_uint(values->SIMDSize, 30, 31);

   dw[3] =
      __gen_uint(values->ThreadGroupIDStartingX, 0, 31);

   dw[4] =
      __gen_uint(values->ThreadGroupIDXDimension, 0, 31);

   dw[5] =
      __gen_uint(values->ThreadGroupIDStartingY, 0, 31);

   dw[6] =
      __gen_uint(values->ThreadGroupIDYDimension, 0, 31);

   dw[7] =
      __gen_uint(values->ThreadGroupIDStartingZ, 0, 31);

   dw[8] =
      __gen_uint(values->ThreadGroupIDZDimension, 0, 31);

   dw[9] =
      __gen_uint(values->RightExecutionMask, 0, 31);

   dw[10] =
      __gen_uint(values->BottomExecutionMask, 0, 31);
}

#define GFX7_MEDIA_CURBE_LOAD_length           4
#define GFX7_MEDIA_CURBE_LOAD_length_bias      2
#define GFX7_MEDIA_CURBE_LOAD_header            \
   .DWordLength                         =      2,  \
   .SubOpcode                           =      1,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_CURBE_LOAD {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             CURBETotalDataLength;
   uint32_t                             CURBEDataStartAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_CURBE_LOAD_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_MEDIA_CURBE_LOAD * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] = 0;

   dw[2] =
      __gen_uint(values->CURBETotalDataLength, 0, 16);

   dw[3] =
      __gen_uint(values->CURBEDataStartAddress, 0, 31);
}

#define GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD_length      4
#define GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD_length_bias      2
#define GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD_header\
   .DWordLength                         =      2,  \
   .SubOpcode                           =      2,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorTotalLength;
   uint64_t                             InterfaceDescriptorDataStartAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX7_MEDIA_INTERFACE_DESCRIPTOR_LOAD * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] = 0;

   dw[2] =
      __gen_uint(values->InterfaceDescriptorTotalLength, 0, 16);

   dw[3] =
      __gen_offset(values->InterfaceDescriptorDataStartAddress, 0, 31);
}

#define GFX7_MEDIA_OBJECT_length_bias          2
#define GFX7_MEDIA_OBJECT_header                \
   .DWordLength                         =      4,  \
   .MediaCommandSubOpcode               =      0,  \
   .MediaCommandOpcode                  =      1,  \
   .MediaCommandPipeline                =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_OBJECT {
   uint32_t                             DWordLength;
   uint32_t                             MediaCommandSubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             MediaCommandPipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             HalfSliceDestinationSelect;
#define HalfSlice1                               2
#define HalfSlice0                               1
#define Eitherhalfslice                          0
   uint32_t                             UseScoreboard;
#define Notusingscoreboard                       0
#define Usingscoreboard                          1
   uint32_t                             ThreadSynchronization;
#define Nothreadsynchronization                  0
#define Threaddispatchissynchronizedbythespawnrootthreadmessage 1
   bool                                 ChildrenPresent;
   __gen_address_type                   IndirectDataStartAddress;
   uint32_t                             ScoreboardX;
   uint32_t                             ScoredboardY;
   uint32_t                             ScoreboardMask;
   uint32_t                             ScoreboardColor;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_OBJECT_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_MEDIA_OBJECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->MediaCommandSubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->MediaCommandPipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 4);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->HalfSliceDestinationSelect, 17, 18) |
      __gen_uint(values->UseScoreboard, 21, 21) |
      __gen_uint(values->ThreadSynchronization, 24, 24) |
      __gen_uint(values->ChildrenPresent, 31, 31);

   dw[3] = __gen_address(data, &dw[3], values->IndirectDataStartAddress, 0, 0, 31);

   dw[4] =
      __gen_uint(values->ScoreboardX, 0, 8) |
      __gen_uint(values->ScoredboardY, 16, 24);

   dw[5] =
      __gen_uint(values->ScoreboardMask, 0, 7) |
      __gen_uint(values->ScoreboardColor, 16, 19);
}

#define GFX7_MEDIA_OBJECT_PRT_length          16
#define GFX7_MEDIA_OBJECT_PRT_length_bias      2
#define GFX7_MEDIA_OBJECT_PRT_header            \
   .DWordLength                         =     14,  \
   .SubOpcode                           =      2,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_OBJECT_PRT {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             PRT_FenceType;
#define Rootthreadqueue                          0
#define VFEstateflush                            1
   bool                                 PRT_FenceNeeded;
   bool                                 ChildrenPresent;
   uint32_t                             InlineData[12];
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_OBJECT_PRT_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_MEDIA_OBJECT_PRT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 4);

   dw[2] =
      __gen_uint(values->PRT_FenceType, 22, 22) |
      __gen_uint(values->PRT_FenceNeeded, 23, 23) |
      __gen_uint(values->ChildrenPresent, 31, 31);

   dw[3] = 0;

   dw[4] =
      __gen_uint(values->InlineData[0], 0, 31);

   dw[5] =
      __gen_uint(values->InlineData[1], 0, 31);

   dw[6] =
      __gen_uint(values->InlineData[2], 0, 31);

   dw[7] =
      __gen_uint(values->InlineData[3], 0, 31);

   dw[8] =
      __gen_uint(values->InlineData[4], 0, 31);

   dw[9] =
      __gen_uint(values->InlineData[5], 0, 31);

   dw[10] =
      __gen_uint(values->InlineData[6], 0, 31);

   dw[11] =
      __gen_uint(values->InlineData[7], 0, 31);

   dw[12] =
      __gen_uint(values->InlineData[8], 0, 31);

   dw[13] =
      __gen_uint(values->InlineData[9], 0, 31);

   dw[14] =
      __gen_uint(values->InlineData[10], 0, 31);

   dw[15] =
      __gen_uint(values->InlineData[11], 0, 31);
}

#define GFX7_MEDIA_OBJECT_WALKER_length_bias      2
#define GFX7_MEDIA_OBJECT_WALKER_header         \
   .DWordLength                         =     15,  \
   .SubOpcode                           =      3,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_OBJECT_WALKER {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             UseScoreboard;
#define Notusingscoreboard                       0
#define Usingscoreboard                          1
   uint32_t                             ThreadSynchronization;
#define Nothreadsynchronization                  0
#define Threaddispatchissynchronizedbythespawnrootthreadmessage 1
   uint32_t                             ChildrenPresent;
   uint64_t                             IndirectDataStartAddress;
   uint32_t                             ScoreboardMask;
   int32_t                              MidLoopUnitX;
   int32_t                              LocalMidLoopUnitY;
   uint32_t                             MiddleLoopExtraSteps;
   uint32_t                             ColorCountMinusOne;
   uint32_t                             Repel;
   uint32_t                             DualMode;
   uint32_t                             LocalLoopExecCount;
   uint32_t                             GlobalLoopExecCount;
   uint32_t                             BlockResolutionX;
   uint32_t                             BlockResolutionY;
   uint32_t                             LocalStartX;
   uint32_t                             LocalStartY;
   uint32_t                             LocalEndX;
   uint32_t                             LocalEndY;
   int32_t                              LocalOuterLoopStrideX;
   int32_t                              LocalOuterLoopStrideY;
   int32_t                              LocalInnerLoopUnitX;
   int32_t                              LocalInnerLoopUnitY;
   uint32_t                             GlobalResolutionX;
   uint32_t                             GlobalResolutionY;
   int32_t                              GlobalStartX;
   int32_t                              GlobalStartY;
   int32_t                              GlobalOuterLoopStrideX;
   int32_t                              GlobalOuterLoopStrideY;
   int32_t                              GlobalInnerLoopUnitX;
   int32_t                              GlobalInnerLoopUnitY;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_OBJECT_WALKER_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_MEDIA_OBJECT_WALKER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 4);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->UseScoreboard, 21, 21) |
      __gen_uint(values->ThreadSynchronization, 24, 24) |
      __gen_uint(values->ChildrenPresent, 31, 31);

   dw[3] =
      __gen_offset(values->IndirectDataStartAddress, 0, 31);

   dw[4] = 0;

   dw[5] =
      __gen_uint(values->ScoreboardMask, 0, 7);

   dw[6] =
      __gen_sint(values->MidLoopUnitX, 8, 9) |
      __gen_sint(values->LocalMidLoopUnitY, 12, 13) |
      __gen_uint(values->MiddleLoopExtraSteps, 16, 20) |
      __gen_uint(values->ColorCountMinusOne, 24, 27) |
      __gen_uint(values->Repel, 30, 30) |
      __gen_uint(values->DualMode, 31, 31);

   dw[7] =
      __gen_uint(values->LocalLoopExecCount, 0, 9) |
      __gen_uint(values->GlobalLoopExecCount, 16, 25);

   dw[8] =
      __gen_uint(values->BlockResolutionX, 0, 8) |
      __gen_uint(values->BlockResolutionY, 16, 24);

   dw[9] =
      __gen_uint(values->LocalStartX, 0, 8) |
      __gen_uint(values->LocalStartY, 16, 24);

   dw[10] =
      __gen_uint(values->LocalEndX, 0, 8) |
      __gen_uint(values->LocalEndY, 16, 24);

   dw[11] =
      __gen_sint(values->LocalOuterLoopStrideX, 0, 9) |
      __gen_sint(values->LocalOuterLoopStrideY, 16, 25);

   dw[12] =
      __gen_sint(values->LocalInnerLoopUnitX, 0, 9) |
      __gen_sint(values->LocalInnerLoopUnitY, 16, 25);

   dw[13] =
      __gen_uint(values->GlobalResolutionX, 0, 8) |
      __gen_uint(values->GlobalResolutionY, 16, 24);

   dw[14] =
      __gen_sint(values->GlobalStartX, 0, 9) |
      __gen_sint(values->GlobalStartY, 16, 25);

   dw[15] =
      __gen_sint(values->GlobalOuterLoopStrideX, 0, 9) |
      __gen_sint(values->GlobalOuterLoopStrideY, 16, 25);

   dw[16] =
      __gen_sint(values->GlobalInnerLoopUnitX, 0, 9) |
      __gen_sint(values->GlobalInnerLoopUnitY, 16, 25);
}

#define GFX7_MEDIA_STATE_FLUSH_length          2
#define GFX7_MEDIA_STATE_FLUSH_length_bias      2
#define GFX7_MEDIA_STATE_FLUSH_header           \
   .DWordLength                         =      0,  \
   .SubOpcode                           =      4,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_STATE_FLUSH {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             WatermarkRequired;
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_STATE_FLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_MEDIA_STATE_FLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5) |
      __gen_uint(values->WatermarkRequired, 6, 6);
}

#define GFX7_MEDIA_VFE_STATE_length            8
#define GFX7_MEDIA_VFE_STATE_length_bias       2
#define GFX7_MEDIA_VFE_STATE_header             \
   .DWordLength                         =      6,  \
   .SubOpcode                           =      0,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX7_MEDIA_VFE_STATE {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   bool                                 GPGPUMode;
   uint32_t                             GatewayMMIOAccessControl;
#define NoMMIOreadwriteallowed                   0
#define MMIOreadwritetoanyaddress                2
   uint32_t                             BypassGatewayControl;
#define MaintainingOpenGatewayForwardMsgCloseGatewayprotocollegacymode 0
#define BypassingOpenGatewayCloseGatewayprotocol 1
   uint32_t                             ResetGatewayTimer;
#define Maintainingtheexistingtimestampstate     0
#define Resettingrelativetimerandlatchingtheglobaltimestamp 1
   uint32_t                             NumberofURBEntries;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             CURBEAllocationSize;
   uint32_t                             URBEntryAllocationSize;
   uint32_t                             ScoreboardMask;
   uint32_t                             ScoreboardType;
#define StallingScoreboard                       0
#define NonStallingScoreboard                    1
   uint32_t                             ScoreboardEnable;
#define Scoreboarddisabled                       0
#define Scoreboardenabled                        1
   int32_t                              Scoreboard0DeltaX;
   int32_t                              Scoreboard0DeltaY;
   int32_t                              Scoreboard1DeltaX;
   int32_t                              Scoreboard1DeltaY;
   int32_t                              Scoreboard2DeltaX;
   int32_t                              Scoreboard2DeltaY;
   int32_t                              Scoreboard3DeltaX;
   int32_t                              Scoreboard3DeltaY;
   int32_t                              Scoreboard4DeltaX;
   int32_t                              Scoreboard4DeltaY;
   int32_t                              Scoreboard5DeltaX;
   int32_t                              Scoreboard5DeltaY;
   int32_t                              Scoreboard6DeltaX;
   int32_t                              Scoreboard6DeltaY;
   int32_t                              Scoreboard7DeltaX;
   int32_t                              Scoreboard7DeltaY;
};

static inline __attribute__((always_inline)) void
GFX7_MEDIA_VFE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX7_MEDIA_VFE_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   dw[1] = __gen_address(data, &dw[1], values->ScratchSpaceBasePointer, v1, 10, 31);

   dw[2] =
      __gen_uint(values->GPGPUMode, 2, 2) |
      __gen_uint(values->GatewayMMIOAccessControl, 3, 4) |
      __gen_uint(values->BypassGatewayControl, 6, 6) |
      __gen_uint(values->ResetGatewayTimer, 7, 7) |
      __gen_uint(values->NumberofURBEntries, 8, 15) |
      __gen_uint(values->MaximumNumberofThreads, 16, 31);

   dw[3] = 0;

   dw[4] =
      __gen_uint(values->CURBEAllocationSize, 0, 15) |
      __gen_uint(values->URBEntryAllocationSize, 16, 31);

   dw[5] =
      __gen_uint(values->ScoreboardMask, 0, 7) |
      __gen_uint(values->ScoreboardType, 30, 30) |
      __gen_uint(values->ScoreboardEnable, 31, 31);

   dw[6] =
      __gen_sint(values->Scoreboard0DeltaX, 0, 3) |
      __gen_sint(values->Scoreboard0DeltaY, 4, 7) |
      __gen_sint(values->Scoreboard1DeltaX, 8, 11) |
      __gen_sint(values->Scoreboard1DeltaY, 12, 15) |
      __gen_sint(values->Scoreboard2DeltaX, 16, 19) |
      __gen_sint(values->Scoreboard2DeltaY, 20, 23) |
      __gen_sint(values->Scoreboard3DeltaX, 24, 27) |
      __gen_sint(values->Scoreboard3DeltaY, 28, 31);

   dw[7] =
      __gen_sint(values->Scoreboard4DeltaX, 0, 3) |
      __gen_sint(values->Scoreboard4DeltaY, 4, 7) |
      __gen_sint(values->Scoreboard5DeltaX, 8, 11) |
      __gen_sint(values->Scoreboard5DeltaY, 12, 15) |
      __gen_sint(values->Scoreboard6DeltaX, 16, 19) |
      __gen_sint(values->Scoreboard6DeltaY, 20, 23) |
      __gen_sint(values->Scoreboard7DeltaX, 24, 27) |
      __gen_sint(values->Scoreboard7DeltaY, 28, 31);
}

#define GFX7_MI_ARB_CHECK_length               1
#define GFX7_MI_ARB_CHECK_length_bias          1
#define GFX7_MI_ARB_CHECK_header                \
   .MICommandOpcode                     =      5,  \
   .CommandType                         =      0

struct GFX7_MI_ARB_CHECK {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_ARB_CHECK_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_MI_ARB_CHECK * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_ARB_ON_OFF_length              1
#define GFX7_MI_ARB_ON_OFF_length_bias         1
#define GFX7_MI_ARB_ON_OFF_header               \
   .MICommandOpcode                     =      8,  \
   .CommandType                         =      0

struct GFX7_MI_ARB_ON_OFF {
   bool                                 ArbitrationEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_ARB_ON_OFF_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_MI_ARB_ON_OFF * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ArbitrationEnable, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_BATCH_BUFFER_END_length        1
#define GFX7_MI_BATCH_BUFFER_END_length_bias      1
#define GFX7_MI_BATCH_BUFFER_END_header         \
   .MICommandOpcode                     =     10,  \
   .CommandType                         =      0

struct GFX7_MI_BATCH_BUFFER_END {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_BATCH_BUFFER_END_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_MI_BATCH_BUFFER_END * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_BATCH_BUFFER_START_length      2
#define GFX7_MI_BATCH_BUFFER_START_length_bias      2
#define GFX7_MI_BATCH_BUFFER_START_header       \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     49,  \
   .CommandType                         =      0

struct GFX7_MI_BATCH_BUFFER_START {
   uint32_t                             DWordLength;
   uint32_t                             AddressSpaceIndicator;
#define ASI_GGTT                                 0
#define ASI_PPGTT                                1
   bool                                 ClearCommandBufferEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   __gen_address_type                   BatchBufferStartAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MI_BATCH_BUFFER_START_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_MI_BATCH_BUFFER_START * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AddressSpaceIndicator, 8, 8) |
      __gen_uint(values->ClearCommandBufferEnable, 11, 11) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] = __gen_address(data, &dw[1], values->BatchBufferStartAddress, 0, 2, 31);
}

#define GFX7_MI_CLFLUSH_length_bias            2
#define GFX7_MI_CLFLUSH_header                  \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     39,  \
   .CommandType                         =      0

struct GFX7_MI_CLFLUSH {
   uint32_t                             DWordLength;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             StartingCachelineOffset;
   __gen_address_type                   PageBaseAddress;
   __gen_address_type                   PageBaseAddressHigh;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_MI_CLFLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_MI_CLFLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 9) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->StartingCachelineOffset, 6, 11);
   dw[1] = __gen_address(data, &dw[1], values->PageBaseAddress, v1, 12, 31);

   dw[2] = __gen_address(data, &dw[2], values->PageBaseAddressHigh, 0, 0, 15);
}

#define GFX7_MI_CONDITIONAL_BATCH_BUFFER_END_length      2
#define GFX7_MI_CONDITIONAL_BATCH_BUFFER_END_length_bias      2
#define GFX7_MI_CONDITIONAL_BATCH_BUFFER_END_header\
   .DWordLength                         =      0,  \
   .CompareSemaphore                    =      0,  \
   .MICommandOpcode                     =     54,  \
   .CommandType                         =      0

struct GFX7_MI_CONDITIONAL_BATCH_BUFFER_END {
   uint32_t                             DWordLength;
   uint32_t                             CompareSemaphore;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             CompareDataDword;
   __gen_address_type                   CompareAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MI_CONDITIONAL_BATCH_BUFFER_END_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX7_MI_CONDITIONAL_BATCH_BUFFER_END * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->CompareSemaphore, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->CompareDataDword, 0, 31);
}

#define GFX7_MI_FLUSH_length                   1
#define GFX7_MI_FLUSH_length_bias              1
#define GFX7_MI_FLUSH_header                    \
   .MICommandOpcode                     =      4,  \
   .CommandType                         =      0

struct GFX7_MI_FLUSH {
   uint32_t                             StateInstructionCacheInvalidate;
#define DontInvalidate                           0
#define Invalidate                               1
   uint32_t                             RenderCacheFlushInhibit;
#define Flush                                    0
#define DontFlush                                1
   uint32_t                             GlobalSnapshotCountReset;
#define DontReset                                0
#define Reset                                    1
   bool                                 GenericMediaStateClear;
   bool                                 IndirectStatePointersDisable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_FLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX7_MI_FLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->StateInstructionCacheInvalidate, 1, 1) |
      __gen_uint(values->RenderCacheFlushInhibit, 2, 2) |
      __gen_uint(values->GlobalSnapshotCountReset, 3, 3) |
      __gen_uint(values->GenericMediaStateClear, 4, 4) |
      __gen_uint(values->IndirectStatePointersDisable, 5, 5) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_LOAD_REGISTER_IMM_length       3
#define GFX7_MI_LOAD_REGISTER_IMM_length_bias      2
#define GFX7_MI_LOAD_REGISTER_IMM_header        \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     34,  \
   .CommandType                         =      0

struct GFX7_MI_LOAD_REGISTER_IMM {
   uint32_t                             DWordLength;
   uint32_t                             ByteWriteDisables;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterOffset;
   uint32_t                             DataDWord;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX7_MI_LOAD_REGISTER_IMM_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_MI_LOAD_REGISTER_IMM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->ByteWriteDisables, 8, 11) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterOffset, 2, 22);

   dw[2] =
      __gen_uint(values->DataDWord, 0, 31);
}

#define GFX7_MI_LOAD_REGISTER_MEM_length       3
#define GFX7_MI_LOAD_REGISTER_MEM_length_bias      2
#define GFX7_MI_LOAD_REGISTER_MEM_header        \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     41,  \
   .CommandType                         =      0

struct GFX7_MI_LOAD_REGISTER_MEM {
   uint32_t                             DWordLength;
   bool                                 AsyncModeEnable;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterAddress;
   __gen_address_type                   MemoryAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MI_LOAD_REGISTER_MEM_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_MI_LOAD_REGISTER_MEM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AsyncModeEnable, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterAddress, 2, 22);

   dw[2] = __gen_address(data, &dw[2], values->MemoryAddress, 0, 2, 31);
}

#define GFX7_MI_NOOP_length                    1
#define GFX7_MI_NOOP_length_bias               1
#define GFX7_MI_NOOP_header                     \
   .MICommandOpcode                     =      0,  \
   .CommandType                         =      0

struct GFX7_MI_NOOP {
   uint32_t                             IdentificationNumber;
   bool                                 IdentificationNumberRegisterWriteEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_NOOP_pack(__attribute__((unused)) __gen_user_data *data,
                  __attribute__((unused)) void * restrict dst,
                  __attribute__((unused)) const struct GFX7_MI_NOOP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->IdentificationNumber, 0, 21) |
      __gen_uint(values->IdentificationNumberRegisterWriteEnable, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_PREDICATE_length               1
#define GFX7_MI_PREDICATE_length_bias          1
#define GFX7_MI_PREDICATE_header                \
   .MICommandOpcode                     =     12,  \
   .CommandType                         =      0

struct GFX7_MI_PREDICATE {
   uint32_t                             CompareOperation;
#define COMPARE_TRUE                             0
#define COMPARE_FALSE                            1
#define COMPARE_SRCS_EQUAL                       2
#define COMPARE_DELTAS_EQUAL                     3
   uint32_t                             CombineOperation;
#define COMBINE_SET                              0
#define COMBINE_AND                              1
#define COMBINE_OR                               2
#define COMBINE_XOR                              3
   uint32_t                             LoadOperation;
#define LOAD_KEEP                                0
#define LOAD_LOAD                                2
#define LOAD_LOADINV                             3
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_PREDICATE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_MI_PREDICATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->CompareOperation, 0, 1) |
      __gen_uint(values->CombineOperation, 3, 4) |
      __gen_uint(values->LoadOperation, 6, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_REPORT_HEAD_length             1
#define GFX7_MI_REPORT_HEAD_length_bias        1
#define GFX7_MI_REPORT_HEAD_header              \
   .MICommandOpcode                     =      7,  \
   .CommandType                         =      0

struct GFX7_MI_REPORT_HEAD {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_REPORT_HEAD_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_MI_REPORT_HEAD * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_REPORT_PERF_COUNT_length       3
#define GFX7_MI_REPORT_PERF_COUNT_length_bias      2
#define GFX7_MI_REPORT_PERF_COUNT_header        \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     40,  \
   .CommandType                         =      0

struct GFX7_MI_REPORT_PERF_COUNT {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   bool                                 UseGlobalGTT;
   __gen_address_type                   MemoryAddress;
   uint32_t                             ReportID;
};

static inline __attribute__((always_inline)) void
GFX7_MI_REPORT_PERF_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX7_MI_REPORT_PERF_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 5) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->UseGlobalGTT, 0, 0);
   dw[1] = __gen_address(data, &dw[1], values->MemoryAddress, v1, 6, 31);

   dw[2] =
      __gen_uint(values->ReportID, 0, 31);
}

#define GFX7_MI_SEMAPHORE_MBOX_length          3
#define GFX7_MI_SEMAPHORE_MBOX_length_bias      2
#define GFX7_MI_SEMAPHORE_MBOX_header           \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     22,  \
   .CommandType                         =      0

struct GFX7_MI_SEMAPHORE_MBOX {
   uint32_t                             DWordLength;
   uint32_t                             RegisterSelect;
#define RVSYNC                                   0
#define RBSYNC                                   2
#define UseGeneralRegisterSelect                 3
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             SemaphoreDataDword;
};

static inline __attribute__((always_inline)) void
GFX7_MI_SEMAPHORE_MBOX_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_MI_SEMAPHORE_MBOX * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->RegisterSelect, 16, 17) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SemaphoreDataDword, 0, 31);

   dw[2] = 0;
}

#define GFX7_MI_SET_CONTEXT_length             2
#define GFX7_MI_SET_CONTEXT_length_bias        2
#define GFX7_MI_SET_CONTEXT_header              \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     24,  \
   .CommandType                         =      0

struct GFX7_MI_SET_CONTEXT {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             RestoreInhibit;
   uint32_t                             ForceRestore;
   bool                                 ExtendedStateRestoreEnable;
   bool                                 ExtendedStateSaveEnable;
   uint32_t                             ReservedMustbe1;
   __gen_address_type                   LogicalContextAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MI_SET_CONTEXT_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_MI_SET_CONTEXT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->RestoreInhibit, 0, 0) |
      __gen_uint(values->ForceRestore, 1, 1) |
      __gen_uint(values->ExtendedStateRestoreEnable, 2, 2) |
      __gen_uint(values->ExtendedStateSaveEnable, 3, 3) |
      __gen_uint(values->ReservedMustbe1, 8, 8);
   dw[1] = __gen_address(data, &dw[1], values->LogicalContextAddress, v1, 12, 31);
}

#define GFX7_MI_STORE_DATA_IMM_length          4
#define GFX7_MI_STORE_DATA_IMM_length_bias      2
#define GFX7_MI_STORE_DATA_IMM_header           \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     32,  \
   .CommandType                         =      0

struct GFX7_MI_STORE_DATA_IMM {
   uint32_t                             DWordLength;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             CoreModeEnable;
   __gen_address_type                   Address;
   uint64_t                             ImmediateData;
};

static inline __attribute__((always_inline)) void
GFX7_MI_STORE_DATA_IMM_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_MI_STORE_DATA_IMM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 5) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] = 0;

   const uint32_t v2 =
      __gen_uint(values->CoreModeEnable, 0, 0);
   dw[2] = __gen_address(data, &dw[2], values->Address, v2, 2, 31);

   const uint64_t v3 =
      __gen_uint(values->ImmediateData, 0, 63);
   dw[3] = v3;
   dw[4] = v3 >> 32;
}

#define GFX7_MI_STORE_DATA_INDEX_length        3
#define GFX7_MI_STORE_DATA_INDEX_length_bias      2
#define GFX7_MI_STORE_DATA_INDEX_header         \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     33,  \
   .CommandType                         =      0

struct GFX7_MI_STORE_DATA_INDEX {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             Offset;
   uint32_t                             DataDWord0;
   uint32_t                             DataDWord1;
};

static inline __attribute__((always_inline)) void
GFX7_MI_STORE_DATA_INDEX_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_MI_STORE_DATA_INDEX * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->Offset, 2, 11);

   dw[2] =
      __gen_uint(values->DataDWord0, 0, 31);
}

#define GFX7_MI_STORE_REGISTER_MEM_length      3
#define GFX7_MI_STORE_REGISTER_MEM_length_bias      2
#define GFX7_MI_STORE_REGISTER_MEM_header       \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     36,  \
   .CommandType                         =      0

struct GFX7_MI_STORE_REGISTER_MEM {
   uint32_t                             DWordLength;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterAddress;
   __gen_address_type                   MemoryAddress;
};

static inline __attribute__((always_inline)) void
GFX7_MI_STORE_REGISTER_MEM_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_MI_STORE_REGISTER_MEM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterAddress, 2, 22);

   dw[2] = __gen_address(data, &dw[2], values->MemoryAddress, 0, 2, 31);
}

#define GFX7_MI_SUSPEND_FLUSH_length           1
#define GFX7_MI_SUSPEND_FLUSH_length_bias      1
#define GFX7_MI_SUSPEND_FLUSH_header            \
   .MICommandOpcode                     =     11,  \
   .CommandType                         =      0

struct GFX7_MI_SUSPEND_FLUSH {
   bool                                 SuspendFlush;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_SUSPEND_FLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_MI_SUSPEND_FLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SuspendFlush, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_TOPOLOGY_FILTER_length         1
#define GFX7_MI_TOPOLOGY_FILTER_length_bias      1
#define GFX7_MI_TOPOLOGY_FILTER_header          \
   .MICommandOpcode                     =     13,  \
   .CommandType                         =      0

struct GFX7_MI_TOPOLOGY_FILTER {
   enum GFX7_3D_Prim_Topo_Type          TopologyFilterValue;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_TOPOLOGY_FILTER_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX7_MI_TOPOLOGY_FILTER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->TopologyFilterValue, 0, 5) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_URB_CLEAR_length               2
#define GFX7_MI_URB_CLEAR_length_bias          2
#define GFX7_MI_URB_CLEAR_header                \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     25,  \
   .CommandType                         =      0

struct GFX7_MI_URB_CLEAR {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             URBAddress;
   uint32_t                             URBClearLength;
};

static inline __attribute__((always_inline)) void
GFX7_MI_URB_CLEAR_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_MI_URB_CLEAR * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->URBAddress, 0, 13) |
      __gen_uint(values->URBClearLength, 16, 28);
}

#define GFX7_MI_USER_INTERRUPT_length          1
#define GFX7_MI_USER_INTERRUPT_length_bias      1
#define GFX7_MI_USER_INTERRUPT_header           \
   .MICommandOpcode                     =      2,  \
   .CommandType                         =      0

struct GFX7_MI_USER_INTERRUPT {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_USER_INTERRUPT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_MI_USER_INTERRUPT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_MI_WAIT_FOR_EVENT_length          1
#define GFX7_MI_WAIT_FOR_EVENT_length_bias      1
#define GFX7_MI_WAIT_FOR_EVENT_header           \
   .MICommandOpcode                     =      3,  \
   .CommandType                         =      0

struct GFX7_MI_WAIT_FOR_EVENT {
   bool                                 DisplayPipeAScanLineWaitEnable;
   bool                                 DisplayPlaneAFlipPendingWaitEnable;
   bool                                 DisplaySpriteAFlipPendingWaitEnable;
   bool                                 DisplayPipeAVerticalBlankWaitEnable;
   bool                                 DisplayPipeAHorizontalBlankWaitEnable;
   bool                                 DisplayPipeBScanLineWaitEnable;
   bool                                 DisplayPlaneBFlipPendingWaitEnable;
   bool                                 DisplaySpriteBFlipPendingWaitEnable;
   bool                                 DisplayPipeBVerticalBlankWaitEnable;
   bool                                 DisplayPipeBHorizontalBlankWaitEnable;
   bool                                 DisplayPipeCScanLineWaitEnable;
   bool                                 DisplayPlaneCFlipPendingWaitEnable;
   uint32_t                             ConditionCodeWaitSelect;
#define Notenabled                               0
   bool                                 DisplaySpriteCFlipPendingWaitEnable;
   bool                                 DisplayPipeCVerticalBlankWaitEnable;
   bool                                 DisplayPipeCHorizontalBlankWaitEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_MI_WAIT_FOR_EVENT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_MI_WAIT_FOR_EVENT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DisplayPipeAScanLineWaitEnable, 0, 0) |
      __gen_uint(values->DisplayPlaneAFlipPendingWaitEnable, 1, 1) |
      __gen_uint(values->DisplaySpriteAFlipPendingWaitEnable, 2, 2) |
      __gen_uint(values->DisplayPipeAVerticalBlankWaitEnable, 3, 3) |
      __gen_uint(values->DisplayPipeAHorizontalBlankWaitEnable, 5, 5) |
      __gen_uint(values->DisplayPipeBScanLineWaitEnable, 8, 8) |
      __gen_uint(values->DisplayPlaneBFlipPendingWaitEnable, 9, 9) |
      __gen_uint(values->DisplaySpriteBFlipPendingWaitEnable, 10, 10) |
      __gen_uint(values->DisplayPipeBVerticalBlankWaitEnable, 11, 11) |
      __gen_uint(values->DisplayPipeBHorizontalBlankWaitEnable, 13, 13) |
      __gen_uint(values->DisplayPipeCScanLineWaitEnable, 14, 14) |
      __gen_uint(values->DisplayPlaneCFlipPendingWaitEnable, 15, 15) |
      __gen_uint(values->ConditionCodeWaitSelect, 16, 19) |
      __gen_uint(values->DisplaySpriteCFlipPendingWaitEnable, 20, 20) |
      __gen_uint(values->DisplayPipeCVerticalBlankWaitEnable, 21, 21) |
      __gen_uint(values->DisplayPipeCHorizontalBlankWaitEnable, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_PIPELINE_SELECT_length            1
#define GFX7_PIPELINE_SELECT_length_bias       1
#define GFX7_PIPELINE_SELECT_header             \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      1,  \
   .CommandType                         =      3

struct GFX7_PIPELINE_SELECT {
   uint32_t                             PipelineSelection;
#define _3D                                      0
#define Media                                    1
#define GPGPU                                    2
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX7_PIPELINE_SELECT_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX7_PIPELINE_SELECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PipelineSelection, 0, 1) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX7_PIPE_CONTROL_length               5
#define GFX7_PIPE_CONTROL_length_bias          2
#define GFX7_PIPE_CONTROL_header                \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      2,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX7_PIPE_CONTROL {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 DepthCacheFlushEnable;
   bool                                 StallAtPixelScoreboard;
   bool                                 StateCacheInvalidationEnable;
   bool                                 ConstantCacheInvalidationEnable;
   bool                                 VFCacheInvalidationEnable;
   bool                                 DCFlushEnable;
   bool                                 PipeControlFlushEnable;
   bool                                 NotifyEnable;
   bool                                 IndirectStatePointersDisable;
   bool                                 TextureCacheInvalidationEnable;
   bool                                 InstructionCacheInvalidateEnable;
   bool                                 RenderTargetCacheFlushEnable;
   bool                                 DepthStallEnable;
   uint32_t                             PostSyncOperation;
#define NoWrite                                  0
#define WriteImmediateData                       1
#define WritePSDepthCount                        2
#define WriteTimestamp                           3
   bool                                 GenericMediaStateClear;
   bool                                 TLBInvalidate;
   bool                                 GlobalSnapshotCountReset;
   bool                                 CommandStreamerStallEnable;
   uint32_t                             StoreDataIndex;
   uint32_t                             LRIPostSyncOperation;
#define NoLRIOperation                           0
#define MMIOWriteImmediateData                   1
   uint32_t                             DestinationAddressType;
#define DAT_PPGTT                                0
#define DAT_GGTT                                 1
   __gen_address_type                   Address;
   uint64_t                             ImmediateData;
};

static inline __attribute__((always_inline)) void
GFX7_PIPE_CONTROL_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_PIPE_CONTROL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DepthCacheFlushEnable, 0, 0) |
      __gen_uint(values->StallAtPixelScoreboard, 1, 1) |
      __gen_uint(values->StateCacheInvalidationEnable, 2, 2) |
      __gen_uint(values->ConstantCacheInvalidationEnable, 3, 3) |
      __gen_uint(values->VFCacheInvalidationEnable, 4, 4) |
      __gen_uint(values->DCFlushEnable, 5, 5) |
      __gen_uint(values->PipeControlFlushEnable, 7, 7) |
      __gen_uint(values->NotifyEnable, 8, 8) |
      __gen_uint(values->IndirectStatePointersDisable, 9, 9) |
      __gen_uint(values->TextureCacheInvalidationEnable, 10, 10) |
      __gen_uint(values->InstructionCacheInvalidateEnable, 11, 11) |
      __gen_uint(values->RenderTargetCacheFlushEnable, 12, 12) |
      __gen_uint(values->DepthStallEnable, 13, 13) |
      __gen_uint(values->PostSyncOperation, 14, 15) |
      __gen_uint(values->GenericMediaStateClear, 16, 16) |
      __gen_uint(values->TLBInvalidate, 18, 18) |
      __gen_uint(values->GlobalSnapshotCountReset, 19, 19) |
      __gen_uint(values->CommandStreamerStallEnable, 20, 20) |
      __gen_uint(values->StoreDataIndex, 21, 21) |
      __gen_uint(values->LRIPostSyncOperation, 23, 23) |
      __gen_uint(values->DestinationAddressType, 24, 24);

   dw[2] = __gen_address(data, &dw[2], values->Address, 0, 2, 31);

   const uint64_t v3 =
      __gen_uint(values->ImmediateData, 0, 63);
   dw[3] = v3;
   dw[4] = v3 >> 32;
}

#define GFX7_STATE_BASE_ADDRESS_length        10
#define GFX7_STATE_BASE_ADDRESS_length_bias      2
#define GFX7_STATE_BASE_ADDRESS_header          \
   .DWordLength                         =      8,  \
   ._3DCommandSubOpcode                 =      1,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX7_STATE_BASE_ADDRESS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 GeneralStateBaseAddressModifyEnable;
   uint32_t                             StatelessDataPortAccessForceWriteThru;
   uint32_t                             StatelessDataPortAccessMOCS;
   uint32_t                             GeneralStateMOCS;
   __gen_address_type                   GeneralStateBaseAddress;
   bool                                 SurfaceStateBaseAddressModifyEnable;
   uint32_t                             SurfaceStateMOCS;
   __gen_address_type                   SurfaceStateBaseAddress;
   bool                                 DynamicStateBaseAddressModifyEnable;
   uint32_t                             DynamicStateMOCS;
   __gen_address_type                   DynamicStateBaseAddress;
   bool                                 IndirectObjectBaseAddressModifyEnable;
   uint32_t                             IndirectObjectMOCS;
   __gen_address_type                   IndirectObjectBaseAddress;
   bool                                 InstructionBaseAddressModifyEnable;
   uint32_t                             InstructionMOCS;
   __gen_address_type                   InstructionBaseAddress;
   bool                                 GeneralStateAccessUpperBoundModifyEnable;
   __gen_address_type                   GeneralStateAccessUpperBound;
   bool                                 DynamicStateAccessUpperBoundModifyEnable;
   __gen_address_type                   DynamicStateAccessUpperBound;
   bool                                 IndirectObjectAccessUpperBoundModifyEnable;
   __gen_address_type                   IndirectObjectAccessUpperBound;
   bool                                 InstructionAccessUpperBoundModifyEnable;
   __gen_address_type                   InstructionAccessUpperBound;
};

static inline __attribute__((always_inline)) void
GFX7_STATE_BASE_ADDRESS_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX7_STATE_BASE_ADDRESS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->GeneralStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->StatelessDataPortAccessForceWriteThru, 3, 3) |
      __gen_uint(values->StatelessDataPortAccessMOCS, 4, 7) |
      __gen_uint(values->GeneralStateMOCS, 8, 11);
   dw[1] = __gen_address(data, &dw[1], values->GeneralStateBaseAddress, v1, 12, 31);

   const uint32_t v2 =
      __gen_uint(values->SurfaceStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->SurfaceStateMOCS, 8, 11);
   dw[2] = __gen_address(data, &dw[2], values->SurfaceStateBaseAddress, v2, 12, 31);

   const uint32_t v3 =
      __gen_uint(values->DynamicStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->DynamicStateMOCS, 8, 11);
   dw[3] = __gen_address(data, &dw[3], values->DynamicStateBaseAddress, v3, 12, 31);

   const uint32_t v4 =
      __gen_uint(values->IndirectObjectBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->IndirectObjectMOCS, 8, 11);
   dw[4] = __gen_address(data, &dw[4], values->IndirectObjectBaseAddress, v4, 12, 31);

   const uint32_t v5 =
      __gen_uint(values->InstructionBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->InstructionMOCS, 8, 11);
   dw[5] = __gen_address(data, &dw[5], values->InstructionBaseAddress, v5, 12, 31);

   const uint32_t v6 =
      __gen_uint(values->GeneralStateAccessUpperBoundModifyEnable, 0, 0);
   dw[6] = __gen_address(data, &dw[6], values->GeneralStateAccessUpperBound, v6, 12, 31);

   const uint32_t v7 =
      __gen_uint(values->DynamicStateAccessUpperBoundModifyEnable, 0, 0);
   dw[7] = __gen_address(data, &dw[7], values->DynamicStateAccessUpperBound, v7, 12, 31);

   const uint32_t v8 =
      __gen_uint(values->IndirectObjectAccessUpperBoundModifyEnable, 0, 0);
   dw[8] = __gen_address(data, &dw[8], values->IndirectObjectAccessUpperBound, v8, 12, 31);

   const uint32_t v9 =
      __gen_uint(values->InstructionAccessUpperBoundModifyEnable, 0, 0);
   dw[9] = __gen_address(data, &dw[9], values->InstructionAccessUpperBound, v9, 12, 31);
}

#define GFX7_STATE_PREFETCH_length             2
#define GFX7_STATE_PREFETCH_length_bias        2
#define GFX7_STATE_PREFETCH_header              \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =      3,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX7_STATE_PREFETCH {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PrefetchCount;
   __gen_address_type                   PrefetchPointer;
};

static inline __attribute__((always_inline)) void
GFX7_STATE_PREFETCH_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX7_STATE_PREFETCH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->PrefetchCount, 0, 2);
   dw[1] = __gen_address(data, &dw[1], values->PrefetchPointer, v1, 6, 31);
}

#define GFX7_STATE_SIP_length                  2
#define GFX7_STATE_SIP_length_bias             2
#define GFX7_STATE_SIP_header                   \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =      2,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX7_STATE_SIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             SystemInstructionPointer;
};

static inline __attribute__((always_inline)) void
GFX7_STATE_SIP_pack(__attribute__((unused)) __gen_user_data *data,
                    __attribute__((unused)) void * restrict dst,
                    __attribute__((unused)) const struct GFX7_STATE_SIP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->SystemInstructionPointer, 4, 31);
}

#define GFX7_SWTESS_BASE_ADDRESS_length        2
#define GFX7_SWTESS_BASE_ADDRESS_length_bias      2
#define GFX7_SWTESS_BASE_ADDRESS_header         \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =      3,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX7_SWTESS_BASE_ADDRESS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SWTessellationMOCS;
   __gen_address_type                   SWTessellationBaseAddress;
};

static inline __attribute__((always_inline)) void
GFX7_SWTESS_BASE_ADDRESS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_SWTESS_BASE_ADDRESS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->SWTessellationMOCS, 8, 11);
   dw[1] = __gen_address(data, &dw[1], values->SWTessellationBaseAddress, v1, 12, 31);
}

#define GFX7_BCS_FAULT_REG_num            0x4294
#define GFX7_BCS_FAULT_REG_length              1
struct GFX7_BCS_FAULT_REG {
   bool                                 ValidBit;
   uint32_t                             FaultType;
#define PageFault                                0
#define InvalidPDFault                           1
#define UnloadedPDFault                          2
#define InvalidandUnloadedPDfault                3
   uint32_t                             SRCIDofFault;
   uint32_t                             GTTSEL;
#define PPGTT                                    0
#define GGTT                                     1
   __gen_address_type                   VirtualAddressofFault;
};

static inline __attribute__((always_inline)) void
GFX7_BCS_FAULT_REG_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_BCS_FAULT_REG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint32_t v0 =
      __gen_uint(values->ValidBit, 0, 0) |
      __gen_uint(values->FaultType, 1, 2) |
      __gen_uint(values->SRCIDofFault, 3, 10) |
      __gen_uint(values->GTTSEL, 11, 11);
   dw[0] = __gen_address(data, &dw[0], values->VirtualAddressofFault, v0, 12, 31);
}

#define GFX7_BCS_INSTDONE_num             0x2206c
#define GFX7_BCS_INSTDONE_length               1
struct GFX7_BCS_INSTDONE {
   bool                                 RingEnable;
   bool                                 BlitterIDLE;
   bool                                 GABIDLE;
   bool                                 BCSDone;
};

static inline __attribute__((always_inline)) void
GFX7_BCS_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_BCS_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingEnable, 0, 0) |
      __gen_uint(values->BlitterIDLE, 1, 1) |
      __gen_uint(values->GABIDLE, 2, 2) |
      __gen_uint(values->BCSDone, 3, 3);
}

#define GFX7_BCS_RING_BUFFER_CTL_num      0x2203c
#define GFX7_BCS_RING_BUFFER_CTL_length        1
struct GFX7_BCS_RING_BUFFER_CTL {
   bool                                 RingBufferEnable;
   uint32_t                             AutomaticReportHeadPointer;
#define MI_AUTOREPORT_OFF                        0
#define MI_AUTOREPORT_64KB                       1
#define MI_AUTOREPORT_4KB                        2
#define MI_AUTOREPORT_128KB                      3
   bool                                 DisableRegisterAccesses;
   bool                                 SemaphoreWait;
   bool                                 RBWait;
   uint32_t                             BufferLengthinpages1;
};

static inline __attribute__((always_inline)) void
GFX7_BCS_RING_BUFFER_CTL_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_BCS_RING_BUFFER_CTL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingBufferEnable, 0, 0) |
      __gen_uint(values->AutomaticReportHeadPointer, 1, 2) |
      __gen_uint(values->DisableRegisterAccesses, 8, 8) |
      __gen_uint(values->SemaphoreWait, 10, 10) |
      __gen_uint(values->RBWait, 11, 11) |
      __gen_uint(values->BufferLengthinpages1, 12, 20);
}

#define GFX7_CL_INVOCATION_COUNT_num      0x2338
#define GFX7_CL_INVOCATION_COUNT_length        2
struct GFX7_CL_INVOCATION_COUNT {
   uint64_t                             CLInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_CL_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_CL_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CLInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_CL_PRIMITIVES_COUNT_num      0x2340
#define GFX7_CL_PRIMITIVES_COUNT_length        2
struct GFX7_CL_PRIMITIVES_COUNT {
   uint64_t                             CLPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_CL_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_CL_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CLPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_CS_INVOCATION_COUNT_num      0x2290
#define GFX7_CS_INVOCATION_COUNT_length        2
struct GFX7_CS_INVOCATION_COUNT {
   uint64_t                             CSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_CS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_CS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_DS_INVOCATION_COUNT_num      0x2308
#define GFX7_DS_INVOCATION_COUNT_length        2
struct GFX7_DS_INVOCATION_COUNT {
   uint64_t                             DSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_DS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_DS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->DSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_ERR_INT_num                  0x44040
#define GFX7_ERR_INT_length                    1
struct GFX7_ERR_INT {
   bool                                 PrimaryAGTTFaultStatus;
   bool                                 PrimaryBGTTFaultStatus;
   bool                                 SpriteAGTTFaultStatus;
   bool                                 SpriteBGTTFaultStatus;
   bool                                 CursorAGTTFaultStatus;
   bool                                 CursorBGTTFaultStatus;
   bool                                 Invalidpagetableentrydata;
   bool                                 InvalidGTTpagetableentry;
};

static inline __attribute__((always_inline)) void
GFX7_ERR_INT_pack(__attribute__((unused)) __gen_user_data *data,
                  __attribute__((unused)) void * restrict dst,
                  __attribute__((unused)) const struct GFX7_ERR_INT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PrimaryAGTTFaultStatus, 0, 0) |
      __gen_uint(values->PrimaryBGTTFaultStatus, 1, 1) |
      __gen_uint(values->SpriteAGTTFaultStatus, 2, 2) |
      __gen_uint(values->SpriteBGTTFaultStatus, 3, 3) |
      __gen_uint(values->CursorAGTTFaultStatus, 4, 4) |
      __gen_uint(values->CursorBGTTFaultStatus, 5, 5) |
      __gen_uint(values->Invalidpagetableentrydata, 6, 6) |
      __gen_uint(values->InvalidGTTpagetableentry, 7, 7);
}

#define GFX7_GFX_ARB_ERROR_RPT_num        0x40a0
#define GFX7_GFX_ARB_ERROR_RPT_length          1
struct GFX7_GFX_ARB_ERROR_RPT {
   bool                                 TLBPageFaultError;
   bool                                 ContextPageFaultError;
   bool                                 InvalidPageDirectoryentryerror;
   bool                                 HardwareStatusPageFaultError;
   bool                                 TLBPageVTDTranslationError;
   bool                                 ContextPageVTDTranslationError;
   bool                                 PageDirectoryEntryVTDTranslationError;
   bool                                 HardwareStatusPageVTDTranslationError;
   bool                                 UnloadedPDError;
};

static inline __attribute__((always_inline)) void
GFX7_GFX_ARB_ERROR_RPT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_GFX_ARB_ERROR_RPT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->TLBPageFaultError, 0, 0) |
      __gen_uint(values->ContextPageFaultError, 1, 1) |
      __gen_uint(values->InvalidPageDirectoryentryerror, 2, 2) |
      __gen_uint(values->HardwareStatusPageFaultError, 3, 3) |
      __gen_uint(values->TLBPageVTDTranslationError, 4, 4) |
      __gen_uint(values->ContextPageVTDTranslationError, 5, 5) |
      __gen_uint(values->PageDirectoryEntryVTDTranslationError, 6, 6) |
      __gen_uint(values->HardwareStatusPageVTDTranslationError, 7, 7) |
      __gen_uint(values->UnloadedPDError, 8, 8);
}

#define GFX7_GS_INVOCATION_COUNT_num      0x2328
#define GFX7_GS_INVOCATION_COUNT_length        2
struct GFX7_GS_INVOCATION_COUNT {
   uint64_t                             GSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_GS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_GS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->GSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_GS_PRIMITIVES_COUNT_num      0x2330
#define GFX7_GS_PRIMITIVES_COUNT_length        2
struct GFX7_GS_PRIMITIVES_COUNT {
   uint64_t                             GSPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_GS_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_GS_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->GSPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_HS_INVOCATION_COUNT_num      0x2300
#define GFX7_HS_INVOCATION_COUNT_length        2
struct GFX7_HS_INVOCATION_COUNT {
   uint64_t                             HSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_HS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_HS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->HSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_IA_PRIMITIVES_COUNT_num      0x2318
#define GFX7_IA_PRIMITIVES_COUNT_length        2
struct GFX7_IA_PRIMITIVES_COUNT {
   uint64_t                             IAPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_IA_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_IA_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->IAPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_IA_VERTICES_COUNT_num        0x2310
#define GFX7_IA_VERTICES_COUNT_length          2
struct GFX7_IA_VERTICES_COUNT {
   uint64_t                             IAVerticesCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_IA_VERTICES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX7_IA_VERTICES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->IAVerticesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_INSTDONE_1_num               0x206c
#define GFX7_INSTDONE_1_length                 1
struct GFX7_INSTDONE_1 {
   bool                                 PRB0RingEnable;
   bool                                 VFGDone;
   bool                                 VSDone;
   bool                                 HSDone;
   bool                                 TEDone;
   bool                                 DSDone;
   bool                                 GSDone;
   bool                                 SOLDone;
   bool                                 CLDone;
   bool                                 SFDone;
   bool                                 TDGDone;
   bool                                 URBMDone;
   bool                                 SVGDone;
   bool                                 GAFSDone;
   bool                                 VFEDone;
   bool                                 TSGDone;
   bool                                 GAFMDone;
   bool                                 GAMDone;
   bool                                 SDEDone;
   bool                                 RCCFBCCSDone;
};

static inline __attribute__((always_inline)) void
GFX7_INSTDONE_1_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_INSTDONE_1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PRB0RingEnable, 0, 0) |
      __gen_uint(values->VFGDone, 1, 1) |
      __gen_uint(values->VSDone, 2, 2) |
      __gen_uint(values->HSDone, 3, 3) |
      __gen_uint(values->TEDone, 4, 4) |
      __gen_uint(values->DSDone, 5, 5) |
      __gen_uint(values->GSDone, 6, 6) |
      __gen_uint(values->SOLDone, 7, 7) |
      __gen_uint(values->CLDone, 8, 8) |
      __gen_uint(values->SFDone, 9, 9) |
      __gen_uint(values->TDGDone, 12, 12) |
      __gen_uint(values->URBMDone, 13, 13) |
      __gen_uint(values->SVGDone, 14, 14) |
      __gen_uint(values->GAFSDone, 15, 15) |
      __gen_uint(values->VFEDone, 16, 16) |
      __gen_uint(values->TSGDone, 17, 17) |
      __gen_uint(values->GAFMDone, 18, 18) |
      __gen_uint(values->GAMDone, 19, 19) |
      __gen_uint(values->SDEDone, 22, 22) |
      __gen_uint(values->RCCFBCCSDone, 23, 23);
}

#define GFX7_INSTPM_num                   0x20c0
#define GFX7_INSTPM_length                     1
struct GFX7_INSTPM {
   bool                                 _3DStateInstructionDisable;
   bool                                 _3DRenderingInstructionDisable;
   bool                                 MediaInstructionDisable;
   bool                                 CONSTANT_BUFFERAddressOffsetDisable;
   bool                                 _3DStateInstructionDisableMask;
   bool                                 _3DRenderingInstructionDisableMask;
   bool                                 MediaInstructionDisableMask;
   bool                                 CONSTANT_BUFFERAddressOffsetDisableMask;
};

static inline __attribute__((always_inline)) void
GFX7_INSTPM_pack(__attribute__((unused)) __gen_user_data *data,
                 __attribute__((unused)) void * restrict dst,
                 __attribute__((unused)) const struct GFX7_INSTPM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->_3DStateInstructionDisable, 1, 1) |
      __gen_uint(values->_3DRenderingInstructionDisable, 2, 2) |
      __gen_uint(values->MediaInstructionDisable, 3, 3) |
      __gen_uint(values->CONSTANT_BUFFERAddressOffsetDisable, 6, 6) |
      __gen_uint(values->_3DStateInstructionDisableMask, 17, 17) |
      __gen_uint(values->_3DRenderingInstructionDisableMask, 18, 18) |
      __gen_uint(values->MediaInstructionDisableMask, 19, 19) |
      __gen_uint(values->CONSTANT_BUFFERAddressOffsetDisableMask, 22, 22);
}

#define GFX7_L3CNTLREG2_num               0xb020
#define GFX7_L3CNTLREG2_length                 1
struct GFX7_L3CNTLREG2 {
   bool                                 SLMEnable;
   uint32_t                             URBAllocation;
   bool                                 URBLowBandwidth;
   uint32_t                             ALLAllocation;
   uint32_t                             ROAllocation;
   bool                                 ROLowBandwidth;
   uint32_t                             DCAllocation;
   bool                                 DCLowBandwidth;
};

static inline __attribute__((always_inline)) void
GFX7_L3CNTLREG2_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_L3CNTLREG2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SLMEnable, 0, 0) |
      __gen_uint(values->URBAllocation, 1, 6) |
      __gen_uint(values->URBLowBandwidth, 7, 7) |
      __gen_uint(values->ALLAllocation, 8, 13) |
      __gen_uint(values->ROAllocation, 14, 19) |
      __gen_uint(values->ROLowBandwidth, 20, 20) |
      __gen_uint(values->DCAllocation, 21, 26) |
      __gen_uint(values->DCLowBandwidth, 27, 27);
}

#define GFX7_L3CNTLREG3_num               0xb024
#define GFX7_L3CNTLREG3_length                 1
struct GFX7_L3CNTLREG3 {
   uint32_t                             ISAllocation;
   bool                                 ISLowBandwidth;
   uint32_t                             CAllocation;
   bool                                 CLowBandwidth;
   uint32_t                             TAllocation;
   bool                                 TLowBandwidth;
};

static inline __attribute__((always_inline)) void
GFX7_L3CNTLREG3_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX7_L3CNTLREG3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ISAllocation, 1, 6) |
      __gen_uint(values->ISLowBandwidth, 7, 7) |
      __gen_uint(values->CAllocation, 8, 13) |
      __gen_uint(values->CLowBandwidth, 14, 14) |
      __gen_uint(values->TAllocation, 15, 20) |
      __gen_uint(values->TLowBandwidth, 21, 21);
}

#define GFX7_L3SQCREG1_num                0xb010
#define GFX7_L3SQCREG1_length                  1
struct GFX7_L3SQCREG1 {
   uint32_t                             L3SQGeneralPriorityCreditInitialization;
#define SQGPCI_DEFAULT                           7
#define BYT_SQGPCI_DEFAULT                       13
   uint32_t                             L3SQHighPriorityCreditInitialization;
#define SQHPCI_DEFAULT                           3
   bool                                 ConvertDC_UC;
   bool                                 ConvertIS_UC;
   bool                                 ConvertC_UC;
   bool                                 ConvertT_UC;
};

static inline __attribute__((always_inline)) void
GFX7_L3SQCREG1_pack(__attribute__((unused)) __gen_user_data *data,
                    __attribute__((unused)) void * restrict dst,
                    __attribute__((unused)) const struct GFX7_L3SQCREG1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->L3SQGeneralPriorityCreditInitialization, 20, 23) |
      __gen_uint(values->L3SQHighPriorityCreditInitialization, 16, 19) |
      __gen_uint(values->ConvertDC_UC, 24, 24) |
      __gen_uint(values->ConvertIS_UC, 25, 25) |
      __gen_uint(values->ConvertC_UC, 26, 26) |
      __gen_uint(values->ConvertT_UC, 27, 27);
}

#define GFX7_PS_INVOCATION_COUNT_num      0x2348
#define GFX7_PS_INVOCATION_COUNT_length        2
struct GFX7_PS_INVOCATION_COUNT {
   uint64_t                             PSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_PS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_PS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_RCS_FAULT_REG_num            0x4094
#define GFX7_RCS_FAULT_REG_length              1
struct GFX7_RCS_FAULT_REG {
   bool                                 ValidBit;
   uint32_t                             FaultType;
#define PageFault                                0
#define InvalidPDFault                           1
#define UnloadedPDFault                          2
#define InvalidandUnloadedPDfault                3
   uint32_t                             SRCIDofFault;
   uint32_t                             GTTSEL;
#define PPGTT                                    0
#define GGTT                                     1
   __gen_address_type                   VirtualAddressofFault;
};

static inline __attribute__((always_inline)) void
GFX7_RCS_FAULT_REG_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_RCS_FAULT_REG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint32_t v0 =
      __gen_uint(values->ValidBit, 0, 0) |
      __gen_uint(values->FaultType, 1, 2) |
      __gen_uint(values->SRCIDofFault, 3, 10) |
      __gen_uint(values->GTTSEL, 11, 11);
   dw[0] = __gen_address(data, &dw[0], values->VirtualAddressofFault, v0, 12, 31);
}

#define GFX7_RCS_RING_BUFFER_CTL_num      0x203c
#define GFX7_RCS_RING_BUFFER_CTL_length        1
struct GFX7_RCS_RING_BUFFER_CTL {
   bool                                 RingBufferEnable;
   uint32_t                             AutomaticReportHeadPointer;
#define MI_AUTOREPORT_OFF                        0
#define MI_AUTOREPORT_64KBMI_AUTOREPORT_4KB      1
#define MI_AUTOREPORT_128KB                      3
   bool                                 SemaphoreWait;
   bool                                 RBWait;
   uint32_t                             BufferLengthinpages1;
};

static inline __attribute__((always_inline)) void
GFX7_RCS_RING_BUFFER_CTL_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_RCS_RING_BUFFER_CTL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingBufferEnable, 0, 0) |
      __gen_uint(values->AutomaticReportHeadPointer, 1, 2) |
      __gen_uint(values->SemaphoreWait, 10, 10) |
      __gen_uint(values->RBWait, 11, 11) |
      __gen_uint(values->BufferLengthinpages1, 12, 20);
}

#define GFX7_ROW_INSTDONE_num             0xe164
#define GFX7_ROW_INSTDONE_length               1
struct GFX7_ROW_INSTDONE {
   bool                                 BCDone;
   bool                                 PSDDone;
   bool                                 DCDone;
   bool                                 DAPRDone;
   bool                                 TDLDone;
   bool                                 GWDone;
   bool                                 ICDone;
   bool                                 EU00DoneSS0;
   bool                                 EU01DoneSS0;
   bool                                 EU02DoneSS0;
   bool                                 EU03DoneSS0;
   bool                                 MA0DoneSS0;
   bool                                 EU10DoneSS0;
   bool                                 EU11DoneSS0;
   bool                                 EU12DoneSS0;
   bool                                 EU13DoneSS0;
   bool                                 MA1Done;
};

static inline __attribute__((always_inline)) void
GFX7_ROW_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_ROW_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BCDone, 0, 0) |
      __gen_uint(values->PSDDone, 1, 1) |
      __gen_uint(values->DCDone, 2, 2) |
      __gen_uint(values->DAPRDone, 3, 3) |
      __gen_uint(values->TDLDone, 6, 6) |
      __gen_uint(values->GWDone, 8, 8) |
      __gen_uint(values->ICDone, 12, 12) |
      __gen_uint(values->EU00DoneSS0, 16, 16) |
      __gen_uint(values->EU01DoneSS0, 17, 17) |
      __gen_uint(values->EU02DoneSS0, 18, 18) |
      __gen_uint(values->EU03DoneSS0, 19, 19) |
      __gen_uint(values->MA0DoneSS0, 20, 20) |
      __gen_uint(values->EU10DoneSS0, 21, 21) |
      __gen_uint(values->EU11DoneSS0, 22, 22) |
      __gen_uint(values->EU12DoneSS0, 23, 23) |
      __gen_uint(values->EU13DoneSS0, 24, 24) |
      __gen_uint(values->MA1Done, 25, 25);
}

#define GFX7_RPSTAT1_num                  0xa01c
#define GFX7_RPSTAT1_length                    1
struct GFX7_RPSTAT1 {
   uint32_t                             PreviousGTFrequency;
   uint32_t                             CurrentGTFrequency;
};

static inline __attribute__((always_inline)) void
GFX7_RPSTAT1_pack(__attribute__((unused)) __gen_user_data *data,
                  __attribute__((unused)) void * restrict dst,
                  __attribute__((unused)) const struct GFX7_RPSTAT1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PreviousGTFrequency, 0, 6) |
      __gen_uint(values->CurrentGTFrequency, 7, 14);
}

#define GFX7_SAMPLER_INSTDONE_num         0xe160
#define GFX7_SAMPLER_INSTDONE_length           1
struct GFX7_SAMPLER_INSTDONE {
   bool                                 VMEDone;
   bool                                 PL0Done;
   bool                                 SO0Done;
   bool                                 DG0Done;
   bool                                 FT0Done;
   bool                                 DM0Done;
   bool                                 SCDone;
   bool                                 FL0Done;
   bool                                 QCDone;
   bool                                 SVSMDone;
   bool                                 SI0Done;
   bool                                 MT0Done;
   bool                                 AVSDone;
   bool                                 IEFDone;
   bool                                 VDIDone;
   bool                                 SVSMARB3;
   bool                                 SVSMARB2;
   bool                                 SVSMARB1;
   bool                                 SVSMAdapter;
};

static inline __attribute__((always_inline)) void
GFX7_SAMPLER_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SAMPLER_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->VMEDone, 0, 0) |
      __gen_uint(values->PL0Done, 1, 1) |
      __gen_uint(values->SO0Done, 2, 2) |
      __gen_uint(values->DG0Done, 3, 3) |
      __gen_uint(values->FT0Done, 4, 4) |
      __gen_uint(values->DM0Done, 5, 5) |
      __gen_uint(values->SCDone, 6, 6) |
      __gen_uint(values->FL0Done, 7, 7) |
      __gen_uint(values->QCDone, 8, 8) |
      __gen_uint(values->SVSMDone, 9, 9) |
      __gen_uint(values->SI0Done, 10, 10) |
      __gen_uint(values->MT0Done, 11, 11) |
      __gen_uint(values->AVSDone, 12, 12) |
      __gen_uint(values->IEFDone, 13, 13) |
      __gen_uint(values->VDIDone, 14, 14) |
      __gen_uint(values->SVSMARB3, 15, 15) |
      __gen_uint(values->SVSMARB2, 16, 16) |
      __gen_uint(values->SVSMARB1, 17, 17) |
      __gen_uint(values->SVSMAdapter, 18, 18);
}

#define GFX7_SC_INSTDONE_num              0x7100
#define GFX7_SC_INSTDONE_length                1
struct GFX7_SC_INSTDONE {
   bool                                 SVLDone;
   bool                                 WMFEDone;
   bool                                 WMBEDone;
   bool                                 HIZDone;
   bool                                 STCDone;
   bool                                 IZDone;
   bool                                 SBEDone;
   bool                                 RCZDone;
   bool                                 RCCDone;
   bool                                 RCPBEDone;
   bool                                 RCPFEDone;
   bool                                 DAPBDone;
   bool                                 DAPRBEDone;
   bool                                 IECPDone;
   bool                                 SARBDone;
   bool                                 VSCDone;
};

static inline __attribute__((always_inline)) void
GFX7_SC_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX7_SC_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SVLDone, 0, 0) |
      __gen_uint(values->WMFEDone, 1, 1) |
      __gen_uint(values->WMBEDone, 2, 2) |
      __gen_uint(values->HIZDone, 3, 3) |
      __gen_uint(values->STCDone, 4, 4) |
      __gen_uint(values->IZDone, 5, 5) |
      __gen_uint(values->SBEDone, 6, 6) |
      __gen_uint(values->RCZDone, 8, 8) |
      __gen_uint(values->RCCDone, 9, 9) |
      __gen_uint(values->RCPBEDone, 10, 10) |
      __gen_uint(values->RCPFEDone, 11, 11) |
      __gen_uint(values->DAPBDone, 12, 12) |
      __gen_uint(values->DAPRBEDone, 13, 13) |
      __gen_uint(values->IECPDone, 14, 14) |
      __gen_uint(values->SARBDone, 15, 15) |
      __gen_uint(values->VSCDone, 16, 16);
}

#define GFX7_SO_NUM_PRIMS_WRITTEN0_num    0x5200
#define GFX7_SO_NUM_PRIMS_WRITTEN0_length      2
struct GFX7_SO_NUM_PRIMS_WRITTEN0 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_NUM_PRIMS_WRITTEN0_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_SO_NUM_PRIMS_WRITTEN0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_NUM_PRIMS_WRITTEN1_num    0x5208
#define GFX7_SO_NUM_PRIMS_WRITTEN1_length      2
struct GFX7_SO_NUM_PRIMS_WRITTEN1 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_NUM_PRIMS_WRITTEN1_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_SO_NUM_PRIMS_WRITTEN1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_NUM_PRIMS_WRITTEN2_num    0x5210
#define GFX7_SO_NUM_PRIMS_WRITTEN2_length      2
struct GFX7_SO_NUM_PRIMS_WRITTEN2 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_NUM_PRIMS_WRITTEN2_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_SO_NUM_PRIMS_WRITTEN2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_NUM_PRIMS_WRITTEN3_num    0x5218
#define GFX7_SO_NUM_PRIMS_WRITTEN3_length      2
struct GFX7_SO_NUM_PRIMS_WRITTEN3 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_NUM_PRIMS_WRITTEN3_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX7_SO_NUM_PRIMS_WRITTEN3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_PRIM_STORAGE_NEEDED0_num  0x5240
#define GFX7_SO_PRIM_STORAGE_NEEDED0_length      2
struct GFX7_SO_PRIM_STORAGE_NEEDED0 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_PRIM_STORAGE_NEEDED0_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_SO_PRIM_STORAGE_NEEDED0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_PRIM_STORAGE_NEEDED1_num  0x5248
#define GFX7_SO_PRIM_STORAGE_NEEDED1_length      2
struct GFX7_SO_PRIM_STORAGE_NEEDED1 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_PRIM_STORAGE_NEEDED1_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_SO_PRIM_STORAGE_NEEDED1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_PRIM_STORAGE_NEEDED2_num  0x5250
#define GFX7_SO_PRIM_STORAGE_NEEDED2_length      2
struct GFX7_SO_PRIM_STORAGE_NEEDED2 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_PRIM_STORAGE_NEEDED2_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_SO_PRIM_STORAGE_NEEDED2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_PRIM_STORAGE_NEEDED3_num  0x5258
#define GFX7_SO_PRIM_STORAGE_NEEDED3_length      2
struct GFX7_SO_PRIM_STORAGE_NEEDED3 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX7_SO_PRIM_STORAGE_NEEDED3_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX7_SO_PRIM_STORAGE_NEEDED3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX7_SO_WRITE_OFFSET0_num         0x5280
#define GFX7_SO_WRITE_OFFSET0_length           1
struct GFX7_SO_WRITE_OFFSET0 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX7_SO_WRITE_OFFSET0_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SO_WRITE_OFFSET0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX7_SO_WRITE_OFFSET1_num         0x5284
#define GFX7_SO_WRITE_OFFSET1_length           1
struct GFX7_SO_WRITE_OFFSET1 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX7_SO_WRITE_OFFSET1_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SO_WRITE_OFFSET1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX7_SO_WRITE_OFFSET2_num         0x5288
#define GFX7_SO_WRITE_OFFSET2_length           1
struct GFX7_SO_WRITE_OFFSET2 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX7_SO_WRITE_OFFSET2_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SO_WRITE_OFFSET2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX7_SO_WRITE_OFFSET3_num         0x528c
#define GFX7_SO_WRITE_OFFSET3_length           1
struct GFX7_SO_WRITE_OFFSET3 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX7_SO_WRITE_OFFSET3_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX7_SO_WRITE_OFFSET3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX7_VCS_FAULT_REG_num            0x4194
#define GFX7_VCS_FAULT_REG_length              1
struct GFX7_VCS_FAULT_REG {
   bool                                 ValidBit;
   uint32_t                             FaultType;
#define PageFault                                0
#define InvalidPDFault                           1
#define UnloadedPDFault                          2
#define InvalidandUnloadedPDfault                3
   uint32_t                             SRCIDofFault;
   uint32_t                             GTTSEL;
#define PPGTT                                    0
#define GGTT                                     1
   __gen_address_type                   VirtualAddressofFault;
};

static inline __attribute__((always_inline)) void
GFX7_VCS_FAULT_REG_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX7_VCS_FAULT_REG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint32_t v0 =
      __gen_uint(values->ValidBit, 0, 0) |
      __gen_uint(values->FaultType, 1, 2) |
      __gen_uint(values->SRCIDofFault, 3, 10) |
      __gen_uint(values->GTTSEL, 11, 11);
   dw[0] = __gen_address(data, &dw[0], values->VirtualAddressofFault, v0, 12, 31);
}

#define GFX7_VCS_INSTDONE_num             0x1206c
#define GFX7_VCS_INSTDONE_length               1
struct GFX7_VCS_INSTDONE {
   bool                                 RingEnable;
   bool                                 USBDone;
   bool                                 QRCDone;
   bool                                 SECDone;
   bool                                 MPCDone;
   bool                                 VFTDone;
   bool                                 BSPDone;
   bool                                 VLFDone;
   bool                                 VOPDone;
   bool                                 VMCDone;
   bool                                 VIPDone;
   bool                                 VITDone;
   bool                                 VDSDone;
   bool                                 VMXDone;
   bool                                 VCPDone;
   bool                                 VCDDone;
   bool                                 VADDone;
   bool                                 VMDDone;
   bool                                 VISDone;
   bool                                 VACDone;
   bool                                 VAMDone;
   bool                                 JPGDone;
   bool                                 VBPDone;
   bool                                 VHRDone;
   bool                                 VCIDone;
   bool                                 VCRDone;
   bool                                 VINDone;
   bool                                 VPRDone;
   bool                                 VTQDone;
   bool                                 VCSDone;
   bool                                 GACDone;
};

static inline __attribute__((always_inline)) void
GFX7_VCS_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX7_VCS_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingEnable, 0, 0) |
      __gen_uint(values->USBDone, 1, 1) |
      __gen_uint(values->QRCDone, 2, 2) |
      __gen_uint(values->SECDone, 3, 3) |
      __gen_uint(values->MPCDone, 4, 4) |
      __gen_uint(values->VFTDone, 5, 5) |
      __gen_uint(values->BSPDone, 6, 6) |
      __gen_uint(values->VLFDone, 7, 7) |
      __gen_uint(values->VOPDone, 8, 8) |
      __gen_uint(values->VMCDone, 9, 9) |
      __gen_uint(values->VIPDone, 10, 10) |
      __gen_uint(values->VITDone, 11, 11) |
      __gen_uint(values->VDSDone, 12, 12) |
      __gen_uint(values->VMXDone, 13, 13) |
      __gen_uint(values->VCPDone, 14, 14) |
      __gen_uint(values->VCDDone, 15, 15) |
      __gen_uint(values->VADDone, 16, 16) |
      __gen_uint(values->VMDDone, 17, 17) |
      __gen_uint(values->VISDone, 18, 18) |
      __gen_uint(values->VACDone, 19, 19) |
      __gen_uint(values->VAMDone, 20, 20) |
      __gen_uint(values->JPGDone, 21, 21) |
      __gen_uint(values->VBPDone, 22, 22) |
      __gen_uint(values->VHRDone, 23, 23) |
      __gen_uint(values->VCIDone, 24, 24) |
      __gen_uint(values->VCRDone, 25, 25) |
      __gen_uint(values->VINDone, 26, 26) |
      __gen_uint(values->VPRDone, 27, 27) |
      __gen_uint(values->VTQDone, 28, 28) |
      __gen_uint(values->VCSDone, 30, 30) |
      __gen_uint(values->GACDone, 31, 31);
}

#define GFX7_VCS_RING_BUFFER_CTL_num      0x1203c
#define GFX7_VCS_RING_BUFFER_CTL_length        1
struct GFX7_VCS_RING_BUFFER_CTL {
   bool                                 RingBufferEnable;
   uint32_t                             AutomaticReportHeadPointer;
#define MI_AUTOREPORT_OFF                        0
#define MI_AUTOREPORT_64KB                       1
#define MI_AUTOREPORT_4KB                        2
#define MI_AUTOREPORT_128KB                      3
   bool                                 DisableRegisterAccesses;
   bool                                 SemaphoreWait;
   bool                                 RBWait;
   uint32_t                             BufferLengthinpages1;
};

static inline __attribute__((always_inline)) void
GFX7_VCS_RING_BUFFER_CTL_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_VCS_RING_BUFFER_CTL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingBufferEnable, 0, 0) |
      __gen_uint(values->AutomaticReportHeadPointer, 1, 2) |
      __gen_uint(values->DisableRegisterAccesses, 8, 8) |
      __gen_uint(values->SemaphoreWait, 10, 10) |
      __gen_uint(values->RBWait, 11, 11) |
      __gen_uint(values->BufferLengthinpages1, 12, 20);
}

#define GFX7_VS_INVOCATION_COUNT_num      0x2320
#define GFX7_VS_INVOCATION_COUNT_length        2
struct GFX7_VS_INVOCATION_COUNT {
   uint64_t                             VSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX7_VS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX7_VS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->VSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#endif /* GFX7_PACK_H */
