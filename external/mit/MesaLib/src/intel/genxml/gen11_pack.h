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


/* Instructions, enums and structures for ICL.
 *
 * This file has been generated, do not hand edit.
 */

#ifndef GFX11_PACK_H
#define GFX11_PACK_H

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


enum GFX11_3D_Color_Buffer_Blend_Factor {
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

enum GFX11_3D_Color_Buffer_Blend_Function {
   BLENDFUNCTION_ADD                    =      0,
   BLENDFUNCTION_SUBTRACT               =      1,
   BLENDFUNCTION_REVERSE_SUBTRACT       =      2,
   BLENDFUNCTION_MIN                    =      3,
   BLENDFUNCTION_MAX                    =      4,
};

enum GFX11_3D_Compare_Function {
   COMPAREFUNCTION_ALWAYS               =      0,
   COMPAREFUNCTION_NEVER                =      1,
   COMPAREFUNCTION_LESS                 =      2,
   COMPAREFUNCTION_EQUAL                =      3,
   COMPAREFUNCTION_LEQUAL               =      4,
   COMPAREFUNCTION_GREATER              =      5,
   COMPAREFUNCTION_NOTEQUAL             =      6,
   COMPAREFUNCTION_GEQUAL               =      7,
};

enum GFX11_3D_Logic_Op_Function {
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

enum GFX11_3D_Prim_Topo_Type {
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

enum GFX11_3D_Stencil_Operation {
   STENCILOP_KEEP                       =      0,
   STENCILOP_ZERO                       =      1,
   STENCILOP_REPLACE                    =      2,
   STENCILOP_INCRSAT                    =      3,
   STENCILOP_DECRSAT                    =      4,
   STENCILOP_INCR                       =      5,
   STENCILOP_DECR                       =      6,
   STENCILOP_INVERT                     =      7,
};

enum GFX11_3D_Vertex_Component_Control {
   VFCOMP_NOSTORE                       =      0,
   VFCOMP_STORE_SRC                     =      1,
   VFCOMP_STORE_0                       =      2,
   VFCOMP_STORE_1_FP                    =      3,
   VFCOMP_STORE_1_INT                   =      4,
   VFCOMP_STORE_PID                     =      7,
};

enum GFX11_Atomic_OPCODE {
   MI_ATOMIC_OP_AND                     =      1,
   MI_ATOMIC_OP_OR                      =      2,
   MI_ATOMIC_OP_XOR                     =      3,
   MI_ATOMIC_OP_MOVE                    =      4,
   MI_ATOMIC_OP_INC                     =      5,
   MI_ATOMIC_OP_DEC                     =      6,
   MI_ATOMIC_OP_ADD                     =      7,
   MI_ATOMIC_OP_SUB                     =      8,
   MI_ATOMIC_OP_RSUB                    =      9,
   MI_ATOMIC_OP_IMAX                    =     10,
   MI_ATOMIC_OP_IMIN                    =     11,
   MI_ATOMIC_OP_UMAX                    =     12,
   MI_ATOMIC_OP_UMIN                    =     13,
   MI_ATOMIC_OP_CMP_WR                  =     14,
   MI_ATOMIC_OP_PREDEC                  =     15,
   MI_ATOMIC_OP_AND8B                   =     33,
   MI_ATOMIC_OP_OR8B                    =     34,
   MI_ATOMIC_OP_XOR8B                   =     35,
   MI_ATOMIC_OP_MOVE8B                  =     36,
   MI_ATOMIC_OP_INC8B                   =     37,
   MI_ATOMIC_OP_DEC8B                   =     38,
   MI_ATOMIC_OP_ADD8B                   =     39,
   MI_ATOMIC_OP_SUB8B                   =     40,
   MI_ATOMIC_OP_RSUB8B                  =     41,
   MI_ATOMIC_OP_IMAX8B                  =     42,
   MI_ATOMIC_OP_IMIN8B                  =     43,
   MI_ATOMIC_OP_UMAX8B                  =     44,
   MI_ATOMIC_OP_UMIN8B                  =     45,
   MI_ATOMIC_OP_CMP_WR8B                =     46,
   MI_ATOMIC_OP_PREDEC8B                =     47,
   MI_ATOMIC_OP_CMP_WR16B               =     78,
};

enum GFX11_Attribute_Component_Format {
   ACF_DISABLED                         =      0,
   ACF_XY                               =      1,
   ACF_XYZ                              =      2,
   ACF_XYZW                             =      3,
};

enum GFX11_COMPONENT_ENABLES {
   CE_NONE                              =      0,
   CE_X                                 =      1,
   CE_Y                                 =      2,
   CE_XY                                =      3,
   CE_Z                                 =      4,
   CE_XZ                                =      5,
   CE_YZ                                =      6,
   CE_XYZ                               =      7,
   CE_W                                 =      8,
   CE_XW                                =      9,
   CE_YW                                =     10,
   CE_XYW                               =     11,
   CE_ZW                                =     12,
   CE_XZW                               =     13,
   CE_YZW                               =     14,
   CE_XYZW                              =     15,
};

enum GFX11_ShaderChannelSelect {
   SCS_ZERO                             =      0,
   SCS_ONE                              =      1,
   SCS_RED                              =      4,
   SCS_GREEN                            =      5,
   SCS_BLUE                             =      6,
   SCS_ALPHA                            =      7,
};

enum GFX11_TextureCoordinateMode {
   TCM_WRAP                             =      0,
   TCM_MIRROR                           =      1,
   TCM_CLAMP                            =      2,
   TCM_CUBE                             =      3,
   TCM_CLAMP_BORDER                     =      4,
   TCM_MIRROR_ONCE                      =      5,
   TCM_HALF_BORDER                      =      6,
   TCM_MIRROR_101                       =      7,
};

enum GFX11_WRAP_SHORTEST_ENABLE {
   WSE_X                                =      1,
   WSE_Y                                =      2,
   WSE_XY                               =      3,
   WSE_Z                                =      4,
   WSE_XZ                               =      5,
   WSE_YZ                               =      6,
   WSE_XYZ                              =      7,
   WSE_W                                =      8,
   WSE_XW                               =      9,
   WSE_YW                               =     10,
   WSE_XYW                              =     11,
   WSE_ZW                               =     12,
   WSE_XZW                              =     13,
   WSE_YZW                              =     14,
   WSE_XYZW                             =     15,
};

#define GFX11_3DSTATE_CONSTANT_BODY_length     10
struct GFX11_3DSTATE_CONSTANT_BODY {
   uint32_t                             ReadLength[4];
   __gen_address_type                   Buffer[4];
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ReadLength[0], 0, 15) |
      __gen_uint(values->ReadLength[1], 16, 31);

   dw[1] =
      __gen_uint(values->ReadLength[2], 0, 15) |
      __gen_uint(values->ReadLength[3], 16, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->Buffer[0], 0, 5, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->Buffer[1], 0, 5, 63);
   dw[4] = v4_address;
   dw[5] = v4_address >> 32;

   const uint64_t v6_address =
      __gen_address(data, &dw[6], values->Buffer[2], 0, 5, 63);
   dw[6] = v6_address;
   dw[7] = v6_address >> 32;

   const uint64_t v8_address =
      __gen_address(data, &dw[8], values->Buffer[3], 0, 5, 63);
   dw[8] = v8_address;
   dw[9] = v8_address >> 32;
}

#define GFX11_BINDING_TABLE_EDIT_ENTRY_length      1
struct GFX11_BINDING_TABLE_EDIT_ENTRY {
   uint64_t                             SurfaceStatePointer;
   uint32_t                             BindingTableIndex;
};

static inline __attribute__((always_inline)) void
GFX11_BINDING_TABLE_EDIT_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX11_BINDING_TABLE_EDIT_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->SurfaceStatePointer, 0, 15) |
      __gen_uint(values->BindingTableIndex, 16, 23);
}

#define GFX11_BINDING_TABLE_STATE_length       1
struct GFX11_BINDING_TABLE_STATE {
   uint64_t                             SurfaceStatePointer;
};

static inline __attribute__((always_inline)) void
GFX11_BINDING_TABLE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_BINDING_TABLE_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->SurfaceStatePointer, 6, 31);
}

#define GFX11_BLEND_STATE_ENTRY_length         2
struct GFX11_BLEND_STATE_ENTRY {
   bool                                 WriteDisableBlue;
   bool                                 WriteDisableGreen;
   bool                                 WriteDisableRed;
   bool                                 WriteDisableAlpha;
   enum GFX11_3D_Color_Buffer_Blend_Function AlphaBlendFunction;
   enum GFX11_3D_Color_Buffer_Blend_Factor DestinationAlphaBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Factor SourceAlphaBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Function ColorBlendFunction;
   enum GFX11_3D_Color_Buffer_Blend_Factor DestinationBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Factor SourceBlendFactor;
   bool                                 ColorBufferBlendEnable;
   bool                                 PostBlendColorClampEnable;
   bool                                 PreBlendColorClampEnable;
   uint32_t                             ColorClampRange;
#define COLORCLAMP_UNORM                         0
#define COLORCLAMP_SNORM                         1
#define COLORCLAMP_RTFORMAT                      2
   bool                                 PreBlendSourceOnlyClampEnable;
   enum GFX11_3D_Logic_Op_Function      LogicOpFunction;
   bool                                 LogicOpEnable;
};

static inline __attribute__((always_inline)) void
GFX11_BLEND_STATE_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_BLEND_STATE_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->WriteDisableBlue, 0, 0) |
      __gen_uint(values->WriteDisableGreen, 1, 1) |
      __gen_uint(values->WriteDisableRed, 2, 2) |
      __gen_uint(values->WriteDisableAlpha, 3, 3) |
      __gen_uint(values->AlphaBlendFunction, 5, 7) |
      __gen_uint(values->DestinationAlphaBlendFactor, 8, 12) |
      __gen_uint(values->SourceAlphaBlendFactor, 13, 17) |
      __gen_uint(values->ColorBlendFunction, 18, 20) |
      __gen_uint(values->DestinationBlendFactor, 21, 25) |
      __gen_uint(values->SourceBlendFactor, 26, 30) |
      __gen_uint(values->ColorBufferBlendEnable, 31, 31);

   dw[1] =
      __gen_uint(values->PostBlendColorClampEnable, 0, 0) |
      __gen_uint(values->PreBlendColorClampEnable, 1, 1) |
      __gen_uint(values->ColorClampRange, 2, 3) |
      __gen_uint(values->PreBlendSourceOnlyClampEnable, 4, 4) |
      __gen_uint(values->LogicOpFunction, 27, 30) |
      __gen_uint(values->LogicOpEnable, 31, 31);
}

#define GFX11_BLEND_STATE_length               1
struct GFX11_BLEND_STATE {
   uint32_t                             YDitherOffset;
   uint32_t                             XDitherOffset;
   bool                                 ColorDitherEnable;
   enum GFX11_3D_Compare_Function       AlphaTestFunction;
   bool                                 AlphaTestEnable;
   bool                                 AlphaToCoverageDitherEnable;
   bool                                 AlphaToOneEnable;
   bool                                 IndependentAlphaBlendEnable;
   bool                                 AlphaToCoverageEnable;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_BLEND_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_BLEND_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->YDitherOffset, 19, 20) |
      __gen_uint(values->XDitherOffset, 21, 22) |
      __gen_uint(values->ColorDitherEnable, 23, 23) |
      __gen_uint(values->AlphaTestFunction, 24, 26) |
      __gen_uint(values->AlphaTestEnable, 27, 27) |
      __gen_uint(values->AlphaToCoverageDitherEnable, 28, 28) |
      __gen_uint(values->AlphaToOneEnable, 29, 29) |
      __gen_uint(values->IndependentAlphaBlendEnable, 30, 30) |
      __gen_uint(values->AlphaToCoverageEnable, 31, 31);
}

#define GFX11_CC_VIEWPORT_length               2
struct GFX11_CC_VIEWPORT {
   float                                MinimumDepth;
   float                                MaximumDepth;
};

static inline __attribute__((always_inline)) void
GFX11_CC_VIEWPORT_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_CC_VIEWPORT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_float(values->MinimumDepth);

   dw[1] =
      __gen_float(values->MaximumDepth);
}

#define GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_length      2
struct GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY {
   float                                Table1XFilterCoefficientn2;
   float                                Table1YFilterCoefficientn2;
   float                                Table1XFilterCoefficientn3;
   float                                Table1YFilterCoefficientn3;
   float                                Table1XFilterCoefficientn4;
   float                                Table1YFilterCoefficientn4;
   float                                Table1XFilterCoefficientn5;
   float                                Table1YFilterCoefficientn5;
};

static inline __attribute__((always_inline)) void
GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->Table1XFilterCoefficientn2, 0, 7, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn2, 8, 15, 6) |
      __gen_sfixed(values->Table1XFilterCoefficientn3, 16, 23, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn3, 24, 31, 6);

   dw[1] =
      __gen_sfixed(values->Table1XFilterCoefficientn4, 0, 7, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn4, 8, 15, 6) |
      __gen_sfixed(values->Table1XFilterCoefficientn5, 16, 23, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn5, 24, 31, 6);
}

#define GFX11_CLEAR_COLOR_length               8
struct GFX11_CLEAR_COLOR {
   int32_t                              RawClearColorRed;
   int32_t                              RawClearColorGreen;
   int32_t                              RawClearColorBlue;
   int32_t                              RawClearColorAlpha;
   uint64_t                             ConvertedClearValueHiLow;
};

static inline __attribute__((always_inline)) void
GFX11_CLEAR_COLOR_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_CLEAR_COLOR * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sint(values->RawClearColorRed, 0, 31);

   dw[1] =
      __gen_sint(values->RawClearColorGreen, 0, 31);

   dw[2] =
      __gen_sint(values->RawClearColorBlue, 0, 31);

   dw[3] =
      __gen_sint(values->RawClearColorAlpha, 0, 31);

   const uint64_t v4 =
      __gen_uint(values->ConvertedClearValueHiLow, 0, 63);
   dw[4] = v4;
   dw[5] = v4 >> 32;

   dw[6] = 0;

   dw[7] = 0;
}

#define GFX11_COLOR_CALC_STATE_length          6
struct GFX11_COLOR_CALC_STATE {
   uint32_t                             AlphaTestFormat;
#define ALPHATEST_UNORM8                         0
#define ALPHATEST_FLOAT32                        1
   bool                                 RoundDisableFunctionDisable;
   uint32_t                             AlphaReferenceValueAsUNORM8;
   float                                AlphaReferenceValueAsFLOAT32;
   float                                BlendConstantColorRed;
   float                                BlendConstantColorGreen;
   float                                BlendConstantColorBlue;
   float                                BlendConstantColorAlpha;
};

static inline __attribute__((always_inline)) void
GFX11_COLOR_CALC_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_COLOR_CALC_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->AlphaTestFormat, 0, 0) |
      __gen_uint(values->RoundDisableFunctionDisable, 15, 15);

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

#define GFX11_EXECUTION_UNIT_EXTENDED_MESSAGE_DESCRIPTOR_length      1
struct GFX11_EXECUTION_UNIT_EXTENDED_MESSAGE_DESCRIPTOR {
   uint32_t                             TargetFunctionID;
   uint32_t                             EndOfThread;
#define NoTermination                            0
#define EOT                                      1
   uint32_t                             ExtendedMessageLength;
};

static inline __attribute__((always_inline)) void
GFX11_EXECUTION_UNIT_EXTENDED_MESSAGE_DESCRIPTOR_pack(__attribute__((unused)) __gen_user_data *data,
                                                      __attribute__((unused)) void * restrict dst,
                                                      __attribute__((unused)) const struct GFX11_EXECUTION_UNIT_EXTENDED_MESSAGE_DESCRIPTOR * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->TargetFunctionID, 0, 3) |
      __gen_uint(values->EndOfThread, 5, 5) |
      __gen_uint(values->ExtendedMessageLength, 6, 9);
}

#define GFX11_FILTER_COEFFICIENT_length        1
struct GFX11_FILTER_COEFFICIENT {
   float                                FilterCoefficient;
};

static inline __attribute__((always_inline)) void
GFX11_FILTER_COEFFICIENT_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_FILTER_COEFFICIENT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->FilterCoefficient, 0, 7, 6);
}

#define GFX11_FRAMEDELTAQP_length              2
struct GFX11_FRAMEDELTAQP {
   int32_t                              FrameDeltaQP[8];
};

static inline __attribute__((always_inline)) void
GFX11_FRAMEDELTAQP_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_FRAMEDELTAQP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sint(values->FrameDeltaQP[0], 0, 7) |
      __gen_sint(values->FrameDeltaQP[1], 8, 15) |
      __gen_sint(values->FrameDeltaQP[2], 16, 23) |
      __gen_sint(values->FrameDeltaQP[3], 24, 31);

   dw[1] =
      __gen_sint(values->FrameDeltaQP[4], 0, 7) |
      __gen_sint(values->FrameDeltaQP[5], 8, 15) |
      __gen_sint(values->FrameDeltaQP[6], 16, 23) |
      __gen_sint(values->FrameDeltaQP[7], 24, 31);
}

#define GFX11_FRAMEDELTAQPRANGE_length         2
struct GFX11_FRAMEDELTAQPRANGE {
   uint32_t                             FrameDeltaQPRange[8];
};

static inline __attribute__((always_inline)) void
GFX11_FRAMEDELTAQPRANGE_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_FRAMEDELTAQPRANGE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->FrameDeltaQPRange[0], 0, 7) |
      __gen_uint(values->FrameDeltaQPRange[1], 8, 15) |
      __gen_uint(values->FrameDeltaQPRange[2], 16, 23) |
      __gen_uint(values->FrameDeltaQPRange[3], 24, 31);

   dw[1] =
      __gen_uint(values->FrameDeltaQPRange[4], 0, 7) |
      __gen_uint(values->FrameDeltaQPRange[5], 8, 15) |
      __gen_uint(values->FrameDeltaQPRange[6], 16, 23) |
      __gen_uint(values->FrameDeltaQPRange[7], 24, 31);
}

#define GFX11_GATHER_CONSTANT_ENTRY_length      1
struct GFX11_GATHER_CONSTANT_ENTRY {
   uint32_t                             BindingTableIndexOffset;
   uint32_t                             ChannelMask;
   uint64_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_GATHER_CONSTANT_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_GATHER_CONSTANT_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BindingTableIndexOffset, 0, 3) |
      __gen_uint(values->ChannelMask, 4, 7) |
      __gen_offset(values->ConstantBufferOffset, 8, 15);
}

#define GFX11_HEVC_ARBITRATION_PRIORITY_length      1
struct GFX11_HEVC_ARBITRATION_PRIORITY {
   uint32_t                             Priority;
#define Highestpriority                          0
#define Secondhighestpriority                    1
#define Thirdhighestpriority                     2
#define Lowestpriority                           3
};

static inline __attribute__((always_inline)) void
GFX11_HEVC_ARBITRATION_PRIORITY_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_HEVC_ARBITRATION_PRIORITY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->Priority, 0, 1);
}

#define GFX11_MEMORYADDRESSATTRIBUTES_length      1
struct GFX11_MEMORYADDRESSATTRIBUTES {
   uint32_t                             MOCS;
   struct GFX11_HEVC_ARBITRATION_PRIORITY ArbitrationPriorityControl;
   bool                                 MemoryCompressionEnable;
   uint32_t                             MemoryCompressionMode;
   uint32_t                             RowStoreScratchBufferCacheSelect;
   uint32_t                             TiledResourceMode;
#define TRMODE_NONE                              0
#define TRMODE_TILEYF                            1
#define TRMODE_TILEYS                            2
};

static inline __attribute__((always_inline)) void
GFX11_MEMORYADDRESSATTRIBUTES_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_MEMORYADDRESSATTRIBUTES * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   uint32_t v0_0;
   GFX11_HEVC_ARBITRATION_PRIORITY_pack(data, &v0_0, &values->ArbitrationPriorityControl);

   dw[0] =
      __gen_uint(values->MOCS, 1, 6) |
      __gen_uint(v0_0, 7, 8) |
      __gen_uint(values->MemoryCompressionEnable, 9, 9) |
      __gen_uint(values->MemoryCompressionMode, 10, 10) |
      __gen_uint(values->RowStoreScratchBufferCacheSelect, 12, 12) |
      __gen_uint(values->TiledResourceMode, 13, 14);
}

#define GFX11_HCP_PAK_INSERT_OBJECT_INDIRECT_PAYLOAD_length      4
struct GFX11_HCP_PAK_INSERT_OBJECT_INDIRECT_PAYLOAD {
   uint32_t                             IndirectPayloadDataSizeinbits;
   __gen_address_type                   IndirectPayloadBaseAddress;
   struct GFX11_MEMORYADDRESSATTRIBUTES IndirectPayloadBaseAddress2;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_PAK_INSERT_OBJECT_INDIRECT_PAYLOAD_pack(__attribute__((unused)) __gen_user_data *data,
                                                  __attribute__((unused)) void * restrict dst,
                                                  __attribute__((unused)) const struct GFX11_HCP_PAK_INSERT_OBJECT_INDIRECT_PAYLOAD * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->IndirectPayloadDataSizeinbits, 0, 31);

   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->IndirectPayloadBaseAddress, 0, 0, 63);
   dw[1] = v1_address;
   dw[2] = v1_address >> 32;

   GFX11_MEMORYADDRESSATTRIBUTES_pack(data, &dw[3], &values->IndirectPayloadBaseAddress2);
}

#define GFX11_HCP_REF_LIST_ENTRY_length        1
struct GFX11_HCP_REF_LIST_ENTRY {
   uint32_t                             ReferencePicturetbValue;
   uint32_t                             ListEntry;
   uint32_t                             ChromaWeightedPrediction;
#define Default                                  0
#define Explicit                                 1
   uint32_t                             LumaWeightedPrediction;
#define Default                                  0
#define Explicit                                 1
   bool                                 LongTermReference;
   bool                                 FieldPic;
   bool                                 TopField;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_REF_LIST_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_HCP_REF_LIST_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ReferencePicturetbValue, 0, 7) |
      __gen_uint(values->ListEntry, 8, 10) |
      __gen_uint(values->ChromaWeightedPrediction, 11, 11) |
      __gen_uint(values->LumaWeightedPrediction, 12, 12) |
      __gen_uint(values->LongTermReference, 13, 13) |
      __gen_uint(values->FieldPic, 14, 14) |
      __gen_uint(values->TopField, 15, 15);
}

#define GFX11_HCP_TILE_POSITION_IN_CTB_length      1
struct GFX11_HCP_TILE_POSITION_IN_CTB {
   uint32_t                             CtbPos0i;
   uint32_t                             CtbPos1i;
   uint32_t                             CtbPos2i;
   uint32_t                             CtbPos3i;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_TILE_POSITION_IN_CTB_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX11_HCP_TILE_POSITION_IN_CTB * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->CtbPos0i, 0, 7) |
      __gen_uint(values->CtbPos1i, 8, 15) |
      __gen_uint(values->CtbPos2i, 16, 23) |
      __gen_uint(values->CtbPos3i, 24, 31);
}

#define GFX11_HCP_TILE_POSITION_IN_CTB_MSB_length      2
struct GFX11_HCP_TILE_POSITION_IN_CTB_MSB {
   uint32_t                             CtbRowPositionofTileColumn[22];
};

static inline __attribute__((always_inline)) void
GFX11_HCP_TILE_POSITION_IN_CTB_MSB_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX11_HCP_TILE_POSITION_IN_CTB_MSB * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->CtbRowPositionofTileColumn[0], 0, 1) |
      __gen_uint(values->CtbRowPositionofTileColumn[1], 2, 3) |
      __gen_uint(values->CtbRowPositionofTileColumn[2], 4, 5) |
      __gen_uint(values->CtbRowPositionofTileColumn[3], 6, 7) |
      __gen_uint(values->CtbRowPositionofTileColumn[4], 8, 9) |
      __gen_uint(values->CtbRowPositionofTileColumn[5], 10, 11) |
      __gen_uint(values->CtbRowPositionofTileColumn[6], 12, 13) |
      __gen_uint(values->CtbRowPositionofTileColumn[7], 14, 15) |
      __gen_uint(values->CtbRowPositionofTileColumn[8], 16, 17) |
      __gen_uint(values->CtbRowPositionofTileColumn[9], 18, 19) |
      __gen_uint(values->CtbRowPositionofTileColumn[10], 20, 21) |
      __gen_uint(values->CtbRowPositionofTileColumn[11], 22, 23) |
      __gen_uint(values->CtbRowPositionofTileColumn[12], 24, 25) |
      __gen_uint(values->CtbRowPositionofTileColumn[13], 26, 27) |
      __gen_uint(values->CtbRowPositionofTileColumn[14], 28, 29) |
      __gen_uint(values->CtbRowPositionofTileColumn[15], 30, 31);

   dw[1] =
      __gen_uint(values->CtbRowPositionofTileColumn[16], 0, 1) |
      __gen_uint(values->CtbRowPositionofTileColumn[17], 2, 3) |
      __gen_uint(values->CtbRowPositionofTileColumn[18], 4, 5) |
      __gen_uint(values->CtbRowPositionofTileColumn[19], 6, 7) |
      __gen_uint(values->CtbRowPositionofTileColumn[20], 8, 9) |
      __gen_uint(values->CtbRowPositionofTileColumn[21], 10, 11);
}

#define GFX11_HCP_WEIGHTOFFSET_CHROMA_ENTRY_length      1
struct GFX11_HCP_WEIGHTOFFSET_CHROMA_ENTRY {
   int32_t                              DeltaChromaWeightLX0;
   uint32_t                             ChromaOffsetLX0;
   int32_t                              DeltaChromaWeightLX1;
   uint32_t                             ChromaOffsetLX1;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_WEIGHTOFFSET_CHROMA_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_HCP_WEIGHTOFFSET_CHROMA_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sint(values->DeltaChromaWeightLX0, 0, 7) |
      __gen_uint(values->ChromaOffsetLX0, 8, 15) |
      __gen_sint(values->DeltaChromaWeightLX1, 16, 23) |
      __gen_uint(values->ChromaOffsetLX1, 24, 31);
}

#define GFX11_HCP_WEIGHTOFFSET_CHROMA_EXT_ENTRY_length      1
struct GFX11_HCP_WEIGHTOFFSET_CHROMA_EXT_ENTRY {
   uint32_t                             ChromaOffsetLX0MSBytei;
   uint32_t                             ChromaOffsetLX0MSBytei1;
   uint32_t                             ChromaOffsetLX1MSBytei;
   uint32_t                             ChromaOffsetLX1MSBytei1;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_WEIGHTOFFSET_CHROMA_EXT_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_HCP_WEIGHTOFFSET_CHROMA_EXT_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ChromaOffsetLX0MSBytei, 0, 7) |
      __gen_uint(values->ChromaOffsetLX0MSBytei1, 8, 15) |
      __gen_uint(values->ChromaOffsetLX1MSBytei, 16, 23) |
      __gen_uint(values->ChromaOffsetLX1MSBytei1, 24, 31);
}

#define GFX11_HCP_WEIGHTOFFSET_LUMA_ENTRY_length      1
struct GFX11_HCP_WEIGHTOFFSET_LUMA_ENTRY {
   int32_t                              DeltaLumaWeightLX;
   uint32_t                             LumaOffsetLX;
   uint32_t                             LumaOffsetLXMSByte;
};

static inline __attribute__((always_inline)) void
GFX11_HCP_WEIGHTOFFSET_LUMA_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX11_HCP_WEIGHTOFFSET_LUMA_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sint(values->DeltaLumaWeightLX, 0, 7) |
      __gen_uint(values->LumaOffsetLX, 8, 15) |
      __gen_uint(values->LumaOffsetLXMSByte, 24, 31);
}

#define GFX11_HEVC_VP9_RDOQ_LAMBDA_FIELDS_length      1
struct GFX11_HEVC_VP9_RDOQ_LAMBDA_FIELDS {
   uint32_t                             LambdaValue0;
   uint32_t                             LambdaValue1;
};

static inline __attribute__((always_inline)) void
GFX11_HEVC_VP9_RDOQ_LAMBDA_FIELDS_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX11_HEVC_VP9_RDOQ_LAMBDA_FIELDS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->LambdaValue0, 0, 15) |
      __gen_uint(values->LambdaValue1, 16, 31);
}

#define GFX11_HUC_VIRTUAL_ADDR_REGION_length      3
struct GFX11_HUC_VIRTUAL_ADDR_REGION {
   __gen_address_type                   Address;
   struct GFX11_MEMORYADDRESSATTRIBUTES MemoryAddressAttributes;
};

static inline __attribute__((always_inline)) void
GFX11_HUC_VIRTUAL_ADDR_REGION_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_HUC_VIRTUAL_ADDR_REGION * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0_address =
      __gen_address(data, &dw[0], values->Address, 0, 0, 63);
   dw[0] = v0_address;
   dw[1] = v0_address >> 32;

   GFX11_MEMORYADDRESSATTRIBUTES_pack(data, &dw[2], &values->MemoryAddressAttributes);
}

#define GFX11_IMAGE_STATE_COST_length          2
struct GFX11_IMAGE_STATE_COST {
   uint32_t                             MV0Cost;
   uint32_t                             MV1Cost;
   uint32_t                             MV2Cost;
   uint32_t                             MV3Cost;
   uint32_t                             MV4Cost;
   uint32_t                             MV5Cost;
   uint32_t                             MV6Cost;
   uint32_t                             MV7Cost;
};

static inline __attribute__((always_inline)) void
GFX11_IMAGE_STATE_COST_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_IMAGE_STATE_COST * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MV0Cost, 0, 7) |
      __gen_uint(values->MV1Cost, 8, 15) |
      __gen_uint(values->MV2Cost, 16, 23) |
      __gen_uint(values->MV3Cost, 24, 31);

   dw[1] =
      __gen_uint(values->MV4Cost, 0, 7) |
      __gen_uint(values->MV5Cost, 8, 15) |
      __gen_uint(values->MV6Cost, 16, 23) |
      __gen_uint(values->MV7Cost, 24, 31);
}

#define GFX11_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT_length      3
struct GFX11_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT {
   bool                                 MBErrorConcealmentPSliceWeightPredictionDisable;
   bool                                 MBErrorConcealmentPSliceMotionVectorsOverrideDisable;
   bool                                 MBErrorConcealmentBSpatialWeightPredictionDisable;
   bool                                 MBErrorConcealmentBSpatialMotionVectorsOverrideDisable;
   uint32_t                             MBErrorConcealmentBSpatialPredictionMode;
   bool                                 MBHeaderErrorHandling;
   bool                                 EntropyErrorHandling;
   bool                                 MPRErrorHandling;
   bool                                 BSDPrematureCompleteErrorHandling;
   uint32_t                             ConcealmentPictureID;
   bool                                 MBErrorConcealmentBTemporalWeightPredictionDisable;
   bool                                 MBErrorConcealmentBTemporalMotionVectorsOverrideEnable;
   uint32_t                             MBErrorConcealmentBTemporalPredictionMode;
   bool                                 IntraPredMode4x48x8LumaErrorControl;
   bool                                 InitCurrentMBNumber;
   uint32_t                             ConcealmentMethod;
   uint32_t                             FirstMBBitOffset;
   bool                                 LastSlice;
   bool                                 EmulationPreventionBytePresent;
   bool                                 FixPrevMBSkipped;
   uint32_t                             FirstMBByteOffsetofSliceDataorSliceHeader;
   bool                                 IntraPredictionErrorControl;
   bool                                 Intra8x84x4PredictionErrorConcealmentControl;
   uint32_t                             BSliceTemporalInterConcealmentMode;
   uint32_t                             BSliceSpatialInterConcealmentMode;
   uint32_t                             BSliceInterDirectTypeConcealmentMode;
   uint32_t                             BSliceConcealmentMode;
#define IntraConcealment                         1
#define InterConcealment                         0
   uint32_t                             PSliceInterConcealmentMode;
   uint32_t                             PSliceConcealmentMode;
#define IntraConcealment                         1
#define InterConcealment                         0
   uint32_t                             ConcealmentReferencePictureFieldBit;
   uint32_t                             ISliceConcealmentMode;
#define IntraConcealment                         1
#define InterConcealment                         0
};

static inline __attribute__((always_inline)) void
GFX11_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT_pack(__attribute__((unused)) __gen_user_data *data,
                                                          __attribute__((unused)) void * restrict dst,
                                                          __attribute__((unused)) const struct GFX11_INLINE_DATA_DESCRIPTION_FOR_MFD_AVC_BSD_OBJECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MBErrorConcealmentPSliceWeightPredictionDisable, 0, 0) |
      __gen_uint(values->MBErrorConcealmentPSliceMotionVectorsOverrideDisable, 1, 1) |
      __gen_uint(values->MBErrorConcealmentBSpatialWeightPredictionDisable, 3, 3) |
      __gen_uint(values->MBErrorConcealmentBSpatialMotionVectorsOverrideDisable, 4, 4) |
      __gen_uint(values->MBErrorConcealmentBSpatialPredictionMode, 6, 7) |
      __gen_uint(values->MBHeaderErrorHandling, 8, 8) |
      __gen_uint(values->EntropyErrorHandling, 10, 10) |
      __gen_uint(values->MPRErrorHandling, 12, 12) |
      __gen_uint(values->BSDPrematureCompleteErrorHandling, 14, 14) |
      __gen_uint(values->ConcealmentPictureID, 16, 21) |
      __gen_uint(values->MBErrorConcealmentBTemporalWeightPredictionDisable, 24, 24) |
      __gen_uint(values->MBErrorConcealmentBTemporalMotionVectorsOverrideEnable, 25, 25) |
      __gen_uint(values->MBErrorConcealmentBTemporalPredictionMode, 27, 28) |
      __gen_uint(values->IntraPredMode4x48x8LumaErrorControl, 29, 29) |
      __gen_uint(values->InitCurrentMBNumber, 30, 30) |
      __gen_uint(values->ConcealmentMethod, 31, 31);

   dw[1] =
      __gen_uint(values->FirstMBBitOffset, 0, 2) |
      __gen_uint(values->LastSlice, 3, 3) |
      __gen_uint(values->EmulationPreventionBytePresent, 4, 4) |
      __gen_uint(values->FixPrevMBSkipped, 7, 7) |
      __gen_uint(values->FirstMBByteOffsetofSliceDataorSliceHeader, 16, 31);

   dw[2] =
      __gen_uint(values->IntraPredictionErrorControl, 0, 0) |
      __gen_uint(values->Intra8x84x4PredictionErrorConcealmentControl, 1, 1) |
      __gen_uint(values->BSliceTemporalInterConcealmentMode, 4, 6) |
      __gen_uint(values->BSliceSpatialInterConcealmentMode, 8, 10) |
      __gen_uint(values->BSliceInterDirectTypeConcealmentMode, 12, 13) |
      __gen_uint(values->BSliceConcealmentMode, 15, 15) |
      __gen_uint(values->PSliceInterConcealmentMode, 16, 18) |
      __gen_uint(values->PSliceConcealmentMode, 23, 23) |
      __gen_uint(values->ConcealmentReferencePictureFieldBit, 24, 29) |
      __gen_uint(values->ISliceConcealmentMode, 31, 31);
}

#define GFX11_INTERFACE_DESCRIPTOR_DATA_length      8
struct GFX11_INTERFACE_DESCRIPTOR_DATA {
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
   uint32_t                             DenormMode;
#define Ftz                                      0
#define SetByKernel                              1
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
   bool                                 GlobalBarrierEnable;
   uint32_t                             SharedLocalMemorySize;
#define Encodes0K                                0
#define Encodes1K                                1
#define Encodes2K                                2
#define Encodes4K                                3
#define Encodes8K                                4
#define Encodes16K                               5
#define Encodes32K                               6
#define Encodes64K                               7
   bool                                 BarrierEnable;
   uint32_t                             RoundingMode;
#define RTNE                                     0
#define RU                                       1
#define RD                                       2
#define RTZ                                      3
   uint32_t                             CrossThreadConstantDataReadLength;
};

static inline __attribute__((always_inline)) void
GFX11_INTERFACE_DESCRIPTOR_DATA_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_INTERFACE_DESCRIPTOR_DATA * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_offset(values->KernelStartPointer, 6, 47);
   dw[0] = v0;
   dw[1] = v0 >> 32;

   dw[2] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadPriority, 17, 17) |
      __gen_uint(values->SingleProgramFlow, 18, 18) |
      __gen_uint(values->DenormMode, 19, 19);

   dw[3] =
      __gen_uint(values->SamplerCount, 2, 4) |
      __gen_offset(values->SamplerStatePointer, 5, 31);

   dw[4] =
      __gen_uint(values->BindingTableEntryCount, 0, 4) |
      __gen_offset(values->BindingTablePointer, 5, 15);

   dw[5] =
      __gen_uint(values->ConstantURBEntryReadOffset, 0, 15) |
      __gen_uint(values->ConstantURBEntryReadLength, 16, 31);

   dw[6] =
      __gen_uint(values->NumberofThreadsinGPGPUThreadGroup, 0, 9) |
      __gen_uint(values->GlobalBarrierEnable, 15, 15) |
      __gen_uint(values->SharedLocalMemorySize, 16, 20) |
      __gen_uint(values->BarrierEnable, 21, 21) |
      __gen_uint(values->RoundingMode, 22, 23);

   dw[7] =
      __gen_uint(values->CrossThreadConstantDataReadLength, 0, 7);
}

#define GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_length      4
struct GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY {
   float                                Table0XFilterCoefficientn0;
   float                                Table0YFilterCoefficientn0;
   float                                Table0XFilterCoefficientn1;
   float                                Table0YFilterCoefficientn1;
   float                                Table0XFilterCoefficientn2;
   float                                Table0YFilterCoefficientn2;
   float                                Table0XFilterCoefficientn3;
   float                                Table0YFilterCoefficientn3;
   float                                Table0XFilterCoefficientn4;
   float                                Table0YFilterCoefficientn4;
   float                                Table0XFilterCoefficientn5;
   float                                Table0YFilterCoefficientn5;
   float                                Table0XFilterCoefficientn6;
   float                                Table0YFilterCoefficientn6;
   float                                Table0XFilterCoefficientn7;
   float                                Table0YFilterCoefficientn7;
};

static inline __attribute__((always_inline)) void
GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->Table0XFilterCoefficientn0, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn0, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn1, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn1, 24, 31, 6);

   dw[1] =
      __gen_sfixed(values->Table0XFilterCoefficientn2, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn2, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn3, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn3, 24, 31, 6);

   dw[2] =
      __gen_sfixed(values->Table0XFilterCoefficientn4, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn4, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn5, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn5, 24, 31, 6);

   dw[3] =
      __gen_sfixed(values->Table0XFilterCoefficientn6, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn6, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn7, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn7, 24, 31, 6);
}

#define GFX11_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION_length      2
struct GFX11_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION {
   uint32_t                             FirstMBBitOffset;
   bool                                 LastMB;
   bool                                 LastPicSlice;
   uint32_t                             SliceConcealmentType;
   uint32_t                             SliceConcealmentOverride;
   uint32_t                             MBCount;
   uint32_t                             SliceVerticalPosition;
   uint32_t                             SliceHorizontalPosition;
   uint32_t                             NextSliceHorizontalPosition;
   uint32_t                             NextSliceVerticalPosition;
   uint32_t                             QuantizerScaleCode;
};

static inline __attribute__((always_inline)) void
GFX11_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION_pack(__attribute__((unused)) __gen_user_data *data,
                                                        __attribute__((unused)) void * restrict dst,
                                                        __attribute__((unused)) const struct GFX11_MFD_MPEG2_BSD_OBJECT_INLINE_DATA_DESCRIPTION * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->FirstMBBitOffset, 0, 2) |
      __gen_uint(values->LastMB, 3, 3) |
      __gen_uint(values->LastPicSlice, 5, 5) |
      __gen_uint(values->SliceConcealmentType, 6, 6) |
      __gen_uint(values->SliceConcealmentOverride, 7, 7) |
      __gen_uint(values->MBCount, 8, 15) |
      __gen_uint(values->SliceVerticalPosition, 16, 23) |
      __gen_uint(values->SliceHorizontalPosition, 24, 31);

   dw[1] =
      __gen_uint(values->NextSliceHorizontalPosition, 0, 7) |
      __gen_uint(values->NextSliceVerticalPosition, 8, 16) |
      __gen_uint(values->QuantizerScaleCode, 24, 28);
}

#define GFX11_MI_MATH_ALU_INSTRUCTION_length      1
struct GFX11_MI_MATH_ALU_INSTRUCTION {
   uint32_t                             Operand2;
#define MI_ALU_REG0                              0
#define MI_ALU_REG1                              1
#define MI_ALU_REG2                              2
#define MI_ALU_REG3                              3
#define MI_ALU_REG4                              4
#define MI_ALU_REG5                              5
#define MI_ALU_REG6                              6
#define MI_ALU_REG7                              7
#define MI_ALU_REG8                              8
#define MI_ALU_REG9                              9
#define MI_ALU_REG10                             10
#define MI_ALU_REG11                             11
#define MI_ALU_REG12                             12
#define MI_ALU_REG13                             13
#define MI_ALU_REG14                             14
#define MI_ALU_REG15                             15
#define MI_ALU_SRCA                              32
#define MI_ALU_SRCB                              33
#define MI_ALU_ACCU                              49
#define MI_ALU_ZF                                50
#define MI_ALU_CF                                51
   uint32_t                             Operand1;
#define MI_ALU_REG0                              0
#define MI_ALU_REG1                              1
#define MI_ALU_REG2                              2
#define MI_ALU_REG3                              3
#define MI_ALU_REG4                              4
#define MI_ALU_REG5                              5
#define MI_ALU_REG6                              6
#define MI_ALU_REG7                              7
#define MI_ALU_REG8                              8
#define MI_ALU_REG9                              9
#define MI_ALU_REG10                             10
#define MI_ALU_REG11                             11
#define MI_ALU_REG12                             12
#define MI_ALU_REG13                             13
#define MI_ALU_REG14                             14
#define MI_ALU_REG15                             15
#define MI_ALU_SRCA                              32
#define MI_ALU_SRCB                              33
#define MI_ALU_ACCU                              49
#define MI_ALU_ZF                                50
#define MI_ALU_CF                                51
   uint32_t                             ALUOpcode;
#define MI_ALU_NOOP                              0
#define MI_ALU_LOAD                              128
#define MI_ALU_LOADINV                           1152
#define MI_ALU_LOAD0                             129
#define MI_ALU_LOAD1                             1153
#define MI_ALU_ADD                               256
#define MI_ALU_SUB                               257
#define MI_ALU_AND                               258
#define MI_ALU_OR                                259
#define MI_ALU_XOR                               260
#define MI_ALU_STORE                             384
#define MI_ALU_STOREINV                          1408
};

static inline __attribute__((always_inline)) void
GFX11_MI_MATH_ALU_INSTRUCTION_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_MI_MATH_ALU_INSTRUCTION * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->Operand2, 0, 9) |
      __gen_uint(values->Operand1, 10, 19) |
      __gen_uint(values->ALUOpcode, 20, 31);
}

#define GFX11_PALETTE_ENTRY_length             1
struct GFX11_PALETTE_ENTRY {
   uint32_t                             Blue;
   uint32_t                             Green;
   uint32_t                             Red;
   uint32_t                             Alpha;
};

static inline __attribute__((always_inline)) void
GFX11_PALETTE_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_PALETTE_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->Blue, 0, 7) |
      __gen_uint(values->Green, 8, 15) |
      __gen_uint(values->Red, 16, 23) |
      __gen_uint(values->Alpha, 24, 31);
}

#define GFX11_RENDER_SURFACE_STATE_length     16
struct GFX11_RENDER_SURFACE_STATE {
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
#define WriteOnlyCache                           0
#define ReadWriteCache                           1
   bool                                 SamplerL2BypassModeDisable;
   uint32_t                             VerticalLineStrideOffset;
   uint32_t                             VerticalLineStride;
   uint32_t                             TileMode;
#define LINEAR                                   0
#define WMAJOR                                   1
#define XMAJOR                                   2
#define YMAJOR                                   3
   uint32_t                             SurfaceHorizontalAlignment;
#define HALIGN_4                                 1
#define HALIGN_8                                 2
#define HALIGN_16                                3
   uint32_t                             SurfaceVerticalAlignment;
#define VALIGN_4                                 1
#define VALIGN_8                                 2
#define VALIGN_16                                3
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
   uint32_t                             SurfaceQPitch;
   float                                BaseMipLevel;
   uint32_t                             MOCS;
   bool                                 EnableUnormPathInColorPipe;
   uint32_t                             Width;
   uint32_t                             Height;
   uint32_t                             SurfacePitch;
   uint32_t                             TileAddressMappingMode;
#define Gfx9                                     0
#define Gfx10                                    1
   uint32_t                             Depth;
   uint32_t                             MultisamplePositionPaletteIndex;
   uint32_t                             NumberofMultisamples;
#define MULTISAMPLECOUNT_1                       0
#define MULTISAMPLECOUNT_2                       1
#define MULTISAMPLECOUNT_4                       2
#define MULTISAMPLECOUNT_8                       3
#define MULTISAMPLECOUNT_16                      4
   uint32_t                             MultisampledSurfaceStorageFormat;
#define MSFMT_MSS                                0
#define MSFMT_DEPTH_STENCIL                      1
   uint32_t                             RenderTargetViewExtent;
   uint32_t                             MinimumArrayElement;
   uint32_t                             RenderTargetAndSampleUnormRotation;
#define _0DEG                                    0
#define _90DEG                                   1
#define _180DEG                                  2
#define _270DEG                                  3
   uint32_t                             MIPCountLOD;
   uint32_t                             SurfaceMinLOD;
   uint32_t                             MipTailStartLOD;
   uint32_t                             CoherencyType;
#define GPUcoherent                              0
#define IAcoherent                               1
   uint32_t                             TiledResourceMode;
#define NONE                                     0
#define _4KB                                     1
#define _64KB                                    2
#define TILEYF                                   1
#define TILEYS                                   2
   bool                                 EWADisableForCube;
   uint32_t                             YOffset;
   uint32_t                             XOffset;
   uint32_t                             AuxiliarySurfaceMode;
#define AUX_NONE                                 0
#define AUX_CCS_D                                1
#define AUX_APPEND                               2
#define AUX_HIZ                                  3
#define AUX_CCS_E                                5
   uint32_t                             YOffsetforUorUVPlane;
   uint32_t                             AuxiliarySurfacePitch;
   uint32_t                             AuxiliarySurfaceQPitch;
   uint32_t                             XOffsetforUorUVPlane;
   bool                                 SeparateUVPlaneEnable;
   float                                ResourceMinLOD;
   enum GFX11_ShaderChannelSelect       ShaderChannelSelectAlpha;
   enum GFX11_ShaderChannelSelect       ShaderChannelSelectBlue;
   enum GFX11_ShaderChannelSelect       ShaderChannelSelectGreen;
   enum GFX11_ShaderChannelSelect       ShaderChannelSelectRed;
   bool                                 MemoryCompressionEnable;
   uint32_t                             MemoryCompressionMode;
#define Horizontal                               0
#define Vertical                                 1
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             QuiltWidth;
   uint32_t                             QuiltHeight;
   bool                                 ClearValueAddressEnable;
   __gen_address_type                   AuxiliarySurfaceBaseAddress;
   uint32_t                             AuxiliaryTableIndexforMediaCompressedSurface;
   uint32_t                             YOffsetforVPlane;
   uint32_t                             XOffsetforVPlane;
   int32_t                              RedClearColor;
   uint32_t                             ClearColorConversionEnable;
   __gen_address_type                   ClearValueAddress;
   int32_t                              GreenClearColor;
   int32_t                              BlueClearColor;
   int32_t                              AlphaClearColor;
};

static inline __attribute__((always_inline)) void
GFX11_RENDER_SURFACE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_RENDER_SURFACE_STATE * restrict values)
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
      __gen_uint(values->SamplerL2BypassModeDisable, 9, 9) |
      __gen_uint(values->VerticalLineStrideOffset, 10, 10) |
      __gen_uint(values->VerticalLineStride, 11, 11) |
      __gen_uint(values->TileMode, 12, 13) |
      __gen_uint(values->SurfaceHorizontalAlignment, 14, 15) |
      __gen_uint(values->SurfaceVerticalAlignment, 16, 17) |
      __gen_uint(values->SurfaceFormat, 18, 27) |
      __gen_uint(values->SurfaceArray, 28, 28) |
      __gen_uint(values->SurfaceType, 29, 31);

   dw[1] =
      __gen_uint(values->SurfaceQPitch, 0, 14) |
      __gen_ufixed(values->BaseMipLevel, 19, 23, 1) |
      __gen_uint(values->MOCS, 24, 30) |
      __gen_uint(values->EnableUnormPathInColorPipe, 31, 31);

   dw[2] =
      __gen_uint(values->Width, 0, 13) |
      __gen_uint(values->Height, 16, 29);

   dw[3] =
      __gen_uint(values->SurfacePitch, 0, 17) |
      __gen_uint(values->TileAddressMappingMode, 20, 20) |
      __gen_uint(values->Depth, 21, 31);

   dw[4] =
      __gen_uint(values->MultisamplePositionPaletteIndex, 0, 2) |
      __gen_uint(values->NumberofMultisamples, 3, 5) |
      __gen_uint(values->MultisampledSurfaceStorageFormat, 6, 6) |
      __gen_uint(values->RenderTargetViewExtent, 7, 17) |
      __gen_uint(values->MinimumArrayElement, 18, 28) |
      __gen_uint(values->RenderTargetAndSampleUnormRotation, 29, 30);

   dw[5] =
      __gen_uint(values->MIPCountLOD, 0, 3) |
      __gen_uint(values->SurfaceMinLOD, 4, 7) |
      __gen_uint(values->MipTailStartLOD, 8, 11) |
      __gen_uint(values->CoherencyType, 14, 14) |
      __gen_uint(values->TiledResourceMode, 18, 19) |
      __gen_uint(values->EWADisableForCube, 20, 20) |
      __gen_uint(values->YOffset, 21, 23) |
      __gen_uint(values->XOffset, 25, 31);

   dw[6] =
      __gen_uint(values->AuxiliarySurfaceMode, 0, 2) |
      __gen_uint(values->YOffsetforUorUVPlane, 0, 13) |
      __gen_uint(values->AuxiliarySurfacePitch, 3, 11) |
      __gen_uint(values->AuxiliarySurfaceQPitch, 16, 30) |
      __gen_uint(values->XOffsetforUorUVPlane, 16, 29) |
      __gen_uint(values->SeparateUVPlaneEnable, 31, 31);

   dw[7] =
      __gen_ufixed(values->ResourceMinLOD, 0, 11, 8) |
      __gen_uint(values->ShaderChannelSelectAlpha, 16, 18) |
      __gen_uint(values->ShaderChannelSelectBlue, 19, 21) |
      __gen_uint(values->ShaderChannelSelectGreen, 22, 24) |
      __gen_uint(values->ShaderChannelSelectRed, 25, 27) |
      __gen_uint(values->MemoryCompressionEnable, 30, 30) |
      __gen_uint(values->MemoryCompressionMode, 31, 31);

   const uint64_t v8_address =
      __gen_address(data, &dw[8], values->SurfaceBaseAddress, 0, 0, 63);
   dw[8] = v8_address;
   dw[9] = v8_address >> 32;

   const uint64_t v10 =
      __gen_uint(values->QuiltWidth, 0, 4) |
      __gen_uint(values->QuiltHeight, 5, 9) |
      __gen_uint(values->ClearValueAddressEnable, 10, 10) |
      __gen_uint(values->AuxiliaryTableIndexforMediaCompressedSurface, 21, 31) |
      __gen_uint(values->YOffsetforVPlane, 32, 45) |
      __gen_uint(values->XOffsetforVPlane, 48, 61);
   const uint64_t v10_address =
      __gen_address(data, &dw[10], values->AuxiliarySurfaceBaseAddress, v10, 12, 63);
   dw[10] = v10_address;
   dw[11] = (v10_address >> 32) | (v10 >> 32);

   const uint64_t v12 =
      __gen_sint(values->RedClearColor, 0, 31) |
      __gen_uint(values->ClearColorConversionEnable, 5, 5) |
      __gen_sint(values->GreenClearColor, 32, 63);
   const uint64_t v12_address =
      __gen_address(data, &dw[12], values->ClearValueAddress, v12, 6, 47);
   dw[12] = v12_address;
   dw[13] = (v12_address >> 32) | (v12 >> 32);

   dw[14] =
      __gen_sint(values->BlueClearColor, 0, 31);

   dw[15] =
      __gen_sint(values->AlphaClearColor, 0, 31);
}

#define GFX11_ROUNDINGPRECISIONTABLE_3_BITS_length      1
struct GFX11_ROUNDINGPRECISIONTABLE_3_BITS {
   uint32_t                             RoundingPrecision;
#define _116                                     0
#define _216                                     1
#define _316                                     2
#define _416                                     3
#define _516                                     4
#define _616                                     5
#define _716                                     6
#define _816                                     7
};

static inline __attribute__((always_inline)) void
GFX11_ROUNDINGPRECISIONTABLE_3_BITS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_ROUNDINGPRECISIONTABLE_3_BITS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RoundingPrecision, 0, 2);
}

#define GFX11_SAMPLER_BORDER_COLOR_STATE_length      4
struct GFX11_SAMPLER_BORDER_COLOR_STATE {
   float                                BorderColorFloatRed;
   uint32_t                             BorderColor32bitRed;
   float                                BorderColorFloatGreen;
   uint32_t                             BorderColor32bitGreen;
   float                                BorderColorFloatBlue;
   uint32_t                             BorderColor32bitBlue;
   float                                BorderColorFloatAlpha;
   uint32_t                             BorderColor32bitAlpha;
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_BORDER_COLOR_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_SAMPLER_BORDER_COLOR_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_float(values->BorderColorFloatRed) |
      __gen_uint(values->BorderColor32bitRed, 0, 31);

   dw[1] =
      __gen_float(values->BorderColorFloatGreen) |
      __gen_uint(values->BorderColor32bitGreen, 0, 31);

   dw[2] =
      __gen_float(values->BorderColorFloatBlue) |
      __gen_uint(values->BorderColor32bitBlue, 0, 31);

   dw[3] =
      __gen_float(values->BorderColorFloatAlpha) |
      __gen_uint(values->BorderColor32bitAlpha, 0, 31);
}

#define GFX11_SAMPLER_INDIRECT_STATE_BORDER_COLOR_length      4
struct GFX11_SAMPLER_INDIRECT_STATE_BORDER_COLOR {
   int32_t                              BorderColorRedAsS31;
   uint32_t                             BorderColorRedAsU32;
   float                                BorderColorRedAsFloat;
   uint32_t                             BorderColorRedAsU8;
   uint32_t                             BorderColorGreenAsU8;
   uint32_t                             BorderColorBlueAsU8;
   uint32_t                             BorderColorAlphaAsU8;
   int32_t                              BorderColorGreenAsS31;
   uint32_t                             BorderColorGreenAsU32;
   float                                BorderColorGreenAsFloat;
   int32_t                              BorderColorBlueAsS31;
   uint32_t                             BorderColorBlueAsU32;
   float                                BorderColorBlueAsFloat;
   int32_t                              BorderColorAlphaAsS31;
   uint32_t                             BorderColorAlphaAsU32;
   float                                BorderColorAlphaAsFloat;
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_INDIRECT_STATE_BORDER_COLOR_pack(__attribute__((unused)) __gen_user_data *data,
                                               __attribute__((unused)) void * restrict dst,
                                               __attribute__((unused)) const struct GFX11_SAMPLER_INDIRECT_STATE_BORDER_COLOR * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sint(values->BorderColorRedAsS31, 0, 31) |
      __gen_uint(values->BorderColorRedAsU32, 0, 31) |
      __gen_float(values->BorderColorRedAsFloat) |
      __gen_uint(values->BorderColorRedAsU8, 0, 7) |
      __gen_uint(values->BorderColorGreenAsU8, 8, 15) |
      __gen_uint(values->BorderColorBlueAsU8, 16, 23) |
      __gen_uint(values->BorderColorAlphaAsU8, 24, 31);

   dw[1] =
      __gen_sint(values->BorderColorGreenAsS31, 0, 31) |
      __gen_uint(values->BorderColorGreenAsU32, 0, 31) |
      __gen_float(values->BorderColorGreenAsFloat);

   dw[2] =
      __gen_sint(values->BorderColorBlueAsS31, 0, 31) |
      __gen_uint(values->BorderColorBlueAsU32, 0, 31) |
      __gen_float(values->BorderColorBlueAsFloat);

   dw[3] =
      __gen_sint(values->BorderColorAlphaAsS31, 0, 31) |
      __gen_uint(values->BorderColorAlphaAsU32, 0, 31) |
      __gen_float(values->BorderColorAlphaAsFloat);
}

#define GFX11_SAMPLER_STATE_length             4
struct GFX11_SAMPLER_STATE {
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
   uint32_t                             CoarseLODQualityMode;
   uint32_t                             LODPreClampMode;
#define CLAMP_MODE_NONE                          0
#define CLAMP_MODE_OGL                           2
   uint32_t                             TextureBorderColorMode;
#define DX10OGL                                  0
#define DX9                                      1
   bool                                 CPSLODCompensationEnable;
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
   uint32_t                             ChromaKeyMode;
#define KEYFILTER_KILL_ON_ANY_MATCH              0
#define KEYFILTER_REPLACE_BLACK                  1
   uint32_t                             ChromaKeyIndex;
   bool                                 ChromaKeyEnable;
   float                                MaxLOD;
   float                                MinLOD;
   uint32_t                             LODClampMagnificationMode;
#define MIPNONE                                  0
#define MIPFILTER                                1
   uint32_t                             SRGBDECODE;
#define DECODE_EXT                               0
#define SKIP_DECODE_EXT                          1
   uint32_t                             ReturnFilterWeightforNullTexels;
   uint32_t                             ReturnFilterWeightforBorderTexels;
   bool                                 Forcegather4Behavior;
   uint64_t                             BorderColorPointer;
   enum GFX11_TextureCoordinateMode     TCZAddressControlMode;
   enum GFX11_TextureCoordinateMode     TCYAddressControlMode;
   enum GFX11_TextureCoordinateMode     TCXAddressControlMode;
   bool                                 ReductionTypeEnable;
   bool                                 NonnormalizedCoordinateEnable;
   uint32_t                             TrilinearFilterQuality;
#define FULL                                     0
#define HIGH                                     1
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
   uint32_t                             ReductionType;
#define STD_FILTER                               0
#define COMPARISON                               1
#define MINIMUM                                  2
#define MAXIMUM                                  3
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_SAMPLER_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->AnisotropicAlgorithm, 0, 0) |
      __gen_sfixed(values->TextureLODBias, 1, 13, 8) |
      __gen_uint(values->MinModeFilter, 14, 16) |
      __gen_uint(values->MagModeFilter, 17, 19) |
      __gen_uint(values->MipModeFilter, 20, 21) |
      __gen_uint(values->CoarseLODQualityMode, 22, 26) |
      __gen_uint(values->LODPreClampMode, 27, 28) |
      __gen_uint(values->TextureBorderColorMode, 29, 29) |
      __gen_uint(values->CPSLODCompensationEnable, 30, 30) |
      __gen_uint(values->SamplerDisable, 31, 31);

   dw[1] =
      __gen_uint(values->CubeSurfaceControlMode, 0, 0) |
      __gen_uint(values->ShadowFunction, 1, 3) |
      __gen_uint(values->ChromaKeyMode, 4, 4) |
      __gen_uint(values->ChromaKeyIndex, 5, 6) |
      __gen_uint(values->ChromaKeyEnable, 7, 7) |
      __gen_ufixed(values->MaxLOD, 8, 19, 8) |
      __gen_ufixed(values->MinLOD, 20, 31, 8);

   dw[2] =
      __gen_uint(values->LODClampMagnificationMode, 0, 0) |
      __gen_uint(values->SRGBDECODE, 1, 1) |
      __gen_uint(values->ReturnFilterWeightforNullTexels, 2, 2) |
      __gen_uint(values->ReturnFilterWeightforBorderTexels, 3, 3) |
      __gen_uint(values->Forcegather4Behavior, 5, 5) |
      __gen_offset(values->BorderColorPointer, 6, 23);

   dw[3] =
      __gen_uint(values->TCZAddressControlMode, 0, 2) |
      __gen_uint(values->TCYAddressControlMode, 3, 5) |
      __gen_uint(values->TCXAddressControlMode, 6, 8) |
      __gen_uint(values->ReductionTypeEnable, 9, 9) |
      __gen_uint(values->NonnormalizedCoordinateEnable, 10, 10) |
      __gen_uint(values->TrilinearFilterQuality, 11, 12) |
      __gen_uint(values->RAddressMinFilterRoundingEnable, 13, 13) |
      __gen_uint(values->RAddressMagFilterRoundingEnable, 14, 14) |
      __gen_uint(values->VAddressMinFilterRoundingEnable, 15, 15) |
      __gen_uint(values->VAddressMagFilterRoundingEnable, 16, 16) |
      __gen_uint(values->UAddressMinFilterRoundingEnable, 17, 17) |
      __gen_uint(values->UAddressMagFilterRoundingEnable, 18, 18) |
      __gen_uint(values->MaximumAnisotropy, 19, 21) |
      __gen_uint(values->ReductionType, 22, 23);
}

#define GFX11_SAMPLER_STATE_8X8_AVS_COEFFICIENTS_length      8
struct GFX11_SAMPLER_STATE_8X8_AVS_COEFFICIENTS {
   float                                Table0FilterCoefficient[4];
   float                                Table1FilterCoefficient0[4];
   float                                Table1FilterCoefficient1[4];
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_STATE_8X8_AVS_COEFFICIENTS_pack(__attribute__((unused)) __gen_user_data *data,
                                              __attribute__((unused)) void * restrict dst,
                                              __attribute__((unused)) const struct GFX11_SAMPLER_STATE_8X8_AVS_COEFFICIENTS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->Table0FilterCoefficient[0], 0, 7, 6) |
      __gen_sfixed(values->Table0FilterCoefficient[1], 8, 15, 6) |
      __gen_sfixed(values->Table0FilterCoefficient[2], 16, 23, 6) |
      __gen_sfixed(values->Table0FilterCoefficient[3], 24, 31, 6) |
      __gen_sfixed(values->Table1FilterCoefficient0[0], 0, 7, 6) |
      __gen_sfixed(values->Table1FilterCoefficient1[0], 8, 15, 6);

   dw[1] =
      __gen_sfixed(values->Table1FilterCoefficient0[1], 0, 7, 6) |
      __gen_sfixed(values->Table1FilterCoefficient1[1], 8, 15, 6);

   dw[2] =
      __gen_sfixed(values->Table1FilterCoefficient0[2], 0, 7, 6) |
      __gen_sfixed(values->Table1FilterCoefficient1[2], 8, 15, 6);

   dw[3] =
      __gen_sfixed(values->Table1FilterCoefficient0[3], 0, 7, 6) |
      __gen_sfixed(values->Table1FilterCoefficient1[3], 8, 15, 6);

   dw[4] = 0;

   dw[5] = 0;

   dw[6] = 0;

   dw[7] = 0;
}

#define GFX11_SCISSOR_RECT_length              2
struct GFX11_SCISSOR_RECT {
   uint32_t                             ScissorRectangleXMin;
   uint32_t                             ScissorRectangleYMin;
   uint32_t                             ScissorRectangleXMax;
   uint32_t                             ScissorRectangleYMax;
};

static inline __attribute__((always_inline)) void
GFX11_SCISSOR_RECT_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_SCISSOR_RECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ScissorRectangleXMin, 0, 15) |
      __gen_uint(values->ScissorRectangleYMin, 16, 31);

   dw[1] =
      __gen_uint(values->ScissorRectangleXMax, 0, 15) |
      __gen_uint(values->ScissorRectangleYMax, 16, 31);
}

#define GFX11_SFC_AVS_CHROMA_COEFF_TABLE_BODY_length     64
struct GFX11_SFC_AVS_CHROMA_COEFF_TABLE_BODY {
   float                                Table1XFilterCoefficientn2;
   float                                Table1YFilterCoefficientn2;
   float                                Table1XFilterCoefficientn3;
   float                                Table1YFilterCoefficientn3;
   float                                Table1XFilterCoefficientn4;
   float                                Table1YFilterCoefficientn4;
   float                                Table1XFilterCoefficientn5;
   float                                Table1YFilterCoefficientn5;
   struct GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY FilterCoefficients[31];
};

static inline __attribute__((always_inline)) void
GFX11_SFC_AVS_CHROMA_COEFF_TABLE_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                                           __attribute__((unused)) void * restrict dst,
                                           __attribute__((unused)) const struct GFX11_SFC_AVS_CHROMA_COEFF_TABLE_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->Table1XFilterCoefficientn2, 0, 7, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn2, 8, 15, 6) |
      __gen_sfixed(values->Table1XFilterCoefficientn3, 16, 23, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn3, 24, 31, 6);

   dw[1] =
      __gen_sfixed(values->Table1XFilterCoefficientn4, 0, 7, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn4, 8, 15, 6) |
      __gen_sfixed(values->Table1XFilterCoefficientn5, 16, 23, 6) |
      __gen_sfixed(values->Table1YFilterCoefficientn5, 24, 31, 6);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[2], &values->FilterCoefficients[0]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[4], &values->FilterCoefficients[1]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[6], &values->FilterCoefficients[2]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[8], &values->FilterCoefficients[3]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[10], &values->FilterCoefficients[4]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[12], &values->FilterCoefficients[5]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[14], &values->FilterCoefficients[6]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[16], &values->FilterCoefficients[7]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[18], &values->FilterCoefficients[8]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[20], &values->FilterCoefficients[9]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[22], &values->FilterCoefficients[10]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[24], &values->FilterCoefficients[11]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[26], &values->FilterCoefficients[12]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[28], &values->FilterCoefficients[13]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[30], &values->FilterCoefficients[14]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[32], &values->FilterCoefficients[15]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[34], &values->FilterCoefficients[16]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[36], &values->FilterCoefficients[17]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[38], &values->FilterCoefficients[18]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[40], &values->FilterCoefficients[19]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[42], &values->FilterCoefficients[20]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[44], &values->FilterCoefficients[21]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[46], &values->FilterCoefficients[22]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[48], &values->FilterCoefficients[23]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[50], &values->FilterCoefficients[24]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[52], &values->FilterCoefficients[25]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[54], &values->FilterCoefficients[26]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[56], &values->FilterCoefficients[27]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[58], &values->FilterCoefficients[28]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[60], &values->FilterCoefficients[29]);

   GFX11_CHROMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[62], &values->FilterCoefficients[30]);
}

#define GFX11_SFC_AVS_LUMA_COEFF_TABLE_BODY_length    128
struct GFX11_SFC_AVS_LUMA_COEFF_TABLE_BODY {
   float                                Table0XFilterCoefficientn0;
   float                                Table0YFilterCoefficientn0;
   float                                Table0XFilterCoefficientn1;
   float                                Table0YFilterCoefficientn1;
   float                                Table0XFilterCoefficientn2;
   float                                Table0YFilterCoefficientn2;
   float                                Table0XFilterCoefficientn3;
   float                                Table0YFilterCoefficientn3;
   float                                Table0XFilterCoefficientn4;
   float                                Table0YFilterCoefficientn4;
   float                                Table0XFilterCoefficientn5;
   float                                Table0YFilterCoefficientn5;
   float                                Table0XFilterCoefficientn6;
   float                                Table0YFilterCoefficientn6;
   float                                Table0XFilterCoefficientn7;
   float                                Table0YFilterCoefficientn7;
   struct GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY FilterCoefficients[31];
};

static inline __attribute__((always_inline)) void
GFX11_SFC_AVS_LUMA_COEFF_TABLE_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_SFC_AVS_LUMA_COEFF_TABLE_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_sfixed(values->Table0XFilterCoefficientn0, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn0, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn1, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn1, 24, 31, 6);

   dw[1] =
      __gen_sfixed(values->Table0XFilterCoefficientn2, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn2, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn3, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn3, 24, 31, 6);

   dw[2] =
      __gen_sfixed(values->Table0XFilterCoefficientn4, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn4, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn5, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn5, 24, 31, 6);

   dw[3] =
      __gen_sfixed(values->Table0XFilterCoefficientn6, 0, 7, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn6, 8, 15, 6) |
      __gen_sfixed(values->Table0XFilterCoefficientn7, 16, 23, 6) |
      __gen_sfixed(values->Table0YFilterCoefficientn7, 24, 31, 6);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[4], &values->FilterCoefficients[0]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[8], &values->FilterCoefficients[1]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[12], &values->FilterCoefficients[2]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[16], &values->FilterCoefficients[3]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[20], &values->FilterCoefficients[4]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[24], &values->FilterCoefficients[5]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[28], &values->FilterCoefficients[6]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[32], &values->FilterCoefficients[7]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[36], &values->FilterCoefficients[8]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[40], &values->FilterCoefficients[9]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[44], &values->FilterCoefficients[10]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[48], &values->FilterCoefficients[11]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[52], &values->FilterCoefficients[12]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[56], &values->FilterCoefficients[13]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[60], &values->FilterCoefficients[14]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[64], &values->FilterCoefficients[15]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[68], &values->FilterCoefficients[16]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[72], &values->FilterCoefficients[17]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[76], &values->FilterCoefficients[18]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[80], &values->FilterCoefficients[19]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[84], &values->FilterCoefficients[20]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[88], &values->FilterCoefficients[21]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[92], &values->FilterCoefficients[22]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[96], &values->FilterCoefficients[23]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[100], &values->FilterCoefficients[24]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[104], &values->FilterCoefficients[25]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[108], &values->FilterCoefficients[26]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[112], &values->FilterCoefficients[27]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[116], &values->FilterCoefficients[28]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[120], &values->FilterCoefficients[29]);

   GFX11_LUMA_FILTER_COEFFICIENTS_ARRAY_pack(data, &dw[124], &values->FilterCoefficients[30]);
}

#define GFX11_SFC_AVS_STATE_BODY_length        3
struct GFX11_SFC_AVS_STATE_BODY {
   uint32_t                             TransitionAreawith8Pixels;
   uint32_t                             TransitionAreawith4Pixels;
   uint32_t                             SharpnessLevel;
   uint32_t                             MaxDerivative8Pixels;
   uint32_t                             MaxDerivative4Pixels;
   uint32_t                             InputVerticalSiting;
#define _18                                      1
#define _28                                      2
#define _38                                      3
#define _48                                      4
#define _58                                      5
#define _68                                      6
#define _78                                      7
#define _88                                      8
};

static inline __attribute__((always_inline)) void
GFX11_SFC_AVS_STATE_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_SFC_AVS_STATE_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->TransitionAreawith8Pixels, 0, 2) |
      __gen_uint(values->TransitionAreawith4Pixels, 4, 6) |
      __gen_uint(values->SharpnessLevel, 24, 31);

   dw[1] =
      __gen_uint(values->MaxDerivative8Pixels, 0, 7) |
      __gen_uint(values->MaxDerivative4Pixels, 16, 23);

   dw[2] =
      __gen_uint(values->InputVerticalSiting, 0, 3);
}

#define GFX11_SFC_IEF_STATE_BODY_length       23
struct GFX11_SFC_IEF_STATE_BODY {
   uint32_t                             GainFactor;
   uint32_t                             WeakEdgeThreshold;
   uint32_t                             StrongEdgeThreshold;
   float                                R3xCoefficient;
   float                                R3cCoefficient;
   uint32_t                             GlobalNoiseEstimation;
   uint32_t                             NonEdgeWeight;
   uint32_t                             RegularWeight;
   uint32_t                             StrongEdgeWeight;
   float                                R5xCoefficient;
   float                                R5cxCoefficient;
   float                                R5cCoefficient;
   float                                STDSinalpha;
   float                                STDCosalpha;
   uint32_t                             Sat_Max;
   uint32_t                             Hue_Max;
   float                                S3U;
   uint32_t                             DiamondMargin;
   bool                                 VY_STD_Enable;
   uint32_t                             U_Mid;
   uint32_t                             V_Mid;
   int32_t                              Diamond_dv;
   uint32_t                             Diamond_Th;
   float                                Diamond_alpha;
   uint32_t                             HS_margin;
   int32_t                              Diamond_du;
   uint32_t                             SkinDetailFactor;
#define DetailRevealed                           0
#define NotDetailRevealed                        1
   uint32_t                             Y_point_1;
   uint32_t                             Y_point_2;
   uint32_t                             Y_point_3;
   uint32_t                             Y_point_4;
   float                                INV_Margin_VYL;
   float                                INV_Margin_VYU;
   uint32_t                             P0L;
   uint32_t                             P1L;
   uint32_t                             P2L;
   uint32_t                             P3L;
   uint32_t                             B0L;
   uint32_t                             B1L;
   uint32_t                             B2L;
   uint32_t                             B3L;
   float                                S0L;
   float                                Y_Slope_2;
   float                                S1L;
   float                                S2L;
   float                                S3L;
   uint32_t                             P0U;
   uint32_t                             P1U;
   float                                Y_Slope1;
   uint32_t                             P2U;
   uint32_t                             P3U;
   uint32_t                             B0U;
   uint32_t                             B1U;
   uint32_t                             B2U;
   uint32_t                             B3U;
   float                                S0U;
   float                                S1U;
   float                                S2U;
   bool                                 TransformEnable;
   bool                                 YUVChannelSwap;
   float                                C0;
   float                                C1;
   float                                C2;
   float                                C3;
   float                                C4;
   float                                C5;
   float                                C6;
   float                                C7;
   float                                C8;
   float                                OffsetIn1;
   float                                OffsetOut1;
   float                                OffsetIn2;
   float                                OffsetOut2;
   float                                OffsetIn3;
   float                                OffsetOut3;
};

static inline __attribute__((always_inline)) void
GFX11_SFC_IEF_STATE_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_SFC_IEF_STATE_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->GainFactor, 0, 5) |
      __gen_uint(values->WeakEdgeThreshold, 6, 11) |
      __gen_uint(values->StrongEdgeThreshold, 12, 17) |
      __gen_ufixed(values->R3xCoefficient, 18, 22, 5) |
      __gen_ufixed(values->R3cCoefficient, 23, 27, 5);

   dw[1] =
      __gen_uint(values->GlobalNoiseEstimation, 0, 7) |
      __gen_uint(values->NonEdgeWeight, 8, 10) |
      __gen_uint(values->RegularWeight, 11, 13) |
      __gen_uint(values->StrongEdgeWeight, 14, 16) |
      __gen_ufixed(values->R5xCoefficient, 17, 21, 5) |
      __gen_ufixed(values->R5cxCoefficient, 22, 26, 5) |
      __gen_ufixed(values->R5cCoefficient, 27, 31, 5);

   dw[2] =
      __gen_sfixed(values->STDSinalpha, 0, 7, 7) |
      __gen_sfixed(values->STDCosalpha, 8, 15, 7) |
      __gen_uint(values->Sat_Max, 16, 21) |
      __gen_uint(values->Hue_Max, 22, 27);

   dw[3] =
      __gen_sfixed(values->S3U, 0, 10, 8) |
      __gen_uint(values->DiamondMargin, 12, 14) |
      __gen_uint(values->VY_STD_Enable, 15, 15) |
      __gen_uint(values->U_Mid, 16, 23) |
      __gen_uint(values->V_Mid, 24, 31);

   dw[4] =
      __gen_sint(values->Diamond_dv, 0, 6) |
      __gen_uint(values->Diamond_Th, 7, 12) |
      __gen_ufixed(values->Diamond_alpha, 13, 20, 6) |
      __gen_uint(values->HS_margin, 21, 23) |
      __gen_sint(values->Diamond_du, 24, 30) |
      __gen_uint(values->SkinDetailFactor, 31, 31);

   dw[5] =
      __gen_uint(values->Y_point_1, 0, 7) |
      __gen_uint(values->Y_point_2, 8, 15) |
      __gen_uint(values->Y_point_3, 16, 23) |
      __gen_uint(values->Y_point_4, 24, 31);

   dw[6] =
      __gen_ufixed(values->INV_Margin_VYL, 0, 15, 16);

   dw[7] =
      __gen_ufixed(values->INV_Margin_VYU, 0, 15, 16) |
      __gen_uint(values->P0L, 16, 23) |
      __gen_uint(values->P1L, 24, 31);

   dw[8] =
      __gen_uint(values->P2L, 0, 7) |
      __gen_uint(values->P3L, 8, 15) |
      __gen_uint(values->B0L, 16, 23) |
      __gen_uint(values->B1L, 24, 31);

   dw[9] =
      __gen_uint(values->B2L, 0, 7) |
      __gen_uint(values->B3L, 8, 15) |
      __gen_sfixed(values->S0L, 16, 26, 8) |
      __gen_ufixed(values->Y_Slope_2, 27, 31, 3);

   dw[10] =
      __gen_sfixed(values->S1L, 0, 10, 8) |
      __gen_sfixed(values->S2L, 11, 21, 8);

   dw[11] =
      __gen_sfixed(values->S3L, 0, 10, 8) |
      __gen_uint(values->P0U, 11, 18) |
      __gen_uint(values->P1U, 19, 26) |
      __gen_ufixed(values->Y_Slope1, 27, 31, 3);

   dw[12] =
      __gen_uint(values->P2U, 0, 7) |
      __gen_uint(values->P3U, 8, 15) |
      __gen_uint(values->B0U, 16, 23) |
      __gen_uint(values->B1U, 24, 31);

   dw[13] =
      __gen_uint(values->B2U, 0, 7) |
      __gen_uint(values->B3U, 8, 15) |
      __gen_sfixed(values->S0U, 16, 26, 8);

   dw[14] =
      __gen_sfixed(values->S1U, 0, 10, 8) |
      __gen_sfixed(values->S2U, 11, 21, 8);

   dw[15] =
      __gen_uint(values->TransformEnable, 0, 0) |
      __gen_uint(values->YUVChannelSwap, 1, 1) |
      __gen_sfixed(values->C0, 3, 15, 10) |
      __gen_sfixed(values->C1, 16, 28, 10);

   dw[16] =
      __gen_sfixed(values->C2, 0, 12, 10) |
      __gen_sfixed(values->C3, 13, 25, 10);

   dw[17] =
      __gen_sfixed(values->C4, 0, 12, 10) |
      __gen_sfixed(values->C5, 13, 25, 10);

   dw[18] =
      __gen_sfixed(values->C6, 0, 12, 10) |
      __gen_sfixed(values->C7, 13, 25, 10);

   dw[19] =
      __gen_sfixed(values->C8, 0, 12, 10);

   dw[20] =
      __gen_sfixed(values->OffsetIn1, 0, 10, 8) |
      __gen_sfixed(values->OffsetOut1, 11, 21, 8);

   dw[21] =
      __gen_sfixed(values->OffsetIn2, 0, 10, 8) |
      __gen_sfixed(values->OffsetOut2, 11, 21, 8);

   dw[22] =
      __gen_sfixed(values->OffsetIn3, 0, 10, 8) |
      __gen_sfixed(values->OffsetOut3, 11, 21, 8);
}

#define GFX11_SFC_LOCK_BODY_length             1
struct GFX11_SFC_LOCK_BODY {
   uint32_t                             VESFCPipeSelect;
   bool                                 PreScaledOutputSurfaceOutputEnable;
};

static inline __attribute__((always_inline)) void
GFX11_SFC_LOCK_BODY_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_SFC_LOCK_BODY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->VESFCPipeSelect, 0, 0) |
      __gen_uint(values->PreScaledOutputSurfaceOutputEnable, 1, 1);
}

#define GFX11_SF_CLIP_VIEWPORT_length         16
struct GFX11_SF_CLIP_VIEWPORT {
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
   float                                XMinViewPort;
   float                                XMaxViewPort;
   float                                YMinViewPort;
   float                                YMaxViewPort;
};

static inline __attribute__((always_inline)) void
GFX11_SF_CLIP_VIEWPORT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SF_CLIP_VIEWPORT * restrict values)
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

   dw[12] =
      __gen_float(values->XMinViewPort);

   dw[13] =
      __gen_float(values->XMaxViewPort);

   dw[14] =
      __gen_float(values->YMinViewPort);

   dw[15] =
      __gen_float(values->YMaxViewPort);
}

#define GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_length      1
struct GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL {
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
GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL * restrict values)
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

#define GFX11_SLICE_HASH_TABLE_length         32
struct GFX11_SLICE_HASH_TABLE {
   uint32_t                             Entry[16][16];
};

static inline __attribute__((always_inline)) void
GFX11_SLICE_HASH_TABLE_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SLICE_HASH_TABLE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->Entry[0][0], 0, 3) |
      __gen_uint(values->Entry[0][1], 4, 7) |
      __gen_uint(values->Entry[0][2], 8, 11) |
      __gen_uint(values->Entry[0][3], 12, 15) |
      __gen_uint(values->Entry[0][4], 16, 19) |
      __gen_uint(values->Entry[0][5], 20, 23) |
      __gen_uint(values->Entry[0][6], 24, 27) |
      __gen_uint(values->Entry[0][7], 28, 31);

   dw[1] =
      __gen_uint(values->Entry[0][8], 0, 3) |
      __gen_uint(values->Entry[0][9], 4, 7) |
      __gen_uint(values->Entry[0][10], 8, 11) |
      __gen_uint(values->Entry[0][11], 12, 15) |
      __gen_uint(values->Entry[0][12], 16, 19) |
      __gen_uint(values->Entry[0][13], 20, 23) |
      __gen_uint(values->Entry[0][14], 24, 27) |
      __gen_uint(values->Entry[0][15], 28, 31);

   dw[2] =
      __gen_uint(values->Entry[1][0], 0, 3) |
      __gen_uint(values->Entry[1][1], 4, 7) |
      __gen_uint(values->Entry[1][2], 8, 11) |
      __gen_uint(values->Entry[1][3], 12, 15) |
      __gen_uint(values->Entry[1][4], 16, 19) |
      __gen_uint(values->Entry[1][5], 20, 23) |
      __gen_uint(values->Entry[1][6], 24, 27) |
      __gen_uint(values->Entry[1][7], 28, 31);

   dw[3] =
      __gen_uint(values->Entry[1][8], 0, 3) |
      __gen_uint(values->Entry[1][9], 4, 7) |
      __gen_uint(values->Entry[1][10], 8, 11) |
      __gen_uint(values->Entry[1][11], 12, 15) |
      __gen_uint(values->Entry[1][12], 16, 19) |
      __gen_uint(values->Entry[1][13], 20, 23) |
      __gen_uint(values->Entry[1][14], 24, 27) |
      __gen_uint(values->Entry[1][15], 28, 31);

   dw[4] =
      __gen_uint(values->Entry[2][0], 0, 3) |
      __gen_uint(values->Entry[2][1], 4, 7) |
      __gen_uint(values->Entry[2][2], 8, 11) |
      __gen_uint(values->Entry[2][3], 12, 15) |
      __gen_uint(values->Entry[2][4], 16, 19) |
      __gen_uint(values->Entry[2][5], 20, 23) |
      __gen_uint(values->Entry[2][6], 24, 27) |
      __gen_uint(values->Entry[2][7], 28, 31);

   dw[5] =
      __gen_uint(values->Entry[2][8], 0, 3) |
      __gen_uint(values->Entry[2][9], 4, 7) |
      __gen_uint(values->Entry[2][10], 8, 11) |
      __gen_uint(values->Entry[2][11], 12, 15) |
      __gen_uint(values->Entry[2][12], 16, 19) |
      __gen_uint(values->Entry[2][13], 20, 23) |
      __gen_uint(values->Entry[2][14], 24, 27) |
      __gen_uint(values->Entry[2][15], 28, 31);

   dw[6] =
      __gen_uint(values->Entry[3][0], 0, 3) |
      __gen_uint(values->Entry[3][1], 4, 7) |
      __gen_uint(values->Entry[3][2], 8, 11) |
      __gen_uint(values->Entry[3][3], 12, 15) |
      __gen_uint(values->Entry[3][4], 16, 19) |
      __gen_uint(values->Entry[3][5], 20, 23) |
      __gen_uint(values->Entry[3][6], 24, 27) |
      __gen_uint(values->Entry[3][7], 28, 31);

   dw[7] =
      __gen_uint(values->Entry[3][8], 0, 3) |
      __gen_uint(values->Entry[3][9], 4, 7) |
      __gen_uint(values->Entry[3][10], 8, 11) |
      __gen_uint(values->Entry[3][11], 12, 15) |
      __gen_uint(values->Entry[3][12], 16, 19) |
      __gen_uint(values->Entry[3][13], 20, 23) |
      __gen_uint(values->Entry[3][14], 24, 27) |
      __gen_uint(values->Entry[3][15], 28, 31);

   dw[8] =
      __gen_uint(values->Entry[4][0], 0, 3) |
      __gen_uint(values->Entry[4][1], 4, 7) |
      __gen_uint(values->Entry[4][2], 8, 11) |
      __gen_uint(values->Entry[4][3], 12, 15) |
      __gen_uint(values->Entry[4][4], 16, 19) |
      __gen_uint(values->Entry[4][5], 20, 23) |
      __gen_uint(values->Entry[4][6], 24, 27) |
      __gen_uint(values->Entry[4][7], 28, 31);

   dw[9] =
      __gen_uint(values->Entry[4][8], 0, 3) |
      __gen_uint(values->Entry[4][9], 4, 7) |
      __gen_uint(values->Entry[4][10], 8, 11) |
      __gen_uint(values->Entry[4][11], 12, 15) |
      __gen_uint(values->Entry[4][12], 16, 19) |
      __gen_uint(values->Entry[4][13], 20, 23) |
      __gen_uint(values->Entry[4][14], 24, 27) |
      __gen_uint(values->Entry[4][15], 28, 31);

   dw[10] =
      __gen_uint(values->Entry[5][0], 0, 3) |
      __gen_uint(values->Entry[5][1], 4, 7) |
      __gen_uint(values->Entry[5][2], 8, 11) |
      __gen_uint(values->Entry[5][3], 12, 15) |
      __gen_uint(values->Entry[5][4], 16, 19) |
      __gen_uint(values->Entry[5][5], 20, 23) |
      __gen_uint(values->Entry[5][6], 24, 27) |
      __gen_uint(values->Entry[5][7], 28, 31);

   dw[11] =
      __gen_uint(values->Entry[5][8], 0, 3) |
      __gen_uint(values->Entry[5][9], 4, 7) |
      __gen_uint(values->Entry[5][10], 8, 11) |
      __gen_uint(values->Entry[5][11], 12, 15) |
      __gen_uint(values->Entry[5][12], 16, 19) |
      __gen_uint(values->Entry[5][13], 20, 23) |
      __gen_uint(values->Entry[5][14], 24, 27) |
      __gen_uint(values->Entry[5][15], 28, 31);

   dw[12] =
      __gen_uint(values->Entry[6][0], 0, 3) |
      __gen_uint(values->Entry[6][1], 4, 7) |
      __gen_uint(values->Entry[6][2], 8, 11) |
      __gen_uint(values->Entry[6][3], 12, 15) |
      __gen_uint(values->Entry[6][4], 16, 19) |
      __gen_uint(values->Entry[6][5], 20, 23) |
      __gen_uint(values->Entry[6][6], 24, 27) |
      __gen_uint(values->Entry[6][7], 28, 31);

   dw[13] =
      __gen_uint(values->Entry[6][8], 0, 3) |
      __gen_uint(values->Entry[6][9], 4, 7) |
      __gen_uint(values->Entry[6][10], 8, 11) |
      __gen_uint(values->Entry[6][11], 12, 15) |
      __gen_uint(values->Entry[6][12], 16, 19) |
      __gen_uint(values->Entry[6][13], 20, 23) |
      __gen_uint(values->Entry[6][14], 24, 27) |
      __gen_uint(values->Entry[6][15], 28, 31);

   dw[14] =
      __gen_uint(values->Entry[7][0], 0, 3) |
      __gen_uint(values->Entry[7][1], 4, 7) |
      __gen_uint(values->Entry[7][2], 8, 11) |
      __gen_uint(values->Entry[7][3], 12, 15) |
      __gen_uint(values->Entry[7][4], 16, 19) |
      __gen_uint(values->Entry[7][5], 20, 23) |
      __gen_uint(values->Entry[7][6], 24, 27) |
      __gen_uint(values->Entry[7][7], 28, 31);

   dw[15] =
      __gen_uint(values->Entry[7][8], 0, 3) |
      __gen_uint(values->Entry[7][9], 4, 7) |
      __gen_uint(values->Entry[7][10], 8, 11) |
      __gen_uint(values->Entry[7][11], 12, 15) |
      __gen_uint(values->Entry[7][12], 16, 19) |
      __gen_uint(values->Entry[7][13], 20, 23) |
      __gen_uint(values->Entry[7][14], 24, 27) |
      __gen_uint(values->Entry[7][15], 28, 31);

   dw[16] =
      __gen_uint(values->Entry[8][0], 0, 3) |
      __gen_uint(values->Entry[8][1], 4, 7) |
      __gen_uint(values->Entry[8][2], 8, 11) |
      __gen_uint(values->Entry[8][3], 12, 15) |
      __gen_uint(values->Entry[8][4], 16, 19) |
      __gen_uint(values->Entry[8][5], 20, 23) |
      __gen_uint(values->Entry[8][6], 24, 27) |
      __gen_uint(values->Entry[8][7], 28, 31);

   dw[17] =
      __gen_uint(values->Entry[8][8], 0, 3) |
      __gen_uint(values->Entry[8][9], 4, 7) |
      __gen_uint(values->Entry[8][10], 8, 11) |
      __gen_uint(values->Entry[8][11], 12, 15) |
      __gen_uint(values->Entry[8][12], 16, 19) |
      __gen_uint(values->Entry[8][13], 20, 23) |
      __gen_uint(values->Entry[8][14], 24, 27) |
      __gen_uint(values->Entry[8][15], 28, 31);

   dw[18] =
      __gen_uint(values->Entry[9][0], 0, 3) |
      __gen_uint(values->Entry[9][1], 4, 7) |
      __gen_uint(values->Entry[9][2], 8, 11) |
      __gen_uint(values->Entry[9][3], 12, 15) |
      __gen_uint(values->Entry[9][4], 16, 19) |
      __gen_uint(values->Entry[9][5], 20, 23) |
      __gen_uint(values->Entry[9][6], 24, 27) |
      __gen_uint(values->Entry[9][7], 28, 31);

   dw[19] =
      __gen_uint(values->Entry[9][8], 0, 3) |
      __gen_uint(values->Entry[9][9], 4, 7) |
      __gen_uint(values->Entry[9][10], 8, 11) |
      __gen_uint(values->Entry[9][11], 12, 15) |
      __gen_uint(values->Entry[9][12], 16, 19) |
      __gen_uint(values->Entry[9][13], 20, 23) |
      __gen_uint(values->Entry[9][14], 24, 27) |
      __gen_uint(values->Entry[9][15], 28, 31);

   dw[20] =
      __gen_uint(values->Entry[10][0], 0, 3) |
      __gen_uint(values->Entry[10][1], 4, 7) |
      __gen_uint(values->Entry[10][2], 8, 11) |
      __gen_uint(values->Entry[10][3], 12, 15) |
      __gen_uint(values->Entry[10][4], 16, 19) |
      __gen_uint(values->Entry[10][5], 20, 23) |
      __gen_uint(values->Entry[10][6], 24, 27) |
      __gen_uint(values->Entry[10][7], 28, 31);

   dw[21] =
      __gen_uint(values->Entry[10][8], 0, 3) |
      __gen_uint(values->Entry[10][9], 4, 7) |
      __gen_uint(values->Entry[10][10], 8, 11) |
      __gen_uint(values->Entry[10][11], 12, 15) |
      __gen_uint(values->Entry[10][12], 16, 19) |
      __gen_uint(values->Entry[10][13], 20, 23) |
      __gen_uint(values->Entry[10][14], 24, 27) |
      __gen_uint(values->Entry[10][15], 28, 31);

   dw[22] =
      __gen_uint(values->Entry[11][0], 0, 3) |
      __gen_uint(values->Entry[11][1], 4, 7) |
      __gen_uint(values->Entry[11][2], 8, 11) |
      __gen_uint(values->Entry[11][3], 12, 15) |
      __gen_uint(values->Entry[11][4], 16, 19) |
      __gen_uint(values->Entry[11][5], 20, 23) |
      __gen_uint(values->Entry[11][6], 24, 27) |
      __gen_uint(values->Entry[11][7], 28, 31);

   dw[23] =
      __gen_uint(values->Entry[11][8], 0, 3) |
      __gen_uint(values->Entry[11][9], 4, 7) |
      __gen_uint(values->Entry[11][10], 8, 11) |
      __gen_uint(values->Entry[11][11], 12, 15) |
      __gen_uint(values->Entry[11][12], 16, 19) |
      __gen_uint(values->Entry[11][13], 20, 23) |
      __gen_uint(values->Entry[11][14], 24, 27) |
      __gen_uint(values->Entry[11][15], 28, 31);

   dw[24] =
      __gen_uint(values->Entry[12][0], 0, 3) |
      __gen_uint(values->Entry[12][1], 4, 7) |
      __gen_uint(values->Entry[12][2], 8, 11) |
      __gen_uint(values->Entry[12][3], 12, 15) |
      __gen_uint(values->Entry[12][4], 16, 19) |
      __gen_uint(values->Entry[12][5], 20, 23) |
      __gen_uint(values->Entry[12][6], 24, 27) |
      __gen_uint(values->Entry[12][7], 28, 31);

   dw[25] =
      __gen_uint(values->Entry[12][8], 0, 3) |
      __gen_uint(values->Entry[12][9], 4, 7) |
      __gen_uint(values->Entry[12][10], 8, 11) |
      __gen_uint(values->Entry[12][11], 12, 15) |
      __gen_uint(values->Entry[12][12], 16, 19) |
      __gen_uint(values->Entry[12][13], 20, 23) |
      __gen_uint(values->Entry[12][14], 24, 27) |
      __gen_uint(values->Entry[12][15], 28, 31);

   dw[26] =
      __gen_uint(values->Entry[13][0], 0, 3) |
      __gen_uint(values->Entry[13][1], 4, 7) |
      __gen_uint(values->Entry[13][2], 8, 11) |
      __gen_uint(values->Entry[13][3], 12, 15) |
      __gen_uint(values->Entry[13][4], 16, 19) |
      __gen_uint(values->Entry[13][5], 20, 23) |
      __gen_uint(values->Entry[13][6], 24, 27) |
      __gen_uint(values->Entry[13][7], 28, 31);

   dw[27] =
      __gen_uint(values->Entry[13][8], 0, 3) |
      __gen_uint(values->Entry[13][9], 4, 7) |
      __gen_uint(values->Entry[13][10], 8, 11) |
      __gen_uint(values->Entry[13][11], 12, 15) |
      __gen_uint(values->Entry[13][12], 16, 19) |
      __gen_uint(values->Entry[13][13], 20, 23) |
      __gen_uint(values->Entry[13][14], 24, 27) |
      __gen_uint(values->Entry[13][15], 28, 31);

   dw[28] =
      __gen_uint(values->Entry[14][0], 0, 3) |
      __gen_uint(values->Entry[14][1], 4, 7) |
      __gen_uint(values->Entry[14][2], 8, 11) |
      __gen_uint(values->Entry[14][3], 12, 15) |
      __gen_uint(values->Entry[14][4], 16, 19) |
      __gen_uint(values->Entry[14][5], 20, 23) |
      __gen_uint(values->Entry[14][6], 24, 27) |
      __gen_uint(values->Entry[14][7], 28, 31);

   dw[29] =
      __gen_uint(values->Entry[14][8], 0, 3) |
      __gen_uint(values->Entry[14][9], 4, 7) |
      __gen_uint(values->Entry[14][10], 8, 11) |
      __gen_uint(values->Entry[14][11], 12, 15) |
      __gen_uint(values->Entry[14][12], 16, 19) |
      __gen_uint(values->Entry[14][13], 20, 23) |
      __gen_uint(values->Entry[14][14], 24, 27) |
      __gen_uint(values->Entry[14][15], 28, 31);

   dw[30] =
      __gen_uint(values->Entry[15][0], 0, 3) |
      __gen_uint(values->Entry[15][1], 4, 7) |
      __gen_uint(values->Entry[15][2], 8, 11) |
      __gen_uint(values->Entry[15][3], 12, 15) |
      __gen_uint(values->Entry[15][4], 16, 19) |
      __gen_uint(values->Entry[15][5], 20, 23) |
      __gen_uint(values->Entry[15][6], 24, 27) |
      __gen_uint(values->Entry[15][7], 28, 31);

   dw[31] =
      __gen_uint(values->Entry[15][8], 0, 3) |
      __gen_uint(values->Entry[15][9], 4, 7) |
      __gen_uint(values->Entry[15][10], 8, 11) |
      __gen_uint(values->Entry[15][11], 12, 15) |
      __gen_uint(values->Entry[15][12], 16, 19) |
      __gen_uint(values->Entry[15][13], 20, 23) |
      __gen_uint(values->Entry[15][14], 24, 27) |
      __gen_uint(values->Entry[15][15], 28, 31);
}

#define GFX11_SO_DECL_length                   1
struct GFX11_SO_DECL {
   uint32_t                             ComponentMask;
   uint32_t                             RegisterIndex;
   bool                                 HoleFlag;
   uint32_t                             OutputBufferSlot;
};

static inline __attribute__((always_inline)) void
GFX11_SO_DECL_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX11_SO_DECL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ComponentMask, 0, 3) |
      __gen_uint(values->RegisterIndex, 4, 9) |
      __gen_uint(values->HoleFlag, 11, 11) |
      __gen_uint(values->OutputBufferSlot, 12, 13);
}

#define GFX11_SO_DECL_ENTRY_length             2
struct GFX11_SO_DECL_ENTRY {
   struct GFX11_SO_DECL                 Stream0Decl;
   struct GFX11_SO_DECL                 Stream1Decl;
   struct GFX11_SO_DECL                 Stream2Decl;
   struct GFX11_SO_DECL                 Stream3Decl;
};

static inline __attribute__((always_inline)) void
GFX11_SO_DECL_ENTRY_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_SO_DECL_ENTRY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   uint32_t v0_0;
   GFX11_SO_DECL_pack(data, &v0_0, &values->Stream0Decl);

   uint32_t v0_1;
   GFX11_SO_DECL_pack(data, &v0_1, &values->Stream1Decl);

   dw[0] =
      __gen_uint(v0_0, 0, 15) |
      __gen_uint(v0_1, 16, 31);

   uint32_t v1_0;
   GFX11_SO_DECL_pack(data, &v1_0, &values->Stream2Decl);

   uint32_t v1_1;
   GFX11_SO_DECL_pack(data, &v1_1, &values->Stream3Decl);

   dw[1] =
      __gen_uint(v1_0, 0, 15) |
      __gen_uint(v1_1, 16, 31);
}

#define GFX11_VDENC_SURFACE_CONTROL_BITS_length      1
struct GFX11_VDENC_SURFACE_CONTROL_BITS {
   uint32_t                             MOCS;
   uint32_t                             ArbitrationPriorityControl;
#define Highestpriority                          0
#define Secondhighestpriority                    1
#define Thirdhighestpriority                     2
#define Lowestpriority                           3
   bool                                 MemoryCompressionEnable;
   uint32_t                             MemoryCompressionMode;
   uint32_t                             CacheSelect;
   uint32_t                             TiledResourceMode;
#define TRMODE_NONE                              0
#define TRMODE_TILEYF                            1
#define TRMODE_TILEYS                            2
};

static inline __attribute__((always_inline)) void
GFX11_VDENC_SURFACE_CONTROL_BITS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_VDENC_SURFACE_CONTROL_BITS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MOCS, 1, 6) |
      __gen_uint(values->ArbitrationPriorityControl, 7, 8) |
      __gen_uint(values->MemoryCompressionEnable, 9, 9) |
      __gen_uint(values->MemoryCompressionMode, 10, 10) |
      __gen_uint(values->CacheSelect, 12, 12) |
      __gen_uint(values->TiledResourceMode, 13, 14);
}

#define GFX11_VDENC_PICTURE_length             3
struct GFX11_VDENC_PICTURE {
   __gen_address_type                   Address;
   struct GFX11_VDENC_SURFACE_CONTROL_BITS PictureFields;
};

static inline __attribute__((always_inline)) void
GFX11_VDENC_PICTURE_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_VDENC_PICTURE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0_address =
      __gen_address(data, &dw[0], values->Address, 0, 0, 63);
   dw[0] = v0_address;
   dw[1] = v0_address >> 32;

   GFX11_VDENC_SURFACE_CONTROL_BITS_pack(data, &dw[2], &values->PictureFields);
}

#define GFX11_VDENC_SURFACE_STATE_FIELDS_length      4
struct GFX11_VDENC_SURFACE_STATE_FIELDS {
   float                                CrVCbUPixelOffsetVDirection;
   bool                                 SurfaceFormatByteSwizzle;
   uint32_t                             Colorspaceselection;
   uint32_t                             Width;
   uint32_t                             Height;
   uint32_t                             TileWalk;
#define TW_XMAJOR                                0
#define TW_YMAJOR                                1
   uint32_t                             TiledSurface;
   bool                                 HalfPitchforChroma;
   uint32_t                             SurfacePitch;
   uint32_t                             ChromaDownsampleFilterControl;
   uint32_t                             Format;
   uint32_t                             SurfaceFormat;
#define VDENC_YUV422                             0
#define VDENC_RGBA4444                           1
#define VDENC_YUV444                             2
#define VDENC_Y8_UNORM                           3
#define VDENC_PLANAR_420_8                       4
   uint32_t                             YOffsetforUCb;
   uint32_t                             XOffsetforUCb;
   uint32_t                             YOffsetforVCr;
   uint32_t                             XOffsetforVCr;
};

static inline __attribute__((always_inline)) void
GFX11_VDENC_SURFACE_STATE_FIELDS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_VDENC_SURFACE_STATE_FIELDS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_ufixed(values->CrVCbUPixelOffsetVDirection, 0, 1, 2) |
      __gen_uint(values->SurfaceFormatByteSwizzle, 2, 2) |
      __gen_uint(values->Colorspaceselection, 3, 3) |
      __gen_uint(values->Width, 4, 17) |
      __gen_uint(values->Height, 18, 31);

   dw[1] =
      __gen_uint(values->TileWalk, 0, 0) |
      __gen_uint(values->TiledSurface, 1, 1) |
      __gen_uint(values->HalfPitchforChroma, 2, 2) |
      __gen_uint(values->SurfacePitch, 3, 19) |
      __gen_uint(values->ChromaDownsampleFilterControl, 20, 22) |
      __gen_uint(values->Format, 27, 31) |
      __gen_uint(values->SurfaceFormat, 28, 31);

   dw[2] =
      __gen_uint(values->YOffsetforUCb, 0, 14) |
      __gen_uint(values->XOffsetforUCb, 16, 30);

   dw[3] =
      __gen_uint(values->YOffsetforVCr, 0, 15) |
      __gen_uint(values->XOffsetforVCr, 16, 28);
}

#define GFX11_VERTEX_BUFFER_STATE_length       4
struct GFX11_VERTEX_BUFFER_STATE {
   uint32_t                             BufferPitch;
   bool                                 NullVertexBuffer;
   bool                                 AddressModifyEnable;
   uint32_t                             MOCS;
   uint32_t                             VertexBufferIndex;
   __gen_address_type                   BufferStartingAddress;
   uint32_t                             BufferSize;
};

static inline __attribute__((always_inline)) void
GFX11_VERTEX_BUFFER_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_VERTEX_BUFFER_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BufferPitch, 0, 11) |
      __gen_uint(values->NullVertexBuffer, 13, 13) |
      __gen_uint(values->AddressModifyEnable, 14, 14) |
      __gen_uint(values->MOCS, 16, 22) |
      __gen_uint(values->VertexBufferIndex, 26, 31);

   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->BufferStartingAddress, 0, 0, 63);
   dw[1] = v1_address;
   dw[2] = v1_address >> 32;

   dw[3] =
      __gen_uint(values->BufferSize, 0, 31);
}

#define GFX11_VERTEX_ELEMENT_STATE_length      2
struct GFX11_VERTEX_ELEMENT_STATE {
   uint32_t                             SourceElementOffset;
   bool                                 EdgeFlagEnable;
   uint32_t                             SourceElementFormat;
   bool                                 Valid;
   uint32_t                             VertexBufferIndex;
   enum GFX11_3D_Vertex_Component_Control Component3Control;
   enum GFX11_3D_Vertex_Component_Control Component2Control;
   enum GFX11_3D_Vertex_Component_Control Component1Control;
   enum GFX11_3D_Vertex_Component_Control Component0Control;
};

static inline __attribute__((always_inline)) void
GFX11_VERTEX_ELEMENT_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_VERTEX_ELEMENT_STATE * restrict values)
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

#define GFX11_3DPRIMITIVE_length               7
#define GFX11_3DPRIMITIVE_length_bias          2
#define GFX11_3DPRIMITIVE_header                \
   .DWordLength                         =      5,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      3,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DPRIMITIVE {
   uint32_t                             DWordLength;
   bool                                 PredicateEnable;
   bool                                 UAVCoherencyRequired;
   bool                                 IndirectParameterEnable;
   uint32_t                             ExtendedParametersPresent;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   enum GFX11_3D_Prim_Topo_Type         PrimitiveTopologyType;
   uint32_t                             VertexAccessType;
#define SEQUENTIAL                               0
#define RANDOM                                   1
   bool                                 EndOffsetEnable;
   uint32_t                             VertexCountPerInstance;
   uint32_t                             StartVertexLocation;
   uint32_t                             InstanceCount;
   uint32_t                             StartInstanceLocation;
   int32_t                              BaseVertexLocation;
   uint32_t                             ExtendedParameter0;
   uint32_t                             ExtendedParameter1;
   uint32_t                             ExtendedParameter2;
};

static inline __attribute__((always_inline)) void
GFX11_3DPRIMITIVE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_3DPRIMITIVE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->PredicateEnable, 8, 8) |
      __gen_uint(values->UAVCoherencyRequired, 9, 9) |
      __gen_uint(values->IndirectParameterEnable, 10, 10) |
      __gen_uint(values->ExtendedParametersPresent, 11, 11) |
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

#define GFX11_3DSTATE_3D_MODE_length           2
#define GFX11_3DSTATE_3D_MODE_length_bias      2
#define GFX11_3DSTATE_3D_MODE_header            \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     30,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_3D_MODE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             CrossSliceHashingMode;
#define NormalMode                               0
#define Disable                                  1
#define hashing32x32                             3
   uint32_t                             SubsliceHashingMode;
#define hashing8x8                               0
#define hashing16x4                              1
#define hashing8x4                               2
#define hashing16x16                             3
   bool                                 SliceHashingTableEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_3D_MODE_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_3DSTATE_3D_MODE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->CrossSliceHashingMode, 0, 1) |
      __gen_uint(values->SubsliceHashingMode, 2, 3) |
      __gen_uint(values->SliceHashingTableEnable, 6, 6) |
      __gen_mbo(16, 31);
}

#define GFX11_3DSTATE_AA_LINE_PARAMETERS_length      3
#define GFX11_3DSTATE_AA_LINE_PARAMETERS_length_bias      2
#define GFX11_3DSTATE_AA_LINE_PARAMETERS_header \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     10,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_AA_LINE_PARAMETERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   float                                AACoverageSlope;
   float                                AAPointCoverageSlope;
   float                                AACoverageBias;
   float                                AAPointCoverageBias;
   float                                AACoverageEndCapSlope;
   float                                AAPointCoverageEndCapSlope;
   float                                AACoverageEndCapBias;
   float                                AAPointCoverageEndCapBias;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_AA_LINE_PARAMETERS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_AA_LINE_PARAMETERS * restrict values)
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
      __gen_ufixed(values->AAPointCoverageSlope, 8, 15, 8) |
      __gen_ufixed(values->AACoverageBias, 16, 23, 8) |
      __gen_ufixed(values->AAPointCoverageBias, 24, 31, 8);

   dw[2] =
      __gen_ufixed(values->AACoverageEndCapSlope, 0, 7, 8) |
      __gen_ufixed(values->AAPointCoverageEndCapSlope, 8, 15, 8) |
      __gen_ufixed(values->AACoverageEndCapBias, 16, 23, 8) |
      __gen_ufixed(values->AAPointCoverageEndCapBias, 24, 31, 8);
}

#define GFX11_3DSTATE_BINDING_TABLE_EDIT_DS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_EDIT_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     70,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_EDIT_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             BindingTableEditTarget;
#define AllCores                                 3
#define Core1                                    2
#define Core0                                    1
   uint32_t                             BindingTableBlockClear;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_EDIT_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_EDIT_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BindingTableEditTarget, 0, 1) |
      __gen_uint(values->BindingTableBlockClear, 16, 31);
}

#define GFX11_3DSTATE_BINDING_TABLE_EDIT_GS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_EDIT_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     68,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_EDIT_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             BindingTableEditTarget;
#define AllCores                                 3
#define Core1                                    2
#define Core0                                    1
   uint32_t                             BindingTableBlockClear;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_EDIT_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_EDIT_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BindingTableEditTarget, 0, 1) |
      __gen_uint(values->BindingTableBlockClear, 16, 31);
}

#define GFX11_3DSTATE_BINDING_TABLE_EDIT_HS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_EDIT_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     69,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_EDIT_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             BindingTableEditTarget;
#define AllCores                                 3
#define Core1                                    2
#define Core0                                    1
   uint32_t                             BindingTableBlockClear;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_EDIT_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_EDIT_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BindingTableEditTarget, 0, 1) |
      __gen_uint(values->BindingTableBlockClear, 16, 31);
}

#define GFX11_3DSTATE_BINDING_TABLE_EDIT_PS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_EDIT_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     71,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_EDIT_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             BindingTableEditTarget;
#define AllCores                                 3
#define Core1                                    2
#define Core0                                    1
   uint32_t                             BindingTableBlockClear;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_EDIT_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_EDIT_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BindingTableEditTarget, 0, 1) |
      __gen_uint(values->BindingTableBlockClear, 16, 31);
}

#define GFX11_3DSTATE_BINDING_TABLE_EDIT_VS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_EDIT_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     67,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_EDIT_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             BindingTableEditTarget;
#define AllCores                                 3
#define Core1                                    2
#define Core0                                    1
   uint32_t                             BindingTableBlockClear;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_EDIT_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_EDIT_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 8) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BindingTableEditTarget, 0, 1) |
      __gen_uint(values->BindingTableBlockClear, 16, 31);
}

#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS_length      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     40,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoDSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_DS * restrict values)
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

#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS_length      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     41,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoGSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_GS * restrict values)
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

#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS_length      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     39,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoHSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_HS * restrict values)
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

#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS_length      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     42,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoPSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_PS * restrict values)
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

#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS_length      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     38,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoVSBindingTable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POINTERS_VS * restrict values)
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

#define GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC_length      4
#define GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC_length_bias      2
#define GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC_header\
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     25,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MOCS;
   bool                                 BindingTablePoolEnable;
   __gen_address_type                   BindingTablePoolBaseAddress;
   uint32_t                             BindingTablePoolBufferSize;
#define NoValidData                              0
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC_pack(__attribute__((unused)) __gen_user_data *data,
                                            __attribute__((unused)) void * restrict dst,
                                            __attribute__((unused)) const struct GFX11_3DSTATE_BINDING_TABLE_POOL_ALLOC * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->MOCS, 0, 6) |
      __gen_uint(values->BindingTablePoolEnable, 11, 11);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->BindingTablePoolBaseAddress, v1, 12, 63);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->BindingTablePoolBufferSize, 12, 31);
}

#define GFX11_3DSTATE_BLEND_STATE_POINTERS_length      2
#define GFX11_3DSTATE_BLEND_STATE_POINTERS_length_bias      2
#define GFX11_3DSTATE_BLEND_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     36,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_BLEND_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 BlendStatePointerValid;
   uint64_t                             BlendStatePointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_BLEND_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX11_3DSTATE_BLEND_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->BlendStatePointerValid, 0, 0) |
      __gen_offset(values->BlendStatePointer, 6, 31);
}

#define GFX11_3DSTATE_CC_STATE_POINTERS_length      2
#define GFX11_3DSTATE_CC_STATE_POINTERS_length_bias      2
#define GFX11_3DSTATE_CC_STATE_POINTERS_header  \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     14,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CC_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 ColorCalcStatePointerValid;
   uint64_t                             ColorCalcStatePointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CC_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_3DSTATE_CC_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ColorCalcStatePointerValid, 0, 0) |
      __gen_offset(values->ColorCalcStatePointer, 6, 31);
}

#define GFX11_3DSTATE_CHROMA_KEY_length        4
#define GFX11_3DSTATE_CHROMA_KEY_length_bias      2
#define GFX11_3DSTATE_CHROMA_KEY_header         \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CHROMA_KEY {
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
GFX11_3DSTATE_CHROMA_KEY_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_3DSTATE_CHROMA_KEY * restrict values)
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

#define GFX11_3DSTATE_CLEAR_PARAMS_length      3
#define GFX11_3DSTATE_CLEAR_PARAMS_length_bias      2
#define GFX11_3DSTATE_CLEAR_PARAMS_header       \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CLEAR_PARAMS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   float                                DepthClearValue;
   bool                                 DepthClearValueValid;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CLEAR_PARAMS_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_CLEAR_PARAMS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_float(values->DepthClearValue);

   dw[2] =
      __gen_uint(values->DepthClearValueValid, 0, 0);
}

#define GFX11_3DSTATE_CLIP_length              4
#define GFX11_3DSTATE_CLIP_length_bias         2
#define GFX11_3DSTATE_CLIP_header               \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     18,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CLIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             UserClipDistanceCullTestEnableBitmask;
   bool                                 StatisticsEnable;
   bool                                 ForceClipMode;
   bool                                 ForceUserClipDistanceClipTestEnableBitmask;
   bool                                 EarlyCullEnable;
   uint32_t                             VertexSubPixelPrecisionSelect;
#define _8Bit                                    0
#define _4Bit                                    1
   bool                                 ForceUserClipDistanceCullTestEnableBitmask;
   uint32_t                             TriangleFanProvokingVertexSelect;
   uint32_t                             LineStripListProvokingVertexSelect;
   uint32_t                             TriangleStripListProvokingVertexSelect;
   bool                                 NonPerspectiveBarycentricEnable;
   bool                                 PerspectiveDivideDisable;
   uint32_t                             ClipMode;
#define CLIPMODE_NORMAL                          0
#define CLIPMODE_REJECT_ALL                      3
#define CLIPMODE_ACCEPT_ALL                      4
   uint32_t                             UserClipDistanceClipTestEnableBitmask;
   bool                                 GuardbandClipTestEnable;
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
GFX11_3DSTATE_CLIP_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_3DSTATE_CLIP * restrict values)
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
      __gen_uint(values->ForceClipMode, 16, 16) |
      __gen_uint(values->ForceUserClipDistanceClipTestEnableBitmask, 17, 17) |
      __gen_uint(values->EarlyCullEnable, 18, 18) |
      __gen_uint(values->VertexSubPixelPrecisionSelect, 19, 19) |
      __gen_uint(values->ForceUserClipDistanceCullTestEnableBitmask, 20, 20);

   dw[2] =
      __gen_uint(values->TriangleFanProvokingVertexSelect, 0, 1) |
      __gen_uint(values->LineStripListProvokingVertexSelect, 2, 3) |
      __gen_uint(values->TriangleStripListProvokingVertexSelect, 4, 5) |
      __gen_uint(values->NonPerspectiveBarycentricEnable, 8, 8) |
      __gen_uint(values->PerspectiveDivideDisable, 9, 9) |
      __gen_uint(values->ClipMode, 13, 15) |
      __gen_uint(values->UserClipDistanceClipTestEnableBitmask, 16, 23) |
      __gen_uint(values->GuardbandClipTestEnable, 26, 26) |
      __gen_uint(values->ViewportXYClipTestEnable, 28, 28) |
      __gen_uint(values->APIMode, 30, 30) |
      __gen_uint(values->ClipEnable, 31, 31);

   dw[3] =
      __gen_uint(values->MaximumVPIndex, 0, 3) |
      __gen_uint(values->ForceZeroRTAIndexEnable, 5, 5) |
      __gen_ufixed(values->MaximumPointWidth, 6, 16, 3) |
      __gen_ufixed(values->MinimumPointWidth, 17, 27, 3);
}

#define GFX11_3DSTATE_CONSTANT_DS_length      11
#define GFX11_3DSTATE_CONSTANT_DS_length_bias      2
#define GFX11_3DSTATE_CONSTANT_DS_header        \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     26,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CONSTANT_DS {
   uint32_t                             DWordLength;
   uint32_t                             MOCS;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_3DSTATE_CONSTANT_BODY   ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_DS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MOCS, 8, 14) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX11_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX11_3DSTATE_CONSTANT_GS_length      11
#define GFX11_3DSTATE_CONSTANT_GS_length_bias      2
#define GFX11_3DSTATE_CONSTANT_GS_header        \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     22,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CONSTANT_GS {
   uint32_t                             DWordLength;
   uint32_t                             MOCS;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_3DSTATE_CONSTANT_BODY   ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_GS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MOCS, 8, 14) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX11_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX11_3DSTATE_CONSTANT_HS_length      11
#define GFX11_3DSTATE_CONSTANT_HS_length_bias      2
#define GFX11_3DSTATE_CONSTANT_HS_header        \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     25,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CONSTANT_HS {
   uint32_t                             DWordLength;
   uint32_t                             MOCS;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_3DSTATE_CONSTANT_BODY   ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_HS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MOCS, 8, 14) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX11_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX11_3DSTATE_CONSTANT_PS_length      11
#define GFX11_3DSTATE_CONSTANT_PS_length_bias      2
#define GFX11_3DSTATE_CONSTANT_PS_header        \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     23,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CONSTANT_PS {
   uint32_t                             DWordLength;
   uint32_t                             MOCS;
   uint32_t                             DisableGatheratSetShaderHint;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_3DSTATE_CONSTANT_BODY   ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_PS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MOCS, 8, 14) |
      __gen_uint(values->DisableGatheratSetShaderHint, 15, 15) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX11_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX11_3DSTATE_CONSTANT_VS_length      11
#define GFX11_3DSTATE_CONSTANT_VS_length_bias      2
#define GFX11_3DSTATE_CONSTANT_VS_header        \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     21,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CONSTANT_VS {
   uint32_t                             DWordLength;
   uint32_t                             MOCS;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_3DSTATE_CONSTANT_BODY   ConstantBody;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CONSTANT_VS_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_CONSTANT_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MOCS, 8, 14) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   GFX11_3DSTATE_CONSTANT_BODY_pack(data, &dw[1], &values->ConstantBody);
}

#define GFX11_3DSTATE_CPS_length               9
#define GFX11_3DSTATE_CPS_length_bias          2
#define GFX11_3DSTATE_CPS_header                \
   .DWordLength                         =      7,  \
   ._3DCommandSubOpcode                 =     34,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_CPS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   float                                MinCPSizeX;
   bool                                 StatisticsEnable;
   uint32_t                             CoarsePixelShadingMode;
#define CPS_MODE_NONE                            0
#define CPS_MODE_CONSTANT                        1
#define CPS_MODE_RADIAL                          2
   uint32_t                             ScaleAxis;
#define Xaxis                                    0
#define Yaxis                                    1
   float                                MinCPSizeY;
   float                                MaxCPSizeX;
   float                                MaxCPSizeY;
   int32_t                              YFocal;
   int32_t                              XFocal;
   float                                My;
   float                                Mx;
   float                                Rmin;
   float                                Aspect;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_CPS_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_3DSTATE_CPS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_sfixed(values->MinCPSizeX, 0, 10, 7) |
      __gen_uint(values->StatisticsEnable, 11, 11) |
      __gen_uint(values->CoarsePixelShadingMode, 12, 13) |
      __gen_uint(values->ScaleAxis, 14, 14) |
      __gen_sfixed(values->MinCPSizeY, 16, 26, 7);

   dw[2] =
      __gen_sfixed(values->MaxCPSizeX, 0, 10, 7) |
      __gen_sfixed(values->MaxCPSizeY, 16, 26, 7);

   dw[3] =
      __gen_sint(values->YFocal, 0, 15);

   dw[4] =
      __gen_sint(values->XFocal, 0, 15);

   dw[5] =
      __gen_float(values->My);

   dw[6] =
      __gen_float(values->Mx);

   dw[7] =
      __gen_float(values->Rmin);

   dw[8] =
      __gen_float(values->Aspect);
}

#define GFX11_3DSTATE_DEPTH_BUFFER_length      8
#define GFX11_3DSTATE_DEPTH_BUFFER_length_bias      2
#define GFX11_3DSTATE_DEPTH_BUFFER_header       \
   .DWordLength                         =      6,  \
   ._3DCommandSubOpcode                 =      5,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_DEPTH_BUFFER {
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
#define SURFTYPE_2D                              1
#define SURFTYPE_CUBE                            3
#define SURFTYPE_NULL                            7
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             LOD;
   uint32_t                             Width;
   uint32_t                             Height;
   uint32_t                             MOCS;
   uint32_t                             MinimumArrayElement;
   uint32_t                             Depth;
   uint32_t                             MipTailStartLOD;
   uint32_t                             TiledResourceMode;
#define NONE                                     0
#define TILEYF                                   1
#define TILEYS                                   2
   uint32_t                             SurfaceQPitch;
   uint32_t                             RenderTargetViewExtent;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_DEPTH_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_DEPTH_BUFFER * restrict values)
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

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   dw[4] =
      __gen_uint(values->LOD, 0, 3) |
      __gen_uint(values->Width, 4, 17) |
      __gen_uint(values->Height, 18, 31);

   dw[5] =
      __gen_uint(values->MOCS, 0, 6) |
      __gen_uint(values->MinimumArrayElement, 10, 20) |
      __gen_uint(values->Depth, 21, 31);

   dw[6] =
      __gen_uint(values->MipTailStartLOD, 26, 29) |
      __gen_uint(values->TiledResourceMode, 30, 31);

   dw[7] =
      __gen_uint(values->SurfaceQPitch, 0, 14) |
      __gen_uint(values->RenderTargetViewExtent, 21, 31);
}

#define GFX11_3DSTATE_DRAWING_RECTANGLE_length      4
#define GFX11_3DSTATE_DRAWING_RECTANGLE_length_bias      2
#define GFX11_3DSTATE_DRAWING_RECTANGLE_header  \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_DRAWING_RECTANGLE {
   uint32_t                             DWordLength;
   uint32_t                             CoreModeSelect;
#define Legacy                                   0
#define Core0Enabled                             1
#define Core1Enabled                             2
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
GFX11_3DSTATE_DRAWING_RECTANGLE_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_3DSTATE_DRAWING_RECTANGLE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->CoreModeSelect, 14, 15) |
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

#define GFX11_3DSTATE_DS_length               11
#define GFX11_3DSTATE_DS_length_bias           2
#define GFX11_3DSTATE_DS_header                 \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     29,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   bool                                 AccessesUAV;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             ThreadDispatchPriority;
#define High                                     1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   bool                                 VectorMaskEnable;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             PatchURBEntryReadOffset;
   uint32_t                             PatchURBEntryReadLength;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 Enable;
   bool                                 CacheDisable;
   bool                                 ComputeWCoordinateEnable;
   uint32_t                             DispatchMode;
#define DISPATCH_MODE_SIMD8_SINGLE_PATCH         1
#define DISPATCH_MODE_SIMD8_SINGLE_OR_DUAL_PATCH 2
   bool                                 StatisticsEnable;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             UserClipDistanceCullTestEnableBitmask;
   uint32_t                             UserClipDistanceClipTestEnableBitmask;
   uint32_t                             VertexURBEntryOutputLength;
   uint32_t                             VertexURBEntryOutputReadOffset;
   uint64_t                             DUAL_PATCHKernelStartPointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_DS_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_offset(values->KernelStartPointer, 6, 63);
   dw[1] = v1;
   dw[2] = v1 >> 32;

   dw[3] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->AccessesUAV, 14, 14) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadDispatchPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30);

   const uint64_t v4 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->ScratchSpaceBasePointer, v4, 10, 63);
   dw[4] = v4_address;
   dw[5] = (v4_address >> 32) | (v4 >> 32);

   dw[6] =
      __gen_uint(values->PatchURBEntryReadOffset, 4, 9) |
      __gen_uint(values->PatchURBEntryReadLength, 11, 17) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 20, 24);

   dw[7] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->CacheDisable, 1, 1) |
      __gen_uint(values->ComputeWCoordinateEnable, 2, 2) |
      __gen_uint(values->DispatchMode, 3, 4) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->MaximumNumberofThreads, 21, 30);

   dw[8] =
      __gen_uint(values->UserClipDistanceCullTestEnableBitmask, 0, 7) |
      __gen_uint(values->UserClipDistanceClipTestEnableBitmask, 8, 15) |
      __gen_uint(values->VertexURBEntryOutputLength, 16, 20) |
      __gen_uint(values->VertexURBEntryOutputReadOffset, 21, 26);

   const uint64_t v9 =
      __gen_offset(values->DUAL_PATCHKernelStartPointer, 6, 63);
   dw[9] = v9;
   dw[10] = v9 >> 32;
}

#define GFX11_3DSTATE_GATHER_CONSTANT_DS_length_bias      2
#define GFX11_3DSTATE_GATHER_CONSTANT_DS_header \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     55,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_CONSTANT_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             UpdateGatherTableOnly;
#define CommitGather                             0
#define NonCommitGather                          1
   uint32_t                             ConstantBufferBindingTableBlock;
   uint32_t                             ConstantBufferValid;
   uint32_t                             OnDieTable;
#define Load                                     0
#define Read                                     1
   bool                                 ConstantBufferDx9GenerateStall;
   uint64_t                             GatherBufferOffset;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_CONSTANT_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_CONSTANT_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->UpdateGatherTableOnly, 1, 1) |
      __gen_uint(values->ConstantBufferBindingTableBlock, 12, 15) |
      __gen_uint(values->ConstantBufferValid, 16, 31);

   dw[2] =
      __gen_uint(values->OnDieTable, 3, 3) |
      __gen_uint(values->ConstantBufferDx9GenerateStall, 5, 5) |
      __gen_offset(values->GatherBufferOffset, 6, 22);
}

#define GFX11_3DSTATE_GATHER_CONSTANT_GS_length_bias      2
#define GFX11_3DSTATE_GATHER_CONSTANT_GS_header \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     53,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_CONSTANT_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             UpdateGatherTableOnly;
#define CommitGather                             0
#define NonCommitGather                          1
   uint32_t                             ConstantBufferBindingTableBlock;
   uint32_t                             ConstantBufferValid;
   uint32_t                             OnDieTable;
#define Load                                     0
#define Read                                     1
   bool                                 ConstantBufferDx9GenerateStall;
   uint64_t                             GatherBufferOffset;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_CONSTANT_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_CONSTANT_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->UpdateGatherTableOnly, 1, 1) |
      __gen_uint(values->ConstantBufferBindingTableBlock, 12, 15) |
      __gen_uint(values->ConstantBufferValid, 16, 31);

   dw[2] =
      __gen_uint(values->OnDieTable, 3, 3) |
      __gen_uint(values->ConstantBufferDx9GenerateStall, 5, 5) |
      __gen_offset(values->GatherBufferOffset, 6, 22);
}

#define GFX11_3DSTATE_GATHER_CONSTANT_HS_length_bias      2
#define GFX11_3DSTATE_GATHER_CONSTANT_HS_header \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     54,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_CONSTANT_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             UpdateGatherTableOnly;
#define CommitGather                             0
#define NonCommitGather                          1
   uint32_t                             ConstantBufferBindingTableBlock;
   uint32_t                             ConstantBufferValid;
   uint32_t                             OnDieTable;
#define Load                                     0
#define Read                                     1
   bool                                 ConstantBufferDx9GenerateStall;
   uint64_t                             GatherBufferOffset;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_CONSTANT_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_CONSTANT_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->UpdateGatherTableOnly, 1, 1) |
      __gen_uint(values->ConstantBufferBindingTableBlock, 12, 15) |
      __gen_uint(values->ConstantBufferValid, 16, 31);

   dw[2] =
      __gen_uint(values->OnDieTable, 3, 3) |
      __gen_uint(values->ConstantBufferDx9GenerateStall, 5, 5) |
      __gen_offset(values->GatherBufferOffset, 6, 22);
}

#define GFX11_3DSTATE_GATHER_CONSTANT_PS_length_bias      2
#define GFX11_3DSTATE_GATHER_CONSTANT_PS_header \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     56,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_CONSTANT_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 DX9OnDieRegisterReadEnable;
   uint32_t                             UpdateGatherTableOnly;
#define CommitGather                             0
#define NonCommitGather                          1
   uint32_t                             ConstantBufferBindingTableBlock;
   uint32_t                             ConstantBufferValid;
   uint32_t                             OnDieTable;
#define Load                                     0
#define Read                                     1
   bool                                 ConstantBufferDx9Enable;
   bool                                 ConstantBufferDx9GenerateStall;
   uint64_t                             GatherBufferOffset;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_CONSTANT_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_CONSTANT_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DX9OnDieRegisterReadEnable, 0, 0) |
      __gen_uint(values->UpdateGatherTableOnly, 1, 1) |
      __gen_uint(values->ConstantBufferBindingTableBlock, 12, 15) |
      __gen_uint(values->ConstantBufferValid, 16, 31);

   dw[2] =
      __gen_uint(values->OnDieTable, 3, 3) |
      __gen_uint(values->ConstantBufferDx9Enable, 4, 4) |
      __gen_uint(values->ConstantBufferDx9GenerateStall, 5, 5) |
      __gen_offset(values->GatherBufferOffset, 6, 22);
}

#define GFX11_3DSTATE_GATHER_CONSTANT_VS_length_bias      2
#define GFX11_3DSTATE_GATHER_CONSTANT_VS_header \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     52,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_CONSTANT_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 DX9OnDieRegisterReadEnable;
   uint32_t                             UpdateGatherTableOnly;
#define CommitGather                             0
#define NonCommitGather                          1
   uint32_t                             ConstantBufferBindingTableBlock;
   uint32_t                             ConstantBufferValid;
   uint32_t                             OnDieTable;
#define Load                                     0
#define Read                                     1
   bool                                 ConstantBufferDx9Enable;
   bool                                 ConstantBufferDx9GenerateStall;
   uint64_t                             GatherBufferOffset;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_CONSTANT_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                      __attribute__((unused)) void * restrict dst,
                                      __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_CONSTANT_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DX9OnDieRegisterReadEnable, 0, 0) |
      __gen_uint(values->UpdateGatherTableOnly, 1, 1) |
      __gen_uint(values->ConstantBufferBindingTableBlock, 12, 15) |
      __gen_uint(values->ConstantBufferValid, 16, 31);

   dw[2] =
      __gen_uint(values->OnDieTable, 3, 3) |
      __gen_uint(values->ConstantBufferDx9Enable, 4, 4) |
      __gen_uint(values->ConstantBufferDx9GenerateStall, 5, 5) |
      __gen_offset(values->GatherBufferOffset, 6, 22);
}

#define GFX11_3DSTATE_GATHER_POOL_ALLOC_length      4
#define GFX11_3DSTATE_GATHER_POOL_ALLOC_length_bias      2
#define GFX11_3DSTATE_GATHER_POOL_ALLOC_header  \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     26,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GATHER_POOL_ALLOC {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MOCS;
   bool                                 GatherPoolEnable;
   __gen_address_type                   GatherPoolBaseAddress;
   uint32_t                             GatherPoolBufferSize;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GATHER_POOL_ALLOC_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_3DSTATE_GATHER_POOL_ALLOC * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->MOCS, 0, 6) |
      __gen_uint(values->GatherPoolEnable, 11, 11);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->GatherPoolBaseAddress, v1, 12, 63);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->GatherPoolBufferSize, 12, 31);
}

#define GFX11_3DSTATE_GS_length               10
#define GFX11_3DSTATE_GS_length_bias           2
#define GFX11_3DSTATE_GS_header                 \
   .DWordLength                         =      8,  \
   ._3DCommandSubOpcode                 =     17,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   uint32_t                             ExpectedVertexCount;
   bool                                 SoftwareExceptionEnable;
   bool                                 MaskStackExceptionEnable;
   bool                                 AccessesUAV;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             ThreadDispatchPriority;
#define High                                     1
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
   enum GFX11_3D_Prim_Topo_Type         OutputTopology;
   uint32_t                             OutputVertexSize;
   uint32_t                             DispatchGRFStartRegisterForURBData54;
   bool                                 Enable;
   bool                                 DiscardAdjacency;
   uint32_t                             ReorderMode;
#define LEADING                                  0
#define TRAILING                                 1
   uint32_t                             Hint;
   bool                                 IncludePrimitiveID;
   uint32_t                             InvocationsIncrementValue;
   bool                                 StatisticsEnable;
   uint32_t                             DispatchMode;
#define DISPATCH_MODE_SIMD8                      3
   uint32_t                             DefaultStreamId;
   uint32_t                             InstanceControl;
   uint32_t                             ControlDataHeaderSize;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             StaticOutputVertexCount;
   bool                                 StaticOutput;
   uint32_t                             ControlDataFormat;
#define CUT                                      0
#define SID                                      1
   uint32_t                             UserClipDistanceCullTestEnableBitmask;
   uint32_t                             UserClipDistanceClipTestEnableBitmask;
   uint32_t                             VertexURBEntryOutputLength;
   uint32_t                             VertexURBEntryOutputReadOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_GS_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_offset(values->KernelStartPointer, 6, 63);
   dw[1] = v1;
   dw[2] = v1 >> 32;

   dw[3] =
      __gen_uint(values->ExpectedVertexCount, 0, 5) |
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->AccessesUAV, 12, 12) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadDispatchPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleProgramFlow, 31, 31);

   const uint64_t v4 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->ScratchSpaceBasePointer, v4, 10, 63);
   dw[4] = v4_address;
   dw[5] = (v4_address >> 32) | (v4 >> 32);

   dw[6] =
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 0, 3) |
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->IncludeVertexHandles, 10, 10) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->OutputTopology, 17, 22) |
      __gen_uint(values->OutputVertexSize, 23, 28) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData54, 29, 30);

   dw[7] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->DiscardAdjacency, 1, 1) |
      __gen_uint(values->ReorderMode, 2, 2) |
      __gen_uint(values->Hint, 3, 3) |
      __gen_uint(values->IncludePrimitiveID, 4, 4) |
      __gen_uint(values->InvocationsIncrementValue, 5, 9) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->DispatchMode, 11, 12) |
      __gen_uint(values->DefaultStreamId, 13, 14) |
      __gen_uint(values->InstanceControl, 15, 19) |
      __gen_uint(values->ControlDataHeaderSize, 20, 23);

   dw[8] =
      __gen_uint(values->MaximumNumberofThreads, 0, 8) |
      __gen_uint(values->StaticOutputVertexCount, 16, 26) |
      __gen_uint(values->StaticOutput, 30, 30) |
      __gen_uint(values->ControlDataFormat, 31, 31);

   dw[9] =
      __gen_uint(values->UserClipDistanceCullTestEnableBitmask, 0, 7) |
      __gen_uint(values->UserClipDistanceClipTestEnableBitmask, 8, 15) |
      __gen_uint(values->VertexURBEntryOutputLength, 16, 20) |
      __gen_uint(values->VertexURBEntryOutputReadOffset, 21, 26);
}

#define GFX11_3DSTATE_HIER_DEPTH_BUFFER_length      5
#define GFX11_3DSTATE_HIER_DEPTH_BUFFER_length_bias      2
#define GFX11_3DSTATE_HIER_DEPTH_BUFFER_header  \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =      7,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_HIER_DEPTH_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             TiledResourceMode;
#define NONE                                     0
#define TILEYF                                   1
#define TILEYS                                   2
   uint32_t                             MOCS;
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             SurfaceQPitch;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_HIER_DEPTH_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_3DSTATE_HIER_DEPTH_BUFFER * restrict values)
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
      __gen_uint(values->TiledResourceMode, 23, 24) |
      __gen_uint(values->MOCS, 25, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   dw[4] =
      __gen_uint(values->SurfaceQPitch, 0, 14);
}

#define GFX11_3DSTATE_HS_length                9
#define GFX11_3DSTATE_HS_length_bias           2
#define GFX11_3DSTATE_HS_header                 \
   .DWordLength                         =      7,  \
   ._3DCommandSubOpcode                 =     27,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 SoftwareExceptionEnable;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define alternate                                1
   uint32_t                             ThreadDispatchPriority;
#define High                                     1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   uint32_t                             InstanceCount;
   uint32_t                             MaximumNumberofThreads;
   bool                                 StatisticsEnable;
   bool                                 Enable;
   uint64_t                             KernelStartPointer;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   bool                                 IncludePrimitiveID;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   uint32_t                             DispatchMode;
#define DISPATCH_MODE_SINGLE_PATCH               0
#define DISPATCH_MODE__8_PATCH                   2
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 IncludeVertexHandles;
   bool                                 AccessesUAV;
   bool                                 VectorMaskEnable;
   bool                                 SingleProgramFlow;
   uint32_t                             DispatchGRFStartRegisterForURBData5;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_HS_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SoftwareExceptionEnable, 12, 12) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadDispatchPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29);

   dw[2] =
      __gen_uint(values->InstanceCount, 0, 3) |
      __gen_uint(values->MaximumNumberofThreads, 8, 16) |
      __gen_uint(values->StatisticsEnable, 29, 29) |
      __gen_uint(values->Enable, 31, 31);

   const uint64_t v3 =
      __gen_offset(values->KernelStartPointer, 6, 63);
   dw[3] = v3;
   dw[4] = v3 >> 32;

   const uint64_t v5 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   const uint64_t v5_address =
      __gen_address(data, &dw[5], values->ScratchSpaceBasePointer, v5, 10, 63);
   dw[5] = v5_address;
   dw[6] = (v5_address >> 32) | (v5 >> 32);

   dw[7] =
      __gen_uint(values->IncludePrimitiveID, 0, 0) |
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->DispatchMode, 17, 18) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 19, 23) |
      __gen_uint(values->IncludeVertexHandles, 24, 24) |
      __gen_uint(values->AccessesUAV, 25, 25) |
      __gen_uint(values->VectorMaskEnable, 26, 26) |
      __gen_uint(values->SingleProgramFlow, 27, 27) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData5, 28, 28);

   dw[8] = 0;
}

#define GFX11_3DSTATE_INDEX_BUFFER_length      5
#define GFX11_3DSTATE_INDEX_BUFFER_length_bias      2
#define GFX11_3DSTATE_INDEX_BUFFER_header       \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =     10,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_INDEX_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MOCS;
   uint32_t                             IndexFormat;
#define INDEX_BYTE                               0
#define INDEX_WORD                               1
#define INDEX_DWORD                              2
   __gen_address_type                   BufferStartingAddress;
   uint32_t                             BufferSize;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_INDEX_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_INDEX_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->MOCS, 0, 6) |
      __gen_uint(values->IndexFormat, 8, 9);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->BufferStartingAddress, 0, 0, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   dw[4] =
      __gen_uint(values->BufferSize, 0, 31);
}

#define GFX11_3DSTATE_LINE_STIPPLE_length      3
#define GFX11_3DSTATE_LINE_STIPPLE_length_bias      2
#define GFX11_3DSTATE_LINE_STIPPLE_header       \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      8,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_LINE_STIPPLE {
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
GFX11_3DSTATE_LINE_STIPPLE_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_LINE_STIPPLE * restrict values)
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

#define GFX11_3DSTATE_MONOFILTER_SIZE_length      2
#define GFX11_3DSTATE_MONOFILTER_SIZE_length_bias      2
#define GFX11_3DSTATE_MONOFILTER_SIZE_header    \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     17,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_MONOFILTER_SIZE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             MonochromeFilterHeight;
   uint32_t                             MonochromeFilterWidth;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_MONOFILTER_SIZE_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_3DSTATE_MONOFILTER_SIZE * restrict values)
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

#define GFX11_3DSTATE_MULTISAMPLE_length       2
#define GFX11_3DSTATE_MULTISAMPLE_length_bias      2
#define GFX11_3DSTATE_MULTISAMPLE_header        \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     13,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_MULTISAMPLE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             NumberofMultisamples;
   uint32_t                             PixelLocation;
#define CENTER                                   0
#define UL_CORNER                                1
   bool                                 PixelPositionOffsetEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_MULTISAMPLE_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_MULTISAMPLE * restrict values)
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
      __gen_uint(values->PixelLocation, 4, 4) |
      __gen_uint(values->PixelPositionOffsetEnable, 5, 5);
}

#define GFX11_3DSTATE_POLY_STIPPLE_OFFSET_length      2
#define GFX11_3DSTATE_POLY_STIPPLE_OFFSET_length_bias      2
#define GFX11_3DSTATE_POLY_STIPPLE_OFFSET_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =      6,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_POLY_STIPPLE_OFFSET {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PolygonStippleYOffset;
   uint32_t                             PolygonStippleXOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_POLY_STIPPLE_OFFSET_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX11_3DSTATE_POLY_STIPPLE_OFFSET * restrict values)
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

#define GFX11_3DSTATE_POLY_STIPPLE_PATTERN_length     33
#define GFX11_3DSTATE_POLY_STIPPLE_PATTERN_length_bias      2
#define GFX11_3DSTATE_POLY_STIPPLE_PATTERN_header\
   .DWordLength                         =     31,  \
   ._3DCommandSubOpcode                 =      7,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_POLY_STIPPLE_PATTERN {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PatternRow[32];
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_POLY_STIPPLE_PATTERN_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX11_3DSTATE_POLY_STIPPLE_PATTERN * restrict values)
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

#define GFX11_3DSTATE_PS_length               12
#define GFX11_3DSTATE_PS_length_bias           2
#define GFX11_3DSTATE_PS_header                 \
   .DWordLength                         =     10,  \
   ._3DCommandSubOpcode                 =     32,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PS {
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
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             ThreadDispatchPriority;
#define High                                     1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SinglePrecisionDenormalMode;
#define FlushedtoZero                            0
#define Retained                                 1
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
   bool                                 _8PixelDispatchEnable;
   bool                                 _16PixelDispatchEnable;
   bool                                 _32PixelDispatchEnable;
   uint32_t                             PositionXYOffsetSelect;
#define POSOFFSET_NONE                           0
#define POSOFFSET_CENTROID                       2
#define POSOFFSET_SAMPLE                         3
   uint32_t                             RenderTargetResolveType;
#define RESOLVE_DISABLED                         0
#define RESOLVE_PARTIAL                          1
#define FAST_CLEAR_0                             2
#define RESOLVE_FULL                             3
   bool                                 RenderTargetFastClearEnable;
   bool                                 PushConstantEnable;
   uint32_t                             MaximumNumberofThreadsPerPSD;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData2;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData1;
   uint32_t                             DispatchGRFStartRegisterForConstantSetupData0;
   uint64_t                             KernelStartPointer1;
   uint64_t                             KernelStartPointer2;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PS_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_offset(values->KernelStartPointer0, 6, 63);
   dw[1] = v1;
   dw[2] = v1 >> 32;

   dw[3] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->MaskStackExceptionEnable, 11, 11) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->RoundingMode, 14, 15) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadDispatchPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SinglePrecisionDenormalMode, 26, 26) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30) |
      __gen_uint(values->SingleProgramFlow, 31, 31);

   const uint64_t v4 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->ScratchSpaceBasePointer, v4, 10, 63);
   dw[4] = v4_address;
   dw[5] = (v4_address >> 32) | (v4 >> 32);

   dw[6] =
      __gen_uint(values->_8PixelDispatchEnable, 0, 0) |
      __gen_uint(values->_16PixelDispatchEnable, 1, 1) |
      __gen_uint(values->_32PixelDispatchEnable, 2, 2) |
      __gen_uint(values->PositionXYOffsetSelect, 3, 4) |
      __gen_uint(values->RenderTargetResolveType, 6, 7) |
      __gen_uint(values->RenderTargetFastClearEnable, 8, 8) |
      __gen_uint(values->PushConstantEnable, 11, 11) |
      __gen_uint(values->MaximumNumberofThreadsPerPSD, 23, 31);

   dw[7] =
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData2, 0, 6) |
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData1, 8, 14) |
      __gen_uint(values->DispatchGRFStartRegisterForConstantSetupData0, 16, 22);

   const uint64_t v8 =
      __gen_offset(values->KernelStartPointer1, 6, 63);
   dw[8] = v8;
   dw[9] = v8 >> 32;

   const uint64_t v10 =
      __gen_offset(values->KernelStartPointer2, 6, 63);
   dw[10] = v10;
   dw[11] = v10 >> 32;
}

#define GFX11_3DSTATE_PS_BLEND_length          2
#define GFX11_3DSTATE_PS_BLEND_length_bias      2
#define GFX11_3DSTATE_PS_BLEND_header           \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     77,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PS_BLEND {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 IndependentAlphaBlendEnable;
   bool                                 AlphaTestEnable;
   enum GFX11_3D_Color_Buffer_Blend_Factor DestinationBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Factor SourceBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Factor DestinationAlphaBlendFactor;
   enum GFX11_3D_Color_Buffer_Blend_Factor SourceAlphaBlendFactor;
   bool                                 ColorBufferBlendEnable;
   bool                                 HasWriteableRT;
   bool                                 AlphaToCoverageEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PS_BLEND_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_3DSTATE_PS_BLEND * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->IndependentAlphaBlendEnable, 7, 7) |
      __gen_uint(values->AlphaTestEnable, 8, 8) |
      __gen_uint(values->DestinationBlendFactor, 9, 13) |
      __gen_uint(values->SourceBlendFactor, 14, 18) |
      __gen_uint(values->DestinationAlphaBlendFactor, 19, 23) |
      __gen_uint(values->SourceAlphaBlendFactor, 24, 28) |
      __gen_uint(values->ColorBufferBlendEnable, 29, 29) |
      __gen_uint(values->HasWriteableRT, 30, 30) |
      __gen_uint(values->AlphaToCoverageEnable, 31, 31);
}

#define GFX11_3DSTATE_PS_EXTRA_length          2
#define GFX11_3DSTATE_PS_EXTRA_length_bias      2
#define GFX11_3DSTATE_PS_EXTRA_header           \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     79,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PS_EXTRA {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             InputCoverageMaskState;
#define ICMS_NONE                                0
#define ICMS_NORMAL                              1
#define ICMS_INNER_CONSERVATIVE                  2
#define ICMS_DEPTH_COVERAGE                      3
   bool                                 PixelShaderHasUAV;
   bool                                 PixelShaderPullsBary;
   bool                                 PixelShaderIsPerCoarsePixel;
   bool                                 PixelShaderComputesStencil;
   bool                                 PixelShaderIsPerSample;
   bool                                 PixelShaderDisablesAlphaToCoverage;
   bool                                 AttributeEnable;
   bool                                 SimplePSHint;
   bool                                 PixelShaderRequiresSubpixelSampleOffsets;
   bool                                 PixelShaderRequiresNonPerspectiveBaryPlaneCoefficients;
   bool                                 PixelShaderRequiresPerspectiveBaryPlaneCoefficients;
   bool                                 PixelShaderRequiresSourceDepthandorWPlaneCoefficients;
   bool                                 PixelShaderRequiresRequestedCoarsePixelShadingSize;
   bool                                 PixelShaderUsesSourceW;
   bool                                 PixelShaderUsesSourceDepth;
   bool                                 ForceComputedDepth;
   uint32_t                             PixelShaderComputedDepthMode;
#define PSCDEPTH_OFF                             0
#define PSCDEPTH_ON                              1
#define PSCDEPTH_ON_GE                           2
#define PSCDEPTH_ON_LE                           3
   bool                                 PixelShaderKillsPixel;
   bool                                 oMaskPresenttoRenderTarget;
   bool                                 PixelShaderDoesnotwritetoRT;
   bool                                 PixelShaderValid;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PS_EXTRA_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_3DSTATE_PS_EXTRA * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InputCoverageMaskState, 0, 1) |
      __gen_uint(values->PixelShaderHasUAV, 2, 2) |
      __gen_uint(values->PixelShaderPullsBary, 3, 3) |
      __gen_uint(values->PixelShaderIsPerCoarsePixel, 4, 4) |
      __gen_uint(values->PixelShaderComputesStencil, 5, 5) |
      __gen_uint(values->PixelShaderIsPerSample, 6, 6) |
      __gen_uint(values->PixelShaderDisablesAlphaToCoverage, 7, 7) |
      __gen_uint(values->AttributeEnable, 8, 8) |
      __gen_uint(values->SimplePSHint, 9, 9) |
      __gen_uint(values->PixelShaderRequiresSubpixelSampleOffsets, 18, 18) |
      __gen_uint(values->PixelShaderRequiresNonPerspectiveBaryPlaneCoefficients, 19, 19) |
      __gen_uint(values->PixelShaderRequiresPerspectiveBaryPlaneCoefficients, 20, 20) |
      __gen_uint(values->PixelShaderRequiresSourceDepthandorWPlaneCoefficients, 21, 21) |
      __gen_uint(values->PixelShaderRequiresRequestedCoarsePixelShadingSize, 22, 22) |
      __gen_uint(values->PixelShaderUsesSourceW, 23, 23) |
      __gen_uint(values->PixelShaderUsesSourceDepth, 24, 24) |
      __gen_uint(values->ForceComputedDepth, 25, 25) |
      __gen_uint(values->PixelShaderComputedDepthMode, 26, 27) |
      __gen_uint(values->PixelShaderKillsPixel, 28, 28) |
      __gen_uint(values->oMaskPresenttoRenderTarget, 29, 29) |
      __gen_uint(values->PixelShaderDoesnotwritetoRT, 30, 30) |
      __gen_uint(values->PixelShaderValid, 31, 31);
}

#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS_length      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS_length_bias      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     20,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
   uint32_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_DS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 5) |
      __gen_uint(values->ConstantBufferOffset, 16, 20);
}

#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS_length      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS_length_bias      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     21,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
   uint32_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_GS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 5) |
      __gen_uint(values->ConstantBufferOffset, 16, 20);
}

#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS_length      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS_length_bias      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     19,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
   uint32_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_HS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 5) |
      __gen_uint(values->ConstantBufferOffset, 16, 20);
}

#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS_length      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS_length_bias      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     22,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
   uint32_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_PS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 5) |
      __gen_uint(values->ConstantBufferOffset, 16, 20);
}

#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS_length      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS_length_bias      2
#define GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     18,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ConstantBufferSize;
   uint32_t                             ConstantBufferOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_PUSH_CONSTANT_ALLOC_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ConstantBufferSize, 0, 5) |
      __gen_uint(values->ConstantBufferOffset, 16, 20);
}

#define GFX11_3DSTATE_RASTER_length            5
#define GFX11_3DSTATE_RASTER_length_bias       2
#define GFX11_3DSTATE_RASTER_header             \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =     80,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_RASTER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 ViewportZNearClipTestEnable;
   bool                                 ScissorRectangleEnable;
   bool                                 AntialiasingEnable;
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
   uint32_t                             DXMultisampleRasterizationMode;
#define MSRASTMODE_OFF_PIXEL                     0
#define MSRASTMODE_OFF_PATTERN                   1
#define MSRASTMODE_ON_PIXEL                      2
#define MSRASTMODE_ON_PATTERN                    3
   bool                                 DXMultisampleRasterizationEnable;
   bool                                 SmoothPointEnable;
   uint32_t                             ForceMultisampling;
   uint32_t                             CullMode;
#define CULLMODE_BOTH                            0
#define CULLMODE_NONE                            1
#define CULLMODE_FRONT                           2
#define CULLMODE_BACK                            3
   uint32_t                             ForcedSampleCount;
#define FSC_NUMRASTSAMPLES_0                     0
#define FSC_NUMRASTSAMPLES_1                     1
#define FSC_NUMRASTSAMPLES_2                     2
#define FSC_NUMRASTSAMPLES_4                     3
#define FSC_NUMRASTSAMPLES_8                     4
#define FSC_NUMRASTSAMPLES_16                    5
   uint32_t                             FrontWinding;
#define Clockwise                                0
#define CounterClockwise                         1
   uint32_t                             APIMode;
#define DX9OGL                                   0
#define DX100                                    1
#define DX101                                    2
   bool                                 ConservativeRasterizationEnable;
   bool                                 ViewportZFarClipTestEnable;
   float                                GlobalDepthOffsetConstant;
   float                                GlobalDepthOffsetScale;
   float                                GlobalDepthOffsetClamp;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_RASTER_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_3DSTATE_RASTER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ViewportZNearClipTestEnable, 0, 0) |
      __gen_uint(values->ScissorRectangleEnable, 1, 1) |
      __gen_uint(values->AntialiasingEnable, 2, 2) |
      __gen_uint(values->BackFaceFillMode, 3, 4) |
      __gen_uint(values->FrontFaceFillMode, 5, 6) |
      __gen_uint(values->GlobalDepthOffsetEnablePoint, 7, 7) |
      __gen_uint(values->GlobalDepthOffsetEnableWireframe, 8, 8) |
      __gen_uint(values->GlobalDepthOffsetEnableSolid, 9, 9) |
      __gen_uint(values->DXMultisampleRasterizationMode, 10, 11) |
      __gen_uint(values->DXMultisampleRasterizationEnable, 12, 12) |
      __gen_uint(values->SmoothPointEnable, 13, 13) |
      __gen_uint(values->ForceMultisampling, 14, 14) |
      __gen_uint(values->CullMode, 16, 17) |
      __gen_uint(values->ForcedSampleCount, 18, 20) |
      __gen_uint(values->FrontWinding, 21, 21) |
      __gen_uint(values->APIMode, 22, 23) |
      __gen_uint(values->ConservativeRasterizationEnable, 24, 24) |
      __gen_uint(values->ViewportZFarClipTestEnable, 26, 26);

   dw[2] =
      __gen_float(values->GlobalDepthOffsetConstant);

   dw[3] =
      __gen_float(values->GlobalDepthOffsetScale);

   dw[4] =
      __gen_float(values->GlobalDepthOffsetClamp);
}

#define GFX11_3DSTATE_RS_CONSTANT_POINTER_length      4
#define GFX11_3DSTATE_RS_CONSTANT_POINTER_length_bias      2
#define GFX11_3DSTATE_RS_CONSTANT_POINTER_header\
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     84,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_RS_CONSTANT_POINTER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             OperationLoadorStore;
#define RS_Store                                 0
#define RS_Load                                  1
   uint32_t                             ShaderSelect;
#define VS                                       0
#define PS                                       4
   __gen_address_type                   GlobalConstantBufferAddress;
   __gen_address_type                   GlobalConstantBufferAddressHigh;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_RS_CONSTANT_POINTER_pack(__attribute__((unused)) __gen_user_data *data,
                                       __attribute__((unused)) void * restrict dst,
                                       __attribute__((unused)) const struct GFX11_3DSTATE_RS_CONSTANT_POINTER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->OperationLoadorStore, 12, 12) |
      __gen_uint(values->ShaderSelect, 28, 30);

   dw[2] = __gen_address(data, &dw[2], values->GlobalConstantBufferAddress, 0, 6, 31);

   dw[3] = __gen_address(data, &dw[3], values->GlobalConstantBufferAddressHigh, 0, 0, 31);
}

#define GFX11_3DSTATE_SAMPLER_PALETTE_LOAD0_length_bias      2
#define GFX11_3DSTATE_SAMPLER_PALETTE_LOAD0_header\
   ._3DCommandSubOpcode                 =      2,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_PALETTE_LOAD0 {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_PALETTE_LOAD0_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_PALETTE_LOAD0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_3DSTATE_SAMPLER_PALETTE_LOAD1_length_bias      2
#define GFX11_3DSTATE_SAMPLER_PALETTE_LOAD1_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     12,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_PALETTE_LOAD1 {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_PALETTE_LOAD1_pack(__attribute__((unused)) __gen_user_data *data,
                                         __attribute__((unused)) void * restrict dst,
                                         __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_PALETTE_LOAD1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS_length      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS_length_bias      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     45,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoDSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_DS * restrict values)
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

#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS_length      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS_length_bias      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     46,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoGSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_GS * restrict values)
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

#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS_length      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS_length_bias      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     44,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoHSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_HS * restrict values)
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

#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS_length      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS_length_bias      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     47,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoPSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_PS * restrict values)
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

#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS_length      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS_length_bias      2
#define GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     43,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             PointertoVSSamplerState;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS_pack(__attribute__((unused)) __gen_user_data *data,
                                             __attribute__((unused)) void * restrict dst,
                                             __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLER_STATE_POINTERS_VS * restrict values)
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

#define GFX11_3DSTATE_SAMPLE_MASK_length       2
#define GFX11_3DSTATE_SAMPLE_MASK_length_bias      2
#define GFX11_3DSTATE_SAMPLE_MASK_header        \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     24,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLE_MASK {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SampleMask;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLE_MASK_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLE_MASK * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SampleMask, 0, 15);
}

#define GFX11_3DSTATE_SAMPLE_PATTERN_length      9
#define GFX11_3DSTATE_SAMPLE_PATTERN_length_bias      2
#define GFX11_3DSTATE_SAMPLE_PATTERN_header     \
   .DWordLength                         =      7,  \
   ._3DCommandSubOpcode                 =     28,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SAMPLE_PATTERN {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   float                                _16xSample0YOffset;
   float                                _16xSample0XOffset;
   float                                _16xSample1YOffset;
   float                                _16xSample1XOffset;
   float                                _16xSample2YOffset;
   float                                _16xSample2XOffset;
   float                                _16xSample3YOffset;
   float                                _16xSample3XOffset;
   float                                _16xSample4YOffset;
   float                                _16xSample4XOffset;
   float                                _16xSample5YOffset;
   float                                _16xSample5XOffset;
   float                                _16xSample6YOffset;
   float                                _16xSample6XOffset;
   float                                _16xSample7YOffset;
   float                                _16xSample7XOffset;
   float                                _16xSample8YOffset;
   float                                _16xSample8XOffset;
   float                                _16xSample9YOffset;
   float                                _16xSample9XOffset;
   float                                _16xSample10YOffset;
   float                                _16xSample10XOffset;
   float                                _16xSample11YOffset;
   float                                _16xSample11XOffset;
   float                                _16xSample12YOffset;
   float                                _16xSample12XOffset;
   float                                _16xSample13YOffset;
   float                                _16xSample13XOffset;
   float                                _16xSample14YOffset;
   float                                _16xSample14XOffset;
   float                                _16xSample15YOffset;
   float                                _16xSample15XOffset;
   float                                _8xSample4YOffset;
   float                                _8xSample4XOffset;
   float                                _8xSample5YOffset;
   float                                _8xSample5XOffset;
   float                                _8xSample6YOffset;
   float                                _8xSample6XOffset;
   float                                _8xSample7YOffset;
   float                                _8xSample7XOffset;
   float                                _8xSample0YOffset;
   float                                _8xSample0XOffset;
   float                                _8xSample1YOffset;
   float                                _8xSample1XOffset;
   float                                _8xSample2YOffset;
   float                                _8xSample2XOffset;
   float                                _8xSample3YOffset;
   float                                _8xSample3XOffset;
   float                                _4xSample0YOffset;
   float                                _4xSample0XOffset;
   float                                _4xSample1YOffset;
   float                                _4xSample1XOffset;
   float                                _4xSample2YOffset;
   float                                _4xSample2XOffset;
   float                                _4xSample3YOffset;
   float                                _4xSample3XOffset;
   float                                _2xSample0YOffset;
   float                                _2xSample0XOffset;
   float                                _2xSample1YOffset;
   float                                _2xSample1XOffset;
   float                                _1xSample0YOffset;
   float                                _1xSample0XOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SAMPLE_PATTERN_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX11_3DSTATE_SAMPLE_PATTERN * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_ufixed(values->_16xSample0YOffset, 0, 3, 4) |
      __gen_ufixed(values->_16xSample0XOffset, 4, 7, 4) |
      __gen_ufixed(values->_16xSample1YOffset, 8, 11, 4) |
      __gen_ufixed(values->_16xSample1XOffset, 12, 15, 4) |
      __gen_ufixed(values->_16xSample2YOffset, 16, 19, 4) |
      __gen_ufixed(values->_16xSample2XOffset, 20, 23, 4) |
      __gen_ufixed(values->_16xSample3YOffset, 24, 27, 4) |
      __gen_ufixed(values->_16xSample3XOffset, 28, 31, 4);

   dw[2] =
      __gen_ufixed(values->_16xSample4YOffset, 0, 3, 4) |
      __gen_ufixed(values->_16xSample4XOffset, 4, 7, 4) |
      __gen_ufixed(values->_16xSample5YOffset, 8, 11, 4) |
      __gen_ufixed(values->_16xSample5XOffset, 12, 15, 4) |
      __gen_ufixed(values->_16xSample6YOffset, 16, 19, 4) |
      __gen_ufixed(values->_16xSample6XOffset, 20, 23, 4) |
      __gen_ufixed(values->_16xSample7YOffset, 24, 27, 4) |
      __gen_ufixed(values->_16xSample7XOffset, 28, 31, 4);

   dw[3] =
      __gen_ufixed(values->_16xSample8YOffset, 0, 3, 4) |
      __gen_ufixed(values->_16xSample8XOffset, 4, 7, 4) |
      __gen_ufixed(values->_16xSample9YOffset, 8, 11, 4) |
      __gen_ufixed(values->_16xSample9XOffset, 12, 15, 4) |
      __gen_ufixed(values->_16xSample10YOffset, 16, 19, 4) |
      __gen_ufixed(values->_16xSample10XOffset, 20, 23, 4) |
      __gen_ufixed(values->_16xSample11YOffset, 24, 27, 4) |
      __gen_ufixed(values->_16xSample11XOffset, 28, 31, 4);

   dw[4] =
      __gen_ufixed(values->_16xSample12YOffset, 0, 3, 4) |
      __gen_ufixed(values->_16xSample12XOffset, 4, 7, 4) |
      __gen_ufixed(values->_16xSample13YOffset, 8, 11, 4) |
      __gen_ufixed(values->_16xSample13XOffset, 12, 15, 4) |
      __gen_ufixed(values->_16xSample14YOffset, 16, 19, 4) |
      __gen_ufixed(values->_16xSample14XOffset, 20, 23, 4) |
      __gen_ufixed(values->_16xSample15YOffset, 24, 27, 4) |
      __gen_ufixed(values->_16xSample15XOffset, 28, 31, 4);

   dw[5] =
      __gen_ufixed(values->_8xSample4YOffset, 0, 3, 4) |
      __gen_ufixed(values->_8xSample4XOffset, 4, 7, 4) |
      __gen_ufixed(values->_8xSample5YOffset, 8, 11, 4) |
      __gen_ufixed(values->_8xSample5XOffset, 12, 15, 4) |
      __gen_ufixed(values->_8xSample6YOffset, 16, 19, 4) |
      __gen_ufixed(values->_8xSample6XOffset, 20, 23, 4) |
      __gen_ufixed(values->_8xSample7YOffset, 24, 27, 4) |
      __gen_ufixed(values->_8xSample7XOffset, 28, 31, 4);

   dw[6] =
      __gen_ufixed(values->_8xSample0YOffset, 0, 3, 4) |
      __gen_ufixed(values->_8xSample0XOffset, 4, 7, 4) |
      __gen_ufixed(values->_8xSample1YOffset, 8, 11, 4) |
      __gen_ufixed(values->_8xSample1XOffset, 12, 15, 4) |
      __gen_ufixed(values->_8xSample2YOffset, 16, 19, 4) |
      __gen_ufixed(values->_8xSample2XOffset, 20, 23, 4) |
      __gen_ufixed(values->_8xSample3YOffset, 24, 27, 4) |
      __gen_ufixed(values->_8xSample3XOffset, 28, 31, 4);

   dw[7] =
      __gen_ufixed(values->_4xSample0YOffset, 0, 3, 4) |
      __gen_ufixed(values->_4xSample0XOffset, 4, 7, 4) |
      __gen_ufixed(values->_4xSample1YOffset, 8, 11, 4) |
      __gen_ufixed(values->_4xSample1XOffset, 12, 15, 4) |
      __gen_ufixed(values->_4xSample2YOffset, 16, 19, 4) |
      __gen_ufixed(values->_4xSample2XOffset, 20, 23, 4) |
      __gen_ufixed(values->_4xSample3YOffset, 24, 27, 4) |
      __gen_ufixed(values->_4xSample3XOffset, 28, 31, 4);

   dw[8] =
      __gen_ufixed(values->_2xSample0YOffset, 0, 3, 4) |
      __gen_ufixed(values->_2xSample0XOffset, 4, 7, 4) |
      __gen_ufixed(values->_2xSample1YOffset, 8, 11, 4) |
      __gen_ufixed(values->_2xSample1XOffset, 12, 15, 4) |
      __gen_ufixed(values->_1xSample0YOffset, 16, 19, 4) |
      __gen_ufixed(values->_1xSample0XOffset, 20, 23, 4);
}

#define GFX11_3DSTATE_SBE_length               6
#define GFX11_3DSTATE_SBE_length_bias          2
#define GFX11_3DSTATE_SBE_header                \
   .DWordLength                         =      4,  \
   ._3DCommandSubOpcode                 =     31,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SBE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             PrimitiveIDOverrideAttributeSelect;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   bool                                 PrimitiveIDOverrideComponentX;
   bool                                 PrimitiveIDOverrideComponentY;
   bool                                 PrimitiveIDOverrideComponentZ;
   bool                                 PrimitiveIDOverrideComponentW;
   uint32_t                             PointSpriteTextureCoordinateOrigin;
#define UPPERLEFT                                0
#define LOWERLEFT                                1
   bool                                 AttributeSwizzleEnable;
   uint32_t                             NumberofSFOutputAttributes;
   bool                                 ForceVertexURBEntryReadOffset;
   bool                                 ForceVertexURBEntryReadLength;
   uint32_t                             PointSpriteTextureCoordinateEnable;
   uint32_t                             ConstantInterpolationEnable;
   uint32_t                             AttributeActiveComponentFormat[32];
#define ACTIVE_COMPONENT_DISABLED                0
#define ACTIVE_COMPONENT_XY                      1
#define ACTIVE_COMPONENT_XYZ                     2
#define ACTIVE_COMPONENT_XYZW                    3
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SBE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_3DSTATE_SBE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->PrimitiveIDOverrideAttributeSelect, 0, 4) |
      __gen_uint(values->VertexURBEntryReadOffset, 5, 10) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 15) |
      __gen_uint(values->PrimitiveIDOverrideComponentX, 16, 16) |
      __gen_uint(values->PrimitiveIDOverrideComponentY, 17, 17) |
      __gen_uint(values->PrimitiveIDOverrideComponentZ, 18, 18) |
      __gen_uint(values->PrimitiveIDOverrideComponentW, 19, 19) |
      __gen_uint(values->PointSpriteTextureCoordinateOrigin, 20, 20) |
      __gen_uint(values->AttributeSwizzleEnable, 21, 21) |
      __gen_uint(values->NumberofSFOutputAttributes, 22, 27) |
      __gen_uint(values->ForceVertexURBEntryReadOffset, 28, 28) |
      __gen_uint(values->ForceVertexURBEntryReadLength, 29, 29);

   dw[2] =
      __gen_uint(values->PointSpriteTextureCoordinateEnable, 0, 31);

   dw[3] =
      __gen_uint(values->ConstantInterpolationEnable, 0, 31);

   dw[4] =
      __gen_uint(values->AttributeActiveComponentFormat[0], 0, 1) |
      __gen_uint(values->AttributeActiveComponentFormat[1], 2, 3) |
      __gen_uint(values->AttributeActiveComponentFormat[2], 4, 5) |
      __gen_uint(values->AttributeActiveComponentFormat[3], 6, 7) |
      __gen_uint(values->AttributeActiveComponentFormat[4], 8, 9) |
      __gen_uint(values->AttributeActiveComponentFormat[5], 10, 11) |
      __gen_uint(values->AttributeActiveComponentFormat[6], 12, 13) |
      __gen_uint(values->AttributeActiveComponentFormat[7], 14, 15) |
      __gen_uint(values->AttributeActiveComponentFormat[8], 16, 17) |
      __gen_uint(values->AttributeActiveComponentFormat[9], 18, 19) |
      __gen_uint(values->AttributeActiveComponentFormat[10], 20, 21) |
      __gen_uint(values->AttributeActiveComponentFormat[11], 22, 23) |
      __gen_uint(values->AttributeActiveComponentFormat[12], 24, 25) |
      __gen_uint(values->AttributeActiveComponentFormat[13], 26, 27) |
      __gen_uint(values->AttributeActiveComponentFormat[14], 28, 29) |
      __gen_uint(values->AttributeActiveComponentFormat[15], 30, 31);

   dw[5] =
      __gen_uint(values->AttributeActiveComponentFormat[16], 0, 1) |
      __gen_uint(values->AttributeActiveComponentFormat[17], 2, 3) |
      __gen_uint(values->AttributeActiveComponentFormat[18], 4, 5) |
      __gen_uint(values->AttributeActiveComponentFormat[19], 6, 7) |
      __gen_uint(values->AttributeActiveComponentFormat[20], 8, 9) |
      __gen_uint(values->AttributeActiveComponentFormat[21], 10, 11) |
      __gen_uint(values->AttributeActiveComponentFormat[22], 12, 13) |
      __gen_uint(values->AttributeActiveComponentFormat[23], 14, 15) |
      __gen_uint(values->AttributeActiveComponentFormat[24], 16, 17) |
      __gen_uint(values->AttributeActiveComponentFormat[25], 18, 19) |
      __gen_uint(values->AttributeActiveComponentFormat[26], 20, 21) |
      __gen_uint(values->AttributeActiveComponentFormat[27], 22, 23) |
      __gen_uint(values->AttributeActiveComponentFormat[28], 24, 25) |
      __gen_uint(values->AttributeActiveComponentFormat[29], 26, 27) |
      __gen_uint(values->AttributeActiveComponentFormat[30], 28, 29) |
      __gen_uint(values->AttributeActiveComponentFormat[31], 30, 31);
}

#define GFX11_3DSTATE_SBE_SWIZ_length         11
#define GFX11_3DSTATE_SBE_SWIZ_length_bias      2
#define GFX11_3DSTATE_SBE_SWIZ_header           \
   .DWordLength                         =      9,  \
   ._3DCommandSubOpcode                 =     81,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SBE_SWIZ {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   struct GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL Attribute[16];
   uint32_t                             AttributeWrapShortestEnables[16];
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SBE_SWIZ_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_3DSTATE_SBE_SWIZ * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   uint32_t v1_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v1_0, &values->Attribute[0]);

   uint32_t v1_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v1_1, &values->Attribute[1]);

   dw[1] =
      __gen_uint(v1_0, 0, 15) |
      __gen_uint(v1_1, 16, 31);

   uint32_t v2_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v2_0, &values->Attribute[2]);

   uint32_t v2_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v2_1, &values->Attribute[3]);

   dw[2] =
      __gen_uint(v2_0, 0, 15) |
      __gen_uint(v2_1, 16, 31);

   uint32_t v3_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v3_0, &values->Attribute[4]);

   uint32_t v3_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v3_1, &values->Attribute[5]);

   dw[3] =
      __gen_uint(v3_0, 0, 15) |
      __gen_uint(v3_1, 16, 31);

   uint32_t v4_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v4_0, &values->Attribute[6]);

   uint32_t v4_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v4_1, &values->Attribute[7]);

   dw[4] =
      __gen_uint(v4_0, 0, 15) |
      __gen_uint(v4_1, 16, 31);

   uint32_t v5_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v5_0, &values->Attribute[8]);

   uint32_t v5_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v5_1, &values->Attribute[9]);

   dw[5] =
      __gen_uint(v5_0, 0, 15) |
      __gen_uint(v5_1, 16, 31);

   uint32_t v6_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v6_0, &values->Attribute[10]);

   uint32_t v6_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v6_1, &values->Attribute[11]);

   dw[6] =
      __gen_uint(v6_0, 0, 15) |
      __gen_uint(v6_1, 16, 31);

   uint32_t v7_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v7_0, &values->Attribute[12]);

   uint32_t v7_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v7_1, &values->Attribute[13]);

   dw[7] =
      __gen_uint(v7_0, 0, 15) |
      __gen_uint(v7_1, 16, 31);

   uint32_t v8_0;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v8_0, &values->Attribute[14]);

   uint32_t v8_1;
   GFX11_SF_OUTPUT_ATTRIBUTE_DETAIL_pack(data, &v8_1, &values->Attribute[15]);

   dw[8] =
      __gen_uint(v8_0, 0, 15) |
      __gen_uint(v8_1, 16, 31);

   dw[9] =
      __gen_uint(values->AttributeWrapShortestEnables[0], 0, 3) |
      __gen_uint(values->AttributeWrapShortestEnables[1], 4, 7) |
      __gen_uint(values->AttributeWrapShortestEnables[2], 8, 11) |
      __gen_uint(values->AttributeWrapShortestEnables[3], 12, 15) |
      __gen_uint(values->AttributeWrapShortestEnables[4], 16, 19) |
      __gen_uint(values->AttributeWrapShortestEnables[5], 20, 23) |
      __gen_uint(values->AttributeWrapShortestEnables[6], 24, 27) |
      __gen_uint(values->AttributeWrapShortestEnables[7], 28, 31);

   dw[10] =
      __gen_uint(values->AttributeWrapShortestEnables[8], 0, 3) |
      __gen_uint(values->AttributeWrapShortestEnables[9], 4, 7) |
      __gen_uint(values->AttributeWrapShortestEnables[10], 8, 11) |
      __gen_uint(values->AttributeWrapShortestEnables[11], 12, 15) |
      __gen_uint(values->AttributeWrapShortestEnables[12], 16, 19) |
      __gen_uint(values->AttributeWrapShortestEnables[13], 20, 23) |
      __gen_uint(values->AttributeWrapShortestEnables[14], 24, 27) |
      __gen_uint(values->AttributeWrapShortestEnables[15], 28, 31);
}

#define GFX11_3DSTATE_SCISSOR_STATE_POINTERS_length      2
#define GFX11_3DSTATE_SCISSOR_STATE_POINTERS_length_bias      2
#define GFX11_3DSTATE_SCISSOR_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     15,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SCISSOR_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             ScissorRectPointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SCISSOR_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                          __attribute__((unused)) void * restrict dst,
                                          __attribute__((unused)) const struct GFX11_3DSTATE_SCISSOR_STATE_POINTERS * restrict values)
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

#define GFX11_3DSTATE_SF_length                4
#define GFX11_3DSTATE_SF_length_bias           2
#define GFX11_3DSTATE_SF_header                 \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     19,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SF {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 ViewportTransformEnable;
   bool                                 StatisticsEnable;
   bool                                 LegacyGlobalDepthBiasEnable;
   float                                LineWidth;
   uint32_t                             LineEndCapAntialiasingRegionWidth;
#define _05pixels                                0
#define _10pixels                                1
#define _20pixels                                2
#define _40pixels                                3
   float                                PointWidth;
   uint32_t                             PointWidthSource;
#define Vertex                                   0
#define State                                    1
   uint32_t                             VertexSubPixelPrecisionSelect;
#define _8Bit                                    0
#define _4Bit                                    1
   bool                                 SmoothPointEnable;
   uint32_t                             AALineDistanceMode;
#define AALINEDISTANCE_TRUE                      1
   uint32_t                             TriangleFanProvokingVertexSelect;
   uint32_t                             LineStripListProvokingVertexSelect;
   uint32_t                             TriangleStripListProvokingVertexSelect;
   bool                                 LastPixelEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SF_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_SF * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ViewportTransformEnable, 1, 1) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->LegacyGlobalDepthBiasEnable, 11, 11) |
      __gen_ufixed(values->LineWidth, 12, 29, 7);

   dw[2] =
      __gen_uint(values->LineEndCapAntialiasingRegionWidth, 16, 17);

   dw[3] =
      __gen_ufixed(values->PointWidth, 0, 10, 3) |
      __gen_uint(values->PointWidthSource, 11, 11) |
      __gen_uint(values->VertexSubPixelPrecisionSelect, 12, 12) |
      __gen_uint(values->SmoothPointEnable, 13, 13) |
      __gen_uint(values->AALineDistanceMode, 14, 14) |
      __gen_uint(values->TriangleFanProvokingVertexSelect, 25, 26) |
      __gen_uint(values->LineStripListProvokingVertexSelect, 27, 28) |
      __gen_uint(values->TriangleStripListProvokingVertexSelect, 29, 30) |
      __gen_uint(values->LastPixelEnable, 31, 31);
}

#define GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS_length      2
#define GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS_length_bias      2
#define GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     32,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 SliceHashStatePointerValid;
   uint64_t                             SliceHashTableStatePointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS_pack(__attribute__((unused)) __gen_user_data *data,
                                              __attribute__((unused)) void * restrict dst,
                                              __attribute__((unused)) const struct GFX11_3DSTATE_SLICE_TABLE_STATE_POINTERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SliceHashStatePointerValid, 0, 0) |
      __gen_offset(values->SliceHashTableStatePointer, 6, 31);
}

#define GFX11_3DSTATE_SO_BUFFER_length         8
#define GFX11_3DSTATE_SO_BUFFER_length_bias      2
#define GFX11_3DSTATE_SO_BUFFER_header          \
   .DWordLength                         =      6,  \
   ._3DCommandSubOpcode                 =     24,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SO_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 StreamOutputBufferOffsetAddressEnable;
   bool                                 StreamOffsetWriteEnable;
   uint32_t                             MOCS;
   uint32_t                             SOBufferIndex;
   bool                                 SOBufferEnable;
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             SurfaceSize;
   __gen_address_type                   StreamOutputBufferOffsetAddress;
   uint32_t                             StreamOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_SO_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_3DSTATE_SO_BUFFER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->StreamOutputBufferOffsetAddressEnable, 20, 20) |
      __gen_uint(values->StreamOffsetWriteEnable, 21, 21) |
      __gen_uint(values->MOCS, 22, 28) |
      __gen_uint(values->SOBufferIndex, 29, 30) |
      __gen_uint(values->SOBufferEnable, 31, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 2, 47);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   dw[4] =
      __gen_uint(values->SurfaceSize, 0, 29);

   const uint64_t v5_address =
      __gen_address(data, &dw[5], values->StreamOutputBufferOffsetAddress, 0, 2, 47);
   dw[5] = v5_address;
   dw[6] = v5_address >> 32;

   dw[7] =
      __gen_uint(values->StreamOffset, 0, 31);
}

#define GFX11_3DSTATE_SO_DECL_LIST_length_bias      2
#define GFX11_3DSTATE_SO_DECL_LIST_header       \
   ._3DCommandSubOpcode                 =     23,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_SO_DECL_LIST {
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
GFX11_3DSTATE_SO_DECL_LIST_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_SO_DECL_LIST * restrict values)
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

#define GFX11_3DSTATE_STENCIL_BUFFER_length      5
#define GFX11_3DSTATE_STENCIL_BUFFER_length_bias      2
#define GFX11_3DSTATE_STENCIL_BUFFER_header     \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =      6,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_STENCIL_BUFFER {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             SurfacePitch;
   uint32_t                             MOCS;
   bool                                 StencilBufferEnable;
   __gen_address_type                   SurfaceBaseAddress;
   uint32_t                             SurfaceQPitch;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_STENCIL_BUFFER_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX11_3DSTATE_STENCIL_BUFFER * restrict values)
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
      __gen_uint(values->MOCS, 22, 28) |
      __gen_uint(values->StencilBufferEnable, 31, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->SurfaceBaseAddress, 0, 0, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   dw[4] =
      __gen_uint(values->SurfaceQPitch, 0, 14);
}

#define GFX11_3DSTATE_STREAMOUT_length         5
#define GFX11_3DSTATE_STREAMOUT_length_bias      2
#define GFX11_3DSTATE_STREAMOUT_header          \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =     30,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_STREAMOUT {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ForceRendering;
#define Resreved                                 1
#define Force_Off                                2
#define Force_on                                 3
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
   uint32_t                             Buffer0SurfacePitch;
   uint32_t                             Buffer1SurfacePitch;
   uint32_t                             Buffer2SurfacePitch;
   uint32_t                             Buffer3SurfacePitch;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_STREAMOUT_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_3DSTATE_STREAMOUT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ForceRendering, 23, 24) |
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

   dw[3] =
      __gen_uint(values->Buffer0SurfacePitch, 0, 11) |
      __gen_uint(values->Buffer1SurfacePitch, 16, 27);

   dw[4] =
      __gen_uint(values->Buffer2SurfacePitch, 0, 11) |
      __gen_uint(values->Buffer3SurfacePitch, 16, 27);
}

#define GFX11_3DSTATE_TE_length                4
#define GFX11_3DSTATE_TE_length_bias           2
#define GFX11_3DSTATE_TE_header                 \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     28,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_TE {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 TEEnable;
   uint32_t                             TEMode;
#define HW_TESS                                  0
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
GFX11_3DSTATE_TE_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_TE * restrict values)
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

#define GFX11_3DSTATE_URB_CLEAR_length         2
#define GFX11_3DSTATE_URB_CLEAR_length_bias      2
#define GFX11_3DSTATE_URB_CLEAR_header          \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     29,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_URB_CLEAR {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             URBAddress;
   uint32_t                             URBClearLength;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_URB_CLEAR_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_3DSTATE_URB_CLEAR * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->URBAddress, 0, 14) |
      __gen_uint(values->URBClearLength, 16, 29);
}

#define GFX11_3DSTATE_URB_DS_length            2
#define GFX11_3DSTATE_URB_DS_length_bias       2
#define GFX11_3DSTATE_URB_DS_header             \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     50,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_URB_DS {
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
GFX11_3DSTATE_URB_DS_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_3DSTATE_URB_DS * restrict values)
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
      __gen_uint(values->DSURBStartingAddress, 25, 31);
}

#define GFX11_3DSTATE_URB_GS_length            2
#define GFX11_3DSTATE_URB_GS_length_bias       2
#define GFX11_3DSTATE_URB_GS_header             \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     51,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_URB_GS {
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
GFX11_3DSTATE_URB_GS_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_3DSTATE_URB_GS * restrict values)
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
      __gen_uint(values->GSURBStartingAddress, 25, 31);
}

#define GFX11_3DSTATE_URB_HS_length            2
#define GFX11_3DSTATE_URB_HS_length_bias       2
#define GFX11_3DSTATE_URB_HS_header             \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     49,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_URB_HS {
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
GFX11_3DSTATE_URB_HS_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_3DSTATE_URB_HS * restrict values)
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
      __gen_uint(values->HSURBStartingAddress, 25, 31);
}

#define GFX11_3DSTATE_URB_VS_length            2
#define GFX11_3DSTATE_URB_VS_length_bias       2
#define GFX11_3DSTATE_URB_VS_header             \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     48,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_URB_VS {
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
GFX11_3DSTATE_URB_VS_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_3DSTATE_URB_VS * restrict values)
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
      __gen_uint(values->VSURBStartingAddress, 25, 31);
}

#define GFX11_3DSTATE_VERTEX_BUFFERS_length_bias      2
#define GFX11_3DSTATE_VERTEX_BUFFERS_header     \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =      8,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VERTEX_BUFFERS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VERTEX_BUFFERS_pack(__attribute__((unused)) __gen_user_data *data,
                                  __attribute__((unused)) void * restrict dst,
                                  __attribute__((unused)) const struct GFX11_3DSTATE_VERTEX_BUFFERS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_3DSTATE_VERTEX_ELEMENTS_length_bias      2
#define GFX11_3DSTATE_VERTEX_ELEMENTS_header    \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      9,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VERTEX_ELEMENTS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VERTEX_ELEMENTS_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_3DSTATE_VERTEX_ELEMENTS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_3DSTATE_VF_length                2
#define GFX11_3DSTATE_VF_length_bias           2
#define GFX11_3DSTATE_VF_header                 \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     12,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF {
   uint32_t                             DWordLength;
   bool                                 IndexedDrawCutIndexEnable;
   bool                                 ComponentPackingEnable;
   bool                                 SequentialDrawCutIndexEnable;
   bool                                 VertexIDOffsetEnable;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             CutIndex;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_VF * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->IndexedDrawCutIndexEnable, 8, 8) |
      __gen_uint(values->ComponentPackingEnable, 9, 9) |
      __gen_uint(values->SequentialDrawCutIndexEnable, 10, 10) |
      __gen_uint(values->VertexIDOffsetEnable, 11, 11) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->CutIndex, 0, 31);
}

#define GFX11_3DSTATE_VF_COMPONENT_PACKING_length      5
#define GFX11_3DSTATE_VF_COMPONENT_PACKING_length_bias      2
#define GFX11_3DSTATE_VF_COMPONENT_PACKING_header\
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =     85,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_COMPONENT_PACKING {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             VertexElement00Enables;
   uint32_t                             VertexElement01Enables;
   uint32_t                             VertexElement02Enables;
   uint32_t                             VertexElement03Enables;
   uint32_t                             VertexElement04Enables;
   uint32_t                             VertexElement05Enables;
   uint32_t                             VertexElement06Enables;
   uint32_t                             VertexElement07Enables;
   uint32_t                             VertexElement08Enables;
   uint32_t                             VertexElement09Enables;
   uint32_t                             VertexElement10Enables;
   uint32_t                             VertexElement11Enables;
   uint32_t                             VertexElement12Enables;
   uint32_t                             VertexElement13Enables;
   uint32_t                             VertexElement14Enables;
   uint32_t                             VertexElement15Enables;
   uint32_t                             VertexElement16Enables;
   uint32_t                             VertexElement17Enables;
   uint32_t                             VertexElement18Enables;
   uint32_t                             VertexElement19Enables;
   uint32_t                             VertexElement20Enables;
   uint32_t                             VertexElement21Enables;
   uint32_t                             VertexElement22Enables;
   uint32_t                             VertexElement23Enables;
   uint32_t                             VertexElement24Enables;
   uint32_t                             VertexElement25Enables;
   uint32_t                             VertexElement26Enables;
   uint32_t                             VertexElement27Enables;
   uint32_t                             VertexElement28Enables;
   uint32_t                             VertexElement29Enables;
   uint32_t                             VertexElement30Enables;
   uint32_t                             VertexElement31Enables;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_COMPONENT_PACKING_pack(__attribute__((unused)) __gen_user_data *data,
                                        __attribute__((unused)) void * restrict dst,
                                        __attribute__((unused)) const struct GFX11_3DSTATE_VF_COMPONENT_PACKING * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->VertexElement00Enables, 0, 3) |
      __gen_uint(values->VertexElement01Enables, 4, 7) |
      __gen_uint(values->VertexElement02Enables, 8, 11) |
      __gen_uint(values->VertexElement03Enables, 12, 15) |
      __gen_uint(values->VertexElement04Enables, 16, 19) |
      __gen_uint(values->VertexElement05Enables, 20, 23) |
      __gen_uint(values->VertexElement06Enables, 24, 27) |
      __gen_uint(values->VertexElement07Enables, 28, 31);

   dw[2] =
      __gen_uint(values->VertexElement08Enables, 0, 3) |
      __gen_uint(values->VertexElement09Enables, 4, 7) |
      __gen_uint(values->VertexElement10Enables, 8, 11) |
      __gen_uint(values->VertexElement11Enables, 12, 15) |
      __gen_uint(values->VertexElement12Enables, 16, 19) |
      __gen_uint(values->VertexElement13Enables, 20, 23) |
      __gen_uint(values->VertexElement14Enables, 24, 27) |
      __gen_uint(values->VertexElement15Enables, 28, 31);

   dw[3] =
      __gen_uint(values->VertexElement16Enables, 0, 3) |
      __gen_uint(values->VertexElement17Enables, 4, 7) |
      __gen_uint(values->VertexElement18Enables, 8, 11) |
      __gen_uint(values->VertexElement19Enables, 12, 15) |
      __gen_uint(values->VertexElement20Enables, 16, 19) |
      __gen_uint(values->VertexElement21Enables, 20, 23) |
      __gen_uint(values->VertexElement22Enables, 24, 27) |
      __gen_uint(values->VertexElement23Enables, 28, 31);

   dw[4] =
      __gen_uint(values->VertexElement24Enables, 0, 3) |
      __gen_uint(values->VertexElement25Enables, 4, 7) |
      __gen_uint(values->VertexElement26Enables, 8, 11) |
      __gen_uint(values->VertexElement27Enables, 12, 15) |
      __gen_uint(values->VertexElement28Enables, 16, 19) |
      __gen_uint(values->VertexElement29Enables, 20, 23) |
      __gen_uint(values->VertexElement30Enables, 24, 27) |
      __gen_uint(values->VertexElement31Enables, 28, 31);
}

#define GFX11_3DSTATE_VF_INSTANCING_length      3
#define GFX11_3DSTATE_VF_INSTANCING_length_bias      2
#define GFX11_3DSTATE_VF_INSTANCING_header      \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     73,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_INSTANCING {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             VertexElementIndex;
   bool                                 InstancingEnable;
   uint32_t                             InstanceDataStepRate;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_INSTANCING_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_3DSTATE_VF_INSTANCING * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->VertexElementIndex, 0, 5) |
      __gen_uint(values->InstancingEnable, 8, 8);

   dw[2] =
      __gen_uint(values->InstanceDataStepRate, 0, 31);
}

#define GFX11_3DSTATE_VF_SGVS_length           2
#define GFX11_3DSTATE_VF_SGVS_length_bias      2
#define GFX11_3DSTATE_VF_SGVS_header            \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     74,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_SGVS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             VertexIDElementOffset;
   uint32_t                             VertexIDComponentNumber;
#define COMP_0                                   0
#define COMP_1                                   1
#define COMP_2                                   2
#define COMP_3                                   3
   bool                                 VertexIDEnable;
   uint32_t                             InstanceIDElementOffset;
   uint32_t                             InstanceIDComponentNumber;
#define COMP_0                                   0
#define COMP_1                                   1
#define COMP_2                                   2
#define COMP_3                                   3
   bool                                 InstanceIDEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_SGVS_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_3DSTATE_VF_SGVS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->VertexIDElementOffset, 0, 5) |
      __gen_uint(values->VertexIDComponentNumber, 13, 14) |
      __gen_uint(values->VertexIDEnable, 15, 15) |
      __gen_uint(values->InstanceIDElementOffset, 16, 21) |
      __gen_uint(values->InstanceIDComponentNumber, 29, 30) |
      __gen_uint(values->InstanceIDEnable, 31, 31);
}

#define GFX11_3DSTATE_VF_SGVS_2_length         3
#define GFX11_3DSTATE_VF_SGVS_2_length_bias      2
#define GFX11_3DSTATE_VF_SGVS_2_header          \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =     86,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_SGVS_2 {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             XP0ElementOffset;
   uint32_t                             XP0SourceSelect;
#define VERTEX_LOCATION                          1
#define XP0_PARAMETER                            0
   uint32_t                             XP0ComponentNumber;
#define COMP_0                                   0
#define COMP_1                                   1
#define COMP_2                                   2
#define COMP_3                                   3
   uint32_t                             XP0Enable;
   uint32_t                             XP1ElementOffset;
   uint32_t                             XP1SourceSelect;
#define StartingInstanceLocation                 1
#define XP1_PARAMETER                            0
   uint32_t                             XP1ComponentNumber;
#define COMP_0                                   0
#define COMP_1                                   1
#define COMP_2                                   2
#define COMP_3                                   3
   uint32_t                             XP1Enable;
   uint32_t                             XP2ElementOffset;
   uint32_t                             XP2ComponentNumber;
#define COMP_0                                   0
#define COMP_1                                   1
#define COMP_2                                   2
#define COMP_3                                   3
   uint32_t                             XP2Enable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_SGVS_2_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_3DSTATE_VF_SGVS_2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->XP0ElementOffset, 0, 5) |
      __gen_uint(values->XP0SourceSelect, 12, 12) |
      __gen_uint(values->XP0ComponentNumber, 13, 14) |
      __gen_uint(values->XP0Enable, 15, 15) |
      __gen_uint(values->XP1ElementOffset, 16, 21) |
      __gen_uint(values->XP1SourceSelect, 28, 28) |
      __gen_uint(values->XP1ComponentNumber, 29, 30) |
      __gen_uint(values->XP1Enable, 31, 31);

   dw[2] =
      __gen_uint(values->XP2ElementOffset, 0, 5) |
      __gen_uint(values->XP2ComponentNumber, 13, 14) |
      __gen_uint(values->XP2Enable, 15, 15);
}

#define GFX11_3DSTATE_VF_STATISTICS_length      1
#define GFX11_3DSTATE_VF_STATISTICS_length_bias      1
#define GFX11_3DSTATE_VF_STATISTICS_header      \
   ._3DCommandSubOpcode                 =     11,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      1,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_STATISTICS {
   bool                                 StatisticsEnable;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_STATISTICS_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_3DSTATE_VF_STATISTICS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->StatisticsEnable, 0, 0) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_3DSTATE_VF_TOPOLOGY_length       2
#define GFX11_3DSTATE_VF_TOPOLOGY_length_bias      2
#define GFX11_3DSTATE_VF_TOPOLOGY_header        \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     75,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VF_TOPOLOGY {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   enum GFX11_3D_Prim_Topo_Type         PrimitiveTopologyType;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VF_TOPOLOGY_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_3DSTATE_VF_TOPOLOGY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->PrimitiveTopologyType, 0, 5);
}

#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC_length      2
#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC_length_bias      2
#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     35,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             CCViewportPointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC_pack(__attribute__((unused)) __gen_user_data *data,
                                              __attribute__((unused)) void * restrict dst,
                                              __attribute__((unused)) const struct GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_CC * restrict values)
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

#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_length      2
#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_length_bias      2
#define GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_header\
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     33,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             SFClipViewportPointer;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP_pack(__attribute__((unused)) __gen_user_data *data,
                                                   __attribute__((unused)) void * restrict dst,
                                                   __attribute__((unused)) const struct GFX11_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP * restrict values)
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

#define GFX11_3DSTATE_VS_length                9
#define GFX11_3DSTATE_VS_length_bias           2
#define GFX11_3DSTATE_VS_header                 \
   .DWordLength                         =      7,  \
   ._3DCommandSubOpcode                 =     16,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_VS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             KernelStartPointer;
   bool                                 SoftwareExceptionEnable;
   bool                                 AccessesUAV;
   bool                                 IllegalOpcodeExceptionEnable;
   uint32_t                             FloatingPointMode;
#define IEEE754                                  0
#define Alternate                                1
   uint32_t                             ThreadDispatchPriority;
#define High                                     1
   uint32_t                             BindingTableEntryCount;
   uint32_t                             SamplerCount;
#define NoSamplers                               0
#define _14Samplers                              1
#define _58Samplers                              2
#define _912Samplers                             3
#define _1316Samplers                            4
   bool                                 VectorMaskEnable;
   uint32_t                             PerThreadScratchSpace;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             VertexURBEntryReadOffset;
   uint32_t                             VertexURBEntryReadLength;
   uint32_t                             DispatchGRFStartRegisterForURBData;
   bool                                 Enable;
   bool                                 VertexCacheDisable;
   bool                                 SIMD8DispatchEnable;
   bool                                 SIMD8SingleInstanceDispatchEnable;
   bool                                 StatisticsEnable;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             UserClipDistanceCullTestEnableBitmask;
   uint32_t                             UserClipDistanceClipTestEnableBitmask;
   uint32_t                             VertexURBEntryOutputLength;
   uint32_t                             VertexURBEntryOutputReadOffset;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_VS_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_VS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_offset(values->KernelStartPointer, 6, 63);
   dw[1] = v1;
   dw[2] = v1 >> 32;

   dw[3] =
      __gen_uint(values->SoftwareExceptionEnable, 7, 7) |
      __gen_uint(values->AccessesUAV, 12, 12) |
      __gen_uint(values->IllegalOpcodeExceptionEnable, 13, 13) |
      __gen_uint(values->FloatingPointMode, 16, 16) |
      __gen_uint(values->ThreadDispatchPriority, 17, 17) |
      __gen_uint(values->BindingTableEntryCount, 18, 25) |
      __gen_uint(values->SamplerCount, 27, 29) |
      __gen_uint(values->VectorMaskEnable, 30, 30);

   const uint64_t v4 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3);
   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->ScratchSpaceBasePointer, v4, 10, 63);
   dw[4] = v4_address;
   dw[5] = (v4_address >> 32) | (v4 >> 32);

   dw[6] =
      __gen_uint(values->VertexURBEntryReadOffset, 4, 9) |
      __gen_uint(values->VertexURBEntryReadLength, 11, 16) |
      __gen_uint(values->DispatchGRFStartRegisterForURBData, 20, 24);

   dw[7] =
      __gen_uint(values->Enable, 0, 0) |
      __gen_uint(values->VertexCacheDisable, 1, 1) |
      __gen_uint(values->SIMD8DispatchEnable, 2, 2) |
      __gen_uint(values->SIMD8SingleInstanceDispatchEnable, 9, 9) |
      __gen_uint(values->StatisticsEnable, 10, 10) |
      __gen_uint(values->MaximumNumberofThreads, 22, 31);

   dw[8] =
      __gen_uint(values->UserClipDistanceCullTestEnableBitmask, 0, 7) |
      __gen_uint(values->UserClipDistanceClipTestEnableBitmask, 8, 15) |
      __gen_uint(values->VertexURBEntryOutputLength, 16, 20) |
      __gen_uint(values->VertexURBEntryOutputReadOffset, 21, 26);
}

#define GFX11_3DSTATE_WM_length                2
#define GFX11_3DSTATE_WM_length_bias           2
#define GFX11_3DSTATE_WM_header                 \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     20,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_WM {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             ForceKillPixelEnable;
#define ForceOff                                 1
#define ForceON                                  2
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
#define _05pixels                                0
#define _10pixels                                1
#define _20pixels                                2
#define _40pixels                                3
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
   uint32_t                             ForceThreadDispatchEnable;
#define ForceOff                                 1
#define ForceON                                  2
   uint32_t                             EarlyDepthStencilControl;
#define EDSC_NORMAL                              0
#define EDSC_PSEXEC                              1
#define EDSC_PREPS                               2
   bool                                 LegacyDiamondLineRasterization;
   bool                                 LegacyHierarchicalDepthBufferResolveEnable;
   bool                                 LegacyDepthBufferResolveEnable;
   bool                                 LegacyDepthBufferClearEnable;
   bool                                 StatisticsEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_WM_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_3DSTATE_WM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ForceKillPixelEnable, 0, 1) |
      __gen_uint(values->PointRasterizationRule, 2, 2) |
      __gen_uint(values->LineStippleEnable, 3, 3) |
      __gen_uint(values->PolygonStippleEnable, 4, 4) |
      __gen_uint(values->LineAntialiasingRegionWidth, 6, 7) |
      __gen_uint(values->LineEndCapAntialiasingRegionWidth, 8, 9) |
      __gen_uint(values->BarycentricInterpolationMode, 11, 16) |
      __gen_uint(values->PositionZWInterpolationMode, 17, 18) |
      __gen_uint(values->ForceThreadDispatchEnable, 19, 20) |
      __gen_uint(values->EarlyDepthStencilControl, 21, 22) |
      __gen_uint(values->LegacyDiamondLineRasterization, 26, 26) |
      __gen_uint(values->LegacyHierarchicalDepthBufferResolveEnable, 27, 27) |
      __gen_uint(values->LegacyDepthBufferResolveEnable, 28, 28) |
      __gen_uint(values->LegacyDepthBufferClearEnable, 30, 30) |
      __gen_uint(values->StatisticsEnable, 31, 31);
}

#define GFX11_3DSTATE_WM_CHROMAKEY_length      2
#define GFX11_3DSTATE_WM_CHROMAKEY_length_bias      2
#define GFX11_3DSTATE_WM_CHROMAKEY_header       \
   .DWordLength                         =      0,  \
   ._3DCommandSubOpcode                 =     76,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_WM_CHROMAKEY {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 ChromaKeyKillEnable;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_WM_CHROMAKEY_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_3DSTATE_WM_CHROMAKEY * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ChromaKeyKillEnable, 31, 31);
}

#define GFX11_3DSTATE_WM_DEPTH_STENCIL_length      4
#define GFX11_3DSTATE_WM_DEPTH_STENCIL_length_bias      2
#define GFX11_3DSTATE_WM_DEPTH_STENCIL_header   \
   .DWordLength                         =      2,  \
   ._3DCommandSubOpcode                 =     78,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_WM_DEPTH_STENCIL {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 DepthBufferWriteEnable;
   bool                                 DepthTestEnable;
   bool                                 StencilBufferWriteEnable;
   bool                                 StencilTestEnable;
   bool                                 DoubleSidedStencilEnable;
   enum GFX11_3D_Compare_Function       DepthTestFunction;
   enum GFX11_3D_Compare_Function       StencilTestFunction;
   enum GFX11_3D_Stencil_Operation      BackfaceStencilPassDepthPassOp;
   enum GFX11_3D_Stencil_Operation      BackfaceStencilPassDepthFailOp;
   enum GFX11_3D_Stencil_Operation      BackfaceStencilFailOp;
   enum GFX11_3D_Compare_Function       BackfaceStencilTestFunction;
   enum GFX11_3D_Stencil_Operation      StencilPassDepthPassOp;
   enum GFX11_3D_Stencil_Operation      StencilPassDepthFailOp;
   enum GFX11_3D_Stencil_Operation      StencilFailOp;
   uint32_t                             BackfaceStencilWriteMask;
   uint32_t                             BackfaceStencilTestMask;
   uint32_t                             StencilWriteMask;
   uint32_t                             StencilTestMask;
   uint32_t                             BackfaceStencilReferenceValue;
   uint32_t                             StencilReferenceValue;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_WM_DEPTH_STENCIL_pack(__attribute__((unused)) __gen_user_data *data,
                                    __attribute__((unused)) void * restrict dst,
                                    __attribute__((unused)) const struct GFX11_3DSTATE_WM_DEPTH_STENCIL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->DepthBufferWriteEnable, 0, 0) |
      __gen_uint(values->DepthTestEnable, 1, 1) |
      __gen_uint(values->StencilBufferWriteEnable, 2, 2) |
      __gen_uint(values->StencilTestEnable, 3, 3) |
      __gen_uint(values->DoubleSidedStencilEnable, 4, 4) |
      __gen_uint(values->DepthTestFunction, 5, 7) |
      __gen_uint(values->StencilTestFunction, 8, 10) |
      __gen_uint(values->BackfaceStencilPassDepthPassOp, 11, 13) |
      __gen_uint(values->BackfaceStencilPassDepthFailOp, 14, 16) |
      __gen_uint(values->BackfaceStencilFailOp, 17, 19) |
      __gen_uint(values->BackfaceStencilTestFunction, 20, 22) |
      __gen_uint(values->StencilPassDepthPassOp, 23, 25) |
      __gen_uint(values->StencilPassDepthFailOp, 26, 28) |
      __gen_uint(values->StencilFailOp, 29, 31);

   dw[2] =
      __gen_uint(values->BackfaceStencilWriteMask, 0, 7) |
      __gen_uint(values->BackfaceStencilTestMask, 8, 15) |
      __gen_uint(values->StencilWriteMask, 16, 23) |
      __gen_uint(values->StencilTestMask, 24, 31);

   dw[3] =
      __gen_uint(values->BackfaceStencilReferenceValue, 0, 7) |
      __gen_uint(values->StencilReferenceValue, 8, 15);
}

#define GFX11_3DSTATE_WM_HZ_OP_length          5
#define GFX11_3DSTATE_WM_HZ_OP_length_bias      2
#define GFX11_3DSTATE_WM_HZ_OP_header           \
   .DWordLength                         =      3,  \
   ._3DCommandSubOpcode                 =     82,  \
   ._3DCommandOpcode                    =      0,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_3DSTATE_WM_HZ_OP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint32_t                             NumberofMultisamples;
   uint32_t                             StencilClearValue;
   bool                                 FullSurfaceDepthandStencilClear;
   bool                                 PixelPositionOffsetEnable;
   bool                                 HierarchicalDepthBufferResolveEnable;
   bool                                 DepthBufferResolveEnable;
   bool                                 ScissorRectangleEnable;
   bool                                 DepthBufferClearEnable;
   bool                                 StencilBufferClearEnable;
   uint32_t                             ClearRectangleXMin;
   uint32_t                             ClearRectangleYMin;
   uint32_t                             ClearRectangleXMax;
   uint32_t                             ClearRectangleYMax;
   uint32_t                             SampleMask;
};

static inline __attribute__((always_inline)) void
GFX11_3DSTATE_WM_HZ_OP_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_3DSTATE_WM_HZ_OP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->NumberofMultisamples, 13, 15) |
      __gen_uint(values->StencilClearValue, 16, 23) |
      __gen_uint(values->FullSurfaceDepthandStencilClear, 25, 25) |
      __gen_uint(values->PixelPositionOffsetEnable, 26, 26) |
      __gen_uint(values->HierarchicalDepthBufferResolveEnable, 27, 27) |
      __gen_uint(values->DepthBufferResolveEnable, 28, 28) |
      __gen_uint(values->ScissorRectangleEnable, 29, 29) |
      __gen_uint(values->DepthBufferClearEnable, 30, 30) |
      __gen_uint(values->StencilBufferClearEnable, 31, 31);

   dw[2] =
      __gen_uint(values->ClearRectangleXMin, 0, 15) |
      __gen_uint(values->ClearRectangleYMin, 16, 31);

   dw[3] =
      __gen_uint(values->ClearRectangleXMax, 0, 15) |
      __gen_uint(values->ClearRectangleYMax, 16, 31);

   dw[4] =
      __gen_uint(values->SampleMask, 0, 15);
}

#define GFX11_GPGPU_WALKER_length             15
#define GFX11_GPGPU_WALKER_length_bias         2
#define GFX11_GPGPU_WALKER_header               \
   .DWordLength                         =     13,  \
   .SubOpcode                           =      5,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_GPGPU_WALKER {
   uint32_t                             DWordLength;
   bool                                 PredicateEnable;
   bool                                 IndirectParameterEnable;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint64_t                             IndirectDataStartAddress;
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
   uint32_t                             ThreadGroupIDStartingResumeZ;
   uint32_t                             ThreadGroupIDZDimension;
   uint32_t                             RightExecutionMask;
   uint32_t                             BottomExecutionMask;
};

static inline __attribute__((always_inline)) void
GFX11_GPGPU_WALKER_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_GPGPU_WALKER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->PredicateEnable, 8, 8) |
      __gen_uint(values->IndirectParameterEnable, 10, 10) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16);

   dw[3] =
      __gen_offset(values->IndirectDataStartAddress, 6, 31);

   dw[4] =
      __gen_uint(values->ThreadWidthCounterMaximum, 0, 5) |
      __gen_uint(values->ThreadHeightCounterMaximum, 8, 13) |
      __gen_uint(values->ThreadDepthCounterMaximum, 16, 21) |
      __gen_uint(values->SIMDSize, 30, 31);

   dw[5] =
      __gen_uint(values->ThreadGroupIDStartingX, 0, 31);

   dw[6] = 0;

   dw[7] =
      __gen_uint(values->ThreadGroupIDXDimension, 0, 31);

   dw[8] =
      __gen_uint(values->ThreadGroupIDStartingY, 0, 31);

   dw[9] = 0;

   dw[10] =
      __gen_uint(values->ThreadGroupIDYDimension, 0, 31);

   dw[11] =
      __gen_uint(values->ThreadGroupIDStartingResumeZ, 0, 31);

   dw[12] =
      __gen_uint(values->ThreadGroupIDZDimension, 0, 31);

   dw[13] =
      __gen_uint(values->RightExecutionMask, 0, 31);

   dw[14] =
      __gen_uint(values->BottomExecutionMask, 0, 31);
}

#define GFX11_MEDIA_CURBE_LOAD_length          4
#define GFX11_MEDIA_CURBE_LOAD_length_bias      2
#define GFX11_MEDIA_CURBE_LOAD_header           \
   .DWordLength                         =      2,  \
   .SubOpcode                           =      1,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_CURBE_LOAD {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             CURBETotalDataLength;
   uint32_t                             CURBEDataStartAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_CURBE_LOAD_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_MEDIA_CURBE_LOAD * restrict values)
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

#define GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD_length      4
#define GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD_length_bias      2
#define GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD_header\
   .DWordLength                         =      2,  \
   .SubOpcode                           =      2,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorTotalLength;
   uint64_t                             InterfaceDescriptorDataStartAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD_pack(__attribute__((unused)) __gen_user_data *data,
                                           __attribute__((unused)) void * restrict dst,
                                           __attribute__((unused)) const struct GFX11_MEDIA_INTERFACE_DESCRIPTOR_LOAD * restrict values)
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

#define GFX11_MEDIA_OBJECT_length_bias         2
#define GFX11_MEDIA_OBJECT_header               \
   .DWordLength                         =      4,  \
   .MediaCommandSubOpcode               =      0,  \
   .MediaCommandOpcode                  =      1,  \
   .MediaCommandPipeline                =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_OBJECT {
   uint32_t                             DWordLength;
   uint32_t                             MediaCommandSubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             MediaCommandPipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             SubSliceDestinationSelect;
#define Subslice3                                3
#define SubSlice2                                2
#define SubSlice1                                1
#define SubSlice0                                0
   uint32_t                             SliceDestinationSelect;
#define Slice0                                   0
#define Slice1                                   1
#define Slice2                                   2
   uint32_t                             ForceDestination;
   uint32_t                             ThreadSynchronization;
#define Nothreadsynchronization                  0
#define Threaddispatchissynchronizedbythespawnrootthreadmessage 1
   uint32_t                             SliceDestinationSelectMSBs;
   bool                                 ChildrenPresent;
   __gen_address_type                   IndirectDataStartAddress;
   uint32_t                             XPosition;
   uint32_t                             YPosition;
   uint32_t                             BlockColor;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_OBJECT_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_MEDIA_OBJECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 14) |
      __gen_uint(values->MediaCommandSubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->MediaCommandPipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->SubSliceDestinationSelect, 17, 18) |
      __gen_uint(values->SliceDestinationSelect, 19, 20) |
      __gen_uint(values->ForceDestination, 22, 22) |
      __gen_uint(values->ThreadSynchronization, 24, 24) |
      __gen_uint(values->SliceDestinationSelectMSBs, 25, 26) |
      __gen_uint(values->ChildrenPresent, 31, 31);

   dw[3] = __gen_address(data, &dw[3], values->IndirectDataStartAddress, 0, 0, 31);

   dw[4] =
      __gen_uint(values->XPosition, 0, 8) |
      __gen_uint(values->YPosition, 16, 24);

   dw[5] =
      __gen_uint(values->BlockColor, 16, 23);
}

#define GFX11_MEDIA_OBJECT_GRPID_length_bias      2
#define GFX11_MEDIA_OBJECT_GRPID_header         \
   .DWordLength                         =      5,  \
   .MediaCommandSubOpcode               =      6,  \
   .MediaCommandOpcode                  =      1,  \
   .MediaCommandPipeline                =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_OBJECT_GRPID {
   uint32_t                             DWordLength;
   uint32_t                             MediaCommandSubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             MediaCommandPipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             EndofThreadGroup;
   __gen_address_type                   IndirectDataStartAddress;
   uint32_t                             XPosition;
   uint32_t                             YPosition;
   uint32_t                             BlockColor;
   uint32_t                             GroupID;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_OBJECT_GRPID_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_MEDIA_OBJECT_GRPID * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->MediaCommandSubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->MediaCommandPipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->EndofThreadGroup, 23, 23);

   dw[3] = __gen_address(data, &dw[3], values->IndirectDataStartAddress, 0, 0, 31);

   dw[4] =
      __gen_uint(values->XPosition, 0, 8) |
      __gen_uint(values->YPosition, 16, 24);

   dw[5] =
      __gen_uint(values->BlockColor, 16, 23);

   dw[6] =
      __gen_uint(values->GroupID, 0, 31);
}

#define GFX11_MEDIA_OBJECT_PRT_length         16
#define GFX11_MEDIA_OBJECT_PRT_length_bias      2
#define GFX11_MEDIA_OBJECT_PRT_header           \
   .DWordLength                         =     14,  \
   .SubOpcode                           =      2,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_OBJECT_PRT {
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
GFX11_MEDIA_OBJECT_PRT_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_MEDIA_OBJECT_PRT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 14) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5);

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

#define GFX11_MEDIA_OBJECT_WALKER_length_bias      2
#define GFX11_MEDIA_OBJECT_WALKER_header        \
   .DWordLength                         =     15,  \
   .SubOpcode                           =      3,  \
   .MediaCommandOpcode                  =      1,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_OBJECT_WALKER {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   uint32_t                             IndirectDataLength;
   uint32_t                             MaskedDispatch;
   uint32_t                             ThreadSynchronization;
#define Nothreadsynchronization                  0
#define Threaddispatchissynchronizedbythespawnrootthreadmessage 1
   uint32_t                             IndirectDataStartAddress;
   uint32_t                             GroupIDLoopSelect;
#define No_Groups                                0
#define Color_Groups                             1
#define InnerLocal_Groups                        2
#define MidLocal_Groups                          3
#define OuterLocal_Groups                        4
#define InnerGlobal_Groups                       5
   int32_t                              MidLoopUnitX;
   int32_t                              LocalMidLoopUnitY;
   uint32_t                             MiddleLoopExtraSteps;
   uint32_t                             ColorCountMinusOne;
   uint32_t                             LocalLoopExecCount;
   uint32_t                             GlobalLoopExecCount;
   uint32_t                             BlockResolutionX;
   uint32_t                             BlockResolutionY;
   uint32_t                             LocalStartX;
   uint32_t                             LocalStartY;
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
GFX11_MEDIA_OBJECT_WALKER_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_MEDIA_OBJECT_WALKER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 14) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->InterfaceDescriptorOffset, 0, 5);

   dw[2] =
      __gen_uint(values->IndirectDataLength, 0, 16) |
      __gen_uint(values->MaskedDispatch, 22, 23) |
      __gen_uint(values->ThreadSynchronization, 24, 24);

   dw[3] =
      __gen_uint(values->IndirectDataStartAddress, 0, 31);

   dw[4] = 0;

   dw[5] =
      __gen_uint(values->GroupIDLoopSelect, 8, 31);

   dw[6] =
      __gen_sint(values->MidLoopUnitX, 8, 9) |
      __gen_sint(values->LocalMidLoopUnitY, 12, 13) |
      __gen_uint(values->MiddleLoopExtraSteps, 16, 20) |
      __gen_uint(values->ColorCountMinusOne, 24, 31);

   dw[7] =
      __gen_uint(values->LocalLoopExecCount, 0, 11) |
      __gen_uint(values->GlobalLoopExecCount, 16, 27);

   dw[8] =
      __gen_uint(values->BlockResolutionX, 0, 10) |
      __gen_uint(values->BlockResolutionY, 16, 26);

   dw[9] =
      __gen_uint(values->LocalStartX, 0, 10) |
      __gen_uint(values->LocalStartY, 16, 26);

   dw[10] = 0;

   dw[11] =
      __gen_sint(values->LocalOuterLoopStrideX, 0, 11) |
      __gen_sint(values->LocalOuterLoopStrideY, 16, 27);

   dw[12] =
      __gen_sint(values->LocalInnerLoopUnitX, 0, 11) |
      __gen_sint(values->LocalInnerLoopUnitY, 16, 27);

   dw[13] =
      __gen_uint(values->GlobalResolutionX, 0, 10) |
      __gen_uint(values->GlobalResolutionY, 16, 26);

   dw[14] =
      __gen_sint(values->GlobalStartX, 0, 11) |
      __gen_sint(values->GlobalStartY, 16, 27);

   dw[15] =
      __gen_sint(values->GlobalOuterLoopStrideX, 0, 11) |
      __gen_sint(values->GlobalOuterLoopStrideY, 16, 27);

   dw[16] =
      __gen_sint(values->GlobalInnerLoopUnitX, 0, 11) |
      __gen_sint(values->GlobalInnerLoopUnitY, 16, 27);
}

#define GFX11_MEDIA_STATE_FLUSH_length         2
#define GFX11_MEDIA_STATE_FLUSH_length_bias      2
#define GFX11_MEDIA_STATE_FLUSH_header          \
   .DWordLength                         =      0,  \
   .SubOpcode                           =      4,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_STATE_FLUSH {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             InterfaceDescriptorOffset;
   bool                                 FlushtoGO;
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_STATE_FLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_MEDIA_STATE_FLUSH * restrict values)
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
      __gen_uint(values->FlushtoGO, 7, 7);
}

#define GFX11_MEDIA_VFE_STATE_length           9
#define GFX11_MEDIA_VFE_STATE_length_bias      2
#define GFX11_MEDIA_VFE_STATE_header            \
   .DWordLength                         =      7,  \
   .SubOpcode                           =      0,  \
   .MediaCommandOpcode                  =      0,  \
   .Pipeline                            =      2,  \
   .CommandType                         =      3

struct GFX11_MEDIA_VFE_STATE {
   uint32_t                             DWordLength;
   uint32_t                             SubOpcode;
   uint32_t                             MediaCommandOpcode;
   uint32_t                             Pipeline;
   uint32_t                             CommandType;
   uint32_t                             PerThreadScratchSpace;
   uint32_t                             StackSize;
   __gen_address_type                   ScratchSpaceBasePointer;
   uint32_t                             DispatchLoadBalance;
#define ColorLSB                                 1
#define LeastLoaded                              0
   uint32_t                             NumberofURBEntries;
   uint32_t                             MaximumNumberofThreads;
   uint32_t                             MaximumNumberofDualSubslices;
   uint32_t                             CURBEAllocationSize;
   uint32_t                             URBEntryAllocationSize;
};

static inline __attribute__((always_inline)) void
GFX11_MEDIA_VFE_STATE_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_MEDIA_VFE_STATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 15) |
      __gen_uint(values->SubOpcode, 16, 23) |
      __gen_uint(values->MediaCommandOpcode, 24, 26) |
      __gen_uint(values->Pipeline, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->PerThreadScratchSpace, 0, 3) |
      __gen_uint(values->StackSize, 4, 7);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->ScratchSpaceBasePointer, v1, 10, 47);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->DispatchLoadBalance, 2, 2) |
      __gen_uint(values->NumberofURBEntries, 8, 15) |
      __gen_uint(values->MaximumNumberofThreads, 16, 31);

   dw[4] =
      __gen_uint(values->MaximumNumberofDualSubslices, 0, 7);

   dw[5] =
      __gen_uint(values->CURBEAllocationSize, 0, 15) |
      __gen_uint(values->URBEntryAllocationSize, 16, 31);

   dw[6] = 0;

   dw[7] = 0;

   dw[8] = 0;
}

#define GFX11_MI_ARB_CHECK_length              1
#define GFX11_MI_ARB_CHECK_length_bias         1
#define GFX11_MI_ARB_CHECK_header               \
   .MICommandOpcode                     =      5,  \
   .CommandType                         =      0

struct GFX11_MI_ARB_CHECK {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_ARB_CHECK_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_MI_ARB_CHECK * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_ARB_ON_OFF_length             1
#define GFX11_MI_ARB_ON_OFF_length_bias        1
#define GFX11_MI_ARB_ON_OFF_header              \
   .ArbitrationEnable                   =      1,  \
   .MICommandOpcode                     =      8,  \
   .CommandType                         =      0

struct GFX11_MI_ARB_ON_OFF {
   bool                                 ArbitrationEnable;
   bool                                 AllowLiteRestore;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_ARB_ON_OFF_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_MI_ARB_ON_OFF * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ArbitrationEnable, 0, 0) |
      __gen_uint(values->AllowLiteRestore, 1, 1) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_ATOMIC_length                 3
#define GFX11_MI_ATOMIC_length_bias            2
#define GFX11_MI_ATOMIC_header                  \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     47,  \
   .CommandType                         =      0

struct GFX11_MI_ATOMIC {
   uint32_t                             DWordLength;
   enum GFX11_Atomic_OPCODE             ATOMICOPCODE;
   bool                                 ReturnDataControl;
   bool                                 CSSTALL;
   bool                                 InlineData;
   uint32_t                             DataSize;
#define MI_ATOMIC_DWORD                          0
#define MI_ATOMIC_QWORD                          1
#define MI_ATOMIC_OCTWORD                        2
#define MI_ATOMIC_RESERVED                       3
   bool                                 PostSyncOperation;
   uint32_t                             MemoryType;
#define PerProcessGraphicsAddress                0
#define GlobalGraphicsAddress                    1
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   __gen_address_type                   MemoryAddress;
   uint32_t                             Operand1DataDword0;
   uint32_t                             Operand2DataDword0;
   uint32_t                             Operand1DataDword1;
   uint32_t                             Operand2DataDword1;
   uint32_t                             Operand1DataDword2;
   uint32_t                             Operand2DataDword2;
   uint32_t                             Operand1DataDword3;
   uint32_t                             Operand2DataDword3;
};

static inline __attribute__((always_inline)) void
GFX11_MI_ATOMIC_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX11_MI_ATOMIC * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->ATOMICOPCODE, 8, 15) |
      __gen_uint(values->ReturnDataControl, 16, 16) |
      __gen_uint(values->CSSTALL, 17, 17) |
      __gen_uint(values->InlineData, 18, 18) |
      __gen_uint(values->DataSize, 19, 20) |
      __gen_uint(values->PostSyncOperation, 21, 21) |
      __gen_uint(values->MemoryType, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->MemoryAddress, 0, 2, 47);
   dw[1] = v1_address;
   dw[2] = v1_address >> 32;
}

#define GFX11_MI_BATCH_BUFFER_END_length       1
#define GFX11_MI_BATCH_BUFFER_END_length_bias      1
#define GFX11_MI_BATCH_BUFFER_END_header        \
   .MICommandOpcode                     =     10,  \
   .CommandType                         =      0

struct GFX11_MI_BATCH_BUFFER_END {
   bool                                 EndContext;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_BATCH_BUFFER_END_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_MI_BATCH_BUFFER_END * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->EndContext, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_BATCH_BUFFER_START_length      3
#define GFX11_MI_BATCH_BUFFER_START_length_bias      2
#define GFX11_MI_BATCH_BUFFER_START_header      \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     49,  \
   .CommandType                         =      0

struct GFX11_MI_BATCH_BUFFER_START {
   uint32_t                             DWordLength;
   uint32_t                             AddressSpaceIndicator;
#define ASI_GGTT                                 0
#define ASI_PPGTT                                1
   bool                                 ResourceStreamerEnable;
   bool                                 PredicationEnable;
   uint32_t                             SecondLevelBatchBuffer;
#define Firstlevelbatch                          0
#define Secondlevelbatch                         1
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   __gen_address_type                   BatchBufferStartAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_BATCH_BUFFER_START_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_MI_BATCH_BUFFER_START * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AddressSpaceIndicator, 8, 8) |
      __gen_uint(values->ResourceStreamerEnable, 10, 10) |
      __gen_uint(values->PredicationEnable, 15, 15) |
      __gen_uint(values->SecondLevelBatchBuffer, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->BatchBufferStartAddress, 0, 2, 63);
   dw[1] = v1_address;
   dw[2] = v1_address >> 32;
}

#define GFX11_MI_CLFLUSH_length_bias           2
#define GFX11_MI_CLFLUSH_header                 \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     39,  \
   .CommandType                         =      0

struct GFX11_MI_CLFLUSH {
   uint32_t                             DWordLength;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             StartingCachelineOffset;
   __gen_address_type                   PageBaseAddress;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_MI_CLFLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_MI_CLFLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 9) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->StartingCachelineOffset, 6, 11);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->PageBaseAddress, v1, 12, 47);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);
}

#define GFX11_MI_CONDITIONAL_BATCH_BUFFER_END_length      4
#define GFX11_MI_CONDITIONAL_BATCH_BUFFER_END_length_bias      2
#define GFX11_MI_CONDITIONAL_BATCH_BUFFER_END_header\
   .DWordLength                         =      2,  \
   .CompareSemaphore                    =      0,  \
   .MICommandOpcode                     =     54,  \
   .CommandType                         =      0

struct GFX11_MI_CONDITIONAL_BATCH_BUFFER_END {
   uint32_t                             DWordLength;
   uint32_t                             CompareMaskMode;
#define CompareMaskModeDisabled                  0
#define CompareMaskModeEnabled                   1
   uint32_t                             CompareSemaphore;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             CompareDataDword;
   __gen_address_type                   CompareAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_CONDITIONAL_BATCH_BUFFER_END_pack(__attribute__((unused)) __gen_user_data *data,
                                           __attribute__((unused)) void * restrict dst,
                                           __attribute__((unused)) const struct GFX11_MI_CONDITIONAL_BATCH_BUFFER_END * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->CompareMaskMode, 19, 19) |
      __gen_uint(values->CompareSemaphore, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->CompareDataDword, 0, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->CompareAddress, 0, 3, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;
}

#define GFX11_MI_COPY_MEM_MEM_length           5
#define GFX11_MI_COPY_MEM_MEM_length_bias      2
#define GFX11_MI_COPY_MEM_MEM_header            \
   .DWordLength                         =      3,  \
   .MICommandOpcode                     =     46,  \
   .CommandType                         =      0

struct GFX11_MI_COPY_MEM_MEM {
   uint32_t                             DWordLength;
   bool                                 UseGlobalGTTDestination;
   bool                                 UseGlobalGTTSource;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   __gen_address_type                   DestinationMemoryAddress;
   __gen_address_type                   SourceMemoryAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_COPY_MEM_MEM_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_MI_COPY_MEM_MEM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->UseGlobalGTTDestination, 21, 21) |
      __gen_uint(values->UseGlobalGTTSource, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->DestinationMemoryAddress, 0, 2, 63);
   dw[1] = v1_address;
   dw[2] = v1_address >> 32;

   const uint64_t v3_address =
      __gen_address(data, &dw[3], values->SourceMemoryAddress, 0, 2, 63);
   dw[3] = v3_address;
   dw[4] = v3_address >> 32;
}

#define GFX11_MI_DISPLAY_FLIP_length           3
#define GFX11_MI_DISPLAY_FLIP_length_bias      2
#define GFX11_MI_DISPLAY_FLIP_header            \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     20,  \
   .CommandType                         =      0

struct GFX11_MI_DISPLAY_FLIP {
   uint32_t                             DWordLength;
   uint32_t                             DisplayPlaneSelect;
#define DisplayPlane1                            0
#define DisplayPlane2                            1
#define DisplayPlane3                            2
#define DisplayPlane4                            4
#define DisplayPlane5                            5
#define DisplayPlane6                            6
#define DisplayPlane7                            7
#define DisplayPlane8                            8
#define DisplayPlane9                            9
#define DisplayPlane10                           10
#define DisplayPlane11                           11
#define DisplayPlane12                           12
#define DisplayPlane13                           13
#define DisplayPlane14                           14
#define DisplayPlane15                           15
#define DisplayPlane16                           16
#define DisplayPlane17                           17
#define DisplayPlane18                           18
#define DisplayPlane19                           19
#define DisplayPlane20                           20
#define DisplayPlane21                           21
#define DisplayPlane22                           22
#define DisplayPlane23                           23
#define DisplayPlane24                           24
#define DisplayPlane25                           25
#define DisplayPlane26                           26
#define DisplayPlane27                           27
#define DisplayPlane28                           28
#define DisplayPlane29                           29
#define DisplayPlane30                           30
#define DisplayPlane31                           31
#define DisplayPlane32                           32
   bool                                 AsyncFlipIndicator;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             TileParameter;
   uint32_t                             DisplayBufferPitch;
   bool                                 Stereoscopic3DMode;
   uint32_t                             FlipType;
#define SyncFlip                                 0
#define AsyncFlip                                1
#define Stereo3DFlip                             2
   uint32_t                             VRRMasterFlip;
   __gen_address_type                   DisplayBufferBaseAddress;
   __gen_address_type                   LeftEyeDisplayBufferBaseAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_DISPLAY_FLIP_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_MI_DISPLAY_FLIP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->DisplayPlaneSelect, 8, 13) |
      __gen_uint(values->AsyncFlipIndicator, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->TileParameter, 0, 2) |
      __gen_uint(values->DisplayBufferPitch, 6, 15) |
      __gen_uint(values->Stereoscopic3DMode, 31, 31);

   const uint32_t v2 =
      __gen_uint(values->FlipType, 0, 1) |
      __gen_uint(values->VRRMasterFlip, 11, 11);
   dw[2] = __gen_address(data, &dw[2], values->DisplayBufferBaseAddress, v2, 12, 31);
}

#define GFX11_MI_FORCE_WAKEUP_length           2
#define GFX11_MI_FORCE_WAKEUP_length_bias      2
#define GFX11_MI_FORCE_WAKEUP_header            \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     29,  \
   .CommandType                         =      0

struct GFX11_MI_FORCE_WAKEUP {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             ForceMediaSlice0Awake;
   uint32_t                             ForceRenderAwake;
   uint32_t                             ForceMediaSlice1Awake;
   uint32_t                             ForceMediaSlice2Awake;
   uint32_t                             ForceMediaSlice3Awake;
   uint32_t                             MaskBits;
};

static inline __attribute__((always_inline)) void
GFX11_MI_FORCE_WAKEUP_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_MI_FORCE_WAKEUP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->ForceMediaSlice0Awake, 0, 0) |
      __gen_uint(values->ForceRenderAwake, 1, 1) |
      __gen_uint(values->ForceMediaSlice1Awake, 2, 2) |
      __gen_uint(values->ForceMediaSlice2Awake, 3, 3) |
      __gen_uint(values->ForceMediaSlice3Awake, 4, 4) |
      __gen_uint(values->MaskBits, 16, 31);
}

#define GFX11_MI_LOAD_REGISTER_IMM_length      3
#define GFX11_MI_LOAD_REGISTER_IMM_length_bias      2
#define GFX11_MI_LOAD_REGISTER_IMM_header       \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     34,  \
   .CommandType                         =      0

struct GFX11_MI_LOAD_REGISTER_IMM {
   uint32_t                             DWordLength;
   uint32_t                             ByteWriteDisables;
   uint32_t                             AddCSMMIOStartOffset;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterOffset;
   uint32_t                             DataDWord;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_MI_LOAD_REGISTER_IMM_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_MI_LOAD_REGISTER_IMM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->ByteWriteDisables, 8, 11) |
      __gen_uint(values->AddCSMMIOStartOffset, 19, 19) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterOffset, 2, 22);

   dw[2] =
      __gen_uint(values->DataDWord, 0, 31);
}

#define GFX11_MI_LOAD_REGISTER_MEM_length      4
#define GFX11_MI_LOAD_REGISTER_MEM_length_bias      2
#define GFX11_MI_LOAD_REGISTER_MEM_header       \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     41,  \
   .CommandType                         =      0

struct GFX11_MI_LOAD_REGISTER_MEM {
   uint32_t                             DWordLength;
   uint32_t                             AddCSMMIOStartOffset;
   uint32_t                             AddLoopVariable;
   bool                                 AsyncModeEnable;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterAddress;
   __gen_address_type                   MemoryAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_LOAD_REGISTER_MEM_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_MI_LOAD_REGISTER_MEM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AddCSMMIOStartOffset, 19, 19) |
      __gen_uint(values->AddLoopVariable, 20, 20) |
      __gen_uint(values->AsyncModeEnable, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterAddress, 2, 22);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->MemoryAddress, 0, 2, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;
}

#define GFX11_MI_LOAD_REGISTER_REG_length      3
#define GFX11_MI_LOAD_REGISTER_REG_length_bias      2
#define GFX11_MI_LOAD_REGISTER_REG_header       \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     42,  \
   .CommandType                         =      0

struct GFX11_MI_LOAD_REGISTER_REG {
   uint32_t                             DWordLength;
   uint32_t                             AddCSMMIOStartOffsetSource;
   uint32_t                             AddCSMMIOStartOffsetDestination;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             SourceRegisterAddress;
   uint64_t                             DestinationRegisterAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_LOAD_REGISTER_REG_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_MI_LOAD_REGISTER_REG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AddCSMMIOStartOffsetSource, 18, 18) |
      __gen_uint(values->AddCSMMIOStartOffsetDestination, 19, 19) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->SourceRegisterAddress, 2, 22);

   dw[2] =
      __gen_offset(values->DestinationRegisterAddress, 2, 22);
}

#define GFX11_MI_LOAD_SCAN_LINES_EXCL_length      2
#define GFX11_MI_LOAD_SCAN_LINES_EXCL_length_bias      2
#define GFX11_MI_LOAD_SCAN_LINES_EXCL_header    \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     19,  \
   .CommandType                         =      0

struct GFX11_MI_LOAD_SCAN_LINES_EXCL {
   uint32_t                             DWordLength;
   uint32_t                             DisplayPlaneSelect;
#define DisplayPlaneA                            0
#define DisplayPlaneB                            1
#define DisplayPlaneC                            4
#define DisplayPlaneD                            5
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             EndScanLineNumber;
   uint32_t                             StartScanLineNumber;
};

static inline __attribute__((always_inline)) void
GFX11_MI_LOAD_SCAN_LINES_EXCL_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_MI_LOAD_SCAN_LINES_EXCL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 5) |
      __gen_uint(values->DisplayPlaneSelect, 19, 21) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->EndScanLineNumber, 0, 12) |
      __gen_uint(values->StartScanLineNumber, 16, 28);
}

#define GFX11_MI_LOAD_SCAN_LINES_INCL_length      2
#define GFX11_MI_LOAD_SCAN_LINES_INCL_length_bias      2
#define GFX11_MI_LOAD_SCAN_LINES_INCL_header    \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     18,  \
   .CommandType                         =      0

struct GFX11_MI_LOAD_SCAN_LINES_INCL {
   uint32_t                             DWordLength;
   uint32_t                             ScanLineEventDoneForward;
   uint32_t                             DisplayPlaneSelect;
#define DisplayPlane1A                           0
#define DisplayPlane1B                           1
#define DisplayPlane1C                           4
#define DisplayPlane1D                           5
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             EndScanLineNumber;
   uint32_t                             StartScanLineNumber;
};

static inline __attribute__((always_inline)) void
GFX11_MI_LOAD_SCAN_LINES_INCL_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_MI_LOAD_SCAN_LINES_INCL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 5) |
      __gen_uint(values->ScanLineEventDoneForward, 17, 18) |
      __gen_uint(values->DisplayPlaneSelect, 19, 21) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->EndScanLineNumber, 0, 12) |
      __gen_uint(values->StartScanLineNumber, 16, 28);
}

#define GFX11_MI_MATH_length_bias              2
#define GFX11_MI_MATH_header                    \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     26,  \
   .CommandType                         =      0

struct GFX11_MI_MATH {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   /* variable length fields follow */
};

static inline __attribute__((always_inline)) void
GFX11_MI_MATH_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX11_MI_MATH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_NOOP_length                   1
#define GFX11_MI_NOOP_length_bias              1
#define GFX11_MI_NOOP_header                    \
   .MICommandOpcode                     =      0,  \
   .CommandType                         =      0

struct GFX11_MI_NOOP {
   uint32_t                             IdentificationNumber;
   bool                                 IdentificationNumberRegisterWriteEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_NOOP_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX11_MI_NOOP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->IdentificationNumber, 0, 21) |
      __gen_uint(values->IdentificationNumberRegisterWriteEnable, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_PREDICATE_length              1
#define GFX11_MI_PREDICATE_length_bias         1
#define GFX11_MI_PREDICATE_header               \
   .MICommandOpcode                     =     12,  \
   .CommandType                         =      0

struct GFX11_MI_PREDICATE {
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
GFX11_MI_PREDICATE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_MI_PREDICATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->CompareOperation, 0, 1) |
      __gen_uint(values->CombineOperation, 3, 4) |
      __gen_uint(values->LoadOperation, 6, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_REPORT_HEAD_length            1
#define GFX11_MI_REPORT_HEAD_length_bias       1
#define GFX11_MI_REPORT_HEAD_header             \
   .MICommandOpcode                     =      7,  \
   .CommandType                         =      0

struct GFX11_MI_REPORT_HEAD {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_REPORT_HEAD_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_MI_REPORT_HEAD * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_REPORT_PERF_COUNT_length      4
#define GFX11_MI_REPORT_PERF_COUNT_length_bias      2
#define GFX11_MI_REPORT_PERF_COUNT_header       \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     40,  \
   .CommandType                         =      0

struct GFX11_MI_REPORT_PERF_COUNT {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   bool                                 UseGlobalGTT;
   uint32_t                             CoreModeEnable;
   __gen_address_type                   MemoryAddress;
   uint32_t                             ReportID;
};

static inline __attribute__((always_inline)) void
GFX11_MI_REPORT_PERF_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_MI_REPORT_PERF_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 5) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->UseGlobalGTT, 0, 0) |
      __gen_uint(values->CoreModeEnable, 4, 4);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->MemoryAddress, v1, 6, 63);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->ReportID, 0, 31);
}

#define GFX11_MI_RS_CONTEXT_length             1
#define GFX11_MI_RS_CONTEXT_length_bias        1
#define GFX11_MI_RS_CONTEXT_header              \
   .MICommandOpcode                     =     15,  \
   .CommandType                         =      0

struct GFX11_MI_RS_CONTEXT {
   uint32_t                             ResourceStreamerSave;
#define RS_Restore                               0
#define RS_Save                                  1
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_RS_CONTEXT_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_MI_RS_CONTEXT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ResourceStreamerSave, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_RS_CONTROL_length             1
#define GFX11_MI_RS_CONTROL_length_bias        1
#define GFX11_MI_RS_CONTROL_header              \
   .MICommandOpcode                     =      6,  \
   .CommandType                         =      0

struct GFX11_MI_RS_CONTROL {
   uint32_t                             ResourceStreamerControl;
#define RS_Stop                                  0
#define RS_Start                                 1
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_RS_CONTROL_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_MI_RS_CONTROL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ResourceStreamerControl, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_RS_STORE_DATA_IMM_length      4
#define GFX11_MI_RS_STORE_DATA_IMM_length_bias      2
#define GFX11_MI_RS_STORE_DATA_IMM_header       \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     43,  \
   .CommandType                         =      0

struct GFX11_MI_RS_STORE_DATA_IMM {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             CoreModeEnable;
   __gen_address_type                   DestinationAddress;
   uint32_t                             DataDWord0;
};

static inline __attribute__((always_inline)) void
GFX11_MI_RS_STORE_DATA_IMM_pack(__attribute__((unused)) __gen_user_data *data,
                                __attribute__((unused)) void * restrict dst,
                                __attribute__((unused)) const struct GFX11_MI_RS_STORE_DATA_IMM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->CoreModeEnable, 0, 0);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->DestinationAddress, v1, 2, 63);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->DataDWord0, 0, 31);
}

#define GFX11_MI_SEMAPHORE_SIGNAL_length       2
#define GFX11_MI_SEMAPHORE_SIGNAL_length_bias      2
#define GFX11_MI_SEMAPHORE_SIGNAL_header        \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     27,  \
   .CommandType                         =      0

struct GFX11_MI_SEMAPHORE_SIGNAL {
   uint32_t                             DWordLength;
   uint32_t                             TargetEngineSelect;
#define RCS                                      0
#define VCS0                                     1
#define BCS                                      2
#define VECS                                     3
#define VCS1                                     4
#define VCS2                                     6
#define VCS3                                     7
#define VCS4                                     8
#define VCS5                                     9
#define VCS6                                     10
#define VCS7                                     11
#define VECS1                                    12
#define VECS2                                    13
#define VECS3                                    14
   bool                                 PostSyncOperation;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             TargetContextID;
};

static inline __attribute__((always_inline)) void
GFX11_MI_SEMAPHORE_SIGNAL_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_MI_SEMAPHORE_SIGNAL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->TargetEngineSelect, 15, 18) |
      __gen_uint(values->PostSyncOperation, 21, 21) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->TargetContextID, 0, 31);
}

#define GFX11_MI_SEMAPHORE_WAIT_length         4
#define GFX11_MI_SEMAPHORE_WAIT_length_bias      2
#define GFX11_MI_SEMAPHORE_WAIT_header          \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     28,  \
   .CommandType                         =      0

struct GFX11_MI_SEMAPHORE_WAIT {
   uint32_t                             DWordLength;
   uint32_t                             CompareOperation;
#define COMPARE_SAD_GREATER_THAN_SDD             0
#define COMPARE_SAD_GREATER_THAN_OR_EQUAL_SDD    1
#define COMPARE_SAD_LESS_THAN_SDD                2
#define COMPARE_SAD_LESS_THAN_OR_EQUAL_SDD       3
#define COMPARE_SAD_EQUAL_SDD                    4
#define COMPARE_SAD_NOT_EQUAL_SDD                5
   uint32_t                             WaitMode;
#define PollingMode                              1
#define SignalMode                               0
   bool                                 RegisterPollMode;
   uint32_t                             MemoryType;
#define PerProcessGraphicsAddress                0
#define GlobalGraphicsAddress                    1
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             SemaphoreDataDword;
   __gen_address_type                   SemaphoreAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_SEMAPHORE_WAIT_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_MI_SEMAPHORE_WAIT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->CompareOperation, 12, 14) |
      __gen_uint(values->WaitMode, 15, 15) |
      __gen_uint(values->RegisterPollMode, 16, 16) |
      __gen_uint(values->MemoryType, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->SemaphoreDataDword, 0, 31);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->SemaphoreAddress, 0, 2, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;
}

#define GFX11_MI_SET_CONTEXT_length            2
#define GFX11_MI_SET_CONTEXT_length_bias       2
#define GFX11_MI_SET_CONTEXT_header             \
   .DWordLength                         =      0,  \
   .MICommandOpcode                     =     24,  \
   .CommandType                         =      0

struct GFX11_MI_SET_CONTEXT {
   uint32_t                             DWordLength;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             RestoreInhibit;
   uint32_t                             ForceRestore;
   bool                                 ResourceStreamerStateRestoreEnable;
   bool                                 ResourceStreamerStateSaveEnable;
   bool                                 CoreModeEnable;
   uint32_t                             ReservedMustbe1;
   __gen_address_type                   LogicalContextAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_SET_CONTEXT_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_MI_SET_CONTEXT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint32_t v1 =
      __gen_uint(values->RestoreInhibit, 0, 0) |
      __gen_uint(values->ForceRestore, 1, 1) |
      __gen_uint(values->ResourceStreamerStateRestoreEnable, 2, 2) |
      __gen_uint(values->ResourceStreamerStateSaveEnable, 3, 3) |
      __gen_uint(values->CoreModeEnable, 4, 4) |
      __gen_uint(values->ReservedMustbe1, 8, 8);
   dw[1] = __gen_address(data, &dw[1], values->LogicalContextAddress, v1, 12, 31);
}

#define GFX11_MI_SET_PREDICATE_length          1
#define GFX11_MI_SET_PREDICATE_length_bias      1
#define GFX11_MI_SET_PREDICATE_header           \
   .MICommandOpcode                     =      1,  \
   .CommandType                         =      0

struct GFX11_MI_SET_PREDICATE {
   uint32_t                             PREDICATEENABLE;
#define NOOPNever                                0
#define NOOPonResult2clear                       1
#define NOOPonResult2set                         2
#define NOOPonResultclear                        3
#define NOOPonResultset                          4
#define NOOPAlways                               15
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_SET_PREDICATE_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_MI_SET_PREDICATE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PREDICATEENABLE, 0, 3) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_STORE_DATA_IMM_length         4
#define GFX11_MI_STORE_DATA_IMM_length_bias      2
#define GFX11_MI_STORE_DATA_IMM_header          \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     32,  \
   .CommandType                         =      0

struct GFX11_MI_STORE_DATA_IMM {
   uint32_t                             DWordLength;
   uint32_t                             StoreQword;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             CoreModeEnable;
   __gen_address_type                   Address;
   uint64_t                             ImmediateData;
};

static inline __attribute__((always_inline)) void
GFX11_MI_STORE_DATA_IMM_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_MI_STORE_DATA_IMM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 9) |
      __gen_uint(values->StoreQword, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->CoreModeEnable, 0, 0);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->Address, v1, 2, 47);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   const uint64_t v3 =
      __gen_uint(values->ImmediateData, 0, 63);
   dw[3] = v3;
   dw[4] = v3 >> 32;
}

#define GFX11_MI_STORE_DATA_INDEX_length       3
#define GFX11_MI_STORE_DATA_INDEX_length_bias      2
#define GFX11_MI_STORE_DATA_INDEX_header        \
   .DWordLength                         =      1,  \
   .MICommandOpcode                     =     33,  \
   .CommandType                         =      0

struct GFX11_MI_STORE_DATA_INDEX {
   uint32_t                             DWordLength;
   uint32_t                             UsePerProcessHardwareStatusPage;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint32_t                             Offset;
   uint32_t                             DataDWord0;
   uint32_t                             DataDWord1;
};

static inline __attribute__((always_inline)) void
GFX11_MI_STORE_DATA_INDEX_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_MI_STORE_DATA_INDEX * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->UsePerProcessHardwareStatusPage, 21, 21) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_uint(values->Offset, 2, 11);

   dw[2] =
      __gen_uint(values->DataDWord0, 0, 31);
}

#define GFX11_MI_STORE_REGISTER_MEM_length      4
#define GFX11_MI_STORE_REGISTER_MEM_length_bias      2
#define GFX11_MI_STORE_REGISTER_MEM_header      \
   .DWordLength                         =      2,  \
   .MICommandOpcode                     =     36,  \
   .CommandType                         =      0

struct GFX11_MI_STORE_REGISTER_MEM {
   uint32_t                             DWordLength;
   uint32_t                             AddCSMMIOStartOffset;
   bool                                 PredicateEnable;
   bool                                 UseGlobalGTT;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
   uint64_t                             RegisterAddress;
   __gen_address_type                   MemoryAddress;
};

static inline __attribute__((always_inline)) void
GFX11_MI_STORE_REGISTER_MEM_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_MI_STORE_REGISTER_MEM * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->AddCSMMIOStartOffset, 19, 19) |
      __gen_uint(values->PredicateEnable, 21, 21) |
      __gen_uint(values->UseGlobalGTT, 22, 22) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);

   dw[1] =
      __gen_offset(values->RegisterAddress, 2, 22);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->MemoryAddress, 0, 2, 63);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;
}

#define GFX11_MI_SUSPEND_FLUSH_length          1
#define GFX11_MI_SUSPEND_FLUSH_length_bias      1
#define GFX11_MI_SUSPEND_FLUSH_header           \
   .MICommandOpcode                     =     11,  \
   .CommandType                         =      0

struct GFX11_MI_SUSPEND_FLUSH {
   bool                                 SuspendFlush;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_SUSPEND_FLUSH_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_MI_SUSPEND_FLUSH * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->SuspendFlush, 0, 0) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_TOPOLOGY_FILTER_length        1
#define GFX11_MI_TOPOLOGY_FILTER_length_bias      1
#define GFX11_MI_TOPOLOGY_FILTER_header         \
   .MICommandOpcode                     =     13,  \
   .CommandType                         =      0

struct GFX11_MI_TOPOLOGY_FILTER {
   enum GFX11_3D_Prim_Topo_Type         TopologyFilterValue;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_TOPOLOGY_FILTER_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_MI_TOPOLOGY_FILTER * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->TopologyFilterValue, 0, 5) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_USER_INTERRUPT_length         1
#define GFX11_MI_USER_INTERRUPT_length_bias      1
#define GFX11_MI_USER_INTERRUPT_header          \
   .MICommandOpcode                     =      2,  \
   .CommandType                         =      0

struct GFX11_MI_USER_INTERRUPT {
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_USER_INTERRUPT_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_MI_USER_INTERRUPT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_WAIT_FOR_EVENT_length         1
#define GFX11_MI_WAIT_FOR_EVENT_length_bias      1
#define GFX11_MI_WAIT_FOR_EVENT_header          \
   .MICommandOpcode                     =      3,  \
   .CommandType                         =      0

struct GFX11_MI_WAIT_FOR_EVENT {
   bool                                 DisplayPlnae1AScanLineWaitEnable;
   bool                                 DisplayPlane1FlipPendingWaitEnable;
   bool                                 DisplayPlane4FlipPendingWaitEnable;
   bool                                 DisplayPlane1AVerticalBlankWaitEnable;
   bool                                 DisplayPlane7FlipPendingWaitEnable;
   bool                                 DisplayPlane8FlipPendingWaitEnable;
   bool                                 DisplayPlane1BScanLineWaitEnable;
   bool                                 DisplayPlane2FlipPendingWaitEnable;
   bool                                 DisplayPlane5FlipPendingWaitEnable;
   bool                                 DisplayPlane1BVerticalBlankWaitEnable;
   bool                                 DisplayPlane1CScanLineWaitEnable;
   bool                                 DisplayPlane3FlipPendingWaitEnable;
   bool                                 DisplayPlane9FlipPendingWaitEnable;
   bool                                 DisplayPlane10FlipPendingWaitEnable;
   bool                                 DisplayPlane11FlipPendingWaitEnable;
   bool                                 DisplayPlane12FlipPendingWaitEnable;
   bool                                 DisplayPlane6FlipPendingWaitEnable;
   bool                                 DisplayPlane1CVerticalBlankWaitEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_WAIT_FOR_EVENT_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_MI_WAIT_FOR_EVENT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DisplayPlnae1AScanLineWaitEnable, 0, 0) |
      __gen_uint(values->DisplayPlane1FlipPendingWaitEnable, 1, 1) |
      __gen_uint(values->DisplayPlane4FlipPendingWaitEnable, 2, 2) |
      __gen_uint(values->DisplayPlane1AVerticalBlankWaitEnable, 3, 3) |
      __gen_uint(values->DisplayPlane7FlipPendingWaitEnable, 6, 6) |
      __gen_uint(values->DisplayPlane8FlipPendingWaitEnable, 7, 7) |
      __gen_uint(values->DisplayPlane1BScanLineWaitEnable, 8, 8) |
      __gen_uint(values->DisplayPlane2FlipPendingWaitEnable, 9, 9) |
      __gen_uint(values->DisplayPlane5FlipPendingWaitEnable, 10, 10) |
      __gen_uint(values->DisplayPlane1BVerticalBlankWaitEnable, 11, 11) |
      __gen_uint(values->DisplayPlane1CScanLineWaitEnable, 14, 14) |
      __gen_uint(values->DisplayPlane3FlipPendingWaitEnable, 15, 15) |
      __gen_uint(values->DisplayPlane9FlipPendingWaitEnable, 16, 16) |
      __gen_uint(values->DisplayPlane10FlipPendingWaitEnable, 17, 17) |
      __gen_uint(values->DisplayPlane11FlipPendingWaitEnable, 18, 18) |
      __gen_uint(values->DisplayPlane12FlipPendingWaitEnable, 19, 19) |
      __gen_uint(values->DisplayPlane6FlipPendingWaitEnable, 20, 20) |
      __gen_uint(values->DisplayPlane1CVerticalBlankWaitEnable, 21, 21) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_MI_WAIT_FOR_EVENT_2_length       1
#define GFX11_MI_WAIT_FOR_EVENT_2_length_bias      1
#define GFX11_MI_WAIT_FOR_EVENT_2_header        \
   .MICommandOpcode                     =      4,  \
   .CommandType                         =      0

struct GFX11_MI_WAIT_FOR_EVENT_2 {
   uint32_t                             DisplayPlaneFlipPendingWaitEnable;
   uint32_t                             DisplayPipeVerticalBlankWaitEnable;
   uint32_t                             DisplayPipeScanLineWaitEnable;
   uint32_t                             MICommandOpcode;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_MI_WAIT_FOR_EVENT_2_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_MI_WAIT_FOR_EVENT_2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DisplayPlaneFlipPendingWaitEnable, 0, 5) |
      __gen_uint(values->DisplayPipeVerticalBlankWaitEnable, 8, 10) |
      __gen_uint(values->DisplayPipeScanLineWaitEnable, 12, 14) |
      __gen_uint(values->MICommandOpcode, 23, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_PIPELINE_SELECT_length           1
#define GFX11_PIPELINE_SELECT_length_bias      1
#define GFX11_PIPELINE_SELECT_header            \
   ._3DCommandSubOpcode                 =      4,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      1,  \
   .CommandType                         =      3

struct GFX11_PIPELINE_SELECT {
   uint32_t                             PipelineSelection;
#define _3D                                      0
#define Media                                    1
#define GPGPU                                    2
   bool                                 MediaSamplerDOPClockGateEnable;
   bool                                 ForceMediaAwake;
   uint32_t                             MaskBits;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
};

static inline __attribute__((always_inline)) void
GFX11_PIPELINE_SELECT_pack(__attribute__((unused)) __gen_user_data *data,
                           __attribute__((unused)) void * restrict dst,
                           __attribute__((unused)) const struct GFX11_PIPELINE_SELECT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PipelineSelection, 0, 1) |
      __gen_uint(values->MediaSamplerDOPClockGateEnable, 4, 4) |
      __gen_uint(values->ForceMediaAwake, 5, 5) |
      __gen_uint(values->MaskBits, 8, 15) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);
}

#define GFX11_PIPE_CONTROL_length              6
#define GFX11_PIPE_CONTROL_length_bias         2
#define GFX11_PIPE_CONTROL_header               \
   .DWordLength                         =      4,  \
   ._3DCommandSubOpcode                 =      0,  \
   ._3DCommandOpcode                    =      2,  \
   .CommandSubType                      =      3,  \
   .CommandType                         =      3

struct GFX11_PIPE_CONTROL {
   uint32_t                             DWordLength;
   bool                                 HDCPipelineFlushEnable;
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
   bool                                 PSDSyncEnable;
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
   bool                                 FlushLLC;
   bool                                 CommandCacheInvalidateEnable;
   __gen_address_type                   Address;
   uint64_t                             ImmediateData;
};

static inline __attribute__((always_inline)) void
GFX11_PIPE_CONTROL_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_PIPE_CONTROL * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->HDCPipelineFlushEnable, 9, 9) |
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
      __gen_uint(values->PSDSyncEnable, 17, 17) |
      __gen_uint(values->TLBInvalidate, 18, 18) |
      __gen_uint(values->GlobalSnapshotCountReset, 19, 19) |
      __gen_uint(values->CommandStreamerStallEnable, 20, 20) |
      __gen_uint(values->StoreDataIndex, 21, 21) |
      __gen_uint(values->LRIPostSyncOperation, 23, 23) |
      __gen_uint(values->DestinationAddressType, 24, 24) |
      __gen_uint(values->FlushLLC, 26, 26) |
      __gen_uint(values->CommandCacheInvalidateEnable, 29, 29);

   const uint64_t v2_address =
      __gen_address(data, &dw[2], values->Address, 0, 2, 47);
   dw[2] = v2_address;
   dw[3] = v2_address >> 32;

   const uint64_t v4 =
      __gen_uint(values->ImmediateData, 0, 63);
   dw[4] = v4;
   dw[5] = v4 >> 32;
}

#define GFX11_STATE_BASE_ADDRESS_length       22
#define GFX11_STATE_BASE_ADDRESS_length_bias      2
#define GFX11_STATE_BASE_ADDRESS_header         \
   .DWordLength                         =     20,  \
   ._3DCommandSubOpcode                 =      1,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX11_STATE_BASE_ADDRESS {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   bool                                 GeneralStateBaseAddressModifyEnable;
   uint32_t                             GeneralStateMOCS;
   __gen_address_type                   GeneralStateBaseAddress;
   uint32_t                             StatelessDataPortAccessMOCS;
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
   bool                                 GeneralStateBufferSizeModifyEnable;
   uint32_t                             GeneralStateBufferSize;
   bool                                 DynamicStateBufferSizeModifyEnable;
   uint32_t                             DynamicStateBufferSize;
   bool                                 IndirectObjectBufferSizeModifyEnable;
   uint32_t                             IndirectObjectBufferSize;
   bool                                 InstructionBuffersizeModifyEnable;
   uint32_t                             InstructionBufferSize;
   bool                                 BindlessSurfaceStateBaseAddressModifyEnable;
   uint32_t                             BindlessSurfaceStateMOCS;
   __gen_address_type                   BindlessSurfaceStateBaseAddress;
   uint32_t                             BindlessSurfaceStateSize;
   bool                                 BindlessSamplerStateBaseAddressModifyEnable;
   uint32_t                             BindlessSamplerStateMOCS;
   __gen_address_type                   BindlessSamplerStateBaseAddress;
   uint32_t                             BindlessSamplerStateBufferSize;
};

static inline __attribute__((always_inline)) void
GFX11_STATE_BASE_ADDRESS_pack(__attribute__((unused)) __gen_user_data *data,
                              __attribute__((unused)) void * restrict dst,
                              __attribute__((unused)) const struct GFX11_STATE_BASE_ADDRESS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_uint(values->GeneralStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->GeneralStateMOCS, 4, 10);
   const uint64_t v1_address =
      __gen_address(data, &dw[1], values->GeneralStateBaseAddress, v1, 12, 63);
   dw[1] = v1_address;
   dw[2] = (v1_address >> 32) | (v1 >> 32);

   dw[3] =
      __gen_uint(values->StatelessDataPortAccessMOCS, 16, 22);

   const uint64_t v4 =
      __gen_uint(values->SurfaceStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->SurfaceStateMOCS, 4, 10);
   const uint64_t v4_address =
      __gen_address(data, &dw[4], values->SurfaceStateBaseAddress, v4, 12, 63);
   dw[4] = v4_address;
   dw[5] = (v4_address >> 32) | (v4 >> 32);

   const uint64_t v6 =
      __gen_uint(values->DynamicStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->DynamicStateMOCS, 4, 10);
   const uint64_t v6_address =
      __gen_address(data, &dw[6], values->DynamicStateBaseAddress, v6, 12, 63);
   dw[6] = v6_address;
   dw[7] = (v6_address >> 32) | (v6 >> 32);

   const uint64_t v8 =
      __gen_uint(values->IndirectObjectBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->IndirectObjectMOCS, 4, 10);
   const uint64_t v8_address =
      __gen_address(data, &dw[8], values->IndirectObjectBaseAddress, v8, 12, 63);
   dw[8] = v8_address;
   dw[9] = (v8_address >> 32) | (v8 >> 32);

   const uint64_t v10 =
      __gen_uint(values->InstructionBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->InstructionMOCS, 4, 10);
   const uint64_t v10_address =
      __gen_address(data, &dw[10], values->InstructionBaseAddress, v10, 12, 63);
   dw[10] = v10_address;
   dw[11] = (v10_address >> 32) | (v10 >> 32);

   dw[12] =
      __gen_uint(values->GeneralStateBufferSizeModifyEnable, 0, 0) |
      __gen_uint(values->GeneralStateBufferSize, 12, 31);

   dw[13] =
      __gen_uint(values->DynamicStateBufferSizeModifyEnable, 0, 0) |
      __gen_uint(values->DynamicStateBufferSize, 12, 31);

   dw[14] =
      __gen_uint(values->IndirectObjectBufferSizeModifyEnable, 0, 0) |
      __gen_uint(values->IndirectObjectBufferSize, 12, 31);

   dw[15] =
      __gen_uint(values->InstructionBuffersizeModifyEnable, 0, 0) |
      __gen_uint(values->InstructionBufferSize, 12, 31);

   const uint64_t v16 =
      __gen_uint(values->BindlessSurfaceStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->BindlessSurfaceStateMOCS, 4, 10);
   const uint64_t v16_address =
      __gen_address(data, &dw[16], values->BindlessSurfaceStateBaseAddress, v16, 12, 63);
   dw[16] = v16_address;
   dw[17] = (v16_address >> 32) | (v16 >> 32);

   dw[18] =
      __gen_uint(values->BindlessSurfaceStateSize, 12, 31);

   const uint64_t v19 =
      __gen_uint(values->BindlessSamplerStateBaseAddressModifyEnable, 0, 0) |
      __gen_uint(values->BindlessSamplerStateMOCS, 4, 10);
   const uint64_t v19_address =
      __gen_address(data, &dw[19], values->BindlessSamplerStateBaseAddress, v19, 12, 63);
   dw[19] = v19_address;
   dw[20] = (v19_address >> 32) | (v19 >> 32);

   dw[21] =
      __gen_uint(values->BindlessSamplerStateBufferSize, 12, 31);
}

#define GFX11_STATE_SIP_length                 3
#define GFX11_STATE_SIP_length_bias            2
#define GFX11_STATE_SIP_header                  \
   .DWordLength                         =      1,  \
   ._3DCommandSubOpcode                 =      2,  \
   ._3DCommandOpcode                    =      1,  \
   .CommandSubType                      =      0,  \
   .CommandType                         =      3

struct GFX11_STATE_SIP {
   uint32_t                             DWordLength;
   uint32_t                             _3DCommandSubOpcode;
   uint32_t                             _3DCommandOpcode;
   uint32_t                             CommandSubType;
   uint32_t                             CommandType;
   uint64_t                             SystemInstructionPointer;
};

static inline __attribute__((always_inline)) void
GFX11_STATE_SIP_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX11_STATE_SIP * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DWordLength, 0, 7) |
      __gen_uint(values->_3DCommandSubOpcode, 16, 23) |
      __gen_uint(values->_3DCommandOpcode, 24, 26) |
      __gen_uint(values->CommandSubType, 27, 28) |
      __gen_uint(values->CommandType, 29, 31);

   const uint64_t v1 =
      __gen_offset(values->SystemInstructionPointer, 4, 63);
   dw[1] = v1;
   dw[2] = v1 >> 32;
}

#define GFX11_3D_CHICKEN3_num             0x2090
#define GFX11_3D_CHICKEN3_length               1
struct GFX11_3D_CHICKEN3 {
   bool                                 AALineQualityFix;
   bool                                 AALineQualityFixMask;
};

static inline __attribute__((always_inline)) void
GFX11_3D_CHICKEN3_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_3D_CHICKEN3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->AALineQualityFix, 5, 5) |
      __gen_uint(values->AALineQualityFixMask, 21, 21);
}

#define GFX11_BCS_INSTDONE_num            0x2206c
#define GFX11_BCS_INSTDONE_length              1
struct GFX11_BCS_INSTDONE {
   bool                                 RingEnable;
   bool                                 BlitterIDLE;
   bool                                 GABIDLE;
   bool                                 BCSDone;
};

static inline __attribute__((always_inline)) void
GFX11_BCS_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_BCS_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->RingEnable, 0, 0) |
      __gen_uint(values->BlitterIDLE, 1, 1) |
      __gen_uint(values->GABIDLE, 2, 2) |
      __gen_uint(values->BCSDone, 3, 3);
}

#define GFX11_CACHE_MODE_0_num            0x7000
#define GFX11_CACHE_MODE_0_length              1
struct GFX11_CACHE_MODE_0 {
   bool                                 DisableBytesharingfor3DTYFLOD1surfacesfor3264128bpp;
   bool                                 Disableclockgatinginthepixelbackend;
   bool                                 HierarchicalZDisable;
   bool                                 RCCEvictionPolicy;
   bool                                 STCPMAOptimizationDisable;
   bool                                 STCReadHitWonlyOptimizationDisable;
   bool                                 DepthRelatedCachePipelinedFlushDisable;
   bool                                 SamplerL2TLBPrefetchEnable;
   bool                                 RCZPMANotPromotedAllocationstalloptimizationDisableduetochangeindepthparameters;
   uint32_t                             MSAACompressionPlaneNumberThresholdforeLLC;
   bool                                 DisableRepackingforCompression;
   bool                                 DisableBytesharingfor3DTYFLOD1surfacesfor3264128bppMask;
   bool                                 DisableclockgatinginthepixelbackendMask;
   bool                                 HierarchicalZDisableMask;
   bool                                 RCCEvictionPolicyMask;
   bool                                 STCPMAOptimizationDisableMask;
   bool                                 STCReadHitWonlyOptimizationDisableMask;
   bool                                 DepthRelatedCachePipelinedFlushDisableMask;
   bool                                 SamplerL2TLBPrefetchEnableMask;
   bool                                 RCZPMANotPromotedAllocationstalloptimizationDisableduetochangeindepthparametersMask;
   uint32_t                             MSAACompressionPlaneNumberThresholdforeLLCMask;
   bool                                 DisableRepackingforCompressionMask;
};

static inline __attribute__((always_inline)) void
GFX11_CACHE_MODE_0_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_CACHE_MODE_0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->DisableBytesharingfor3DTYFLOD1surfacesfor3264128bpp, 0, 0) |
      __gen_uint(values->Disableclockgatinginthepixelbackend, 1, 1) |
      __gen_uint(values->HierarchicalZDisable, 3, 3) |
      __gen_uint(values->RCCEvictionPolicy, 4, 4) |
      __gen_uint(values->STCPMAOptimizationDisable, 5, 5) |
      __gen_uint(values->STCReadHitWonlyOptimizationDisable, 6, 6) |
      __gen_uint(values->DepthRelatedCachePipelinedFlushDisable, 8, 8) |
      __gen_uint(values->SamplerL2TLBPrefetchEnable, 9, 9) |
      __gen_uint(values->RCZPMANotPromotedAllocationstalloptimizationDisableduetochangeindepthparameters, 10, 10) |
      __gen_uint(values->MSAACompressionPlaneNumberThresholdforeLLC, 12, 14) |
      __gen_uint(values->DisableRepackingforCompression, 15, 15) |
      __gen_uint(values->DisableBytesharingfor3DTYFLOD1surfacesfor3264128bppMask, 16, 16) |
      __gen_uint(values->DisableclockgatinginthepixelbackendMask, 17, 17) |
      __gen_uint(values->HierarchicalZDisableMask, 19, 19) |
      __gen_uint(values->RCCEvictionPolicyMask, 20, 20) |
      __gen_uint(values->STCPMAOptimizationDisableMask, 21, 21) |
      __gen_uint(values->STCReadHitWonlyOptimizationDisableMask, 22, 22) |
      __gen_uint(values->DepthRelatedCachePipelinedFlushDisableMask, 24, 24) |
      __gen_uint(values->SamplerL2TLBPrefetchEnableMask, 25, 25) |
      __gen_uint(values->RCZPMANotPromotedAllocationstalloptimizationDisableduetochangeindepthparametersMask, 26, 26) |
      __gen_uint(values->MSAACompressionPlaneNumberThresholdforeLLCMask, 28, 30) |
      __gen_uint(values->DisableRepackingforCompressionMask, 31, 31);
}

#define GFX11_CACHE_MODE_1_num            0x7004
#define GFX11_CACHE_MODE_1_length              1
struct GFX11_CACHE_MODE_1 {
   bool                                 PartialResolveDisableInVC;
   bool                                 RCZPMAPromoted2NotPromotedAllocationstalloptimizationDisable;
   bool                                 MCSCacheDisable;
   bool                                 MSCRAWHazardAvoidanceBit;
   uint32_t                             NPEarlyZFailsDisable;
   bool                                 BlendOptimizationFixDisable;
   bool                                 ColorCompressionDisable;
   bool                                 PartialResolveDisableInVCMask;
   bool                                 RCZPMAPromoted2NotPromotedAllocationstalloptimizationDisableMask;
   bool                                 MCSCacheDisableMask;
   bool                                 MSCRAWHazardAvoidanceBitMask;
   bool                                 NPEarlyZFailsDisableMask;
   bool                                 BlendOptimizationFixDisableMask;
   bool                                 ColorCompressionDisableMask;
};

static inline __attribute__((always_inline)) void
GFX11_CACHE_MODE_1_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_CACHE_MODE_1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PartialResolveDisableInVC, 1, 1) |
      __gen_uint(values->RCZPMAPromoted2NotPromotedAllocationstalloptimizationDisable, 3, 3) |
      __gen_uint(values->MCSCacheDisable, 5, 5) |
      __gen_uint(values->MSCRAWHazardAvoidanceBit, 9, 9) |
      __gen_uint(values->NPEarlyZFailsDisable, 13, 13) |
      __gen_uint(values->BlendOptimizationFixDisable, 14, 14) |
      __gen_uint(values->ColorCompressionDisable, 15, 15) |
      __gen_uint(values->PartialResolveDisableInVCMask, 17, 17) |
      __gen_uint(values->RCZPMAPromoted2NotPromotedAllocationstalloptimizationDisableMask, 19, 19) |
      __gen_uint(values->MCSCacheDisableMask, 21, 21) |
      __gen_uint(values->MSCRAWHazardAvoidanceBitMask, 25, 25) |
      __gen_uint(values->NPEarlyZFailsDisableMask, 29, 29) |
      __gen_uint(values->BlendOptimizationFixDisableMask, 30, 30) |
      __gen_uint(values->ColorCompressionDisableMask, 31, 31);
}

#define GFX11_GT_MODE_num                 0x7008
#define GFX11_GT_MODE_length                   1
struct GFX11_GT_MODE {
   uint32_t                             BindingTableAlignment;
#define BTP_15_5                                 0
#define BTP_18_8                                 1
   bool                                 BindingTableAlignmentMask;
};

static inline __attribute__((always_inline)) void
GFX11_GT_MODE_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX11_GT_MODE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BindingTableAlignment, 10, 10) |
      __gen_uint(values->BindingTableAlignmentMask, 26, 26);
}

#define GFX11_CACHE_MODE_SS_num           0xe420
#define GFX11_CACHE_MODE_SS_length             1
struct GFX11_CACHE_MODE_SS {
   bool                                 InstructionLevel1CacheDisable;
   bool                                 InstructionLevel1CacheandInFlightQueueDisable;
   bool                                 FloatBlendOptimizationEnable;
   bool                                 PerSampleBlendOptDisable;
   bool                                 InstructionLevel1CacheDisableMask;
   bool                                 InstructionLevel1CacheandInFlightQueueDisableMask;
   bool                                 FloatBlendOptimizationEnableMask;
   bool                                 PerSampleBlendOptDisableMask;
};

static inline __attribute__((always_inline)) void
GFX11_CACHE_MODE_SS_pack(__attribute__((unused)) __gen_user_data *data,
                         __attribute__((unused)) void * restrict dst,
                         __attribute__((unused)) const struct GFX11_CACHE_MODE_SS * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->InstructionLevel1CacheDisable, 0, 0) |
      __gen_uint(values->InstructionLevel1CacheandInFlightQueueDisable, 1, 1) |
      __gen_uint(values->FloatBlendOptimizationEnable, 4, 4) |
      __gen_uint(values->PerSampleBlendOptDisable, 11, 11) |
      __gen_uint(values->InstructionLevel1CacheDisableMask, 16, 16) |
      __gen_uint(values->InstructionLevel1CacheandInFlightQueueDisableMask, 17, 17) |
      __gen_uint(values->FloatBlendOptimizationEnableMask, 20, 20) |
      __gen_uint(values->PerSampleBlendOptDisableMask, 27, 27);
}

#define GFX11_CL_INVOCATION_COUNT_num     0x2338
#define GFX11_CL_INVOCATION_COUNT_length       2
struct GFX11_CL_INVOCATION_COUNT {
   uint64_t                             CLInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_CL_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_CL_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CLInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_CL_PRIMITIVES_COUNT_num     0x2340
#define GFX11_CL_PRIMITIVES_COUNT_length       2
struct GFX11_CL_PRIMITIVES_COUNT {
   uint64_t                             CLPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_CL_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_CL_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CLPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_COMMON_SLICE_CHICKEN3_num   0x7304
#define GFX11_COMMON_SLICE_CHICKEN3_length      1
struct GFX11_COMMON_SLICE_CHICKEN3 {
   uint32_t                             PSThreadPanicDispatch;
   uint32_t                             PSThreadPanicDispatchMask;
};

static inline __attribute__((always_inline)) void
GFX11_COMMON_SLICE_CHICKEN3_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_COMMON_SLICE_CHICKEN3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PSThreadPanicDispatch, 6, 7) |
      __gen_uint(values->PSThreadPanicDispatchMask, 22, 23);
}

#define GFX11_COMMON_SLICE_CHICKEN4_num   0x7300
#define GFX11_COMMON_SLICE_CHICKEN4_length      1
struct GFX11_COMMON_SLICE_CHICKEN4 {
   bool                                 EnableHardwareFilteringinWM;
   bool                                 EnableHardwareFilteringinWMMask;
};

static inline __attribute__((always_inline)) void
GFX11_COMMON_SLICE_CHICKEN4_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_COMMON_SLICE_CHICKEN4 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->EnableHardwareFilteringinWM, 5, 5) |
      __gen_uint(values->EnableHardwareFilteringinWMMask, 21, 21);
}

#define GFX11_CS_CHICKEN1_num             0x2580
#define GFX11_CS_CHICKEN1_length               1
struct GFX11_CS_CHICKEN1 {
   uint32_t                             ReplayMode;
#define MidcmdbufferPreemption                   0
#define ObjectLevelPreemption                    1
   bool                                 ReplayModeMask;
};

static inline __attribute__((always_inline)) void
GFX11_CS_CHICKEN1_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_CS_CHICKEN1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->ReplayMode, 0, 0) |
      __gen_uint(values->ReplayModeMask, 16, 16);
}

#define GFX11_CS_DEBUG_MODE2_num          0x20d8
#define GFX11_CS_DEBUG_MODE2_length            1
struct GFX11_CS_DEBUG_MODE2 {
   bool                                 _3DRenderingInstructionDisable;
   bool                                 MediaInstructionDisable;
   bool                                 CONSTANT_BUFFERAddressOffsetDisable;
   bool                                 _3DRenderingInstructionDisableMask;
   bool                                 MediaInstructionDisableMask;
   bool                                 CONSTANT_BUFFERAddressOffsetDisableMask;
};

static inline __attribute__((always_inline)) void
GFX11_CS_DEBUG_MODE2_pack(__attribute__((unused)) __gen_user_data *data,
                          __attribute__((unused)) void * restrict dst,
                          __attribute__((unused)) const struct GFX11_CS_DEBUG_MODE2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->_3DRenderingInstructionDisable, 0, 0) |
      __gen_uint(values->MediaInstructionDisable, 1, 1) |
      __gen_uint(values->CONSTANT_BUFFERAddressOffsetDisable, 4, 4) |
      __gen_uint(values->_3DRenderingInstructionDisableMask, 16, 16) |
      __gen_uint(values->MediaInstructionDisableMask, 17, 17) |
      __gen_uint(values->CONSTANT_BUFFERAddressOffsetDisableMask, 20, 20);
}

#define GFX11_CS_INVOCATION_COUNT_num     0x2290
#define GFX11_CS_INVOCATION_COUNT_length       2
struct GFX11_CS_INVOCATION_COUNT {
   uint64_t                             CSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_CS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_CS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->CSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_DS_INVOCATION_COUNT_num     0x2308
#define GFX11_DS_INVOCATION_COUNT_length       2
struct GFX11_DS_INVOCATION_COUNT {
   uint64_t                             DSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_DS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_DS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->DSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_GS_INVOCATION_COUNT_num     0x2328
#define GFX11_GS_INVOCATION_COUNT_length       2
struct GFX11_GS_INVOCATION_COUNT {
   uint64_t                             GSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_GS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_GS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->GSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_GS_PRIMITIVES_COUNT_num     0x2330
#define GFX11_GS_PRIMITIVES_COUNT_length       2
struct GFX11_GS_PRIMITIVES_COUNT {
   uint64_t                             GSPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_GS_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_GS_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->GSPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_HALF_SLICE_CHICKEN7_num     0xe194
#define GFX11_HALF_SLICE_CHICKEN7_length       1
struct GFX11_HALF_SLICE_CHICKEN7 {
   bool                                 EnabledTexelOffsetPrecisionFix;
   bool                                 EnabledTexelOffsetPrecisionFixMask;
};

static inline __attribute__((always_inline)) void
GFX11_HALF_SLICE_CHICKEN7_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_HALF_SLICE_CHICKEN7 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->EnabledTexelOffsetPrecisionFix, 1, 1) |
      __gen_uint(values->EnabledTexelOffsetPrecisionFixMask, 17, 17);
}

#define GFX11_HS_INVOCATION_COUNT_num     0x2300
#define GFX11_HS_INVOCATION_COUNT_length       2
struct GFX11_HS_INVOCATION_COUNT {
   uint64_t                             HSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_HS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_HS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->HSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_IA_PRIMITIVES_COUNT_num     0x2318
#define GFX11_IA_PRIMITIVES_COUNT_length       2
struct GFX11_IA_PRIMITIVES_COUNT {
   uint64_t                             IAPrimitivesCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_IA_PRIMITIVES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_IA_PRIMITIVES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->IAPrimitivesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_IA_VERTICES_COUNT_num       0x2310
#define GFX11_IA_VERTICES_COUNT_length         2
struct GFX11_IA_VERTICES_COUNT {
   uint64_t                             IAVerticesCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_IA_VERTICES_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                             __attribute__((unused)) void * restrict dst,
                             __attribute__((unused)) const struct GFX11_IA_VERTICES_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->IAVerticesCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_INSTDONE_1_num              0x206c
#define GFX11_INSTDONE_1_length                1
struct GFX11_INSTDONE_1 {
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
   bool                                 TDG1Done;
   bool                                 TDG0Done;
   bool                                 URBMDone;
   bool                                 SVGDone;
   bool                                 GAFSDone;
   bool                                 VFEDone;
   bool                                 TSG0Done;
   bool                                 GAFMDone;
   bool                                 GAMDone;
   bool                                 RSDone;
   bool                                 CSDone;
   bool                                 SDEDone;
   bool                                 RCCFBCCSDone;
   bool                                 TSG1Done;
};

static inline __attribute__((always_inline)) void
GFX11_INSTDONE_1_pack(__attribute__((unused)) __gen_user_data *data,
                      __attribute__((unused)) void * restrict dst,
                      __attribute__((unused)) const struct GFX11_INSTDONE_1 * restrict values)
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
      __gen_uint(values->TDG1Done, 11, 11) |
      __gen_uint(values->TDG0Done, 12, 12) |
      __gen_uint(values->URBMDone, 13, 13) |
      __gen_uint(values->SVGDone, 14, 14) |
      __gen_uint(values->GAFSDone, 15, 15) |
      __gen_uint(values->VFEDone, 16, 16) |
      __gen_uint(values->TSG0Done, 17, 17) |
      __gen_uint(values->GAFMDone, 18, 18) |
      __gen_uint(values->GAMDone, 19, 19) |
      __gen_uint(values->RSDone, 20, 20) |
      __gen_uint(values->CSDone, 21, 21) |
      __gen_uint(values->SDEDone, 22, 22) |
      __gen_uint(values->RCCFBCCSDone, 23, 23) |
      __gen_uint(values->TSG1Done, 24, 24);
}

#define GFX11_L3CNTLREG_num               0x7034
#define GFX11_L3CNTLREG_length                 1
struct GFX11_L3CNTLREG {
   uint32_t                             URBAllocation;
   bool                                 ErrorDetectionBehaviorControl;
   bool                                 UseFullWays;
   uint32_t                             ROAllocation;
   uint32_t                             DCAllocation;
   uint32_t                             AllAllocation;
};

static inline __attribute__((always_inline)) void
GFX11_L3CNTLREG_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX11_L3CNTLREG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->URBAllocation, 1, 7) |
      __gen_uint(values->ErrorDetectionBehaviorControl, 9, 9) |
      __gen_uint(values->UseFullWays, 10, 10) |
      __gen_uint(values->ROAllocation, 11, 17) |
      __gen_uint(values->DCAllocation, 18, 24) |
      __gen_uint(values->AllAllocation, 25, 31);
}

#define GFX11_PERFCNT1_num                0x91b8
#define GFX11_PERFCNT1_length                  2
struct GFX11_PERFCNT1 {
   uint64_t                             Value;
   uint32_t                             EventSelection;
   bool                                 CounterClear;
   bool                                 EdgeDetect;
   bool                                 OverflowEnable;
   bool                                 CounterEnable;
};

static inline __attribute__((always_inline)) void
GFX11_PERFCNT1_pack(__attribute__((unused)) __gen_user_data *data,
                    __attribute__((unused)) void * restrict dst,
                    __attribute__((unused)) const struct GFX11_PERFCNT1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->Value, 0, 43) |
      __gen_uint(values->EventSelection, 52, 59) |
      __gen_uint(values->CounterClear, 60, 60) |
      __gen_uint(values->EdgeDetect, 61, 61) |
      __gen_uint(values->OverflowEnable, 62, 62) |
      __gen_uint(values->CounterEnable, 63, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_PERFCNT2_num                0x91c0
#define GFX11_PERFCNT2_length                  2
struct GFX11_PERFCNT2 {
   uint64_t                             Value;
   uint32_t                             EventSelection;
   bool                                 CounterClear;
   bool                                 EdgeDetect;
   bool                                 OverflowEnable;
   bool                                 CounterEnable;
};

static inline __attribute__((always_inline)) void
GFX11_PERFCNT2_pack(__attribute__((unused)) __gen_user_data *data,
                    __attribute__((unused)) void * restrict dst,
                    __attribute__((unused)) const struct GFX11_PERFCNT2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->Value, 0, 43) |
      __gen_uint(values->EventSelection, 52, 59) |
      __gen_uint(values->CounterClear, 60, 60) |
      __gen_uint(values->EdgeDetect, 61, 61) |
      __gen_uint(values->OverflowEnable, 62, 62) |
      __gen_uint(values->CounterEnable, 63, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_PS_INVOCATION_COUNT_num     0x2348
#define GFX11_PS_INVOCATION_COUNT_length       2
struct GFX11_PS_INVOCATION_COUNT {
   uint64_t                             PSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_PS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_PS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_ROW_INSTDONE_num            0xe164
#define GFX11_ROW_INSTDONE_length              1
struct GFX11_ROW_INSTDONE {
   bool                                 BCDone;
   bool                                 PSDDone;
   bool                                 DAPRDone;
   bool                                 TDLDone;
   bool                                 ICDone;
   bool                                 MA0Done;
   bool                                 EU00DoneSS0;
   bool                                 EU01DoneSS0;
   bool                                 EU02DoneSS0;
   bool                                 EU03DoneSS0;
   bool                                 EU10DoneSS0;
   bool                                 EU11DoneSS0;
   bool                                 EU12DoneSS0;
   bool                                 EU13DoneSS0;
   bool                                 MA1DoneSS0;
};

static inline __attribute__((always_inline)) void
GFX11_ROW_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_ROW_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->BCDone, 0, 0) |
      __gen_uint(values->PSDDone, 1, 1) |
      __gen_uint(values->DAPRDone, 3, 3) |
      __gen_uint(values->TDLDone, 6, 6) |
      __gen_uint(values->ICDone, 12, 12) |
      __gen_uint(values->MA0Done, 15, 15) |
      __gen_uint(values->EU00DoneSS0, 16, 16) |
      __gen_uint(values->EU01DoneSS0, 17, 17) |
      __gen_uint(values->EU02DoneSS0, 18, 18) |
      __gen_uint(values->EU03DoneSS0, 19, 19) |
      __gen_uint(values->EU10DoneSS0, 21, 21) |
      __gen_uint(values->EU11DoneSS0, 22, 22) |
      __gen_uint(values->EU12DoneSS0, 23, 23) |
      __gen_uint(values->EU13DoneSS0, 24, 24) |
      __gen_uint(values->MA1DoneSS0, 26, 26);
}

#define GFX11_RPSTAT0_num                 0xa01c
#define GFX11_RPSTAT0_length                   1
struct GFX11_RPSTAT0 {
   uint32_t                             PreviousGTFrequency;
   uint32_t                             CurrentGTFrequency;
};

static inline __attribute__((always_inline)) void
GFX11_RPSTAT0_pack(__attribute__((unused)) __gen_user_data *data,
                   __attribute__((unused)) void * restrict dst,
                   __attribute__((unused)) const struct GFX11_RPSTAT0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->PreviousGTFrequency, 0, 8) |
      __gen_uint(values->CurrentGTFrequency, 23, 31);
}

#define GFX11_SAMPLER_INSTDONE_num        0xe160
#define GFX11_SAMPLER_INSTDONE_length          1
struct GFX11_SAMPLER_INSTDONE {
   bool                                 IMEDone;
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
   bool                                 CREDone;
   bool                                 SVSM_ARB_SIFM;
   bool                                 SVSMARB2;
   bool                                 SVSMARB1;
   bool                                 SVSMAdapter;
   bool                                 BDMDone;
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SAMPLER_INSTDONE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->IMEDone, 0, 0) |
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
      __gen_uint(values->CREDone, 14, 14) |
      __gen_uint(values->SVSM_ARB_SIFM, 15, 15) |
      __gen_uint(values->SVSMARB2, 16, 16) |
      __gen_uint(values->SVSMARB1, 17, 17) |
      __gen_uint(values->SVSMAdapter, 18, 18) |
      __gen_uint(values->BDMDone, 19, 19);
}

#define GFX11_SAMPLER_MODE_num            0xe18c
#define GFX11_SAMPLER_MODE_length              1
struct GFX11_SAMPLER_MODE {
   bool                                 HeaderlessMessageforPreemptableContexts;
   bool                                 HeaderlessMessageforPreemptableContextsMask;
};

static inline __attribute__((always_inline)) void
GFX11_SAMPLER_MODE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_SAMPLER_MODE * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->HeaderlessMessageforPreemptableContexts, 5, 5) |
      __gen_uint(values->HeaderlessMessageforPreemptableContextsMask, 21, 21);
}

#define GFX11_SC_INSTDONE_num             0x7100
#define GFX11_SC_INSTDONE_length               1
struct GFX11_SC_INSTDONE {
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
   bool                                 SARBDone;
   bool                                 DC0Done;
   bool                                 DC1Done;
   bool                                 DC2Done;
   bool                                 DC3Done;
   bool                                 GW0Done;
   bool                                 GW1Done;
   bool                                 GW2Done;
   bool                                 GW3Done;
   bool                                 TDCDone;
   bool                                 SFBEDone;
};

static inline __attribute__((always_inline)) void
GFX11_SC_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                       __attribute__((unused)) void * restrict dst,
                       __attribute__((unused)) const struct GFX11_SC_INSTDONE * restrict values)
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
      __gen_uint(values->SARBDone, 15, 15) |
      __gen_uint(values->DC0Done, 16, 16) |
      __gen_uint(values->DC1Done, 17, 17) |
      __gen_uint(values->DC2Done, 18, 18) |
      __gen_uint(values->DC3Done, 19, 19) |
      __gen_uint(values->GW0Done, 20, 20) |
      __gen_uint(values->GW1Done, 21, 21) |
      __gen_uint(values->GW2Done, 22, 22) |
      __gen_uint(values->GW3Done, 23, 23) |
      __gen_uint(values->TDCDone, 24, 24) |
      __gen_uint(values->SFBEDone, 25, 25);
}

#define GFX11_SLICE_COMMON_ECO_CHICKEN1_num 0x731c
#define GFX11_SLICE_COMMON_ECO_CHICKEN1_length      1
struct GFX11_SLICE_COMMON_ECO_CHICKEN1 {
   bool                                 StateCacheRedirectToCSSectionEnable;
   bool                                 StateCacheRedirectToCSSectionEnableMask;
};

static inline __attribute__((always_inline)) void
GFX11_SLICE_COMMON_ECO_CHICKEN1_pack(__attribute__((unused)) __gen_user_data *data,
                                     __attribute__((unused)) void * restrict dst,
                                     __attribute__((unused)) const struct GFX11_SLICE_COMMON_ECO_CHICKEN1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->StateCacheRedirectToCSSectionEnable, 11, 11) |
      __gen_uint(values->StateCacheRedirectToCSSectionEnableMask, 27, 27);
}

#define GFX11_SO_NUM_PRIMS_WRITTEN0_num   0x5200
#define GFX11_SO_NUM_PRIMS_WRITTEN0_length      2
struct GFX11_SO_NUM_PRIMS_WRITTEN0 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_NUM_PRIMS_WRITTEN0_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_SO_NUM_PRIMS_WRITTEN0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_NUM_PRIMS_WRITTEN1_num   0x5208
#define GFX11_SO_NUM_PRIMS_WRITTEN1_length      2
struct GFX11_SO_NUM_PRIMS_WRITTEN1 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_NUM_PRIMS_WRITTEN1_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_SO_NUM_PRIMS_WRITTEN1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_NUM_PRIMS_WRITTEN2_num   0x5210
#define GFX11_SO_NUM_PRIMS_WRITTEN2_length      2
struct GFX11_SO_NUM_PRIMS_WRITTEN2 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_NUM_PRIMS_WRITTEN2_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_SO_NUM_PRIMS_WRITTEN2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_NUM_PRIMS_WRITTEN3_num   0x5218
#define GFX11_SO_NUM_PRIMS_WRITTEN3_length      2
struct GFX11_SO_NUM_PRIMS_WRITTEN3 {
   uint64_t                             NumPrimsWrittenCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_NUM_PRIMS_WRITTEN3_pack(__attribute__((unused)) __gen_user_data *data,
                                 __attribute__((unused)) void * restrict dst,
                                 __attribute__((unused)) const struct GFX11_SO_NUM_PRIMS_WRITTEN3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->NumPrimsWrittenCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_PRIM_STORAGE_NEEDED0_num 0x5240
#define GFX11_SO_PRIM_STORAGE_NEEDED0_length      2
struct GFX11_SO_PRIM_STORAGE_NEEDED0 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_PRIM_STORAGE_NEEDED0_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_SO_PRIM_STORAGE_NEEDED0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_PRIM_STORAGE_NEEDED1_num 0x5248
#define GFX11_SO_PRIM_STORAGE_NEEDED1_length      2
struct GFX11_SO_PRIM_STORAGE_NEEDED1 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_PRIM_STORAGE_NEEDED1_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_SO_PRIM_STORAGE_NEEDED1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_PRIM_STORAGE_NEEDED2_num 0x5250
#define GFX11_SO_PRIM_STORAGE_NEEDED2_length      2
struct GFX11_SO_PRIM_STORAGE_NEEDED2 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_PRIM_STORAGE_NEEDED2_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_SO_PRIM_STORAGE_NEEDED2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_PRIM_STORAGE_NEEDED3_num 0x5258
#define GFX11_SO_PRIM_STORAGE_NEEDED3_length      2
struct GFX11_SO_PRIM_STORAGE_NEEDED3 {
   uint64_t                             PrimStorageNeededCount;
};

static inline __attribute__((always_inline)) void
GFX11_SO_PRIM_STORAGE_NEEDED3_pack(__attribute__((unused)) __gen_user_data *data,
                                   __attribute__((unused)) void * restrict dst,
                                   __attribute__((unused)) const struct GFX11_SO_PRIM_STORAGE_NEEDED3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->PrimStorageNeededCount, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#define GFX11_SO_WRITE_OFFSET0_num        0x5280
#define GFX11_SO_WRITE_OFFSET0_length          1
struct GFX11_SO_WRITE_OFFSET0 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX11_SO_WRITE_OFFSET0_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SO_WRITE_OFFSET0 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX11_SO_WRITE_OFFSET1_num        0x5284
#define GFX11_SO_WRITE_OFFSET1_length          1
struct GFX11_SO_WRITE_OFFSET1 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX11_SO_WRITE_OFFSET1_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SO_WRITE_OFFSET1 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX11_SO_WRITE_OFFSET2_num        0x5288
#define GFX11_SO_WRITE_OFFSET2_length          1
struct GFX11_SO_WRITE_OFFSET2 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX11_SO_WRITE_OFFSET2_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SO_WRITE_OFFSET2 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX11_SO_WRITE_OFFSET3_num        0x528c
#define GFX11_SO_WRITE_OFFSET3_length          1
struct GFX11_SO_WRITE_OFFSET3 {
   uint64_t                             WriteOffset;
};

static inline __attribute__((always_inline)) void
GFX11_SO_WRITE_OFFSET3_pack(__attribute__((unused)) __gen_user_data *data,
                            __attribute__((unused)) void * restrict dst,
                            __attribute__((unused)) const struct GFX11_SO_WRITE_OFFSET3 * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_offset(values->WriteOffset, 2, 31);
}

#define GFX11_TCCNTLREG_num               0xb0a4
#define GFX11_TCCNTLREG_length                 1
struct GFX11_TCCNTLREG {
   bool                                 URBPartialWriteMergingEnable;
   bool                                 ColorZPartialWriteMergingEnable;
   bool                                 L3DataPartialWriteMergingEnable;
   bool                                 TCDisable;
};

static inline __attribute__((always_inline)) void
GFX11_TCCNTLREG_pack(__attribute__((unused)) __gen_user_data *data,
                     __attribute__((unused)) void * restrict dst,
                     __attribute__((unused)) const struct GFX11_TCCNTLREG * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   dw[0] =
      __gen_uint(values->URBPartialWriteMergingEnable, 0, 0) |
      __gen_uint(values->ColorZPartialWriteMergingEnable, 1, 1) |
      __gen_uint(values->L3DataPartialWriteMergingEnable, 2, 2) |
      __gen_uint(values->TCDisable, 3, 3);
}

#define GFX11_VCS_INSTDONE_num            0x1206c
#define GFX11_VCS_INSTDONE_length              1
struct GFX11_VCS_INSTDONE {
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
   bool                                 VINDone;
   bool                                 VPRDone;
   bool                                 VTQDone;
   bool                                 Reserved;
   bool                                 VCSDone;
   bool                                 GACDone;
};

static inline __attribute__((always_inline)) void
GFX11_VCS_INSTDONE_pack(__attribute__((unused)) __gen_user_data *data,
                        __attribute__((unused)) void * restrict dst,
                        __attribute__((unused)) const struct GFX11_VCS_INSTDONE * restrict values)
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
      __gen_uint(values->VINDone, 26, 26) |
      __gen_uint(values->VPRDone, 27, 27) |
      __gen_uint(values->VTQDone, 28, 28) |
      __gen_uint(values->Reserved, 29, 29) |
      __gen_uint(values->VCSDone, 30, 30) |
      __gen_uint(values->GACDone, 31, 31);
}

#define GFX11_VS_INVOCATION_COUNT_num     0x2320
#define GFX11_VS_INVOCATION_COUNT_length       2
struct GFX11_VS_INVOCATION_COUNT {
   uint64_t                             VSInvocationCountReport;
};

static inline __attribute__((always_inline)) void
GFX11_VS_INVOCATION_COUNT_pack(__attribute__((unused)) __gen_user_data *data,
                               __attribute__((unused)) void * restrict dst,
                               __attribute__((unused)) const struct GFX11_VS_INVOCATION_COUNT * restrict values)
{
   uint32_t * restrict dw = (uint32_t * restrict) dst;

   const uint64_t v0 =
      __gen_uint(values->VSInvocationCountReport, 0, 63);
   dw[0] = v0;
   dw[1] = v0 >> 32;
}

#endif /* GFX11_PACK_H */
