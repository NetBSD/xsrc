
/* Generated code, see midgard.xml and gen_pack_header.py
 *
 * Packets, enums and structures for Panfrost.
 *
 * This file has been generated, do not hand edit.
 */

#ifndef PAN_PACK_H
#define PAN_PACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <inttypes.h>
#include "util/macros.h"
#include "util/u_math.h"

#define __gen_unpack_float(x, y, z) uif(__gen_unpack_uint(x, y, z))

static inline uint64_t
__gen_uint(uint64_t v, uint32_t start, uint32_t end)
{
#ifndef NDEBUG
   const int width = end - start + 1;
   if (width < 64) {
      const uint64_t max = (1ull << width) - 1;
      assert(v <= max);
   }
#endif

   return v << start;
}

static inline uint32_t
__gen_sint(int32_t v, uint32_t start, uint32_t end)
{
#ifndef NDEBUG
   const int width = end - start + 1;
   if (width < 64) {
      const int64_t max = (1ll << (width - 1)) - 1;
      const int64_t min = -(1ll << (width - 1));
      assert(min <= v && v <= max);
   }
#endif

   return (((uint32_t) v) << start) & ((2ll << end) - 1);
}

static inline uint32_t
__gen_padded(uint32_t v, uint32_t start, uint32_t end)
{
    unsigned shift = __builtin_ctz(v);
    unsigned odd = v >> (shift + 1);

#ifndef NDEBUG
    assert((v >> shift) & 1);
    assert(shift <= 31);
    assert(odd <= 7);
    assert((end - start + 1) == 8);
#endif

    return __gen_uint(shift | (odd << 5), start, end);
}


static inline uint64_t
__gen_unpack_uint(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   uint64_t val = 0;
   const int width = end - start + 1;
   const uint64_t mask = (width == 64 ? ~0 : (1ull << width) - 1 );

   for (uint32_t byte = start / 8; byte <= end / 8; byte++) {
      val |= ((uint64_t) cl[byte]) << ((byte - start / 8) * 8);
   }

   return (val >> (start % 8)) & mask;
}

static inline uint64_t
__gen_unpack_sint(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   int size = end - start + 1;
   int64_t val = __gen_unpack_uint(cl, start, end);

   /* Get the sign bit extended. */
   return (val << (64 - size)) >> (64 - size);
}

static inline uint64_t
__gen_unpack_padded(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   unsigned val = __gen_unpack_uint(cl, start, end);
   unsigned shift = val & 0b11111;
   unsigned odd = val >> 5;

   return (2*odd + 1) << shift;
}

#define PREFIX1(A) MALI_ ## A
#define PREFIX2(A, B) MALI_ ## A ## _ ## B
#define PREFIX4(A, B, C, D) MALI_ ## A ## _ ## B ## _ ## C ## _ ## D

#define pan_prepare(dst, T)                                 \
   *(dst) = (struct PREFIX1(T)){ PREFIX2(T, header) }

#define pan_pack(dst, T, name)                              \
   for (struct PREFIX1(T) name = { PREFIX2(T, header) }, \
        *_loop_terminate = (void *) (dst);                  \
        __builtin_expect(_loop_terminate != NULL, 1);       \
        ({ PREFIX2(T, pack)((uint32_t *) (dst), &name);  \
           _loop_terminate = NULL; }))

#define pan_unpack(src, T, name)                        \
        struct PREFIX1(T) name;                         \
        PREFIX2(T, unpack)((uint8_t *)(src), &name)

#define pan_print(fp, T, var, indent)                   \
        PREFIX2(T, print)(fp, &(var), indent)

#define pan_size(T) PREFIX2(T, LENGTH)
#define pan_alignment(T) PREFIX2(T, ALIGN)

#define pan_section_offset(A, S) \
        PREFIX4(A, SECTION, S, OFFSET)

#define pan_section_ptr(base, A, S) \
        ((void *)((uint8_t *)(base) + pan_section_offset(A, S)))

#define pan_section_pack(dst, A, S, name)                                                         \
   for (PREFIX4(A, SECTION, S, TYPE) name = { PREFIX4(A, SECTION, S, header) }, \
        *_loop_terminate = (void *) (dst);                                                        \
        __builtin_expect(_loop_terminate != NULL, 1);                                             \
        ({ PREFIX4(A, SECTION, S, pack) (pan_section_ptr(dst, A, S), &name);              \
           _loop_terminate = NULL; }))

#define pan_section_unpack(src, A, S, name)                               \
        PREFIX4(A, SECTION, S, TYPE) name;                             \
        PREFIX4(A, SECTION, S, unpack)(pan_section_ptr(src, A, S), &name)

#define pan_section_print(fp, A, S, var, indent)                          \
        PREFIX4(A, SECTION, S, print)(fp, &(var), indent)

#define pan_merge(packed1, packed2, type)         do {                 for (unsigned i = 0; i < (PREFIX2(type, LENGTH) / 4); ++i)                         (packed1).opaque[i] |= (packed2).opaque[i];         } while(0)

/* From presentations, 16x16 tiles externally. Use shift for fast computation
 * of tile numbers. */

#define MALI_TILE_SHIFT 4
#define MALI_TILE_LENGTH (1 << MALI_TILE_SHIFT)


enum mali_channel {
        MALI_CHANNEL_R                       =      0,
        MALI_CHANNEL_G                       =      1,
        MALI_CHANNEL_B                       =      2,
        MALI_CHANNEL_A                       =      3,
        MALI_CHANNEL_0                       =      4,
        MALI_CHANNEL_1                       =      5,
};

static inline const char *
mali_channel_as_str(enum mali_channel imm)
{
    switch (imm) {
    case MALI_CHANNEL_R: return "R";
    case MALI_CHANNEL_G: return "G";
    case MALI_CHANNEL_B: return "B";
    case MALI_CHANNEL_A: return "A";
    case MALI_CHANNEL_0: return "0";
    case MALI_CHANNEL_1: return "1";
    default: return "XXX: INVALID";
    }
}

enum mali_texture_dimension {
        MALI_TEXTURE_DIMENSION_CUBE          =      0,
        MALI_TEXTURE_DIMENSION_1D            =      1,
        MALI_TEXTURE_DIMENSION_2D            =      2,
        MALI_TEXTURE_DIMENSION_3D            =      3,
};

static inline const char *
mali_texture_dimension_as_str(enum mali_texture_dimension imm)
{
    switch (imm) {
    case MALI_TEXTURE_DIMENSION_CUBE: return "Cube";
    case MALI_TEXTURE_DIMENSION_1D: return "1D";
    case MALI_TEXTURE_DIMENSION_2D: return "2D";
    case MALI_TEXTURE_DIMENSION_3D: return "3D";
    default: return "XXX: INVALID";
    }
}

enum mali_sample_pattern {
        MALI_SAMPLE_PATTERN_SINGLE_SAMPLED   =      0,
        MALI_SAMPLE_PATTERN_ORDERED_4X_GRID  =      1,
        MALI_SAMPLE_PATTERN_ROTATED_4X_GRID  =      2,
        MALI_SAMPLE_PATTERN_D3D_8X_GRID      =      3,
        MALI_SAMPLE_PATTERN_D3D_16X_GRID     =      4,
};

static inline const char *
mali_sample_pattern_as_str(enum mali_sample_pattern imm)
{
    switch (imm) {
    case MALI_SAMPLE_PATTERN_SINGLE_SAMPLED: return "Single-sampled";
    case MALI_SAMPLE_PATTERN_ORDERED_4X_GRID: return "Ordered 4x Grid";
    case MALI_SAMPLE_PATTERN_ROTATED_4X_GRID: return "Rotated 4x Grid";
    case MALI_SAMPLE_PATTERN_D3D_8X_GRID: return "D3D 8x Grid";
    case MALI_SAMPLE_PATTERN_D3D_16X_GRID: return "D3D 16x Grid";
    default: return "XXX: INVALID";
    }
}

enum mali_blend_operand_a {
        MALI_BLEND_OPERAND_A_ZERO            =      1,
        MALI_BLEND_OPERAND_A_SRC             =      2,
        MALI_BLEND_OPERAND_A_DEST            =      3,
};

static inline const char *
mali_blend_operand_a_as_str(enum mali_blend_operand_a imm)
{
    switch (imm) {
    case MALI_BLEND_OPERAND_A_ZERO: return "Zero";
    case MALI_BLEND_OPERAND_A_SRC: return "Src";
    case MALI_BLEND_OPERAND_A_DEST: return "Dest";
    default: return "XXX: INVALID";
    }
}

enum mali_blend_operand_b {
        MALI_BLEND_OPERAND_B_SRC_MINUS_DEST  =      0,
        MALI_BLEND_OPERAND_B_SRC_PLUS_DEST   =      1,
        MALI_BLEND_OPERAND_B_SRC             =      2,
        MALI_BLEND_OPERAND_B_DEST            =      3,
};

static inline const char *
mali_blend_operand_b_as_str(enum mali_blend_operand_b imm)
{
    switch (imm) {
    case MALI_BLEND_OPERAND_B_SRC_MINUS_DEST: return "Src Minus Dest";
    case MALI_BLEND_OPERAND_B_SRC_PLUS_DEST: return "Src Plus Dest";
    case MALI_BLEND_OPERAND_B_SRC: return "Src";
    case MALI_BLEND_OPERAND_B_DEST: return "Dest";
    default: return "XXX: INVALID";
    }
}

enum mali_blend_operand_c {
        MALI_BLEND_OPERAND_C_ZERO            =      1,
        MALI_BLEND_OPERAND_C_SRC             =      2,
        MALI_BLEND_OPERAND_C_DEST            =      3,
        MALI_BLEND_OPERAND_C_SRC_X_2         =      4,
        MALI_BLEND_OPERAND_C_SRC_ALPHA       =      5,
        MALI_BLEND_OPERAND_C_DEST_ALPHA      =      6,
        MALI_BLEND_OPERAND_C_CONSTANT        =      7,
};

static inline const char *
mali_blend_operand_c_as_str(enum mali_blend_operand_c imm)
{
    switch (imm) {
    case MALI_BLEND_OPERAND_C_ZERO: return "Zero";
    case MALI_BLEND_OPERAND_C_SRC: return "Src";
    case MALI_BLEND_OPERAND_C_DEST: return "Dest";
    case MALI_BLEND_OPERAND_C_SRC_X_2: return "Src x 2";
    case MALI_BLEND_OPERAND_C_SRC_ALPHA: return "Src Alpha";
    case MALI_BLEND_OPERAND_C_DEST_ALPHA: return "Dest Alpha";
    case MALI_BLEND_OPERAND_C_CONSTANT: return "Constant";
    default: return "XXX: INVALID";
    }
}

struct MALI_BLEND_FUNCTION {
   enum mali_blend_operand_a            a;
   bool                                 negate_a;
   enum mali_blend_operand_b            b;
   bool                                 negate_b;
   enum mali_blend_operand_c            c;
   bool                                 invert_c;
};

#define MALI_BLEND_FUNCTION_header              \
   0

static inline void
MALI_BLEND_FUNCTION_print(FILE *fp, const struct MALI_BLEND_FUNCTION * values, unsigned indent)
{
   fprintf(fp, "%*sA: %s\n", indent, "", mali_blend_operand_a_as_str(values->a));
   fprintf(fp, "%*sNegate A: %s\n", indent, "", values->negate_a ? "true" : "false");
   fprintf(fp, "%*sB: %s\n", indent, "", mali_blend_operand_b_as_str(values->b));
   fprintf(fp, "%*sNegate B: %s\n", indent, "", values->negate_b ? "true" : "false");
   fprintf(fp, "%*sC: %s\n", indent, "", mali_blend_operand_c_as_str(values->c));
   fprintf(fp, "%*sInvert C: %s\n", indent, "", values->invert_c ? "true" : "false");
}

struct MALI_BLEND_EQUATION {
   struct MALI_BLEND_FUNCTION           rgb;
   struct MALI_BLEND_FUNCTION           alpha;
   uint32_t                             color_mask;
};

#define MALI_BLEND_EQUATION_header              \
   .rgb = { MALI_BLEND_FUNCTION_header },  \
   .alpha = { MALI_BLEND_FUNCTION_header }

static inline void
MALI_BLEND_EQUATION_pack(uint32_t * restrict cl,
                         const struct MALI_BLEND_EQUATION * restrict values)
{
   cl[ 0] = __gen_uint(values->rgb.a, 0, 1) |
            __gen_uint(values->rgb.negate_a, 3, 3) |
            __gen_uint(values->rgb.b, 4, 5) |
            __gen_uint(values->rgb.negate_b, 7, 7) |
            __gen_uint(values->rgb.c, 8, 10) |
            __gen_uint(values->rgb.invert_c, 11, 11) |
            __gen_uint(values->alpha.a, 12, 13) |
            __gen_uint(values->alpha.negate_a, 15, 15) |
            __gen_uint(values->alpha.b, 16, 17) |
            __gen_uint(values->alpha.negate_b, 19, 19) |
            __gen_uint(values->alpha.c, 20, 22) |
            __gen_uint(values->alpha.invert_c, 23, 23) |
            __gen_uint(values->color_mask, 28, 31);
}


#define MALI_BLEND_EQUATION_LENGTH 4
struct mali_blend_equation_packed { uint32_t opaque[1]; };
static inline void
MALI_BLEND_EQUATION_unpack(const uint8_t * restrict cl,
                           struct MALI_BLEND_EQUATION * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf044044) fprintf(stderr, "XXX: Invalid field of Blend Equation unpacked at word 0\n");
   values->rgb.a = (enum mali_blend_operand_a)__gen_unpack_uint(cl, 0, 1);
   values->rgb.negate_a = __gen_unpack_uint(cl, 3, 3);
   values->rgb.b = (enum mali_blend_operand_b)__gen_unpack_uint(cl, 4, 5);
   values->rgb.negate_b = __gen_unpack_uint(cl, 7, 7);
   values->rgb.c = (enum mali_blend_operand_c)__gen_unpack_uint(cl, 8, 10);
   values->rgb.invert_c = __gen_unpack_uint(cl, 11, 11);
   values->alpha.a = (enum mali_blend_operand_a)__gen_unpack_uint(cl, 12, 13);
   values->alpha.negate_a = __gen_unpack_uint(cl, 15, 15);
   values->alpha.b = (enum mali_blend_operand_b)__gen_unpack_uint(cl, 16, 17);
   values->alpha.negate_b = __gen_unpack_uint(cl, 19, 19);
   values->alpha.c = (enum mali_blend_operand_c)__gen_unpack_uint(cl, 20, 22);
   values->alpha.invert_c = __gen_unpack_uint(cl, 23, 23);
   values->color_mask = __gen_unpack_uint(cl, 28, 31);
}

static inline void
MALI_BLEND_EQUATION_print(FILE *fp, const struct MALI_BLEND_EQUATION * values, unsigned indent)
{
   fprintf(fp, "%*sRGB:\n", indent, "");
   MALI_BLEND_FUNCTION_print(fp, &values->rgb, indent + 2);
   fprintf(fp, "%*sAlpha:\n", indent, "");
   MALI_BLEND_FUNCTION_print(fp, &values->alpha, indent + 2);
   fprintf(fp, "%*sColor Mask: %u\n", indent, "", values->color_mask);
}

enum mali_format {
        MALI_ETC2_RGB8                       =      1,
        MALI_ETC2_R11_UNORM                  =      2,
        MALI_ETC2_RGBA8                      =      3,
        MALI_ETC2_RG11_UNORM                 =      4,
        MALI_BC1_UNORM                       =      7,
        MALI_BC2_UNORM                       =      8,
        MALI_BC3_UNORM                       =      9,
        MALI_BC4_UNORM                       =     10,
        MALI_BC4_SNORM                       =     11,
        MALI_BC5_UNORM                       =     12,
        MALI_BC5_SNORM                       =     13,
        MALI_BC6H_UF16                       =     14,
        MALI_BC6H_SF16                       =     15,
        MALI_BC7_UNORM                       =     16,
        MALI_ETC2_R11_SNORM                  =     17,
        MALI_ETC2_RG11_SNORM                 =     18,
        MALI_ETC2_RGB8A1                     =     19,
        MALI_ASTC_3D_LDR                     =     20,
        MALI_ASTC_3D_HDR                     =     21,
        MALI_ASTC_2D_LDR                     =     22,
        MALI_ASTC_2D_HDR                     =     23,
};

static inline const char *
mali_format_as_str(enum mali_format imm)
{
    switch (imm) {
    case MALI_ETC2_RGB8: return "ETC2 RGB8";
    case MALI_ETC2_R11_UNORM: return "ETC2 R11 UNORM";
    case MALI_ETC2_RGBA8: return "ETC2 RGBA8";
    case MALI_ETC2_RG11_UNORM: return "ETC2 RG11 UNORM";
    case MALI_BC1_UNORM: return "BC1 UNORM";
    case MALI_BC2_UNORM: return "BC2 UNORM";
    case MALI_BC3_UNORM: return "BC3 UNORM";
    case MALI_BC4_UNORM: return "BC4 UNORM";
    case MALI_BC4_SNORM: return "BC4 SNORM";
    case MALI_BC5_UNORM: return "BC5 UNORM";
    case MALI_BC5_SNORM: return "BC5 SNORM";
    case MALI_BC6H_UF16: return "BC6H UF16";
    case MALI_BC6H_SF16: return "BC6H SF16";
    case MALI_BC7_UNORM: return "BC7 UNORM";
    case MALI_ETC2_R11_SNORM: return "ETC2 R11 SNORM";
    case MALI_ETC2_RG11_SNORM: return "ETC2 RG11 SNORM";
    case MALI_ETC2_RGB8A1: return "ETC2 RGB8A1";
    case MALI_ASTC_3D_LDR: return "ASTC 3D LDR";
    case MALI_ASTC_3D_HDR: return "ASTC 3D HDR";
    case MALI_ASTC_2D_LDR: return "ASTC 2D LDR";
    case MALI_ASTC_2D_HDR: return "ASTC 2D HDR";
    default: return "XXX: INVALID";
    }
}

enum mali_func {
        MALI_FUNC_NEVER                      =      0,
        MALI_FUNC_LESS                       =      1,
        MALI_FUNC_EQUAL                      =      2,
        MALI_FUNC_LEQUAL                     =      3,
        MALI_FUNC_GREATER                    =      4,
        MALI_FUNC_NOT_EQUAL                  =      5,
        MALI_FUNC_GEQUAL                     =      6,
        MALI_FUNC_ALWAYS                     =      7,
};

static inline const char *
mali_func_as_str(enum mali_func imm)
{
    switch (imm) {
    case MALI_FUNC_NEVER: return "Never";
    case MALI_FUNC_LESS: return "Less";
    case MALI_FUNC_EQUAL: return "Equal";
    case MALI_FUNC_LEQUAL: return "Lequal";
    case MALI_FUNC_GREATER: return "Greater";
    case MALI_FUNC_NOT_EQUAL: return "Not Equal";
    case MALI_FUNC_GEQUAL: return "Gequal";
    case MALI_FUNC_ALWAYS: return "Always";
    default: return "XXX: INVALID";
    }
}

enum mali_color_buffer_internal_format {
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW_VALUE =      0,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R8G8B8A8 =      1,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R10G10B10A2 =      2,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R8G8B8A2 =      3,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R4G4B4A4 =      4,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R5G6B5A0 =      5,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_R5G5B5A1 =      6,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW8 =     32,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW16 =     33,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW32 =     34,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW64 =     35,
        MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW128 =     36,
};

static inline const char *
mali_color_buffer_internal_format_as_str(enum mali_color_buffer_internal_format imm)
{
    switch (imm) {
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW_VALUE: return "Raw Value";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R8G8B8A8: return "R8G8B8A8";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R10G10B10A2: return "R10G10B10A2";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R8G8B8A2: return "R8G8B8A2";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R4G4B4A4: return "R4G4B4A4";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R5G6B5A0: return "R5G6B5A0";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_R5G5B5A1: return "R5G5B5A1";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW8: return "RAW8";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW16: return "RAW16";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW32: return "RAW32";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW64: return "RAW64";
    case MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW128: return "RAW128";
    default: return "XXX: INVALID";
    }
}

#include "panfrost-job.h"
#endif
