
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




#define mali_pixel_format_print(fp, format) \
    fprintf(fp, "%*sFormat (v6): %s%s%s %s%s%s%s\n", indent, "", \
        mali_format_as_str((enum mali_format)((format >> 12) & 0xFF)), \
        (format & (1 << 20)) ? " sRGB" : "", \
        (format & (1 << 21)) ? " big-endian" : "", \
        mali_channel_as_str((enum mali_channel)((format >> 0) & 0x7)), \
        mali_channel_as_str((enum mali_channel)((format >> 3) & 0x7)), \
        mali_channel_as_str((enum mali_channel)((format >> 6) & 0x7)), \
        mali_channel_as_str((enum mali_channel)((format >> 9) & 0x7)));


enum mali_attribute_type {
        MALI_ATTRIBUTE_TYPE_1D               =      1,
        MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR   =      2,
        MALI_ATTRIBUTE_TYPE_1D_MODULUS       =      3,
        MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR  =      4,
        MALI_ATTRIBUTE_TYPE_3D_LINEAR        =      5,
        MALI_ATTRIBUTE_TYPE_3D_INTERLEAVED   =      6,
        MALI_ATTRIBUTE_TYPE_1D_PRIMITIVE_INDEX_BUFFER =      7,
        MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR_WRITE_REDUCTION =     10,
        MALI_ATTRIBUTE_TYPE_1D_MODULUS_WRITE_REDUCTION =     11,
        MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR_WRITE_REDUCTION =     12,
        MALI_ATTRIBUTE_TYPE_CONTINUATION     =     32,
};

static inline const char *
mali_attribute_type_as_str(enum mali_attribute_type imm)
{
    switch (imm) {
    case MALI_ATTRIBUTE_TYPE_1D: return "1D";
    case MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR: return "1D POT Divisor";
    case MALI_ATTRIBUTE_TYPE_1D_MODULUS: return "1D Modulus";
    case MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR: return "1D NPOT Divisor";
    case MALI_ATTRIBUTE_TYPE_3D_LINEAR: return "3D Linear";
    case MALI_ATTRIBUTE_TYPE_3D_INTERLEAVED: return "3D Interleaved";
    case MALI_ATTRIBUTE_TYPE_1D_PRIMITIVE_INDEX_BUFFER: return "1D Primitive Index Buffer";
    case MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR_WRITE_REDUCTION: return "1D POT Divisor Write Reduction";
    case MALI_ATTRIBUTE_TYPE_1D_MODULUS_WRITE_REDUCTION: return "1D Modulus Write Reduction";
    case MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR_WRITE_REDUCTION: return "1D NPOT Divisor Write Reduction";
    case MALI_ATTRIBUTE_TYPE_CONTINUATION: return "Continuation";
    default: return "XXX: INVALID";
    }
}

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

enum mali_depth_source {
        MALI_DEPTH_SOURCE_MINIMUM            =      0,
        MALI_DEPTH_SOURCE_MAXIMUM            =      1,
        MALI_DEPTH_SOURCE_FIXED_FUNCTION     =      2,
        MALI_DEPTH_SOURCE_SHADER             =      3,
};

static inline const char *
mali_depth_source_as_str(enum mali_depth_source imm)
{
    switch (imm) {
    case MALI_DEPTH_SOURCE_MINIMUM: return "Minimum";
    case MALI_DEPTH_SOURCE_MAXIMUM: return "Maximum";
    case MALI_DEPTH_SOURCE_FIXED_FUNCTION: return "Fixed function";
    case MALI_DEPTH_SOURCE_SHADER: return "Shader";
    default: return "XXX: INVALID";
    }
}

enum mali_job_type {
        MALI_JOB_TYPE_NOT_STARTED            =      0,
        MALI_JOB_TYPE_NULL                   =      1,
        MALI_JOB_TYPE_WRITE_VALUE            =      2,
        MALI_JOB_TYPE_CACHE_FLUSH            =      3,
        MALI_JOB_TYPE_COMPUTE                =      4,
        MALI_JOB_TYPE_VERTEX                 =      5,
        MALI_JOB_TYPE_GEOMETRY               =      6,
        MALI_JOB_TYPE_TILER                  =      7,
        MALI_JOB_TYPE_FUSED                  =      8,
        MALI_JOB_TYPE_FRAGMENT               =      9,
        MALI_JOB_TYPE_INDEXED_VERTEX         =     10,
};

static inline const char *
mali_job_type_as_str(enum mali_job_type imm)
{
    switch (imm) {
    case MALI_JOB_TYPE_NOT_STARTED: return "Not started";
    case MALI_JOB_TYPE_NULL: return "Null";
    case MALI_JOB_TYPE_WRITE_VALUE: return "Write value";
    case MALI_JOB_TYPE_CACHE_FLUSH: return "Cache flush";
    case MALI_JOB_TYPE_COMPUTE: return "Compute";
    case MALI_JOB_TYPE_VERTEX: return "Vertex";
    case MALI_JOB_TYPE_GEOMETRY: return "Geometry";
    case MALI_JOB_TYPE_TILER: return "Tiler";
    case MALI_JOB_TYPE_FUSED: return "Fused";
    case MALI_JOB_TYPE_FRAGMENT: return "Fragment";
    case MALI_JOB_TYPE_INDEXED_VERTEX: return "Indexed Vertex";
    default: return "XXX: INVALID";
    }
}

enum mali_draw_mode {
        MALI_DRAW_MODE_NONE                  =      0,
        MALI_DRAW_MODE_POINTS                =      1,
        MALI_DRAW_MODE_LINES                 =      2,
        MALI_DRAW_MODE_LINE_STRIP            =      4,
        MALI_DRAW_MODE_LINE_LOOP             =      6,
        MALI_DRAW_MODE_TRIANGLES             =      8,
        MALI_DRAW_MODE_TRIANGLE_STRIP        =     10,
        MALI_DRAW_MODE_TRIANGLE_FAN          =     12,
        MALI_DRAW_MODE_POLYGON               =     13,
        MALI_DRAW_MODE_QUADS                 =     14,
        MALI_DRAW_MODE_QUAD_STRIP            =     15,
};

static inline const char *
mali_draw_mode_as_str(enum mali_draw_mode imm)
{
    switch (imm) {
    case MALI_DRAW_MODE_NONE: return "None";
    case MALI_DRAW_MODE_POINTS: return "Points";
    case MALI_DRAW_MODE_LINES: return "Lines";
    case MALI_DRAW_MODE_LINE_STRIP: return "Line strip";
    case MALI_DRAW_MODE_LINE_LOOP: return "Line loop";
    case MALI_DRAW_MODE_TRIANGLES: return "Triangles";
    case MALI_DRAW_MODE_TRIANGLE_STRIP: return "Triangle strip";
    case MALI_DRAW_MODE_TRIANGLE_FAN: return "Triangle fan";
    case MALI_DRAW_MODE_POLYGON: return "Polygon";
    case MALI_DRAW_MODE_QUADS: return "Quads";
    case MALI_DRAW_MODE_QUAD_STRIP: return "Quad strip";
    default: return "XXX: INVALID";
    }
}

enum mali_exception_access {
        MALI_EXCEPTION_ACCESS_NONE           =      0,
        MALI_EXCEPTION_ACCESS_EXECUTE        =      2,
        MALI_EXCEPTION_ACCESS_READ           =      1,
        MALI_EXCEPTION_ACCESS_WRITE          =      3,
};

static inline const char *
mali_exception_access_as_str(enum mali_exception_access imm)
{
    switch (imm) {
    case MALI_EXCEPTION_ACCESS_NONE: return "None";
    case MALI_EXCEPTION_ACCESS_EXECUTE: return "Execute";
    case MALI_EXCEPTION_ACCESS_READ: return "Read";
    case MALI_EXCEPTION_ACCESS_WRITE: return "Write";
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
        MALI_RGB565                          =     64,
        MALI_RGB5_A1_UNORM                   =     65,
        MALI_A1_BGR5_UNORM                   =     66,
        MALI_RGB10_A2_UNORM                  =     67,
        MALI_A2_BGR10_UNORM                  =     68,
        MALI_RGB10_A2_SNORM                  =     69,
        MALI_A2_BGR10_SNORM                  =     70,
        MALI_RGB10_A2UI                      =     71,
        MALI_A2_BGR10UI                      =     72,
        MALI_RGB10_A2I                       =     73,
        MALI_A2_BGR10I                       =     74,
        MALI_Z24X8_UNORM                     =     77,
        MALI_X8Z24                           =     78,
        MALI_X32_S8X24                       =     79,
        MALI_X24S8_X32                       =     80,
        MALI_R32_FIXED                       =     81,
        MALI_RG32_FIXED                      =     82,
        MALI_RGB32_FIXED                     =     83,
        MALI_RGBA32_FIXED                    =     84,
        MALI_R11F_G11F_B10F                  =     89,
        MALI_B10F_G11F_R11F                  =     90,
        MALI_R9F_G9F_B9F_E5F                 =     91,
        MALI_E5F_B9F_G9F_R9F                 =     92,
        MALI_SNAP_2                          =     93,
        MALI_SNAP_4                          =     94,
        MALI_CONSTANT                        =     95,
        MALI_R1_SNORM                        =     96,
        MALI_R2_SNORM                        =     97,
        MALI_R4_SNORM                        =     98,
        MALI_R8_SNORM                        =     99,
        MALI_R16_SNORM                       =    100,
        MALI_R32_SNORM                       =    101,
        MALI_R64_SNORM                       =    102,
        MALI_RG1_SNORM                       =    104,
        MALI_RG2_SNORM                       =    105,
        MALI_RG4_SNORM                       =    106,
        MALI_RG8_SNORM                       =    107,
        MALI_RG16_SNORM                      =    108,
        MALI_RG32_SNORM                      =    109,
        MALI_RG64_SNORM                      =    110,
        MALI_RGB1_SNORM                      =    112,
        MALI_RGB2_SNORM                      =    113,
        MALI_RGB4_SNORM                      =    114,
        MALI_RGB8_SNORM                      =    115,
        MALI_RGB16_SNORM                     =    116,
        MALI_RGB32_SNORM                     =    117,
        MALI_RGB64_SNORM                     =    118,
        MALI_RGBA1_SNORM                     =    120,
        MALI_RGBA2_SNORM                     =    121,
        MALI_RGBA4_SNORM                     =    122,
        MALI_RGBA8_SNORM                     =    123,
        MALI_RGBA16_SNORM                    =    124,
        MALI_RGBA32_SNORM                    =    125,
        MALI_RGBA64_SNORM                    =    126,
        MALI_R1UI                            =    128,
        MALI_R2UI                            =    129,
        MALI_R4UI                            =    130,
        MALI_R8UI                            =    131,
        MALI_R16UI                           =    132,
        MALI_R32UI                           =    133,
        MALI_R64UI                           =    134,
        MALI_R64F                            =    135,
        MALI_RG1UI                           =    136,
        MALI_RG2UI                           =    137,
        MALI_RG4UI                           =    138,
        MALI_RG8UI                           =    139,
        MALI_RG16UI                          =    140,
        MALI_RG32UI                          =    141,
        MALI_RG64UI                          =    142,
        MALI_RG64F                           =    143,
        MALI_RGB1UI                          =    144,
        MALI_RGB2UI                          =    145,
        MALI_RGB4UI                          =    146,
        MALI_RGB8UI                          =    147,
        MALI_RGB16UI                         =    148,
        MALI_RGB32UI                         =    149,
        MALI_RGB64UI                         =    150,
        MALI_RGB64F                          =    151,
        MALI_RGBA1UI                         =    152,
        MALI_RGBA2UI                         =    153,
        MALI_RGBA4UI                         =    154,
        MALI_RGBA8UI                         =    155,
        MALI_RGBA16UI                        =    156,
        MALI_RGBA32UI                        =    157,
        MALI_RGBA64UI                        =    158,
        MALI_RGBA64F                         =    159,
        MALI_R1_UNORM                        =    160,
        MALI_R2_UNORM                        =    161,
        MALI_R4_UNORM                        =    162,
        MALI_R8_UNORM                        =    163,
        MALI_R16_UNORM                       =    164,
        MALI_R32_UNORM                       =    165,
        MALI_R64_UNORM                       =    166,
        MALI_R32F                            =    167,
        MALI_RG1_UNORM                       =    168,
        MALI_RG2_UNORM                       =    169,
        MALI_RG4_UNORM                       =    170,
        MALI_RG8_UNORM                       =    171,
        MALI_RG16_UNORM                      =    172,
        MALI_RG32_UNORM                      =    173,
        MALI_RG64_UNORM                      =    174,
        MALI_RG32F                           =    175,
        MALI_RGB1_UNORM                      =    176,
        MALI_RGB2_UNORM                      =    177,
        MALI_RGB4_UNORM                      =    178,
        MALI_RGB8_UNORM                      =    179,
        MALI_RGB16_UNORM                     =    180,
        MALI_RGB32_UNORM                     =    181,
        MALI_RGB64_UNORM                     =    182,
        MALI_RGB32F                          =    183,
        MALI_RGBA1_UNORM                     =    184,
        MALI_RGBA2_UNORM                     =    185,
        MALI_RGBA4_UNORM                     =    186,
        MALI_RGBA8_UNORM                     =    187,
        MALI_RGBA16_UNORM                    =    188,
        MALI_RGBA32_UNORM                    =    189,
        MALI_RGBA64_UNORM                    =    190,
        MALI_RGBA32F                         =    191,
        MALI_R1I                             =    192,
        MALI_R2I                             =    193,
        MALI_R4I                             =    194,
        MALI_R8I                             =    195,
        MALI_R16I                            =    196,
        MALI_R32I                            =    197,
        MALI_R64I                            =    198,
        MALI_R16F                            =    199,
        MALI_RG1I                            =    200,
        MALI_RG2I                            =    201,
        MALI_RG4I                            =    202,
        MALI_RG8I                            =    203,
        MALI_RG16I                           =    204,
        MALI_RG32I                           =    205,
        MALI_RG64I                           =    206,
        MALI_RG16F                           =    207,
        MALI_RGB1I                           =    208,
        MALI_RGB2I                           =    209,
        MALI_RGB4I                           =    210,
        MALI_RGB8I                           =    211,
        MALI_RGB16I                          =    212,
        MALI_RGB32I                          =    213,
        MALI_RGB64I                          =    214,
        MALI_RGB16F                          =    215,
        MALI_RGBA1I                          =    216,
        MALI_RGBA2I                          =    217,
        MALI_RGBA4I                          =    218,
        MALI_RGBA8I                          =    219,
        MALI_RGBA16I                         =    220,
        MALI_RGBA32I                         =    221,
        MALI_RGBA64I                         =    222,
        MALI_RGBA16F                         =    223,
        MALI_RGB5_A1_AU                      =    224,
        MALI_RGB5_A1_PU                      =    225,
        MALI_R5G6B5_AU                       =    226,
        MALI_R5G6B5_PU                       =    227,
        MALI_SNAP4_V                         =    230,
        MALI_RGBA4_AU                        =    232,
        MALI_RGBA4_PU                        =    233,
        MALI_RGBA8_TB                        =    237,
        MALI_RGB10_A2_TB                     =    238,
        MALI_TESS_VERTEX_PACK                =    240,
        MALI_RGB8_A2_AU                      =    241,
        MALI_RGB8_A2_PU                      =    242,
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
    case MALI_RGB565: return "RGB565";
    case MALI_RGB5_A1_UNORM: return "RGB5 A1 UNORM";
    case MALI_A1_BGR5_UNORM: return "A1 BGR5 UNORM";
    case MALI_RGB10_A2_UNORM: return "RGB10 A2 UNORM";
    case MALI_A2_BGR10_UNORM: return "A2 BGR10 UNORM";
    case MALI_RGB10_A2_SNORM: return "RGB10 A2 SNORM";
    case MALI_A2_BGR10_SNORM: return "A2 BGR10 SNORM";
    case MALI_RGB10_A2UI: return "RGB10 A2UI";
    case MALI_A2_BGR10UI: return "A2 BGR10UI";
    case MALI_RGB10_A2I: return "RGB10 A2I";
    case MALI_A2_BGR10I: return "A2 BGR10I";
    case MALI_Z24X8_UNORM: return "Z24X8 UNORM";
    case MALI_X8Z24: return "X8Z24";
    case MALI_X32_S8X24: return "X32 S8X24";
    case MALI_X24S8_X32: return "X24S8 X32";
    case MALI_R32_FIXED: return "R32 FIXED";
    case MALI_RG32_FIXED: return "RG32 FIXED";
    case MALI_RGB32_FIXED: return "RGB32 FIXED";
    case MALI_RGBA32_FIXED: return "RGBA32 FIXED";
    case MALI_R11F_G11F_B10F: return "R11F G11F B10F";
    case MALI_B10F_G11F_R11F: return "B10F G11F R11F";
    case MALI_R9F_G9F_B9F_E5F: return "R9F G9F B9F E5F";
    case MALI_E5F_B9F_G9F_R9F: return "E5F B9F G9F R9F";
    case MALI_SNAP_2: return "Snap 2";
    case MALI_SNAP_4: return "Snap 4";
    case MALI_CONSTANT: return "Constant";
    case MALI_R1_SNORM: return "R1 SNORM";
    case MALI_R2_SNORM: return "R2 SNORM";
    case MALI_R4_SNORM: return "R4 SNORM";
    case MALI_R8_SNORM: return "R8 SNORM";
    case MALI_R16_SNORM: return "R16 SNORM";
    case MALI_R32_SNORM: return "R32 SNORM";
    case MALI_R64_SNORM: return "R64 SNORM";
    case MALI_RG1_SNORM: return "RG1 SNORM";
    case MALI_RG2_SNORM: return "RG2 SNORM";
    case MALI_RG4_SNORM: return "RG4 SNORM";
    case MALI_RG8_SNORM: return "RG8 SNORM";
    case MALI_RG16_SNORM: return "RG16 SNORM";
    case MALI_RG32_SNORM: return "RG32 SNORM";
    case MALI_RG64_SNORM: return "RG64 SNORM";
    case MALI_RGB1_SNORM: return "RGB1 SNORM";
    case MALI_RGB2_SNORM: return "RGB2 SNORM";
    case MALI_RGB4_SNORM: return "RGB4 SNORM";
    case MALI_RGB8_SNORM: return "RGB8 SNORM";
    case MALI_RGB16_SNORM: return "RGB16 SNORM";
    case MALI_RGB32_SNORM: return "RGB32 SNORM";
    case MALI_RGB64_SNORM: return "RGB64 SNORM";
    case MALI_RGBA1_SNORM: return "RGBA1 SNORM";
    case MALI_RGBA2_SNORM: return "RGBA2 SNORM";
    case MALI_RGBA4_SNORM: return "RGBA4 SNORM";
    case MALI_RGBA8_SNORM: return "RGBA8 SNORM";
    case MALI_RGBA16_SNORM: return "RGBA16 SNORM";
    case MALI_RGBA32_SNORM: return "RGBA32 SNORM";
    case MALI_RGBA64_SNORM: return "RGBA64 SNORM";
    case MALI_R1UI: return "R1UI";
    case MALI_R2UI: return "R2UI";
    case MALI_R4UI: return "R4UI";
    case MALI_R8UI: return "R8UI";
    case MALI_R16UI: return "R16UI";
    case MALI_R32UI: return "R32UI";
    case MALI_R64UI: return "R64UI";
    case MALI_R64F: return "R64F";
    case MALI_RG1UI: return "RG1UI";
    case MALI_RG2UI: return "RG2UI";
    case MALI_RG4UI: return "RG4UI";
    case MALI_RG8UI: return "RG8UI";
    case MALI_RG16UI: return "RG16UI";
    case MALI_RG32UI: return "RG32UI";
    case MALI_RG64UI: return "RG64UI";
    case MALI_RG64F: return "RG64F";
    case MALI_RGB1UI: return "RGB1UI";
    case MALI_RGB2UI: return "RGB2UI";
    case MALI_RGB4UI: return "RGB4UI";
    case MALI_RGB8UI: return "RGB8UI";
    case MALI_RGB16UI: return "RGB16UI";
    case MALI_RGB32UI: return "RGB32UI";
    case MALI_RGB64UI: return "RGB64UI";
    case MALI_RGB64F: return "RGB64F";
    case MALI_RGBA1UI: return "RGBA1UI";
    case MALI_RGBA2UI: return "RGBA2UI";
    case MALI_RGBA4UI: return "RGBA4UI";
    case MALI_RGBA8UI: return "RGBA8UI";
    case MALI_RGBA16UI: return "RGBA16UI";
    case MALI_RGBA32UI: return "RGBA32UI";
    case MALI_RGBA64UI: return "RGBA64UI";
    case MALI_RGBA64F: return "RGBA64F";
    case MALI_R1_UNORM: return "R1 UNORM";
    case MALI_R2_UNORM: return "R2 UNORM";
    case MALI_R4_UNORM: return "R4 UNORM";
    case MALI_R8_UNORM: return "R8 UNORM";
    case MALI_R16_UNORM: return "R16 UNORM";
    case MALI_R32_UNORM: return "R32 UNORM";
    case MALI_R64_UNORM: return "R64 UNORM";
    case MALI_R32F: return "R32F";
    case MALI_RG1_UNORM: return "RG1 UNORM";
    case MALI_RG2_UNORM: return "RG2 UNORM";
    case MALI_RG4_UNORM: return "RG4 UNORM";
    case MALI_RG8_UNORM: return "RG8 UNORM";
    case MALI_RG16_UNORM: return "RG16 UNORM";
    case MALI_RG32_UNORM: return "RG32 UNORM";
    case MALI_RG64_UNORM: return "RG64 UNORM";
    case MALI_RG32F: return "RG32F";
    case MALI_RGB1_UNORM: return "RGB1 UNORM";
    case MALI_RGB2_UNORM: return "RGB2 UNORM";
    case MALI_RGB4_UNORM: return "RGB4 UNORM";
    case MALI_RGB8_UNORM: return "RGB8 UNORM";
    case MALI_RGB16_UNORM: return "RGB16 UNORM";
    case MALI_RGB32_UNORM: return "RGB32 UNORM";
    case MALI_RGB64_UNORM: return "RGB64 UNORM";
    case MALI_RGB32F: return "RGB32F";
    case MALI_RGBA1_UNORM: return "RGBA1 UNORM";
    case MALI_RGBA2_UNORM: return "RGBA2 UNORM";
    case MALI_RGBA4_UNORM: return "RGBA4 UNORM";
    case MALI_RGBA8_UNORM: return "RGBA8 UNORM";
    case MALI_RGBA16_UNORM: return "RGBA16 UNORM";
    case MALI_RGBA32_UNORM: return "RGBA32 UNORM";
    case MALI_RGBA64_UNORM: return "RGBA64 UNORM";
    case MALI_RGBA32F: return "RGBA32F";
    case MALI_R1I: return "R1I";
    case MALI_R2I: return "R2I";
    case MALI_R4I: return "R4I";
    case MALI_R8I: return "R8I";
    case MALI_R16I: return "R16I";
    case MALI_R32I: return "R32I";
    case MALI_R64I: return "R64I";
    case MALI_R16F: return "R16F";
    case MALI_RG1I: return "RG1I";
    case MALI_RG2I: return "RG2I";
    case MALI_RG4I: return "RG4I";
    case MALI_RG8I: return "RG8I";
    case MALI_RG16I: return "RG16I";
    case MALI_RG32I: return "RG32I";
    case MALI_RG64I: return "RG64I";
    case MALI_RG16F: return "RG16F";
    case MALI_RGB1I: return "RGB1I";
    case MALI_RGB2I: return "RGB2I";
    case MALI_RGB4I: return "RGB4I";
    case MALI_RGB8I: return "RGB8I";
    case MALI_RGB16I: return "RGB16I";
    case MALI_RGB32I: return "RGB32I";
    case MALI_RGB64I: return "RGB64I";
    case MALI_RGB16F: return "RGB16F";
    case MALI_RGBA1I: return "RGBA1I";
    case MALI_RGBA2I: return "RGBA2I";
    case MALI_RGBA4I: return "RGBA4I";
    case MALI_RGBA8I: return "RGBA8I";
    case MALI_RGBA16I: return "RGBA16I";
    case MALI_RGBA32I: return "RGBA32I";
    case MALI_RGBA64I: return "RGBA64I";
    case MALI_RGBA16F: return "RGBA16F";
    case MALI_RGB5_A1_AU: return "RGB5 A1 AU";
    case MALI_RGB5_A1_PU: return "RGB5 A1 PU";
    case MALI_R5G6B5_AU: return "R5G6B5 AU";
    case MALI_R5G6B5_PU: return "R5G6B5 PU";
    case MALI_SNAP4_V: return "Snap4 V";
    case MALI_RGBA4_AU: return "RGBA4 AU";
    case MALI_RGBA4_PU: return "RGBA4 PU";
    case MALI_RGBA8_TB: return "RGBA8 TB";
    case MALI_RGB10_A2_TB: return "RGB10 A2 TB";
    case MALI_TESS_VERTEX_PACK: return "Tess Vertex Pack";
    case MALI_RGB8_A2_AU: return "RGB8 A2 AU";
    case MALI_RGB8_A2_PU: return "RGB8 A2 PU";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_swizzle {
        MALI_YUV_SWIZZLE_YUVA                =      0,
        MALI_YUV_SWIZZLE_YVUA                =      1,
        MALI_YUV_SWIZZLE_UYVA                =      2,
        MALI_YUV_SWIZZLE_UVYA                =      3,
        MALI_YUV_SWIZZLE_VUYA                =      4,
        MALI_YUV_SWIZZLE_VYUA                =      5,
        MALI_YUV_SWIZZLE_Y00A                =      6,
        MALI_YUV_SWIZZLE_YXXA                =      7,
};

static inline const char *
mali_yuv_swizzle_as_str(enum mali_yuv_swizzle imm)
{
    switch (imm) {
    case MALI_YUV_SWIZZLE_YUVA: return "YUVA";
    case MALI_YUV_SWIZZLE_YVUA: return "YVUA";
    case MALI_YUV_SWIZZLE_UYVA: return "UYVA";
    case MALI_YUV_SWIZZLE_UVYA: return "UVYA";
    case MALI_YUV_SWIZZLE_VUYA: return "VUYA";
    case MALI_YUV_SWIZZLE_VYUA: return "VYUA";
    case MALI_YUV_SWIZZLE_Y00A: return "Y00A";
    case MALI_YUV_SWIZZLE_YXXA: return "YXXA";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_conversion_mode {
        MALI_YUV_CONVERSION_MODE_NO_CONVERSION =      0,
        MALI_YUV_CONVERSION_MODE_BT_601      =      3,
        MALI_YUV_CONVERSION_MODE_BT_709      =      4,
        MALI_YUV_CONVERSION_MODE_BT_2020     =      6,
};

static inline const char *
mali_yuv_conversion_mode_as_str(enum mali_yuv_conversion_mode imm)
{
    switch (imm) {
    case MALI_YUV_CONVERSION_MODE_NO_CONVERSION: return "No Conversion";
    case MALI_YUV_CONVERSION_MODE_BT_601: return "BT 601";
    case MALI_YUV_CONVERSION_MODE_BT_709: return "BT 709";
    case MALI_YUV_CONVERSION_MODE_BT_2020: return "BT 2020";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_cr_siting {
        MALI_YUV_CR_SITING_CO_SITED          =      0,
        MALI_YUV_CR_SITING_CENTER_Y          =      1,
        MALI_YUV_CR_SITING_CENTER_X          =      2,
        MALI_YUV_CR_SITING_CENTER            =      3,
        MALI_YUV_CR_SITING_ONE_QUARTER       =      4,
        MALI_YUV_CR_SITING_THREE_QUARTERS    =      5,
        MALI_YUV_CR_SITING_REPLICATED        =      7,
};

static inline const char *
mali_yuv_cr_siting_as_str(enum mali_yuv_cr_siting imm)
{
    switch (imm) {
    case MALI_YUV_CR_SITING_CO_SITED: return "Co-Sited";
    case MALI_YUV_CR_SITING_CENTER_Y: return "Center Y";
    case MALI_YUV_CR_SITING_CENTER_X: return "Center X";
    case MALI_YUV_CR_SITING_CENTER: return "Center";
    case MALI_YUV_CR_SITING_ONE_QUARTER: return "One Quarter";
    case MALI_YUV_CR_SITING_THREE_QUARTERS: return "Three Quarters";
    case MALI_YUV_CR_SITING_REPLICATED: return "Replicated";
    default: return "XXX: INVALID";
    }
}

enum mali_astc_2d_dimension {
        MALI_ASTC_2D_DIMENSION_4             =      0,
        MALI_ASTC_2D_DIMENSION_5             =      1,
        MALI_ASTC_2D_DIMENSION_6             =      2,
        MALI_ASTC_2D_DIMENSION_8             =      4,
        MALI_ASTC_2D_DIMENSION_10            =      6,
        MALI_ASTC_2D_DIMENSION_12            =      7,
};

static inline const char *
mali_astc_2d_dimension_as_str(enum mali_astc_2d_dimension imm)
{
    switch (imm) {
    case MALI_ASTC_2D_DIMENSION_4: return "4";
    case MALI_ASTC_2D_DIMENSION_5: return "5";
    case MALI_ASTC_2D_DIMENSION_6: return "6";
    case MALI_ASTC_2D_DIMENSION_8: return "8";
    case MALI_ASTC_2D_DIMENSION_10: return "10";
    case MALI_ASTC_2D_DIMENSION_12: return "12";
    default: return "XXX: INVALID";
    }
}

enum mali_astc_3d_dimension {
        MALI_ASTC_3D_DIMENSION_4             =      0,
        MALI_ASTC_3D_DIMENSION_5             =      1,
        MALI_ASTC_3D_DIMENSION_6             =      2,
        MALI_ASTC_3D_DIMENSION_3             =      3,
};

static inline const char *
mali_astc_3d_dimension_as_str(enum mali_astc_3d_dimension imm)
{
    switch (imm) {
    case MALI_ASTC_3D_DIMENSION_4: return "4";
    case MALI_ASTC_3D_DIMENSION_5: return "5";
    case MALI_ASTC_3D_DIMENSION_6: return "6";
    case MALI_ASTC_3D_DIMENSION_3: return "3";
    default: return "XXX: INVALID";
    }
}

enum mali_pixel_kill {
        MALI_PIXEL_KILL_FORCE_EARLY          =      0,
        MALI_PIXEL_KILL_STRONG_EARLY         =      1,
        MALI_PIXEL_KILL_WEAK_EARLY           =      2,
        MALI_PIXEL_KILL_FORCE_LATE           =      3,
};

static inline const char *
mali_pixel_kill_as_str(enum mali_pixel_kill imm)
{
    switch (imm) {
    case MALI_PIXEL_KILL_FORCE_EARLY: return "Force Early";
    case MALI_PIXEL_KILL_STRONG_EARLY: return "Strong Early";
    case MALI_PIXEL_KILL_WEAK_EARLY: return "Weak Early";
    case MALI_PIXEL_KILL_FORCE_LATE: return "Force Late";
    default: return "XXX: INVALID";
    }
}

enum mali_block_format {
        MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED =      0,
        MALI_BLOCK_FORMAT_TILED_LINEAR       =      1,
        MALI_BLOCK_FORMAT_LINEAR             =      2,
        MALI_BLOCK_FORMAT_AFBC               =      3,
};

static inline const char *
mali_block_format_as_str(enum mali_block_format imm)
{
    switch (imm) {
    case MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED: return "Tiled U-Interleaved";
    case MALI_BLOCK_FORMAT_TILED_LINEAR: return "Tiled Linear";
    case MALI_BLOCK_FORMAT_LINEAR: return "Linear";
    case MALI_BLOCK_FORMAT_AFBC: return "AFBC";
    default: return "XXX: INVALID";
    }
}

enum mali_mipmap_mode {
        MALI_MIPMAP_MODE_NEAREST             =      0,
        MALI_MIPMAP_MODE_NONE                =      1,
        MALI_MIPMAP_MODE_TRILINEAR           =      3,
};

static inline const char *
mali_mipmap_mode_as_str(enum mali_mipmap_mode imm)
{
    switch (imm) {
    case MALI_MIPMAP_MODE_NEAREST: return "Nearest";
    case MALI_MIPMAP_MODE_NONE: return "None";
    case MALI_MIPMAP_MODE_TRILINEAR: return "Trilinear";
    default: return "XXX: INVALID";
    }
}

enum mali_lod_algorithm {
        MALI_LOD_ALGORITHM_ISOTROPIC         =      0,
        MALI_LOD_ALGORITHM_ANISOTROPIC       =      3,
};

static inline const char *
mali_lod_algorithm_as_str(enum mali_lod_algorithm imm)
{
    switch (imm) {
    case MALI_LOD_ALGORITHM_ISOTROPIC: return "Isotropic";
    case MALI_LOD_ALGORITHM_ANISOTROPIC: return "Anisotropic";
    default: return "XXX: INVALID";
    }
}

enum mali_msaa {
        MALI_MSAA_SINGLE                     =      0,
        MALI_MSAA_AVERAGE                    =      1,
        MALI_MSAA_MULTIPLE                   =      2,
        MALI_MSAA_LAYERED                    =      3,
};

static inline const char *
mali_msaa_as_str(enum mali_msaa imm)
{
    switch (imm) {
    case MALI_MSAA_SINGLE: return "Single";
    case MALI_MSAA_AVERAGE: return "Average";
    case MALI_MSAA_MULTIPLE: return "Multiple";
    case MALI_MSAA_LAYERED: return "Layered";
    default: return "XXX: INVALID";
    }
}

enum mali_index_type {
        MALI_INDEX_TYPE_NONE                 =      0,
        MALI_INDEX_TYPE_UINT8                =      1,
        MALI_INDEX_TYPE_UINT16               =      2,
        MALI_INDEX_TYPE_UINT32               =      3,
};

static inline const char *
mali_index_type_as_str(enum mali_index_type imm)
{
    switch (imm) {
    case MALI_INDEX_TYPE_NONE: return "None";
    case MALI_INDEX_TYPE_UINT8: return "UINT8";
    case MALI_INDEX_TYPE_UINT16: return "UINT16";
    case MALI_INDEX_TYPE_UINT32: return "UINT32";
    default: return "XXX: INVALID";
    }
}

enum mali_occlusion_mode {
        MALI_OCCLUSION_MODE_DISABLED         =      0,
        MALI_OCCLUSION_MODE_PREDICATE        =      1,
        MALI_OCCLUSION_MODE_COUNTER          =      3,
};

static inline const char *
mali_occlusion_mode_as_str(enum mali_occlusion_mode imm)
{
    switch (imm) {
    case MALI_OCCLUSION_MODE_DISABLED: return "Disabled";
    case MALI_OCCLUSION_MODE_PREDICATE: return "Predicate";
    case MALI_OCCLUSION_MODE_COUNTER: return "Counter";
    default: return "XXX: INVALID";
    }
}

enum mali_stencil_op {
        MALI_STENCIL_OP_KEEP                 =      0,
        MALI_STENCIL_OP_REPLACE              =      1,
        MALI_STENCIL_OP_ZERO                 =      2,
        MALI_STENCIL_OP_INVERT               =      3,
        MALI_STENCIL_OP_INCR_WRAP            =      4,
        MALI_STENCIL_OP_DECR_WRAP            =      5,
        MALI_STENCIL_OP_INCR_SAT             =      6,
        MALI_STENCIL_OP_DECR_SAT             =      7,
};

static inline const char *
mali_stencil_op_as_str(enum mali_stencil_op imm)
{
    switch (imm) {
    case MALI_STENCIL_OP_KEEP: return "Keep";
    case MALI_STENCIL_OP_REPLACE: return "Replace";
    case MALI_STENCIL_OP_ZERO: return "Zero";
    case MALI_STENCIL_OP_INVERT: return "Invert";
    case MALI_STENCIL_OP_INCR_WRAP: return "Incr Wrap";
    case MALI_STENCIL_OP_DECR_WRAP: return "Decr Wrap";
    case MALI_STENCIL_OP_INCR_SAT: return "Incr Sat";
    case MALI_STENCIL_OP_DECR_SAT: return "Decr Sat";
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

enum mali_texture_layout {
        MALI_TEXTURE_LAYOUT_TILED            =      1,
        MALI_TEXTURE_LAYOUT_LINEAR           =      2,
        MALI_TEXTURE_LAYOUT_AFBC             =     12,
};

static inline const char *
mali_texture_layout_as_str(enum mali_texture_layout imm)
{
    switch (imm) {
    case MALI_TEXTURE_LAYOUT_TILED: return "Tiled";
    case MALI_TEXTURE_LAYOUT_LINEAR: return "Linear";
    case MALI_TEXTURE_LAYOUT_AFBC: return "AFBC";
    default: return "XXX: INVALID";
    }
}

enum mali_afbc_surface_flag {
        MALI_AFBC_SURFACE_FLAG_YTR           =      1,
        MALI_AFBC_SURFACE_FLAG_SPLIT_BLOCK   =      2,
        MALI_AFBC_SURFACE_FLAG_WIDE_BLOCK    =      4,
        MALI_AFBC_SURFACE_FLAG_TILED_HEADER  =      8,
        MALI_AFBC_SURFACE_FLAG_PREFETCH      =     16,
        MALI_AFBC_SURFACE_FLAG_CHECK_PAYLOAD_RANGE =     32,
};

static inline const char *
mali_afbc_surface_flag_as_str(enum mali_afbc_surface_flag imm)
{
    switch (imm) {
    case MALI_AFBC_SURFACE_FLAG_YTR: return "YTR";
    case MALI_AFBC_SURFACE_FLAG_SPLIT_BLOCK: return "Split Block";
    case MALI_AFBC_SURFACE_FLAG_WIDE_BLOCK: return "Wide Block";
    case MALI_AFBC_SURFACE_FLAG_TILED_HEADER: return "Tiled Header";
    case MALI_AFBC_SURFACE_FLAG_PREFETCH: return "Prefetch";
    case MALI_AFBC_SURFACE_FLAG_CHECK_PAYLOAD_RANGE: return "Check Payload Range";
    default: return "XXX: INVALID";
    }
}

enum mali_wrap_mode {
        MALI_WRAP_MODE_REPEAT                =      8,
        MALI_WRAP_MODE_CLAMP_TO_EDGE         =      9,
        MALI_WRAP_MODE_CLAMP_TO_BORDER       =     11,
        MALI_WRAP_MODE_MIRRORED_REPEAT       =     12,
        MALI_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE =     13,
        MALI_WRAP_MODE_MIRRORED_CLAMP_TO_BORDER =     15,
};

static inline const char *
mali_wrap_mode_as_str(enum mali_wrap_mode imm)
{
    switch (imm) {
    case MALI_WRAP_MODE_REPEAT: return "Repeat";
    case MALI_WRAP_MODE_CLAMP_TO_EDGE: return "Clamp to Edge";
    case MALI_WRAP_MODE_CLAMP_TO_BORDER: return "Clamp to Border";
    case MALI_WRAP_MODE_MIRRORED_REPEAT: return "Mirrored Repeat";
    case MALI_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE: return "Mirrored Clamp to Edge";
    case MALI_WRAP_MODE_MIRRORED_CLAMP_TO_BORDER: return "Mirrored Clamp to Border";
    default: return "XXX: INVALID";
    }
}

struct MALI_ATTRIBUTE {
   uint32_t                             buffer_index;
   bool                                 offset_enable;
   uint32_t                             format;
   int32_t                              offset;
};

#define MALI_ATTRIBUTE_header                   \
   .offset_enable = true

static inline void
MALI_ATTRIBUTE_pack(uint32_t * restrict cl,
                    const struct MALI_ATTRIBUTE * restrict values)
{
   cl[ 0] = __gen_uint(values->buffer_index, 0, 8) |
            __gen_uint(values->offset_enable, 9, 9) |
            __gen_uint(values->format, 10, 31);
   cl[ 1] = __gen_sint(values->offset, 0, 31);
}


#define MALI_ATTRIBUTE_LENGTH 8
#define MALI_ATTRIBUTE_ALIGN 8
struct mali_attribute_packed { uint32_t opaque[2]; };
static inline void
MALI_ATTRIBUTE_unpack(const uint8_t * restrict cl,
                      struct MALI_ATTRIBUTE * restrict values)
{
   values->buffer_index = __gen_unpack_uint(cl, 0, 8);
   values->offset_enable = __gen_unpack_uint(cl, 9, 9);
   values->format = __gen_unpack_uint(cl, 10, 31);
   values->offset = __gen_unpack_sint(cl, 32, 63);
}

static inline void
MALI_ATTRIBUTE_print(FILE *fp, const struct MALI_ATTRIBUTE * values, unsigned indent)
{
   fprintf(fp, "%*sBuffer index: %u\n", indent, "", values->buffer_index);
   fprintf(fp, "%*sOffset enable: %s\n", indent, "", values->offset_enable ? "true" : "false");
   mali_pixel_format_print(fp, values->format);
   fprintf(fp, "%*sOffset: %d\n", indent, "", values->offset);
}

struct MALI_ATTRIBUTE_BUFFER {
   enum mali_attribute_type             type;
   uint64_t                             pointer;
   uint32_t                             stride;
   uint32_t                             size;
   uint32_t                             divisor;
   uint32_t                             divisor_r;
   uint32_t                             divisor_p;
   uint32_t                             divisor_e;
};

#define MALI_ATTRIBUTE_BUFFER_header            \
   .type = MALI_ATTRIBUTE_TYPE_1D,  \
   .divisor = 1

static inline void
MALI_ATTRIBUTE_BUFFER_pack(uint32_t * restrict cl,
                           const struct MALI_ATTRIBUTE_BUFFER * restrict values)
{
   assert((values->pointer & 0x3f) == 0);
   cl[ 0] = __gen_uint(values->type, 0, 5) |
            __gen_uint(values->pointer >> 6, 6, 55);
   cl[ 1] = __gen_uint(values->pointer >> 6, 6, 55) >> 32 |
            __gen_padded(values->divisor, 24, 31) |
            __gen_uint(values->divisor_r, 24, 28) |
            __gen_uint(values->divisor_p, 29, 31) |
            __gen_uint(values->divisor_e, 29, 29);
   cl[ 2] = __gen_uint(values->stride, 0, 31);
   cl[ 3] = __gen_uint(values->size, 0, 31);
}


#define MALI_ATTRIBUTE_BUFFER_LENGTH 16
#define MALI_ATTRIBUTE_BUFFER_ALIGN 32
struct mali_attribute_buffer_packed { uint32_t opaque[4]; };
static inline void
MALI_ATTRIBUTE_BUFFER_unpack(const uint8_t * restrict cl,
                             struct MALI_ATTRIBUTE_BUFFER * restrict values)
{
   values->type = (enum mali_attribute_type)__gen_unpack_uint(cl, 0, 5);
   values->pointer = __gen_unpack_uint(cl, 6, 55) << 6;
   values->stride = __gen_unpack_uint(cl, 64, 95);
   values->size = __gen_unpack_uint(cl, 96, 127);
   values->divisor = __gen_unpack_padded(cl, 56, 63);
   values->divisor_r = __gen_unpack_uint(cl, 56, 60);
   values->divisor_p = __gen_unpack_uint(cl, 61, 63);
   values->divisor_e = __gen_unpack_uint(cl, 61, 61);
}

static inline void
MALI_ATTRIBUTE_BUFFER_print(FILE *fp, const struct MALI_ATTRIBUTE_BUFFER * values, unsigned indent)
{
   fprintf(fp, "%*sType: %s\n", indent, "", mali_attribute_type_as_str(values->type));
   fprintf(fp, "%*sPointer: 0x%" PRIx64 "\n", indent, "", values->pointer);
   fprintf(fp, "%*sStride: %u\n", indent, "", values->stride);
   fprintf(fp, "%*sSize: %u\n", indent, "", values->size);
   fprintf(fp, "%*sDivisor: %u\n", indent, "", values->divisor);
   fprintf(fp, "%*sDivisor R: %u\n", indent, "", values->divisor_r);
   fprintf(fp, "%*sDivisor P: %u\n", indent, "", values->divisor_p);
   fprintf(fp, "%*sDivisor E: %u\n", indent, "", values->divisor_e);
}

struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT {
   enum mali_attribute_type             type;
   uint32_t                             divisor_numerator;
   uint32_t                             divisor;
};

#define MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT_header\
   .type = MALI_ATTRIBUTE_TYPE_CONTINUATION

static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT_pack(uint32_t * restrict cl,
                                             const struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT * restrict values)
{
   cl[ 0] = __gen_uint(values->type, 0, 5);
   cl[ 1] = __gen_uint(values->divisor_numerator, 0, 31);
   cl[ 2] = 0;
   cl[ 3] = __gen_uint(values->divisor, 0, 31);
}


#define MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT_LENGTH 16
struct mali_attribute_buffer_continuation_npot_packed { uint32_t opaque[4]; };
static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT_unpack(const uint8_t * restrict cl,
                                               struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffc0) fprintf(stderr, "XXX: Invalid field of Attribute Buffer Continuation NPOT unpacked at word 0\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Attribute Buffer Continuation NPOT unpacked at word 2\n");
   values->type = (enum mali_attribute_type)__gen_unpack_uint(cl, 0, 5);
   values->divisor_numerator = __gen_unpack_uint(cl, 32, 63);
   values->divisor = __gen_unpack_uint(cl, 96, 127);
}

static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT_print(FILE *fp, const struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_NPOT * values, unsigned indent)
{
   fprintf(fp, "%*sType: %s\n", indent, "", mali_attribute_type_as_str(values->type));
   fprintf(fp, "%*sDivisor Numerator: %u\n", indent, "", values->divisor_numerator);
   fprintf(fp, "%*sDivisor: %u\n", indent, "", values->divisor);
}

struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D {
   enum mali_attribute_type             type;
   uint32_t                             s_dimension;
   uint32_t                             t_dimension;
   uint32_t                             r_dimension;
   uint32_t                             row_stride;
   uint32_t                             slice_stride;
};

#define MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D_header\
   .type = MALI_ATTRIBUTE_TYPE_CONTINUATION

static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D_pack(uint32_t * restrict cl,
                                           const struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D * restrict values)
{
   assert(values->s_dimension >= 1);
   assert(values->t_dimension >= 1);
   assert(values->r_dimension >= 1);
   cl[ 0] = __gen_uint(values->type, 0, 5) |
            __gen_uint(values->s_dimension - 1, 16, 31);
   cl[ 1] = __gen_uint(values->t_dimension - 1, 0, 15) |
            __gen_uint(values->r_dimension - 1, 16, 31);
   cl[ 2] = __gen_uint(values->row_stride, 0, 31);
   cl[ 3] = __gen_uint(values->slice_stride, 0, 31);
}


#define MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D_LENGTH 16
struct mali_attribute_buffer_continuation_3d_packed { uint32_t opaque[4]; };
static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D_unpack(const uint8_t * restrict cl,
                                             struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffc0) fprintf(stderr, "XXX: Invalid field of Attribute Buffer Continuation 3D unpacked at word 0\n");
   values->type = (enum mali_attribute_type)__gen_unpack_uint(cl, 0, 5);
   values->s_dimension = __gen_unpack_uint(cl, 16, 31) + 1;
   values->t_dimension = __gen_unpack_uint(cl, 32, 47) + 1;
   values->r_dimension = __gen_unpack_uint(cl, 48, 63) + 1;
   values->row_stride = __gen_unpack_uint(cl, 64, 95);
   values->slice_stride = __gen_unpack_uint(cl, 96, 127);
}

static inline void
MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D_print(FILE *fp, const struct MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D * values, unsigned indent)
{
   fprintf(fp, "%*sType: %s\n", indent, "", mali_attribute_type_as_str(values->type));
   fprintf(fp, "%*sS dimension: %u\n", indent, "", values->s_dimension);
   fprintf(fp, "%*sT dimension: %u\n", indent, "", values->t_dimension);
   fprintf(fp, "%*sR dimension: %u\n", indent, "", values->r_dimension);
   fprintf(fp, "%*sRow Stride: %u\n", indent, "", values->row_stride);
   fprintf(fp, "%*sSlice Stride: %u\n", indent, "", values->slice_stride);
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

enum mali_register_file_format {
        MALI_REGISTER_FILE_FORMAT_F16        =      0,
        MALI_REGISTER_FILE_FORMAT_F32        =      1,
        MALI_REGISTER_FILE_FORMAT_I32        =      2,
        MALI_REGISTER_FILE_FORMAT_U32        =      3,
        MALI_REGISTER_FILE_FORMAT_I16        =      4,
        MALI_REGISTER_FILE_FORMAT_U16        =      5,
};

static inline const char *
mali_register_file_format_as_str(enum mali_register_file_format imm)
{
    switch (imm) {
    case MALI_REGISTER_FILE_FORMAT_F16: return "F16";
    case MALI_REGISTER_FILE_FORMAT_F32: return "F32";
    case MALI_REGISTER_FILE_FORMAT_I32: return "I32";
    case MALI_REGISTER_FILE_FORMAT_U32: return "U32";
    case MALI_REGISTER_FILE_FORMAT_I16: return "I16";
    case MALI_REGISTER_FILE_FORMAT_U16: return "U16";
    default: return "XXX: INVALID";
    }
}

enum mali_message_type {
        MALI_MESSAGE_TYPE_DISABLED           =      0,
        MALI_MESSAGE_TYPE_LD_VAR             =      1,
        MALI_MESSAGE_TYPE_VAR_TEX            =      2,
};

static inline const char *
mali_message_type_as_str(enum mali_message_type imm)
{
    switch (imm) {
    case MALI_MESSAGE_TYPE_DISABLED: return "Disabled";
    case MALI_MESSAGE_TYPE_LD_VAR: return "LD_VAR";
    case MALI_MESSAGE_TYPE_VAR_TEX: return "VAR_TEX";
    default: return "XXX: INVALID";
    }
}

enum mali_message_preload_register_format {
        MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F32 =      0,
        MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F16 =      1,
};

static inline const char *
mali_message_preload_register_format_as_str(enum mali_message_preload_register_format imm)
{
    switch (imm) {
    case MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F32: return "F32";
    case MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F16: return "F16";
    default: return "XXX: INVALID";
    }
}

enum mali_blend_mode {
        MALI_BLEND_MODE_SHADER               =      0,
        MALI_BLEND_MODE_OPAQUE               =      1,
        MALI_BLEND_MODE_FIXED_FUNCTION       =      2,
        MALI_BLEND_MODE_OFF                  =      3,
};

static inline const char *
mali_blend_mode_as_str(enum mali_blend_mode imm)
{
    switch (imm) {
    case MALI_BLEND_MODE_SHADER: return "Shader";
    case MALI_BLEND_MODE_OPAQUE: return "Opaque";
    case MALI_BLEND_MODE_FIXED_FUNCTION: return "Fixed-Function";
    case MALI_BLEND_MODE_OFF: return "Off";
    default: return "XXX: INVALID";
    }
}

struct MALI_BLEND_SHADER {
   uint32_t                             return_value;
   uint32_t                             pc;
};

#define MALI_BLEND_SHADER_header                \
   0

static inline void
MALI_BLEND_SHADER_pack(uint32_t * restrict cl,
                       const struct MALI_BLEND_SHADER * restrict values)
{
   assert((values->return_value & 0x7) == 0);
   assert((values->pc & 0xf) == 0);
   cl[ 0] = __gen_uint(values->return_value >> 3, 3, 31);
   cl[ 1] = __gen_uint(values->pc >> 4, 4, 31);
}


#define MALI_BLEND_SHADER_LENGTH 8
struct mali_blend_shader_packed { uint32_t opaque[2]; };
static inline void
MALI_BLEND_SHADER_unpack(const uint8_t * restrict cl,
                         struct MALI_BLEND_SHADER * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0x7) fprintf(stderr, "XXX: Invalid field of Blend Shader unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xf) fprintf(stderr, "XXX: Invalid field of Blend Shader unpacked at word 1\n");
   values->return_value = __gen_unpack_uint(cl, 3, 31) << 3;
   values->pc = __gen_unpack_uint(cl, 36, 63) << 4;
}

static inline void
MALI_BLEND_SHADER_print(FILE *fp, const struct MALI_BLEND_SHADER * values, unsigned indent)
{
   fprintf(fp, "%*sReturn Value: %u\n", indent, "", values->return_value);
   fprintf(fp, "%*sPC: %u\n", indent, "", values->pc);
}

struct MALI_INTERNAL_CONVERSION {
   uint32_t                             memory_format;
   bool                                 raw;
   enum mali_register_file_format       register_format;
};

#define MALI_INTERNAL_CONVERSION_header         \
   0

static inline void
MALI_INTERNAL_CONVERSION_pack(uint32_t * restrict cl,
                              const struct MALI_INTERNAL_CONVERSION * restrict values)
{
   cl[ 0] = __gen_uint(values->memory_format, 0, 21) |
            __gen_uint(values->raw, 22, 22) |
            __gen_uint(values->register_format, 24, 26);
}


#define MALI_INTERNAL_CONVERSION_LENGTH 4
struct mali_internal_conversion_packed { uint32_t opaque[1]; };
static inline void
MALI_INTERNAL_CONVERSION_unpack(const uint8_t * restrict cl,
                                struct MALI_INTERNAL_CONVERSION * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf8800000) fprintf(stderr, "XXX: Invalid field of Internal Conversion unpacked at word 0\n");
   values->memory_format = __gen_unpack_uint(cl, 0, 21);
   values->raw = __gen_unpack_uint(cl, 22, 22);
   values->register_format = (enum mali_register_file_format)__gen_unpack_uint(cl, 24, 26);
}

static inline void
MALI_INTERNAL_CONVERSION_print(FILE *fp, const struct MALI_INTERNAL_CONVERSION * values, unsigned indent)
{
   mali_pixel_format_print(fp, values->memory_format);
   fprintf(fp, "%*sRaw: %s\n", indent, "", values->raw ? "true" : "false");
   fprintf(fp, "%*sRegister Format: %s\n", indent, "", mali_register_file_format_as_str(values->register_format));
}

struct MALI_BLEND_FIXED_FUNCTION {
   uint32_t                             num_comps;
   bool                                 alpha_zero_nop;
   bool                                 alpha_one_store;
   uint32_t                             rt;
#define MALI_BIFROST_BLEND_MAX_RT                8
   struct MALI_INTERNAL_CONVERSION      conversion;
};

#define MALI_BLEND_FIXED_FUNCTION_header        \
   .num_comps = 1,  \
   .conversion = { MALI_INTERNAL_CONVERSION_header }

static inline void
MALI_BLEND_FIXED_FUNCTION_pack(uint32_t * restrict cl,
                               const struct MALI_BLEND_FIXED_FUNCTION * restrict values)
{
   assert(values->num_comps >= 1);
   cl[ 0] = __gen_uint(values->num_comps - 1, 3, 4) |
            __gen_uint(values->alpha_zero_nop, 5, 5) |
            __gen_uint(values->alpha_one_store, 6, 6) |
            __gen_uint(values->rt, 16, 19);
   cl[ 1] = __gen_uint(values->conversion.memory_format, 0, 21) |
            __gen_uint(values->conversion.raw, 22, 22) |
            __gen_uint(values->conversion.register_format, 24, 26);
}


#define MALI_BLEND_FIXED_FUNCTION_LENGTH 8
struct mali_blend_fixed_function_packed { uint32_t opaque[2]; };
static inline void
MALI_BLEND_FIXED_FUNCTION_unpack(const uint8_t * restrict cl,
                                 struct MALI_BLEND_FIXED_FUNCTION * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xfff0ff87) fprintf(stderr, "XXX: Invalid field of Blend Fixed-Function unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xf8800000) fprintf(stderr, "XXX: Invalid field of Blend Fixed-Function unpacked at word 1\n");
   values->num_comps = __gen_unpack_uint(cl, 3, 4) + 1;
   values->alpha_zero_nop = __gen_unpack_uint(cl, 5, 5);
   values->alpha_one_store = __gen_unpack_uint(cl, 6, 6);
   values->rt = __gen_unpack_uint(cl, 16, 19);
   values->conversion.memory_format = __gen_unpack_uint(cl, 32, 53);
   values->conversion.raw = __gen_unpack_uint(cl, 54, 54);
   values->conversion.register_format = (enum mali_register_file_format)__gen_unpack_uint(cl, 56, 58);
}

static inline void
MALI_BLEND_FIXED_FUNCTION_print(FILE *fp, const struct MALI_BLEND_FIXED_FUNCTION * values, unsigned indent)
{
   fprintf(fp, "%*sNum Comps: %u\n", indent, "", values->num_comps);
   fprintf(fp, "%*sAlpha Zero NOP: %s\n", indent, "", values->alpha_zero_nop ? "true" : "false");
   fprintf(fp, "%*sAlpha One Store: %s\n", indent, "", values->alpha_one_store ? "true" : "false");
   fprintf(fp, "%*sRT: %u\n", indent, "", values->rt);
   fprintf(fp, "%*sConversion:\n", indent, "");
   MALI_INTERNAL_CONVERSION_print(fp, &values->conversion, indent + 2);
}

struct MALI_INTERNAL_BLEND {
   enum mali_blend_mode                 mode;
   struct MALI_BLEND_SHADER             shader;
   struct MALI_BLEND_FIXED_FUNCTION     fixed_function;
};

#define MALI_INTERNAL_BLEND_header              \
   .shader = { MALI_BLEND_SHADER_header },  \
   .fixed_function = { MALI_BLEND_FIXED_FUNCTION_header }

static inline void
MALI_INTERNAL_BLEND_pack(uint32_t * restrict cl,
                         const struct MALI_INTERNAL_BLEND * restrict values)
{
   cl[ 0] = __gen_uint(values->mode, 0, 1) |
            __gen_uint(values->shader.return_value >> 3, 3, 31) |
            __gen_uint(values->fixed_function.num_comps - 1, 3, 4) |
            __gen_uint(values->fixed_function.alpha_zero_nop, 5, 5) |
            __gen_uint(values->fixed_function.alpha_one_store, 6, 6) |
            __gen_uint(values->fixed_function.rt, 16, 19);
   cl[ 1] = __gen_uint(values->shader.pc >> 4, 4, 31) |
            __gen_uint(values->fixed_function.conversion.memory_format, 0, 21) |
            __gen_uint(values->fixed_function.conversion.raw, 22, 22) |
            __gen_uint(values->fixed_function.conversion.register_format, 24, 26);
}


#define MALI_INTERNAL_BLEND_LENGTH 8
struct mali_internal_blend_packed { uint32_t opaque[2]; };
static inline void
MALI_INTERNAL_BLEND_unpack(const uint8_t * restrict cl,
                           struct MALI_INTERNAL_BLEND * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0x4) fprintf(stderr, "XXX: Invalid field of Internal Blend unpacked at word 0\n");
   values->mode = (enum mali_blend_mode)__gen_unpack_uint(cl, 0, 1);
   values->shader.return_value = __gen_unpack_uint(cl, 3, 31) << 3;
   values->shader.pc = __gen_unpack_uint(cl, 36, 63) << 4;
   values->fixed_function.num_comps = __gen_unpack_uint(cl, 3, 4) + 1;
   values->fixed_function.alpha_zero_nop = __gen_unpack_uint(cl, 5, 5);
   values->fixed_function.alpha_one_store = __gen_unpack_uint(cl, 6, 6);
   values->fixed_function.rt = __gen_unpack_uint(cl, 16, 19);
   values->fixed_function.conversion.memory_format = __gen_unpack_uint(cl, 32, 53);
   values->fixed_function.conversion.raw = __gen_unpack_uint(cl, 54, 54);
   values->fixed_function.conversion.register_format = (enum mali_register_file_format)__gen_unpack_uint(cl, 56, 58);
}

static inline void
MALI_INTERNAL_BLEND_print(FILE *fp, const struct MALI_INTERNAL_BLEND * values, unsigned indent)
{
   fprintf(fp, "%*sMode: %s\n", indent, "", mali_blend_mode_as_str(values->mode));
   fprintf(fp, "%*sShader:\n", indent, "");
   MALI_BLEND_SHADER_print(fp, &values->shader, indent + 2);
   fprintf(fp, "%*sFixed-Function:\n", indent, "");
   MALI_BLEND_FIXED_FUNCTION_print(fp, &values->fixed_function, indent + 2);
}

struct MALI_BLEND {
   bool                                 load_destination;
   bool                                 alpha_to_one;
   bool                                 enable;
   bool                                 srgb;
   bool                                 round_to_fb_precision;
   uint32_t                             constant;
   struct MALI_BLEND_EQUATION           equation;
   struct MALI_INTERNAL_BLEND           internal;
};

#define MALI_BLEND_header                       \
   .load_destination = false,  \
   .enable = true,  \
   .srgb = false,  \
   .round_to_fb_precision = false,  \
   .equation = { MALI_BLEND_EQUATION_header },  \
   .internal = { MALI_INTERNAL_BLEND_header }

static inline void
MALI_BLEND_pack(uint32_t * restrict cl,
                const struct MALI_BLEND * restrict values)
{
   cl[ 0] = __gen_uint(values->load_destination, 0, 0) |
            __gen_uint(values->alpha_to_one, 8, 8) |
            __gen_uint(values->enable, 9, 9) |
            __gen_uint(values->srgb, 10, 10) |
            __gen_uint(values->round_to_fb_precision, 11, 11) |
            __gen_uint(values->constant, 16, 31);
   cl[ 1] = __gen_uint(values->equation.rgb.a, 0, 1) |
            __gen_uint(values->equation.rgb.negate_a, 3, 3) |
            __gen_uint(values->equation.rgb.b, 4, 5) |
            __gen_uint(values->equation.rgb.negate_b, 7, 7) |
            __gen_uint(values->equation.rgb.c, 8, 10) |
            __gen_uint(values->equation.rgb.invert_c, 11, 11) |
            __gen_uint(values->equation.alpha.a, 12, 13) |
            __gen_uint(values->equation.alpha.negate_a, 15, 15) |
            __gen_uint(values->equation.alpha.b, 16, 17) |
            __gen_uint(values->equation.alpha.negate_b, 19, 19) |
            __gen_uint(values->equation.alpha.c, 20, 22) |
            __gen_uint(values->equation.alpha.invert_c, 23, 23) |
            __gen_uint(values->equation.color_mask, 28, 31);
   cl[ 2] = __gen_uint(values->internal.mode, 0, 1) |
            __gen_uint(values->internal.shader.return_value >> 3, 3, 31) |
            __gen_uint(values->internal.fixed_function.num_comps - 1, 3, 4) |
            __gen_uint(values->internal.fixed_function.alpha_zero_nop, 5, 5) |
            __gen_uint(values->internal.fixed_function.alpha_one_store, 6, 6) |
            __gen_uint(values->internal.fixed_function.rt, 16, 19);
   cl[ 3] = __gen_uint(values->internal.shader.pc >> 4, 4, 31) |
            __gen_uint(values->internal.fixed_function.conversion.memory_format, 0, 21) |
            __gen_uint(values->internal.fixed_function.conversion.raw, 22, 22) |
            __gen_uint(values->internal.fixed_function.conversion.register_format, 24, 26);
}


#define MALI_BLEND_LENGTH 16
#define MALI_BLEND_ALIGN 16
struct mali_blend_packed { uint32_t opaque[4]; };
static inline void
MALI_BLEND_unpack(const uint8_t * restrict cl,
                  struct MALI_BLEND * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf0fe) fprintf(stderr, "XXX: Invalid field of Blend unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xf044044) fprintf(stderr, "XXX: Invalid field of Blend unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0x4) fprintf(stderr, "XXX: Invalid field of Blend unpacked at word 2\n");
   values->load_destination = __gen_unpack_uint(cl, 0, 0);
   values->alpha_to_one = __gen_unpack_uint(cl, 8, 8);
   values->enable = __gen_unpack_uint(cl, 9, 9);
   values->srgb = __gen_unpack_uint(cl, 10, 10);
   values->round_to_fb_precision = __gen_unpack_uint(cl, 11, 11);
   values->constant = __gen_unpack_uint(cl, 16, 31);
   values->equation.rgb.a = (enum mali_blend_operand_a)__gen_unpack_uint(cl, 32, 33);
   values->equation.rgb.negate_a = __gen_unpack_uint(cl, 35, 35);
   values->equation.rgb.b = (enum mali_blend_operand_b)__gen_unpack_uint(cl, 36, 37);
   values->equation.rgb.negate_b = __gen_unpack_uint(cl, 39, 39);
   values->equation.rgb.c = (enum mali_blend_operand_c)__gen_unpack_uint(cl, 40, 42);
   values->equation.rgb.invert_c = __gen_unpack_uint(cl, 43, 43);
   values->equation.alpha.a = (enum mali_blend_operand_a)__gen_unpack_uint(cl, 44, 45);
   values->equation.alpha.negate_a = __gen_unpack_uint(cl, 47, 47);
   values->equation.alpha.b = (enum mali_blend_operand_b)__gen_unpack_uint(cl, 48, 49);
   values->equation.alpha.negate_b = __gen_unpack_uint(cl, 51, 51);
   values->equation.alpha.c = (enum mali_blend_operand_c)__gen_unpack_uint(cl, 52, 54);
   values->equation.alpha.invert_c = __gen_unpack_uint(cl, 55, 55);
   values->equation.color_mask = __gen_unpack_uint(cl, 60, 63);
   values->internal.mode = (enum mali_blend_mode)__gen_unpack_uint(cl, 64, 65);
   values->internal.shader.return_value = __gen_unpack_uint(cl, 67, 95) << 3;
   values->internal.shader.pc = __gen_unpack_uint(cl, 100, 127) << 4;
   values->internal.fixed_function.num_comps = __gen_unpack_uint(cl, 67, 68) + 1;
   values->internal.fixed_function.alpha_zero_nop = __gen_unpack_uint(cl, 69, 69);
   values->internal.fixed_function.alpha_one_store = __gen_unpack_uint(cl, 70, 70);
   values->internal.fixed_function.rt = __gen_unpack_uint(cl, 80, 83);
   values->internal.fixed_function.conversion.memory_format = __gen_unpack_uint(cl, 96, 117);
   values->internal.fixed_function.conversion.raw = __gen_unpack_uint(cl, 118, 118);
   values->internal.fixed_function.conversion.register_format = (enum mali_register_file_format)__gen_unpack_uint(cl, 120, 122);
}

static inline void
MALI_BLEND_print(FILE *fp, const struct MALI_BLEND * values, unsigned indent)
{
   fprintf(fp, "%*sLoad Destination: %s\n", indent, "", values->load_destination ? "true" : "false");
   fprintf(fp, "%*sAlpha To One: %s\n", indent, "", values->alpha_to_one ? "true" : "false");
   fprintf(fp, "%*sEnable: %s\n", indent, "", values->enable ? "true" : "false");
   fprintf(fp, "%*ssRGB: %s\n", indent, "", values->srgb ? "true" : "false");
   fprintf(fp, "%*sRound to FB precision: %s\n", indent, "", values->round_to_fb_precision ? "true" : "false");
   fprintf(fp, "%*sConstant: %u\n", indent, "", values->constant);
   fprintf(fp, "%*sEquation:\n", indent, "");
   MALI_BLEND_EQUATION_print(fp, &values->equation, indent + 2);
   fprintf(fp, "%*sInternal:\n", indent, "");
   MALI_INTERNAL_BLEND_print(fp, &values->internal, indent + 2);
}

struct MALI_INVOCATION {
   uint32_t                             invocations;
   uint32_t                             size_y_shift;
   uint32_t                             size_z_shift;
   uint32_t                             workgroups_x_shift;
   uint32_t                             workgroups_y_shift;
   uint32_t                             workgroups_z_shift;
   uint32_t                             thread_group_split;
#define MALI_SPLIT_MIN_EFFICIENT                 2
};

#define MALI_INVOCATION_header                  \
   0

static inline void
MALI_INVOCATION_pack(uint32_t * restrict cl,
                     const struct MALI_INVOCATION * restrict values)
{
   cl[ 0] = __gen_uint(values->invocations, 0, 31);
   cl[ 1] = __gen_uint(values->size_y_shift, 0, 4) |
            __gen_uint(values->size_z_shift, 5, 9) |
            __gen_uint(values->workgroups_x_shift, 10, 15) |
            __gen_uint(values->workgroups_y_shift, 16, 21) |
            __gen_uint(values->workgroups_z_shift, 22, 27) |
            __gen_uint(values->thread_group_split, 28, 31);
}


#define MALI_INVOCATION_LENGTH 8
struct mali_invocation_packed { uint32_t opaque[2]; };
static inline void
MALI_INVOCATION_unpack(const uint8_t * restrict cl,
                       struct MALI_INVOCATION * restrict values)
{
   values->invocations = __gen_unpack_uint(cl, 0, 31);
   values->size_y_shift = __gen_unpack_uint(cl, 32, 36);
   values->size_z_shift = __gen_unpack_uint(cl, 37, 41);
   values->workgroups_x_shift = __gen_unpack_uint(cl, 42, 47);
   values->workgroups_y_shift = __gen_unpack_uint(cl, 48, 53);
   values->workgroups_z_shift = __gen_unpack_uint(cl, 54, 59);
   values->thread_group_split = __gen_unpack_uint(cl, 60, 63);
}

static inline void
MALI_INVOCATION_print(FILE *fp, const struct MALI_INVOCATION * values, unsigned indent)
{
   fprintf(fp, "%*sInvocations: %u\n", indent, "", values->invocations);
   fprintf(fp, "%*sSize Y shift: %u\n", indent, "", values->size_y_shift);
   fprintf(fp, "%*sSize Z shift: %u\n", indent, "", values->size_z_shift);
   fprintf(fp, "%*sWorkgroups X shift: %u\n", indent, "", values->workgroups_x_shift);
   fprintf(fp, "%*sWorkgroups Y shift: %u\n", indent, "", values->workgroups_y_shift);
   fprintf(fp, "%*sWorkgroups Z shift: %u\n", indent, "", values->workgroups_z_shift);
   fprintf(fp, "%*sThread group split: %u\n", indent, "", values->thread_group_split);
}

enum mali_point_size_array_format {
        MALI_POINT_SIZE_ARRAY_FORMAT_NONE    =      0,
        MALI_POINT_SIZE_ARRAY_FORMAT_FP16    =      2,
        MALI_POINT_SIZE_ARRAY_FORMAT_FP32    =      3,
};

static inline const char *
mali_point_size_array_format_as_str(enum mali_point_size_array_format imm)
{
    switch (imm) {
    case MALI_POINT_SIZE_ARRAY_FORMAT_NONE: return "None";
    case MALI_POINT_SIZE_ARRAY_FORMAT_FP16: return "FP16";
    case MALI_POINT_SIZE_ARRAY_FORMAT_FP32: return "FP32";
    default: return "XXX: INVALID";
    }
}

enum mali_primitive_restart {
        MALI_PRIMITIVE_RESTART_NONE          =      0,
        MALI_PRIMITIVE_RESTART_IMPLICIT      =      2,
        MALI_PRIMITIVE_RESTART_EXPLICIT      =      3,
};

static inline const char *
mali_primitive_restart_as_str(enum mali_primitive_restart imm)
{
    switch (imm) {
    case MALI_PRIMITIVE_RESTART_NONE: return "None";
    case MALI_PRIMITIVE_RESTART_IMPLICIT: return "Implicit";
    case MALI_PRIMITIVE_RESTART_EXPLICIT: return "Explicit";
    default: return "XXX: INVALID";
    }
}

struct MALI_PRIMITIVE {
   enum mali_draw_mode                  draw_mode;
   enum mali_index_type                 index_type;
   enum mali_point_size_array_format    point_size_array_format;
   bool                                 primitive_index_enable;
   bool                                 primitive_index_writeback;
   bool                                 first_provoking_vertex;
   bool                                 low_depth_cull;
   bool                                 high_depth_cull;
   bool                                 secondary_shader;
   enum mali_primitive_restart          primitive_restart;
   uint32_t                             job_task_split;
   uint32_t                             base_vertex_offset;
   uint32_t                             primitive_restart_index;
   uint32_t                             index_count;
   uint64_t                             indices;
};

#define MALI_PRIMITIVE_header                   \
   .draw_mode = MALI_DRAW_MODE_NONE,  \
   .index_type = MALI_INDEX_TYPE_NONE,  \
   .first_provoking_vertex = true,  \
   .low_depth_cull = true,  \
   .high_depth_cull = true

static inline void
MALI_PRIMITIVE_pack(uint32_t * restrict cl,
                    const struct MALI_PRIMITIVE * restrict values)
{
   assert(values->index_count >= 1);
   cl[ 0] = __gen_uint(values->draw_mode, 0, 7) |
            __gen_uint(values->index_type, 8, 10) |
            __gen_uint(values->point_size_array_format, 11, 12) |
            __gen_uint(values->primitive_index_enable, 13, 13) |
            __gen_uint(values->primitive_index_writeback, 14, 14) |
            __gen_uint(values->first_provoking_vertex, 15, 15) |
            __gen_uint(values->low_depth_cull, 16, 16) |
            __gen_uint(values->high_depth_cull, 17, 17) |
            __gen_uint(values->secondary_shader, 18, 18) |
            __gen_uint(values->primitive_restart, 19, 20) |
            __gen_uint(values->job_task_split, 26, 31);
   cl[ 1] = __gen_uint(values->base_vertex_offset, 0, 31);
   cl[ 2] = __gen_uint(values->primitive_restart_index, 0, 31);
   cl[ 3] = __gen_uint(values->index_count - 1, 0, 31);
   cl[ 4] = __gen_uint(values->indices, 0, 63);
   cl[ 5] = __gen_uint(values->indices, 0, 63) >> 32;
}


#define MALI_PRIMITIVE_LENGTH 24
struct mali_primitive_packed { uint32_t opaque[6]; };
static inline void
MALI_PRIMITIVE_unpack(const uint8_t * restrict cl,
                      struct MALI_PRIMITIVE * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0x3e00000) fprintf(stderr, "XXX: Invalid field of Primitive unpacked at word 0\n");
   values->draw_mode = (enum mali_draw_mode)__gen_unpack_uint(cl, 0, 7);
   values->index_type = (enum mali_index_type)__gen_unpack_uint(cl, 8, 10);
   values->point_size_array_format = (enum mali_point_size_array_format)__gen_unpack_uint(cl, 11, 12);
   values->primitive_index_enable = __gen_unpack_uint(cl, 13, 13);
   values->primitive_index_writeback = __gen_unpack_uint(cl, 14, 14);
   values->first_provoking_vertex = __gen_unpack_uint(cl, 15, 15);
   values->low_depth_cull = __gen_unpack_uint(cl, 16, 16);
   values->high_depth_cull = __gen_unpack_uint(cl, 17, 17);
   values->secondary_shader = __gen_unpack_uint(cl, 18, 18);
   values->primitive_restart = (enum mali_primitive_restart)__gen_unpack_uint(cl, 19, 20);
   values->job_task_split = __gen_unpack_uint(cl, 26, 31);
   values->base_vertex_offset = __gen_unpack_uint(cl, 32, 63);
   values->primitive_restart_index = __gen_unpack_uint(cl, 64, 95);
   values->index_count = __gen_unpack_uint(cl, 96, 127) + 1;
   values->indices = __gen_unpack_uint(cl, 128, 191);
}

static inline void
MALI_PRIMITIVE_print(FILE *fp, const struct MALI_PRIMITIVE * values, unsigned indent)
{
   fprintf(fp, "%*sDraw mode: %s\n", indent, "", mali_draw_mode_as_str(values->draw_mode));
   fprintf(fp, "%*sIndex type: %s\n", indent, "", mali_index_type_as_str(values->index_type));
   fprintf(fp, "%*sPoint size array format: %s\n", indent, "", mali_point_size_array_format_as_str(values->point_size_array_format));
   fprintf(fp, "%*sPrimitive Index Enable: %s\n", indent, "", values->primitive_index_enable ? "true" : "false");
   fprintf(fp, "%*sPrimitive Index Writeback: %s\n", indent, "", values->primitive_index_writeback ? "true" : "false");
   fprintf(fp, "%*sFirst provoking vertex: %s\n", indent, "", values->first_provoking_vertex ? "true" : "false");
   fprintf(fp, "%*sLow Depth Cull: %s\n", indent, "", values->low_depth_cull ? "true" : "false");
   fprintf(fp, "%*sHigh Depth Cull: %s\n", indent, "", values->high_depth_cull ? "true" : "false");
   fprintf(fp, "%*sSecondary Shader: %s\n", indent, "", values->secondary_shader ? "true" : "false");
   fprintf(fp, "%*sPrimitive restart: %s\n", indent, "", mali_primitive_restart_as_str(values->primitive_restart));
   fprintf(fp, "%*sJob Task Split: %u\n", indent, "", values->job_task_split);
   fprintf(fp, "%*sBase vertex offset: %u\n", indent, "", values->base_vertex_offset);
   fprintf(fp, "%*sPrimitive Restart Index: %u\n", indent, "", values->primitive_restart_index);
   fprintf(fp, "%*sIndex count: %u\n", indent, "", values->index_count);
   fprintf(fp, "%*sIndices: 0x%" PRIx64 "\n", indent, "", values->indices);
}

struct MALI_DRAW {
   bool                                 four_components_per_vertex;
   bool                                 draw_descriptor_is_64b;
   enum mali_occlusion_mode             occlusion_query;
   bool                                 front_face_ccw;
   bool                                 cull_front_face;
   bool                                 cull_back_face;
   uint32_t                             flat_shading_vertex;
   bool                                 exclude_filtered_perf_counters;
   bool                                 primitive_barrier;
   bool                                 clean_fragment_write;
   uint32_t                             instance_size;
   uint32_t                             instance_primitive_size;
   uint32_t                             offset_start;
   uint32_t                             primitive_index_base;
   uint64_t                             position;
   uint64_t                             uniform_buffers;
   uint64_t                             textures;
   uint64_t                             samplers;
   uint64_t                             push_uniforms;
   uint64_t                             state;
   uint64_t                             attribute_buffers;
   uint64_t                             attributes;
   uint64_t                             varying_buffers;
   uint64_t                             varyings;
   uint64_t                             viewport;
   uint64_t                             occlusion;
   uint64_t                             thread_storage;
   uint64_t                             fbd;
};

#define MALI_DRAW_header                        \
   .occlusion_query = MALI_OCCLUSION_MODE_DISABLED,  \
   .instance_size = 1,  \
   .instance_primitive_size = 1

static inline void
MALI_DRAW_pack(uint32_t * restrict cl,
               const struct MALI_DRAW * restrict values)
{
   cl[ 0] = __gen_uint(values->four_components_per_vertex, 0, 0) |
            __gen_uint(values->draw_descriptor_is_64b, 1, 1) |
            __gen_uint(values->occlusion_query, 3, 4) |
            __gen_uint(values->front_face_ccw, 5, 5) |
            __gen_uint(values->cull_front_face, 6, 6) |
            __gen_uint(values->cull_back_face, 7, 7) |
            __gen_uint(values->flat_shading_vertex, 8, 8) |
            __gen_uint(values->exclude_filtered_perf_counters, 9, 9) |
            __gen_uint(values->primitive_barrier, 10, 10) |
            __gen_uint(values->clean_fragment_write, 11, 11) |
            __gen_padded(values->instance_size, 16, 23) |
            __gen_padded(values->instance_primitive_size, 24, 31);
   cl[ 1] = __gen_uint(values->offset_start, 0, 31);
   cl[ 2] = __gen_uint(values->primitive_index_base, 0, 31);
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->position, 0, 63);
   cl[ 5] = __gen_uint(values->position, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->uniform_buffers, 0, 63);
   cl[ 7] = __gen_uint(values->uniform_buffers, 0, 63) >> 32;
   cl[ 8] = __gen_uint(values->textures, 0, 63);
   cl[ 9] = __gen_uint(values->textures, 0, 63) >> 32;
   cl[10] = __gen_uint(values->samplers, 0, 63);
   cl[11] = __gen_uint(values->samplers, 0, 63) >> 32;
   cl[12] = __gen_uint(values->push_uniforms, 0, 63);
   cl[13] = __gen_uint(values->push_uniforms, 0, 63) >> 32;
   cl[14] = __gen_uint(values->state, 0, 63);
   cl[15] = __gen_uint(values->state, 0, 63) >> 32;
   cl[16] = __gen_uint(values->attribute_buffers, 0, 63);
   cl[17] = __gen_uint(values->attribute_buffers, 0, 63) >> 32;
   cl[18] = __gen_uint(values->attributes, 0, 63);
   cl[19] = __gen_uint(values->attributes, 0, 63) >> 32;
   cl[20] = __gen_uint(values->varying_buffers, 0, 63);
   cl[21] = __gen_uint(values->varying_buffers, 0, 63) >> 32;
   cl[22] = __gen_uint(values->varyings, 0, 63);
   cl[23] = __gen_uint(values->varyings, 0, 63) >> 32;
   cl[24] = __gen_uint(values->viewport, 0, 63);
   cl[25] = __gen_uint(values->viewport, 0, 63) >> 32;
   cl[26] = __gen_uint(values->occlusion, 0, 63);
   cl[27] = __gen_uint(values->occlusion, 0, 63) >> 32;
   cl[28] = __gen_uint(values->thread_storage, 0, 63) |
            __gen_uint(values->fbd, 0, 63);
   cl[29] = __gen_uint(values->thread_storage, 0, 63) >> 32 |
            __gen_uint(values->fbd, 0, 63) >> 32;
   cl[30] = 0;
   cl[31] = 0;
}


#define MALI_DRAW_LENGTH 128
#define MALI_DRAW_ALIGN 64
struct mali_draw_packed { uint32_t opaque[32]; };
static inline void
MALI_DRAW_unpack(const uint8_t * restrict cl,
                 struct MALI_DRAW * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf004) fprintf(stderr, "XXX: Invalid field of Draw unpacked at word 0\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Draw unpacked at word 3\n");
   if (((const uint32_t *) cl)[30] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Draw unpacked at word 30\n");
   if (((const uint32_t *) cl)[31] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Draw unpacked at word 31\n");
   values->four_components_per_vertex = __gen_unpack_uint(cl, 0, 0);
   values->draw_descriptor_is_64b = __gen_unpack_uint(cl, 1, 1);
   values->occlusion_query = (enum mali_occlusion_mode)__gen_unpack_uint(cl, 3, 4);
   values->front_face_ccw = __gen_unpack_uint(cl, 5, 5);
   values->cull_front_face = __gen_unpack_uint(cl, 6, 6);
   values->cull_back_face = __gen_unpack_uint(cl, 7, 7);
   values->flat_shading_vertex = __gen_unpack_uint(cl, 8, 8);
   values->exclude_filtered_perf_counters = __gen_unpack_uint(cl, 9, 9);
   values->primitive_barrier = __gen_unpack_uint(cl, 10, 10);
   values->clean_fragment_write = __gen_unpack_uint(cl, 11, 11);
   values->instance_size = __gen_unpack_padded(cl, 16, 23);
   values->instance_primitive_size = __gen_unpack_padded(cl, 24, 31);
   values->offset_start = __gen_unpack_uint(cl, 32, 63);
   values->primitive_index_base = __gen_unpack_uint(cl, 64, 95);
   values->position = __gen_unpack_uint(cl, 128, 191);
   values->uniform_buffers = __gen_unpack_uint(cl, 192, 255);
   values->textures = __gen_unpack_uint(cl, 256, 319);
   values->samplers = __gen_unpack_uint(cl, 320, 383);
   values->push_uniforms = __gen_unpack_uint(cl, 384, 447);
   values->state = __gen_unpack_uint(cl, 448, 511);
   values->attribute_buffers = __gen_unpack_uint(cl, 512, 575);
   values->attributes = __gen_unpack_uint(cl, 576, 639);
   values->varying_buffers = __gen_unpack_uint(cl, 640, 703);
   values->varyings = __gen_unpack_uint(cl, 704, 767);
   values->viewport = __gen_unpack_uint(cl, 768, 831);
   values->occlusion = __gen_unpack_uint(cl, 832, 895);
   values->thread_storage = __gen_unpack_uint(cl, 896, 959);
   values->fbd = __gen_unpack_uint(cl, 896, 959);
}

static inline void
MALI_DRAW_print(FILE *fp, const struct MALI_DRAW * values, unsigned indent)
{
   fprintf(fp, "%*sFour Components Per Vertex: %s\n", indent, "", values->four_components_per_vertex ? "true" : "false");
   fprintf(fp, "%*sDraw Descriptor Is 64b: %s\n", indent, "", values->draw_descriptor_is_64b ? "true" : "false");
   fprintf(fp, "%*sOcclusion query: %s\n", indent, "", mali_occlusion_mode_as_str(values->occlusion_query));
   fprintf(fp, "%*sFront face CCW: %s\n", indent, "", values->front_face_ccw ? "true" : "false");
   fprintf(fp, "%*sCull front face: %s\n", indent, "", values->cull_front_face ? "true" : "false");
   fprintf(fp, "%*sCull back face: %s\n", indent, "", values->cull_back_face ? "true" : "false");
   fprintf(fp, "%*sFlat Shading Vertex: %u\n", indent, "", values->flat_shading_vertex);
   fprintf(fp, "%*sExclude Filtered Perf Counters: %s\n", indent, "", values->exclude_filtered_perf_counters ? "true" : "false");
   fprintf(fp, "%*sPrimitive Barrier: %s\n", indent, "", values->primitive_barrier ? "true" : "false");
   fprintf(fp, "%*sClean Fragment Write: %s\n", indent, "", values->clean_fragment_write ? "true" : "false");
   fprintf(fp, "%*sInstance Size: %u\n", indent, "", values->instance_size);
   fprintf(fp, "%*sInstance Primitive Size: %u\n", indent, "", values->instance_primitive_size);
   fprintf(fp, "%*sOffset start: %u\n", indent, "", values->offset_start);
   fprintf(fp, "%*sPrimitive Index Base: %u\n", indent, "", values->primitive_index_base);
   fprintf(fp, "%*sPosition: 0x%" PRIx64 "\n", indent, "", values->position);
   fprintf(fp, "%*sUniform buffers: 0x%" PRIx64 "\n", indent, "", values->uniform_buffers);
   fprintf(fp, "%*sTextures: 0x%" PRIx64 "\n", indent, "", values->textures);
   fprintf(fp, "%*sSamplers: 0x%" PRIx64 "\n", indent, "", values->samplers);
   fprintf(fp, "%*sPush uniforms: 0x%" PRIx64 "\n", indent, "", values->push_uniforms);
   fprintf(fp, "%*sState: 0x%" PRIx64 "\n", indent, "", values->state);
   fprintf(fp, "%*sAttribute buffers: 0x%" PRIx64 "\n", indent, "", values->attribute_buffers);
   fprintf(fp, "%*sAttributes: 0x%" PRIx64 "\n", indent, "", values->attributes);
   fprintf(fp, "%*sVarying buffers: 0x%" PRIx64 "\n", indent, "", values->varying_buffers);
   fprintf(fp, "%*sVaryings: 0x%" PRIx64 "\n", indent, "", values->varyings);
   fprintf(fp, "%*sViewport: 0x%" PRIx64 "\n", indent, "", values->viewport);
   fprintf(fp, "%*sOcclusion: 0x%" PRIx64 "\n", indent, "", values->occlusion);
   fprintf(fp, "%*sThread Storage: 0x%" PRIx64 "\n", indent, "", values->thread_storage);
   fprintf(fp, "%*sFBD: 0x%" PRIx64 "\n", indent, "", values->fbd);
}

struct MALI_SURFACE {
   uint64_t                             pointer;
};

#define MALI_SURFACE_header                     \
   0

static inline void
MALI_SURFACE_pack(uint32_t * restrict cl,
                  const struct MALI_SURFACE * restrict values)
{
   cl[ 0] = __gen_uint(values->pointer, 0, 63);
   cl[ 1] = __gen_uint(values->pointer, 0, 63) >> 32;
}


#define MALI_SURFACE_LENGTH 8
#define MALI_SURFACE_ALIGN 8
struct mali_surface_packed { uint32_t opaque[2]; };
static inline void
MALI_SURFACE_unpack(const uint8_t * restrict cl,
                    struct MALI_SURFACE * restrict values)
{
   values->pointer = __gen_unpack_uint(cl, 0, 63);
}

static inline void
MALI_SURFACE_print(FILE *fp, const struct MALI_SURFACE * values, unsigned indent)
{
   fprintf(fp, "%*sPointer: 0x%" PRIx64 "\n", indent, "", values->pointer);
}

struct MALI_SURFACE_WITH_STRIDE {
   uint64_t                             pointer;
   int32_t                              row_stride;
   int32_t                              surface_stride;
};

#define MALI_SURFACE_WITH_STRIDE_header         \
   0

static inline void
MALI_SURFACE_WITH_STRIDE_pack(uint32_t * restrict cl,
                              const struct MALI_SURFACE_WITH_STRIDE * restrict values)
{
   cl[ 0] = __gen_uint(values->pointer, 0, 63);
   cl[ 1] = __gen_uint(values->pointer, 0, 63) >> 32;
   cl[ 2] = __gen_sint(values->row_stride, 0, 31);
   cl[ 3] = __gen_sint(values->surface_stride, 0, 31);
}


#define MALI_SURFACE_WITH_STRIDE_LENGTH 16
#define MALI_SURFACE_WITH_STRIDE_ALIGN 8
struct mali_surface_with_stride_packed { uint32_t opaque[4]; };
static inline void
MALI_SURFACE_WITH_STRIDE_unpack(const uint8_t * restrict cl,
                                struct MALI_SURFACE_WITH_STRIDE * restrict values)
{
   values->pointer = __gen_unpack_uint(cl, 0, 63);
   values->row_stride = __gen_unpack_sint(cl, 64, 95);
   values->surface_stride = __gen_unpack_sint(cl, 96, 127);
}

static inline void
MALI_SURFACE_WITH_STRIDE_print(FILE *fp, const struct MALI_SURFACE_WITH_STRIDE * values, unsigned indent)
{
   fprintf(fp, "%*sPointer: 0x%" PRIx64 "\n", indent, "", values->pointer);
   fprintf(fp, "%*sRow stride: %d\n", indent, "", values->row_stride);
   fprintf(fp, "%*sSurface stride: %d\n", indent, "", values->surface_stride);
}

struct MALI_SAMPLER {
   uint32_t                             type;
   enum mali_wrap_mode                  wrap_mode_r;
   enum mali_wrap_mode                  wrap_mode_t;
   enum mali_wrap_mode                  wrap_mode_s;
   bool                                 round_to_nearest_even;
   bool                                 srgb_override;
   bool                                 seamless_cube_map;
   bool                                 clamp_integer_coordinates;
   bool                                 normalized_coordinates;
   bool                                 clamp_integer_array_indices;
   bool                                 minify_nearest;
   bool                                 magnify_nearest;
   bool                                 magnify_cutoff;
   enum mali_mipmap_mode                mipmap_mode;
   uint32_t                             minimum_lod;
   enum mali_func                       compare_function;
   uint32_t                             maximum_lod;
   int32_t                              lod_bias;
   uint32_t                             maximum_anisotropy;
   enum mali_lod_algorithm              lod_algorithm;
   uint32_t                             border_color_r;
   uint32_t                             border_color_g;
   uint32_t                             border_color_b;
   uint32_t                             border_color_a;
};

#define MALI_SAMPLER_header                     \
   .type = 1,  \
   .wrap_mode_r = MALI_WRAP_MODE_CLAMP_TO_EDGE,  \
   .wrap_mode_t = MALI_WRAP_MODE_CLAMP_TO_EDGE,  \
   .wrap_mode_s = MALI_WRAP_MODE_CLAMP_TO_EDGE,  \
   .round_to_nearest_even = false,  \
   .srgb_override = false,  \
   .seamless_cube_map = true,  \
   .normalized_coordinates = true,  \
   .clamp_integer_array_indices = true,  \
   .minify_nearest = false,  \
   .magnify_nearest = false,  \
   .magnify_cutoff = false,  \
   .mipmap_mode = MALI_MIPMAP_MODE_NEAREST,  \
   .minimum_lod = 0,  \
   .compare_function = MALI_FUNC_NEVER,  \
   .maximum_lod = 0,  \
   .lod_bias = 0,  \
   .maximum_anisotropy = 1,  \
   .lod_algorithm = MALI_LOD_ALGORITHM_ISOTROPIC,  \
   .border_color_r = 0.0,  \
   .border_color_g = 0.0,  \
   .border_color_b = 0.0,  \
   .border_color_a = 0.0

static inline void
MALI_SAMPLER_pack(uint32_t * restrict cl,
                  const struct MALI_SAMPLER * restrict values)
{
   assert(values->maximum_anisotropy >= 1);
   cl[ 0] = __gen_uint(values->type, 0, 3) |
            __gen_uint(values->wrap_mode_r, 8, 11) |
            __gen_uint(values->wrap_mode_t, 12, 15) |
            __gen_uint(values->wrap_mode_s, 16, 19) |
            __gen_uint(values->round_to_nearest_even, 21, 21) |
            __gen_uint(values->srgb_override, 22, 22) |
            __gen_uint(values->seamless_cube_map, 23, 23) |
            __gen_uint(values->clamp_integer_coordinates, 24, 24) |
            __gen_uint(values->normalized_coordinates, 25, 25) |
            __gen_uint(values->clamp_integer_array_indices, 26, 26) |
            __gen_uint(values->minify_nearest, 27, 27) |
            __gen_uint(values->magnify_nearest, 28, 28) |
            __gen_uint(values->magnify_cutoff, 29, 29) |
            __gen_uint(values->mipmap_mode, 30, 31);
   cl[ 1] = __gen_uint(values->minimum_lod, 0, 12) |
            __gen_uint(values->compare_function, 13, 15) |
            __gen_uint(values->maximum_lod, 16, 28);
   cl[ 2] = __gen_sint(values->lod_bias, 0, 15) |
            __gen_uint(values->maximum_anisotropy - 1, 16, 20) |
            __gen_uint(values->lod_algorithm, 24, 25);
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->border_color_r, 0, 31);
   cl[ 5] = __gen_uint(values->border_color_g, 0, 31);
   cl[ 6] = __gen_uint(values->border_color_b, 0, 31);
   cl[ 7] = __gen_uint(values->border_color_a, 0, 31);
}


#define MALI_SAMPLER_LENGTH 32
#define MALI_SAMPLER_ALIGN 32
struct mali_sampler_packed { uint32_t opaque[8]; };
static inline void
MALI_SAMPLER_unpack(const uint8_t * restrict cl,
                    struct MALI_SAMPLER * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0x1000f0) fprintf(stderr, "XXX: Invalid field of Sampler unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xe0000000) fprintf(stderr, "XXX: Invalid field of Sampler unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xfce00000) fprintf(stderr, "XXX: Invalid field of Sampler unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Sampler unpacked at word 3\n");
   values->type = __gen_unpack_uint(cl, 0, 3);
   values->wrap_mode_r = (enum mali_wrap_mode)__gen_unpack_uint(cl, 8, 11);
   values->wrap_mode_t = (enum mali_wrap_mode)__gen_unpack_uint(cl, 12, 15);
   values->wrap_mode_s = (enum mali_wrap_mode)__gen_unpack_uint(cl, 16, 19);
   values->round_to_nearest_even = __gen_unpack_uint(cl, 21, 21);
   values->srgb_override = __gen_unpack_uint(cl, 22, 22);
   values->seamless_cube_map = __gen_unpack_uint(cl, 23, 23);
   values->clamp_integer_coordinates = __gen_unpack_uint(cl, 24, 24);
   values->normalized_coordinates = __gen_unpack_uint(cl, 25, 25);
   values->clamp_integer_array_indices = __gen_unpack_uint(cl, 26, 26);
   values->minify_nearest = __gen_unpack_uint(cl, 27, 27);
   values->magnify_nearest = __gen_unpack_uint(cl, 28, 28);
   values->magnify_cutoff = __gen_unpack_uint(cl, 29, 29);
   values->mipmap_mode = (enum mali_mipmap_mode)__gen_unpack_uint(cl, 30, 31);
   values->minimum_lod = __gen_unpack_uint(cl, 32, 44);
   values->compare_function = (enum mali_func)__gen_unpack_uint(cl, 45, 47);
   values->maximum_lod = __gen_unpack_uint(cl, 48, 60);
   values->lod_bias = __gen_unpack_sint(cl, 64, 79);
   values->maximum_anisotropy = __gen_unpack_uint(cl, 80, 84) + 1;
   values->lod_algorithm = (enum mali_lod_algorithm)__gen_unpack_uint(cl, 88, 89);
   values->border_color_r = __gen_unpack_uint(cl, 128, 159);
   values->border_color_g = __gen_unpack_uint(cl, 160, 191);
   values->border_color_b = __gen_unpack_uint(cl, 192, 223);
   values->border_color_a = __gen_unpack_uint(cl, 224, 255);
}

static inline void
MALI_SAMPLER_print(FILE *fp, const struct MALI_SAMPLER * values, unsigned indent)
{
   fprintf(fp, "%*sType: %u\n", indent, "", values->type);
   fprintf(fp, "%*sWrap Mode R: %s\n", indent, "", mali_wrap_mode_as_str(values->wrap_mode_r));
   fprintf(fp, "%*sWrap Mode T: %s\n", indent, "", mali_wrap_mode_as_str(values->wrap_mode_t));
   fprintf(fp, "%*sWrap Mode S: %s\n", indent, "", mali_wrap_mode_as_str(values->wrap_mode_s));
   fprintf(fp, "%*sRound to nearest even: %s\n", indent, "", values->round_to_nearest_even ? "true" : "false");
   fprintf(fp, "%*ssRGB override: %s\n", indent, "", values->srgb_override ? "true" : "false");
   fprintf(fp, "%*sSeamless Cube Map: %s\n", indent, "", values->seamless_cube_map ? "true" : "false");
   fprintf(fp, "%*sClamp integer coordinates: %s\n", indent, "", values->clamp_integer_coordinates ? "true" : "false");
   fprintf(fp, "%*sNormalized Coordinates: %s\n", indent, "", values->normalized_coordinates ? "true" : "false");
   fprintf(fp, "%*sClamp integer array indices: %s\n", indent, "", values->clamp_integer_array_indices ? "true" : "false");
   fprintf(fp, "%*sMinify nearest: %s\n", indent, "", values->minify_nearest ? "true" : "false");
   fprintf(fp, "%*sMagnify nearest: %s\n", indent, "", values->magnify_nearest ? "true" : "false");
   fprintf(fp, "%*sMagnify cutoff: %s\n", indent, "", values->magnify_cutoff ? "true" : "false");
   fprintf(fp, "%*sMipmap Mode: %s\n", indent, "", mali_mipmap_mode_as_str(values->mipmap_mode));
   fprintf(fp, "%*sMinimum LOD: %u\n", indent, "", values->minimum_lod);
   fprintf(fp, "%*sCompare Function: %s\n", indent, "", mali_func_as_str(values->compare_function));
   fprintf(fp, "%*sMaximum LOD: %u\n", indent, "", values->maximum_lod);
   fprintf(fp, "%*sLOD bias: %d\n", indent, "", values->lod_bias);
   fprintf(fp, "%*sMaximum anisotropy: %u\n", indent, "", values->maximum_anisotropy);
   fprintf(fp, "%*sLOD algorithm: %s\n", indent, "", mali_lod_algorithm_as_str(values->lod_algorithm));
   fprintf(fp, "%*sBorder Color R: 0x%X (%f)\n", indent, "", values->border_color_r, uif(values->border_color_r));
   fprintf(fp, "%*sBorder Color G: 0x%X (%f)\n", indent, "", values->border_color_g, uif(values->border_color_g));
   fprintf(fp, "%*sBorder Color B: 0x%X (%f)\n", indent, "", values->border_color_b, uif(values->border_color_b));
   fprintf(fp, "%*sBorder Color A: 0x%X (%f)\n", indent, "", values->border_color_a, uif(values->border_color_a));
}

struct MALI_TEXTURE {
   uint32_t                             type;
   enum mali_texture_dimension          dimension;
   bool                                 sample_corner_position;
   bool                                 normalize_coordinates;
   uint32_t                             format;
   uint32_t                             width;
   uint32_t                             height;
   uint32_t                             swizzle;
   enum mali_texture_layout             texel_ordering;
   uint32_t                             levels;
   uint32_t                             minimum_level;
   uint32_t                             minimum_lod;
   uint32_t                             sample_count;
   uint32_t                             maximum_lod;
   uint64_t                             surfaces;
   uint32_t                             array_size;
   uint32_t                             depth;
};

#define MALI_TEXTURE_header                     \
   .type = 2,  \
   .sample_corner_position = false,  \
   .normalize_coordinates = false,  \
   .levels = 1,  \
   .minimum_lod = 0,  \
   .sample_count = 1,  \
   .maximum_lod = 0,  \
   .array_size = 1,  \
   .depth = 1

static inline void
MALI_TEXTURE_pack(uint32_t * restrict cl,
                  const struct MALI_TEXTURE * restrict values)
{
   assert(values->width >= 1);
   assert(values->height >= 1);
   assert(values->levels >= 1);
   assert(util_is_power_of_two_nonzero(values->sample_count));
   assert(values->array_size >= 1);
   assert(values->depth >= 1);
   cl[ 0] = __gen_uint(values->type, 0, 3) |
            __gen_uint(values->dimension, 4, 5) |
            __gen_uint(values->sample_corner_position, 8, 8) |
            __gen_uint(values->normalize_coordinates, 9, 9) |
            __gen_uint(values->format, 10, 31);
   cl[ 1] = __gen_uint(values->width - 1, 0, 15) |
            __gen_uint(values->height - 1, 16, 31);
   cl[ 2] = __gen_uint(values->swizzle, 0, 11) |
            __gen_uint(values->texel_ordering, 12, 15) |
            __gen_uint(values->levels - 1, 16, 20) |
            __gen_uint(values->minimum_level, 24, 28);
   cl[ 3] = __gen_uint(values->minimum_lod, 0, 12) |
            __gen_uint(util_logbase2(values->sample_count), 13, 15) |
            __gen_uint(values->maximum_lod, 16, 28);
   cl[ 4] = __gen_uint(values->surfaces, 0, 63);
   cl[ 5] = __gen_uint(values->surfaces, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->array_size - 1, 0, 15);
   cl[ 7] = __gen_uint(values->depth - 1, 0, 15);
}


#define MALI_TEXTURE_LENGTH 32
#define MALI_TEXTURE_ALIGN 32
struct mali_texture_packed { uint32_t opaque[8]; };
static inline void
MALI_TEXTURE_unpack(const uint8_t * restrict cl,
                    struct MALI_TEXTURE * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xc0) fprintf(stderr, "XXX: Invalid field of Texture unpacked at word 0\n");
   if (((const uint32_t *) cl)[2] & 0xe0e00000) fprintf(stderr, "XXX: Invalid field of Texture unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xe0000000) fprintf(stderr, "XXX: Invalid field of Texture unpacked at word 3\n");
   if (((const uint32_t *) cl)[6] & 0xffff0000) fprintf(stderr, "XXX: Invalid field of Texture unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffff0000) fprintf(stderr, "XXX: Invalid field of Texture unpacked at word 7\n");
   values->type = __gen_unpack_uint(cl, 0, 3);
   values->dimension = (enum mali_texture_dimension)__gen_unpack_uint(cl, 4, 5);
   values->sample_corner_position = __gen_unpack_uint(cl, 8, 8);
   values->normalize_coordinates = __gen_unpack_uint(cl, 9, 9);
   values->format = __gen_unpack_uint(cl, 10, 31);
   values->width = __gen_unpack_uint(cl, 32, 47) + 1;
   values->height = __gen_unpack_uint(cl, 48, 63) + 1;
   values->swizzle = __gen_unpack_uint(cl, 64, 75);
   values->texel_ordering = (enum mali_texture_layout)__gen_unpack_uint(cl, 76, 79);
   values->levels = __gen_unpack_uint(cl, 80, 84) + 1;
   values->minimum_level = __gen_unpack_uint(cl, 88, 92);
   values->minimum_lod = __gen_unpack_uint(cl, 96, 108);
   values->sample_count = 1U << __gen_unpack_uint(cl, 109, 111);
   values->maximum_lod = __gen_unpack_uint(cl, 112, 124);
   values->surfaces = __gen_unpack_uint(cl, 128, 191);
   values->array_size = __gen_unpack_uint(cl, 192, 207) + 1;
   values->depth = __gen_unpack_uint(cl, 224, 239) + 1;
}

static inline void
MALI_TEXTURE_print(FILE *fp, const struct MALI_TEXTURE * values, unsigned indent)
{
   fprintf(fp, "%*sType: %u\n", indent, "", values->type);
   fprintf(fp, "%*sDimension: %s\n", indent, "", mali_texture_dimension_as_str(values->dimension));
   fprintf(fp, "%*sSample corner position: %s\n", indent, "", values->sample_corner_position ? "true" : "false");
   fprintf(fp, "%*sNormalize coordinates: %s\n", indent, "", values->normalize_coordinates ? "true" : "false");
   mali_pixel_format_print(fp, values->format);
   fprintf(fp, "%*sWidth: %u\n", indent, "", values->width);
   fprintf(fp, "%*sHeight: %u\n", indent, "", values->height);
   fprintf(fp, "%*sSwizzle: %u\n", indent, "", values->swizzle);
   fprintf(fp, "%*sTexel ordering: %s\n", indent, "", mali_texture_layout_as_str(values->texel_ordering));
   fprintf(fp, "%*sLevels: %u\n", indent, "", values->levels);
   fprintf(fp, "%*sMinimum level: %u\n", indent, "", values->minimum_level);
   fprintf(fp, "%*sMinimum LOD: %u\n", indent, "", values->minimum_lod);
   fprintf(fp, "%*sSample count: %u\n", indent, "", values->sample_count);
   fprintf(fp, "%*sMaximum LOD: %u\n", indent, "", values->maximum_lod);
   fprintf(fp, "%*sSurfaces: 0x%" PRIx64 "\n", indent, "", values->surfaces);
   fprintf(fp, "%*sArray size: %u\n", indent, "", values->array_size);
   fprintf(fp, "%*sDepth: %u\n", indent, "", values->depth);
}

enum mali_shader_register_allocation {
        MALI_SHADER_REGISTER_ALLOCATION_64_PER_THREAD =      0,
        MALI_SHADER_REGISTER_ALLOCATION_32_PER_THREAD =      2,
};

static inline const char *
mali_shader_register_allocation_as_str(enum mali_shader_register_allocation imm)
{
    switch (imm) {
    case MALI_SHADER_REGISTER_ALLOCATION_64_PER_THREAD: return "64 Per Thread";
    case MALI_SHADER_REGISTER_ALLOCATION_32_PER_THREAD: return "32 Per Thread";
    default: return "XXX: INVALID";
    }
}

struct MALI_RENDERER_PROPERTIES {
   uint32_t                             uniform_buffer_count;
   enum mali_depth_source               depth_source;
   bool                                 shader_contains_barrier;
   enum mali_shader_register_allocation shader_register_allocation;
   bool                                 shader_modifies_coverage;
   bool                                 allow_forward_pixel_to_kill;
   bool                                 allow_forward_pixel_to_be_killed;
   enum mali_pixel_kill                 pixel_kill_operation;
   enum mali_pixel_kill                 zs_update_operation;
   bool                                 point_sprite_coord_origin_max_y;
   bool                                 stencil_from_shader;
};

#define MALI_RENDERER_PROPERTIES_header         \
   .depth_source = MALI_DEPTH_SOURCE_MINIMUM

static inline void
MALI_RENDERER_PROPERTIES_pack(uint32_t * restrict cl,
                              const struct MALI_RENDERER_PROPERTIES * restrict values)
{
   cl[ 0] = __gen_uint(values->uniform_buffer_count, 0, 7) |
            __gen_uint(values->depth_source, 8, 9) |
            __gen_uint(values->shader_contains_barrier, 11, 11) |
            __gen_uint(values->shader_register_allocation, 12, 13) |
            __gen_uint(values->shader_modifies_coverage, 16, 16) |
            __gen_uint(values->allow_forward_pixel_to_kill, 19, 19) |
            __gen_uint(values->allow_forward_pixel_to_be_killed, 20, 20) |
            __gen_uint(values->pixel_kill_operation, 21, 22) |
            __gen_uint(values->zs_update_operation, 23, 24) |
            __gen_uint(values->point_sprite_coord_origin_max_y, 27, 27) |
            __gen_uint(values->stencil_from_shader, 28, 28);
}


#define MALI_RENDERER_PROPERTIES_LENGTH 4
struct mali_renderer_properties_packed { uint32_t opaque[1]; };
static inline void
MALI_RENDERER_PROPERTIES_unpack(const uint8_t * restrict cl,
                                struct MALI_RENDERER_PROPERTIES * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xe606c400) fprintf(stderr, "XXX: Invalid field of Renderer Properties unpacked at word 0\n");
   values->uniform_buffer_count = __gen_unpack_uint(cl, 0, 7);
   values->depth_source = (enum mali_depth_source)__gen_unpack_uint(cl, 8, 9);
   values->shader_contains_barrier = __gen_unpack_uint(cl, 11, 11);
   values->shader_register_allocation = (enum mali_shader_register_allocation)__gen_unpack_uint(cl, 12, 13);
   values->shader_modifies_coverage = __gen_unpack_uint(cl, 16, 16);
   values->allow_forward_pixel_to_kill = __gen_unpack_uint(cl, 19, 19);
   values->allow_forward_pixel_to_be_killed = __gen_unpack_uint(cl, 20, 20);
   values->pixel_kill_operation = (enum mali_pixel_kill)__gen_unpack_uint(cl, 21, 22);
   values->zs_update_operation = (enum mali_pixel_kill)__gen_unpack_uint(cl, 23, 24);
   values->point_sprite_coord_origin_max_y = __gen_unpack_uint(cl, 27, 27);
   values->stencil_from_shader = __gen_unpack_uint(cl, 28, 28);
}

static inline void
MALI_RENDERER_PROPERTIES_print(FILE *fp, const struct MALI_RENDERER_PROPERTIES * values, unsigned indent)
{
   fprintf(fp, "%*sUniform buffer count: %u\n", indent, "", values->uniform_buffer_count);
   fprintf(fp, "%*sDepth source: %s\n", indent, "", mali_depth_source_as_str(values->depth_source));
   fprintf(fp, "%*sShader contains barrier: %s\n", indent, "", values->shader_contains_barrier ? "true" : "false");
   fprintf(fp, "%*sShader register allocation: %s\n", indent, "", mali_shader_register_allocation_as_str(values->shader_register_allocation));
   fprintf(fp, "%*sShader modifies coverage: %s\n", indent, "", values->shader_modifies_coverage ? "true" : "false");
   fprintf(fp, "%*sAllow forward pixel to kill: %s\n", indent, "", values->allow_forward_pixel_to_kill ? "true" : "false");
   fprintf(fp, "%*sAllow forward pixel to be killed: %s\n", indent, "", values->allow_forward_pixel_to_be_killed ? "true" : "false");
   fprintf(fp, "%*sPixel kill operation: %s\n", indent, "", mali_pixel_kill_as_str(values->pixel_kill_operation));
   fprintf(fp, "%*sZS update operation: %s\n", indent, "", mali_pixel_kill_as_str(values->zs_update_operation));
   fprintf(fp, "%*sPoint sprite coord origin max Y: %s\n", indent, "", values->point_sprite_coord_origin_max_y ? "true" : "false");
   fprintf(fp, "%*sStencil from shader: %s\n", indent, "", values->stencil_from_shader ? "true" : "false");
}

struct MALI_COMPUTE_PRELOAD {
   bool                                 pc;
   bool                                 local_invocation_xy;
   bool                                 local_invocation_z;
   bool                                 work_group_x;
   bool                                 work_group_y;
   bool                                 work_group_z;
   bool                                 global_invocation_x;
   bool                                 global_invocation_y;
   bool                                 global_invocation_z;
};

#define MALI_COMPUTE_PRELOAD_header             \
   0

static inline void
MALI_COMPUTE_PRELOAD_print(FILE *fp, const struct MALI_COMPUTE_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sPC: %s\n", indent, "", values->pc ? "true" : "false");
   fprintf(fp, "%*sLocal Invocation XY: %s\n", indent, "", values->local_invocation_xy ? "true" : "false");
   fprintf(fp, "%*sLocal Invocation Z: %s\n", indent, "", values->local_invocation_z ? "true" : "false");
   fprintf(fp, "%*sWork group X: %s\n", indent, "", values->work_group_x ? "true" : "false");
   fprintf(fp, "%*sWork group Y: %s\n", indent, "", values->work_group_y ? "true" : "false");
   fprintf(fp, "%*sWork group Z: %s\n", indent, "", values->work_group_z ? "true" : "false");
   fprintf(fp, "%*sGlobal Invocation X: %s\n", indent, "", values->global_invocation_x ? "true" : "false");
   fprintf(fp, "%*sGlobal Invocation Y: %s\n", indent, "", values->global_invocation_y ? "true" : "false");
   fprintf(fp, "%*sGlobal Invocation Z: %s\n", indent, "", values->global_invocation_z ? "true" : "false");
}

enum mali_warp_limit {
        MALI_WARP_LIMIT_NONE                 =      0,
        MALI_WARP_LIMIT_2                    =      1,
        MALI_WARP_LIMIT_4                    =      2,
        MALI_WARP_LIMIT_8                    =      3,
};

static inline const char *
mali_warp_limit_as_str(enum mali_warp_limit imm)
{
    switch (imm) {
    case MALI_WARP_LIMIT_NONE: return "None";
    case MALI_WARP_LIMIT_2: return "2";
    case MALI_WARP_LIMIT_4: return "4";
    case MALI_WARP_LIMIT_8: return "8";
    default: return "XXX: INVALID";
    }
}

struct MALI_VERTEX_PRELOAD {
   enum mali_warp_limit                 warp_limit;
   bool                                 pc;
   bool                                 position_result_address_lo;
   bool                                 position_result_address_hi;
   bool                                 vertex_id;
   bool                                 instance_id;
};

#define MALI_VERTEX_PRELOAD_header              \
   0

static inline void
MALI_VERTEX_PRELOAD_print(FILE *fp, const struct MALI_VERTEX_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sWarp limit: %s\n", indent, "", mali_warp_limit_as_str(values->warp_limit));
   fprintf(fp, "%*sPC: %s\n", indent, "", values->pc ? "true" : "false");
   fprintf(fp, "%*sPosition result address lo: %s\n", indent, "", values->position_result_address_lo ? "true" : "false");
   fprintf(fp, "%*sPosition result address hi: %s\n", indent, "", values->position_result_address_hi ? "true" : "false");
   fprintf(fp, "%*sVertex ID: %s\n", indent, "", values->vertex_id ? "true" : "false");
   fprintf(fp, "%*sInstance ID: %s\n", indent, "", values->instance_id ? "true" : "false");
}

struct MALI_FRAGMENT_PRELOAD {
   bool                                 pc;
   bool                                 coverage;
   bool                                 primitive_id;
   bool                                 primitive_flags;
   bool                                 fragment_position;
   bool                                 sample_mask_id;
};

#define MALI_FRAGMENT_PRELOAD_header            \
   0

static inline void
MALI_FRAGMENT_PRELOAD_print(FILE *fp, const struct MALI_FRAGMENT_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sPC: %s\n", indent, "", values->pc ? "true" : "false");
   fprintf(fp, "%*sCoverage: %s\n", indent, "", values->coverage ? "true" : "false");
   fprintf(fp, "%*sPrimitive ID: %s\n", indent, "", values->primitive_id ? "true" : "false");
   fprintf(fp, "%*sPrimitive flags: %s\n", indent, "", values->primitive_flags ? "true" : "false");
   fprintf(fp, "%*sFragment position: %s\n", indent, "", values->fragment_position ? "true" : "false");
   fprintf(fp, "%*sSample mask/ID: %s\n", indent, "", values->sample_mask_id ? "true" : "false");
}

struct MALI_PRELOAD {
   struct MALI_COMPUTE_PRELOAD          compute;
   struct MALI_VERTEX_PRELOAD           vertex;
   struct MALI_FRAGMENT_PRELOAD         fragment;
   uint32_t                             uniform_count;
};

#define MALI_PRELOAD_header                     \
   .compute = { MALI_COMPUTE_PRELOAD_header },  \
   .vertex = { MALI_VERTEX_PRELOAD_header },  \
   .fragment = { MALI_FRAGMENT_PRELOAD_header }

static inline void
MALI_PRELOAD_pack(uint32_t * restrict cl,
                  const struct MALI_PRELOAD * restrict values)
{
   cl[ 0] = __gen_uint(values->compute.pc, 6, 6) |
            __gen_uint(values->compute.local_invocation_xy, 7, 7) |
            __gen_uint(values->compute.local_invocation_z, 8, 8) |
            __gen_uint(values->compute.work_group_x, 9, 9) |
            __gen_uint(values->compute.work_group_y, 10, 10) |
            __gen_uint(values->compute.work_group_z, 11, 11) |
            __gen_uint(values->compute.global_invocation_x, 12, 12) |
            __gen_uint(values->compute.global_invocation_y, 13, 13) |
            __gen_uint(values->compute.global_invocation_z, 14, 14) |
            __gen_uint(values->vertex.warp_limit, 0, 1) |
            __gen_uint(values->vertex.pc, 6, 6) |
            __gen_uint(values->vertex.position_result_address_lo, 10, 10) |
            __gen_uint(values->vertex.position_result_address_hi, 11, 11) |
            __gen_uint(values->vertex.vertex_id, 13, 13) |
            __gen_uint(values->vertex.instance_id, 14, 14) |
            __gen_uint(values->fragment.pc, 6, 6) |
            __gen_uint(values->fragment.coverage, 7, 7) |
            __gen_uint(values->fragment.primitive_id, 9, 9) |
            __gen_uint(values->fragment.primitive_flags, 10, 10) |
            __gen_uint(values->fragment.fragment_position, 11, 11) |
            __gen_uint(values->fragment.sample_mask_id, 13, 13) |
            __gen_uint(values->uniform_count, 15, 21);
}


#define MALI_PRELOAD_LENGTH 4
struct mali_preload_packed { uint32_t opaque[1]; };
static inline void
MALI_PRELOAD_unpack(const uint8_t * restrict cl,
                    struct MALI_PRELOAD * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffc0003c) fprintf(stderr, "XXX: Invalid field of Preload unpacked at word 0\n");
   values->compute.pc = __gen_unpack_uint(cl, 6, 6);
   values->compute.local_invocation_xy = __gen_unpack_uint(cl, 7, 7);
   values->compute.local_invocation_z = __gen_unpack_uint(cl, 8, 8);
   values->compute.work_group_x = __gen_unpack_uint(cl, 9, 9);
   values->compute.work_group_y = __gen_unpack_uint(cl, 10, 10);
   values->compute.work_group_z = __gen_unpack_uint(cl, 11, 11);
   values->compute.global_invocation_x = __gen_unpack_uint(cl, 12, 12);
   values->compute.global_invocation_y = __gen_unpack_uint(cl, 13, 13);
   values->compute.global_invocation_z = __gen_unpack_uint(cl, 14, 14);
   values->vertex.warp_limit = (enum mali_warp_limit)__gen_unpack_uint(cl, 0, 1);
   values->vertex.pc = __gen_unpack_uint(cl, 6, 6);
   values->vertex.position_result_address_lo = __gen_unpack_uint(cl, 10, 10);
   values->vertex.position_result_address_hi = __gen_unpack_uint(cl, 11, 11);
   values->vertex.vertex_id = __gen_unpack_uint(cl, 13, 13);
   values->vertex.instance_id = __gen_unpack_uint(cl, 14, 14);
   values->fragment.pc = __gen_unpack_uint(cl, 6, 6);
   values->fragment.coverage = __gen_unpack_uint(cl, 7, 7);
   values->fragment.primitive_id = __gen_unpack_uint(cl, 9, 9);
   values->fragment.primitive_flags = __gen_unpack_uint(cl, 10, 10);
   values->fragment.fragment_position = __gen_unpack_uint(cl, 11, 11);
   values->fragment.sample_mask_id = __gen_unpack_uint(cl, 13, 13);
   values->uniform_count = __gen_unpack_uint(cl, 15, 21);
}

static inline void
MALI_PRELOAD_print(FILE *fp, const struct MALI_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sCompute:\n", indent, "");
   MALI_COMPUTE_PRELOAD_print(fp, &values->compute, indent + 2);
   fprintf(fp, "%*sVertex:\n", indent, "");
   MALI_VERTEX_PRELOAD_print(fp, &values->vertex, indent + 2);
   fprintf(fp, "%*sFragment:\n", indent, "");
   MALI_FRAGMENT_PRELOAD_print(fp, &values->fragment, indent + 2);
   fprintf(fp, "%*sUniform count: %u\n", indent, "", values->uniform_count);
}

struct MALI_SHADER {
   uint64_t                             shader;
   uint32_t                             sampler_count;
   uint32_t                             texture_count;
   uint32_t                             attribute_count;
   uint32_t                             varying_count;
};

#define MALI_SHADER_header                      \
   0

static inline void
MALI_SHADER_pack(uint32_t * restrict cl,
                 const struct MALI_SHADER * restrict values)
{
   cl[ 0] = __gen_uint(values->shader, 0, 63);
   cl[ 1] = __gen_uint(values->shader, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->sampler_count, 0, 15) |
            __gen_uint(values->texture_count, 16, 31);
   cl[ 3] = __gen_uint(values->attribute_count, 0, 15) |
            __gen_uint(values->varying_count, 16, 31);
}


#define MALI_SHADER_LENGTH 16
struct mali_shader_packed { uint32_t opaque[4]; };
static inline void
MALI_SHADER_unpack(const uint8_t * restrict cl,
                   struct MALI_SHADER * restrict values)
{
   values->shader = __gen_unpack_uint(cl, 0, 63);
   values->sampler_count = __gen_unpack_uint(cl, 64, 79);
   values->texture_count = __gen_unpack_uint(cl, 80, 95);
   values->attribute_count = __gen_unpack_uint(cl, 96, 111);
   values->varying_count = __gen_unpack_uint(cl, 112, 127);
}

static inline void
MALI_SHADER_print(FILE *fp, const struct MALI_SHADER * values, unsigned indent)
{
   fprintf(fp, "%*sShader: 0x%" PRIx64 "\n", indent, "", values->shader);
   fprintf(fp, "%*sSampler count: %u\n", indent, "", values->sampler_count);
   fprintf(fp, "%*sTexture count: %u\n", indent, "", values->texture_count);
   fprintf(fp, "%*sAttribute count: %u\n", indent, "", values->attribute_count);
   fprintf(fp, "%*sVarying count: %u\n", indent, "", values->varying_count);
}

struct MALI_MULTISAMPLE_MISC {
   uint32_t                             sample_mask;
   bool                                 multisample_enable;
   bool                                 multisample_late_coverage;
   bool                                 evaluate_per_sample;
   bool                                 fixed_function_depth_range_fixed;
   bool                                 shader_depth_range_fixed;
   bool                                 overdraw_alpha1;
   bool                                 overdraw_alpha0;
   enum mali_func                       depth_function;
   bool                                 depth_write_mask;
   bool                                 fixed_function_near_discard;
   bool                                 fixed_function_far_discard;
   bool                                 fragment_near_discard;
   bool                                 fragment_far_discard;
};

#define MALI_MULTISAMPLE_MISC_header            \
   0

static inline void
MALI_MULTISAMPLE_MISC_pack(uint32_t * restrict cl,
                           const struct MALI_MULTISAMPLE_MISC * restrict values)
{
   cl[ 0] = __gen_uint(values->sample_mask, 0, 15) |
            __gen_uint(values->multisample_enable, 16, 16) |
            __gen_uint(values->multisample_late_coverage, 17, 17) |
            __gen_uint(values->evaluate_per_sample, 18, 18) |
            __gen_uint(values->fixed_function_depth_range_fixed, 19, 19) |
            __gen_uint(values->shader_depth_range_fixed, 20, 20) |
            __gen_uint(values->overdraw_alpha1, 22, 22) |
            __gen_uint(values->overdraw_alpha0, 23, 23) |
            __gen_uint(values->depth_function, 24, 26) |
            __gen_uint(values->depth_write_mask, 27, 27) |
            __gen_uint(values->fixed_function_near_discard, 28, 28) |
            __gen_uint(values->fixed_function_far_discard, 29, 29) |
            __gen_uint(values->fragment_near_discard, 30, 30) |
            __gen_uint(values->fragment_far_discard, 31, 31);
}


#define MALI_MULTISAMPLE_MISC_LENGTH 4
struct mali_multisample_misc_packed { uint32_t opaque[1]; };
static inline void
MALI_MULTISAMPLE_MISC_unpack(const uint8_t * restrict cl,
                             struct MALI_MULTISAMPLE_MISC * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0x200000) fprintf(stderr, "XXX: Invalid field of Multisample, Misc unpacked at word 0\n");
   values->sample_mask = __gen_unpack_uint(cl, 0, 15);
   values->multisample_enable = __gen_unpack_uint(cl, 16, 16);
   values->multisample_late_coverage = __gen_unpack_uint(cl, 17, 17);
   values->evaluate_per_sample = __gen_unpack_uint(cl, 18, 18);
   values->fixed_function_depth_range_fixed = __gen_unpack_uint(cl, 19, 19);
   values->shader_depth_range_fixed = __gen_unpack_uint(cl, 20, 20);
   values->overdraw_alpha1 = __gen_unpack_uint(cl, 22, 22);
   values->overdraw_alpha0 = __gen_unpack_uint(cl, 23, 23);
   values->depth_function = (enum mali_func)__gen_unpack_uint(cl, 24, 26);
   values->depth_write_mask = __gen_unpack_uint(cl, 27, 27);
   values->fixed_function_near_discard = __gen_unpack_uint(cl, 28, 28);
   values->fixed_function_far_discard = __gen_unpack_uint(cl, 29, 29);
   values->fragment_near_discard = __gen_unpack_uint(cl, 30, 30);
   values->fragment_far_discard = __gen_unpack_uint(cl, 31, 31);
}

static inline void
MALI_MULTISAMPLE_MISC_print(FILE *fp, const struct MALI_MULTISAMPLE_MISC * values, unsigned indent)
{
   fprintf(fp, "%*sSample mask: %u\n", indent, "", values->sample_mask);
   fprintf(fp, "%*sMultisample enable: %s\n", indent, "", values->multisample_enable ? "true" : "false");
   fprintf(fp, "%*sMultisample late coverage: %s\n", indent, "", values->multisample_late_coverage ? "true" : "false");
   fprintf(fp, "%*sEvaluate per-sample: %s\n", indent, "", values->evaluate_per_sample ? "true" : "false");
   fprintf(fp, "%*sFixed-function depth range fixed: %s\n", indent, "", values->fixed_function_depth_range_fixed ? "true" : "false");
   fprintf(fp, "%*sShader depth range fixed: %s\n", indent, "", values->shader_depth_range_fixed ? "true" : "false");
   fprintf(fp, "%*sOverdraw alpha1: %s\n", indent, "", values->overdraw_alpha1 ? "true" : "false");
   fprintf(fp, "%*sOverdraw alpha0: %s\n", indent, "", values->overdraw_alpha0 ? "true" : "false");
   fprintf(fp, "%*sDepth function: %s\n", indent, "", mali_func_as_str(values->depth_function));
   fprintf(fp, "%*sDepth write mask: %s\n", indent, "", values->depth_write_mask ? "true" : "false");
   fprintf(fp, "%*sFixed-function near discard: %s\n", indent, "", values->fixed_function_near_discard ? "true" : "false");
   fprintf(fp, "%*sFixed-function far discard: %s\n", indent, "", values->fixed_function_far_discard ? "true" : "false");
   fprintf(fp, "%*sFragment near discard: %s\n", indent, "", values->fragment_near_discard ? "true" : "false");
   fprintf(fp, "%*sFragment far discard: %s\n", indent, "", values->fragment_far_discard ? "true" : "false");
}

struct MALI_STENCIL_MASK_MISC {
   uint32_t                             stencil_mask_front;
   uint32_t                             stencil_mask_back;
   bool                                 stencil_enable;
   bool                                 alpha_to_coverage;
   bool                                 alpha_to_coverage_invert;
   enum mali_func                       alpha_test_compare_function;
   bool                                 force_seamless_cubemaps;
   bool                                 depth_range_1;
   bool                                 depth_range_2;
   bool                                 single_sampled_lines;
   bool                                 point_snap;
};

#define MALI_STENCIL_MASK_MISC_header           \
   0

static inline void
MALI_STENCIL_MASK_MISC_pack(uint32_t * restrict cl,
                            const struct MALI_STENCIL_MASK_MISC * restrict values)
{
   cl[ 0] = __gen_uint(values->stencil_mask_front, 0, 7) |
            __gen_uint(values->stencil_mask_back, 8, 15) |
            __gen_uint(values->stencil_enable, 16, 16) |
            __gen_uint(values->alpha_to_coverage, 17, 17) |
            __gen_uint(values->alpha_to_coverage_invert, 18, 18) |
            __gen_uint(values->alpha_test_compare_function, 21, 23) |
            __gen_uint(values->force_seamless_cubemaps, 26, 26) |
            __gen_uint(values->depth_range_1, 28, 28) |
            __gen_uint(values->depth_range_2, 29, 29) |
            __gen_uint(values->single_sampled_lines, 30, 30) |
            __gen_uint(values->point_snap, 31, 31);
}


#define MALI_STENCIL_MASK_MISC_LENGTH 4
struct mali_stencil_mask_misc_packed { uint32_t opaque[1]; };
static inline void
MALI_STENCIL_MASK_MISC_unpack(const uint8_t * restrict cl,
                              struct MALI_STENCIL_MASK_MISC * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xb180000) fprintf(stderr, "XXX: Invalid field of Stencil Mask, Misc unpacked at word 0\n");
   values->stencil_mask_front = __gen_unpack_uint(cl, 0, 7);
   values->stencil_mask_back = __gen_unpack_uint(cl, 8, 15);
   values->stencil_enable = __gen_unpack_uint(cl, 16, 16);
   values->alpha_to_coverage = __gen_unpack_uint(cl, 17, 17);
   values->alpha_to_coverage_invert = __gen_unpack_uint(cl, 18, 18);
   values->alpha_test_compare_function = (enum mali_func)__gen_unpack_uint(cl, 21, 23);
   values->force_seamless_cubemaps = __gen_unpack_uint(cl, 26, 26);
   values->depth_range_1 = __gen_unpack_uint(cl, 28, 28);
   values->depth_range_2 = __gen_unpack_uint(cl, 29, 29);
   values->single_sampled_lines = __gen_unpack_uint(cl, 30, 30);
   values->point_snap = __gen_unpack_uint(cl, 31, 31);
}

static inline void
MALI_STENCIL_MASK_MISC_print(FILE *fp, const struct MALI_STENCIL_MASK_MISC * values, unsigned indent)
{
   fprintf(fp, "%*sStencil mask front: %u\n", indent, "", values->stencil_mask_front);
   fprintf(fp, "%*sStencil mask back: %u\n", indent, "", values->stencil_mask_back);
   fprintf(fp, "%*sStencil enable: %s\n", indent, "", values->stencil_enable ? "true" : "false");
   fprintf(fp, "%*sAlpha-to-coverage: %s\n", indent, "", values->alpha_to_coverage ? "true" : "false");
   fprintf(fp, "%*sAlpha-to-coverage Invert: %s\n", indent, "", values->alpha_to_coverage_invert ? "true" : "false");
   fprintf(fp, "%*sAlpha test compare function: %s\n", indent, "", mali_func_as_str(values->alpha_test_compare_function));
   fprintf(fp, "%*sForce seamless cubemaps: %s\n", indent, "", values->force_seamless_cubemaps ? "true" : "false");
   fprintf(fp, "%*sDepth Range 1: %s\n", indent, "", values->depth_range_1 ? "true" : "false");
   fprintf(fp, "%*sDepth Range 2: %s\n", indent, "", values->depth_range_2 ? "true" : "false");
   fprintf(fp, "%*sSingle-sampled lines: %s\n", indent, "", values->single_sampled_lines ? "true" : "false");
   fprintf(fp, "%*sPoint snap: %s\n", indent, "", values->point_snap ? "true" : "false");
}

struct MALI_STENCIL {
   uint32_t                             reference_value;
   uint32_t                             mask;
   enum mali_func                       compare_function;
   enum mali_stencil_op                 stencil_fail;
   enum mali_stencil_op                 depth_fail;
   enum mali_stencil_op                 depth_pass;
};

#define MALI_STENCIL_header                     \
   0

static inline void
MALI_STENCIL_pack(uint32_t * restrict cl,
                  const struct MALI_STENCIL * restrict values)
{
   cl[ 0] = __gen_uint(values->reference_value, 0, 7) |
            __gen_uint(values->mask, 8, 15) |
            __gen_uint(values->compare_function, 16, 18) |
            __gen_uint(values->stencil_fail, 19, 21) |
            __gen_uint(values->depth_fail, 22, 24) |
            __gen_uint(values->depth_pass, 25, 27);
}


#define MALI_STENCIL_LENGTH 4
struct mali_stencil_packed { uint32_t opaque[1]; };
static inline void
MALI_STENCIL_unpack(const uint8_t * restrict cl,
                    struct MALI_STENCIL * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf0000000) fprintf(stderr, "XXX: Invalid field of Stencil unpacked at word 0\n");
   values->reference_value = __gen_unpack_uint(cl, 0, 7);
   values->mask = __gen_unpack_uint(cl, 8, 15);
   values->compare_function = (enum mali_func)__gen_unpack_uint(cl, 16, 18);
   values->stencil_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 19, 21);
   values->depth_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 22, 24);
   values->depth_pass = (enum mali_stencil_op)__gen_unpack_uint(cl, 25, 27);
}

static inline void
MALI_STENCIL_print(FILE *fp, const struct MALI_STENCIL * values, unsigned indent)
{
   fprintf(fp, "%*sReference Value: %u\n", indent, "", values->reference_value);
   fprintf(fp, "%*sMask: %u\n", indent, "", values->mask);
   fprintf(fp, "%*sCompare Function: %s\n", indent, "", mali_func_as_str(values->compare_function));
   fprintf(fp, "%*sStencil Fail: %s\n", indent, "", mali_stencil_op_as_str(values->stencil_fail));
   fprintf(fp, "%*sDepth Fail: %s\n", indent, "", mali_stencil_op_as_str(values->depth_fail));
   fprintf(fp, "%*sDepth Pass: %s\n", indent, "", mali_stencil_op_as_str(values->depth_pass));
}

struct MALI_LD_VAR_PRELOAD {
   uint32_t                             varying_index;
   enum mali_message_preload_register_format register_format;
   uint32_t                             num_components;
};

#define MALI_LD_VAR_PRELOAD_header              \
   .num_components = 1

static inline void
MALI_LD_VAR_PRELOAD_print(FILE *fp, const struct MALI_LD_VAR_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sVarying Index: %u\n", indent, "", values->varying_index);
   fprintf(fp, "%*sRegister Format: %s\n", indent, "", mali_message_preload_register_format_as_str(values->register_format));
   fprintf(fp, "%*sNum Components: %u\n", indent, "", values->num_components);
}

struct MALI_VAR_TEX_PRELOAD {
   uint32_t                             varying_index;
   uint32_t                             sampler_index;
   enum mali_message_preload_register_format register_format;
   bool                                 skip;
   bool                                 zero_lod;
};

#define MALI_VAR_TEX_PRELOAD_header             \
   0

static inline void
MALI_VAR_TEX_PRELOAD_print(FILE *fp, const struct MALI_VAR_TEX_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sVarying Index: %u\n", indent, "", values->varying_index);
   fprintf(fp, "%*sSampler Index: %u\n", indent, "", values->sampler_index);
   fprintf(fp, "%*sRegister Format: %s\n", indent, "", mali_message_preload_register_format_as_str(values->register_format));
   fprintf(fp, "%*sSkip: %s\n", indent, "", values->skip ? "true" : "false");
   fprintf(fp, "%*sZero LOD: %s\n", indent, "", values->zero_lod ? "true" : "false");
}

struct MALI_MESSAGE_PRELOAD {
   enum mali_message_type               type;
   struct MALI_LD_VAR_PRELOAD           ld_var;
   struct MALI_VAR_TEX_PRELOAD          var_tex;
};

#define MALI_MESSAGE_PRELOAD_header             \
   .ld_var = { MALI_LD_VAR_PRELOAD_header },  \
   .var_tex = { MALI_VAR_TEX_PRELOAD_header }

static inline void
MALI_MESSAGE_PRELOAD_pack(uint32_t * restrict cl,
                          const struct MALI_MESSAGE_PRELOAD * restrict values)
{
   cl[ 0] = __gen_uint(values->type, 0, 3) |
            __gen_uint(values->ld_var.varying_index, 4, 8) |
            __gen_uint(values->ld_var.register_format, 9, 10) |
            __gen_uint(values->ld_var.num_components - 1, 11, 12) |
            __gen_uint(values->var_tex.varying_index, 4, 6) |
            __gen_uint(values->var_tex.sampler_index, 7, 8) |
            __gen_uint(values->var_tex.register_format, 9, 10) |
            __gen_uint(values->var_tex.skip, 14, 14) |
            __gen_uint(values->var_tex.zero_lod, 15, 15);
}


#define MALI_MESSAGE_PRELOAD_LENGTH 4
struct mali_message_preload_packed { uint32_t opaque[1]; };
static inline void
MALI_MESSAGE_PRELOAD_unpack(const uint8_t * restrict cl,
                            struct MALI_MESSAGE_PRELOAD * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffff2000) fprintf(stderr, "XXX: Invalid field of Message Preload unpacked at word 0\n");
   values->type = (enum mali_message_type)__gen_unpack_uint(cl, 0, 3);
   values->ld_var.varying_index = __gen_unpack_uint(cl, 4, 8);
   values->ld_var.register_format = (enum mali_message_preload_register_format)__gen_unpack_uint(cl, 9, 10);
   values->ld_var.num_components = __gen_unpack_uint(cl, 11, 12) + 1;
   values->var_tex.varying_index = __gen_unpack_uint(cl, 4, 6);
   values->var_tex.sampler_index = __gen_unpack_uint(cl, 7, 8);
   values->var_tex.register_format = (enum mali_message_preload_register_format)__gen_unpack_uint(cl, 9, 10);
   values->var_tex.skip = __gen_unpack_uint(cl, 14, 14);
   values->var_tex.zero_lod = __gen_unpack_uint(cl, 15, 15);
}

static inline void
MALI_MESSAGE_PRELOAD_print(FILE *fp, const struct MALI_MESSAGE_PRELOAD * values, unsigned indent)
{
   fprintf(fp, "%*sType: %s\n", indent, "", mali_message_type_as_str(values->type));
   fprintf(fp, "%*sLD_VAR:\n", indent, "");
   MALI_LD_VAR_PRELOAD_print(fp, &values->ld_var, indent + 2);
   fprintf(fp, "%*sVAR_TEX:\n", indent, "");
   MALI_VAR_TEX_PRELOAD_print(fp, &values->var_tex, indent + 2);
}

struct MALI_RENDERER_STATE {
   struct MALI_SHADER                   shader;
   struct MALI_RENDERER_PROPERTIES      properties;
   float                                depth_units;
   float                                depth_factor;
   float                                depth_bias_clamp;
   struct MALI_MULTISAMPLE_MISC         multisample_misc;
   struct MALI_STENCIL_MASK_MISC        stencil_mask_misc;
   struct MALI_STENCIL                  stencil_front;
   struct MALI_STENCIL                  stencil_back;
   struct MALI_PRELOAD                  preload;
   float                                alpha_reference;
   uint32_t                             thread_balancing;
   struct MALI_PRELOAD                  secondary_preload;
   uint64_t                             secondary_shader;
};

#define MALI_RENDERER_STATE_header              \
   .shader = { MALI_SHADER_header },  \
   .properties = { MALI_RENDERER_PROPERTIES_header },  \
   .multisample_misc = { MALI_MULTISAMPLE_MISC_header },  \
   .stencil_mask_misc = { MALI_STENCIL_MASK_MISC_header },  \
   .stencil_front = { MALI_STENCIL_header },  \
   .stencil_back = { MALI_STENCIL_header },  \
   .preload = { MALI_PRELOAD_header },  \
   .secondary_preload = { MALI_PRELOAD_header }

static inline void
MALI_RENDERER_STATE_pack(uint32_t * restrict cl,
                         const struct MALI_RENDERER_STATE * restrict values)
{
   cl[ 0] = __gen_uint(values->shader.shader, 0, 63);
   cl[ 1] = __gen_uint(values->shader.shader, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->shader.sampler_count, 0, 15) |
            __gen_uint(values->shader.texture_count, 16, 31);
   cl[ 3] = __gen_uint(values->shader.attribute_count, 0, 15) |
            __gen_uint(values->shader.varying_count, 16, 31);
   cl[ 4] = __gen_uint(values->properties.uniform_buffer_count, 0, 7) |
            __gen_uint(values->properties.depth_source, 8, 9) |
            __gen_uint(values->properties.shader_contains_barrier, 11, 11) |
            __gen_uint(values->properties.shader_register_allocation, 12, 13) |
            __gen_uint(values->properties.shader_modifies_coverage, 16, 16) |
            __gen_uint(values->properties.allow_forward_pixel_to_kill, 19, 19) |
            __gen_uint(values->properties.allow_forward_pixel_to_be_killed, 20, 20) |
            __gen_uint(values->properties.pixel_kill_operation, 21, 22) |
            __gen_uint(values->properties.zs_update_operation, 23, 24) |
            __gen_uint(values->properties.point_sprite_coord_origin_max_y, 27, 27) |
            __gen_uint(values->properties.stencil_from_shader, 28, 28);
   cl[ 5] = __gen_uint(fui(values->depth_units), 0, 32);
   cl[ 6] = __gen_uint(fui(values->depth_factor), 0, 32);
   cl[ 7] = __gen_uint(fui(values->depth_bias_clamp), 0, 32);
   cl[ 8] = __gen_uint(values->multisample_misc.sample_mask, 0, 15) |
            __gen_uint(values->multisample_misc.multisample_enable, 16, 16) |
            __gen_uint(values->multisample_misc.multisample_late_coverage, 17, 17) |
            __gen_uint(values->multisample_misc.evaluate_per_sample, 18, 18) |
            __gen_uint(values->multisample_misc.fixed_function_depth_range_fixed, 19, 19) |
            __gen_uint(values->multisample_misc.shader_depth_range_fixed, 20, 20) |
            __gen_uint(values->multisample_misc.overdraw_alpha1, 22, 22) |
            __gen_uint(values->multisample_misc.overdraw_alpha0, 23, 23) |
            __gen_uint(values->multisample_misc.depth_function, 24, 26) |
            __gen_uint(values->multisample_misc.depth_write_mask, 27, 27) |
            __gen_uint(values->multisample_misc.fixed_function_near_discard, 28, 28) |
            __gen_uint(values->multisample_misc.fixed_function_far_discard, 29, 29) |
            __gen_uint(values->multisample_misc.fragment_near_discard, 30, 30) |
            __gen_uint(values->multisample_misc.fragment_far_discard, 31, 31);
   cl[ 9] = __gen_uint(values->stencil_mask_misc.stencil_mask_front, 0, 7) |
            __gen_uint(values->stencil_mask_misc.stencil_mask_back, 8, 15) |
            __gen_uint(values->stencil_mask_misc.stencil_enable, 16, 16) |
            __gen_uint(values->stencil_mask_misc.alpha_to_coverage, 17, 17) |
            __gen_uint(values->stencil_mask_misc.alpha_to_coverage_invert, 18, 18) |
            __gen_uint(values->stencil_mask_misc.alpha_test_compare_function, 21, 23) |
            __gen_uint(values->stencil_mask_misc.force_seamless_cubemaps, 26, 26) |
            __gen_uint(values->stencil_mask_misc.depth_range_1, 28, 28) |
            __gen_uint(values->stencil_mask_misc.depth_range_2, 29, 29) |
            __gen_uint(values->stencil_mask_misc.single_sampled_lines, 30, 30) |
            __gen_uint(values->stencil_mask_misc.point_snap, 31, 31);
   cl[10] = __gen_uint(values->stencil_front.reference_value, 0, 7) |
            __gen_uint(values->stencil_front.mask, 8, 15) |
            __gen_uint(values->stencil_front.compare_function, 16, 18) |
            __gen_uint(values->stencil_front.stencil_fail, 19, 21) |
            __gen_uint(values->stencil_front.depth_fail, 22, 24) |
            __gen_uint(values->stencil_front.depth_pass, 25, 27);
   cl[11] = __gen_uint(values->stencil_back.reference_value, 0, 7) |
            __gen_uint(values->stencil_back.mask, 8, 15) |
            __gen_uint(values->stencil_back.compare_function, 16, 18) |
            __gen_uint(values->stencil_back.stencil_fail, 19, 21) |
            __gen_uint(values->stencil_back.depth_fail, 22, 24) |
            __gen_uint(values->stencil_back.depth_pass, 25, 27);
   cl[12] = __gen_uint(values->preload.compute.pc, 6, 6) |
            __gen_uint(values->preload.compute.local_invocation_xy, 7, 7) |
            __gen_uint(values->preload.compute.local_invocation_z, 8, 8) |
            __gen_uint(values->preload.compute.work_group_x, 9, 9) |
            __gen_uint(values->preload.compute.work_group_y, 10, 10) |
            __gen_uint(values->preload.compute.work_group_z, 11, 11) |
            __gen_uint(values->preload.compute.global_invocation_x, 12, 12) |
            __gen_uint(values->preload.compute.global_invocation_y, 13, 13) |
            __gen_uint(values->preload.compute.global_invocation_z, 14, 14) |
            __gen_uint(values->preload.vertex.warp_limit, 0, 1) |
            __gen_uint(values->preload.vertex.pc, 6, 6) |
            __gen_uint(values->preload.vertex.position_result_address_lo, 10, 10) |
            __gen_uint(values->preload.vertex.position_result_address_hi, 11, 11) |
            __gen_uint(values->preload.vertex.vertex_id, 13, 13) |
            __gen_uint(values->preload.vertex.instance_id, 14, 14) |
            __gen_uint(values->preload.fragment.pc, 6, 6) |
            __gen_uint(values->preload.fragment.coverage, 7, 7) |
            __gen_uint(values->preload.fragment.primitive_id, 9, 9) |
            __gen_uint(values->preload.fragment.primitive_flags, 10, 10) |
            __gen_uint(values->preload.fragment.fragment_position, 11, 11) |
            __gen_uint(values->preload.fragment.sample_mask_id, 13, 13) |
            __gen_uint(values->preload.uniform_count, 15, 21) |
            __gen_uint(fui(values->alpha_reference), 0, 32);
   cl[13] = __gen_uint(values->thread_balancing, 0, 15) |
            __gen_uint(values->secondary_preload.compute.pc, 6, 6) |
            __gen_uint(values->secondary_preload.compute.local_invocation_xy, 7, 7) |
            __gen_uint(values->secondary_preload.compute.local_invocation_z, 8, 8) |
            __gen_uint(values->secondary_preload.compute.work_group_x, 9, 9) |
            __gen_uint(values->secondary_preload.compute.work_group_y, 10, 10) |
            __gen_uint(values->secondary_preload.compute.work_group_z, 11, 11) |
            __gen_uint(values->secondary_preload.compute.global_invocation_x, 12, 12) |
            __gen_uint(values->secondary_preload.compute.global_invocation_y, 13, 13) |
            __gen_uint(values->secondary_preload.compute.global_invocation_z, 14, 14) |
            __gen_uint(values->secondary_preload.vertex.warp_limit, 0, 1) |
            __gen_uint(values->secondary_preload.vertex.pc, 6, 6) |
            __gen_uint(values->secondary_preload.vertex.position_result_address_lo, 10, 10) |
            __gen_uint(values->secondary_preload.vertex.position_result_address_hi, 11, 11) |
            __gen_uint(values->secondary_preload.vertex.vertex_id, 13, 13) |
            __gen_uint(values->secondary_preload.vertex.instance_id, 14, 14) |
            __gen_uint(values->secondary_preload.fragment.pc, 6, 6) |
            __gen_uint(values->secondary_preload.fragment.coverage, 7, 7) |
            __gen_uint(values->secondary_preload.fragment.primitive_id, 9, 9) |
            __gen_uint(values->secondary_preload.fragment.primitive_flags, 10, 10) |
            __gen_uint(values->secondary_preload.fragment.fragment_position, 11, 11) |
            __gen_uint(values->secondary_preload.fragment.sample_mask_id, 13, 13) |
            __gen_uint(values->secondary_preload.uniform_count, 15, 21);
   cl[14] = __gen_uint(values->secondary_shader, 0, 63);
   cl[15] = __gen_uint(values->secondary_shader, 0, 63) >> 32;
}


#define MALI_RENDERER_STATE_LENGTH 64
#define MALI_RENDERER_STATE_ALIGN 64
struct mali_renderer_state_packed { uint32_t opaque[16]; };
static inline void
MALI_RENDERER_STATE_unpack(const uint8_t * restrict cl,
                           struct MALI_RENDERER_STATE * restrict values)
{
   if (((const uint32_t *) cl)[4] & 0xe606c400) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 4\n");
   if (((const uint32_t *) cl)[8] & 0x200000) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 8\n");
   if (((const uint32_t *) cl)[9] & 0xb180000) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 9\n");
   if (((const uint32_t *) cl)[10] & 0xf0000000) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 10\n");
   if (((const uint32_t *) cl)[11] & 0xf0000000) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 11\n");
   if (((const uint32_t *) cl)[13] & 0xffc00000) fprintf(stderr, "XXX: Invalid field of Renderer State unpacked at word 13\n");
   values->shader.shader = __gen_unpack_uint(cl, 0, 63);
   values->shader.sampler_count = __gen_unpack_uint(cl, 64, 79);
   values->shader.texture_count = __gen_unpack_uint(cl, 80, 95);
   values->shader.attribute_count = __gen_unpack_uint(cl, 96, 111);
   values->shader.varying_count = __gen_unpack_uint(cl, 112, 127);
   values->properties.uniform_buffer_count = __gen_unpack_uint(cl, 128, 135);
   values->properties.depth_source = (enum mali_depth_source)__gen_unpack_uint(cl, 136, 137);
   values->properties.shader_contains_barrier = __gen_unpack_uint(cl, 139, 139);
   values->properties.shader_register_allocation = (enum mali_shader_register_allocation)__gen_unpack_uint(cl, 140, 141);
   values->properties.shader_modifies_coverage = __gen_unpack_uint(cl, 144, 144);
   values->properties.allow_forward_pixel_to_kill = __gen_unpack_uint(cl, 147, 147);
   values->properties.allow_forward_pixel_to_be_killed = __gen_unpack_uint(cl, 148, 148);
   values->properties.pixel_kill_operation = (enum mali_pixel_kill)__gen_unpack_uint(cl, 149, 150);
   values->properties.zs_update_operation = (enum mali_pixel_kill)__gen_unpack_uint(cl, 151, 152);
   values->properties.point_sprite_coord_origin_max_y = __gen_unpack_uint(cl, 155, 155);
   values->properties.stencil_from_shader = __gen_unpack_uint(cl, 156, 156);
   values->depth_units = __gen_unpack_float(cl, 160, 191);
   values->depth_factor = __gen_unpack_float(cl, 192, 223);
   values->depth_bias_clamp = __gen_unpack_float(cl, 224, 255);
   values->multisample_misc.sample_mask = __gen_unpack_uint(cl, 256, 271);
   values->multisample_misc.multisample_enable = __gen_unpack_uint(cl, 272, 272);
   values->multisample_misc.multisample_late_coverage = __gen_unpack_uint(cl, 273, 273);
   values->multisample_misc.evaluate_per_sample = __gen_unpack_uint(cl, 274, 274);
   values->multisample_misc.fixed_function_depth_range_fixed = __gen_unpack_uint(cl, 275, 275);
   values->multisample_misc.shader_depth_range_fixed = __gen_unpack_uint(cl, 276, 276);
   values->multisample_misc.overdraw_alpha1 = __gen_unpack_uint(cl, 278, 278);
   values->multisample_misc.overdraw_alpha0 = __gen_unpack_uint(cl, 279, 279);
   values->multisample_misc.depth_function = (enum mali_func)__gen_unpack_uint(cl, 280, 282);
   values->multisample_misc.depth_write_mask = __gen_unpack_uint(cl, 283, 283);
   values->multisample_misc.fixed_function_near_discard = __gen_unpack_uint(cl, 284, 284);
   values->multisample_misc.fixed_function_far_discard = __gen_unpack_uint(cl, 285, 285);
   values->multisample_misc.fragment_near_discard = __gen_unpack_uint(cl, 286, 286);
   values->multisample_misc.fragment_far_discard = __gen_unpack_uint(cl, 287, 287);
   values->stencil_mask_misc.stencil_mask_front = __gen_unpack_uint(cl, 288, 295);
   values->stencil_mask_misc.stencil_mask_back = __gen_unpack_uint(cl, 296, 303);
   values->stencil_mask_misc.stencil_enable = __gen_unpack_uint(cl, 304, 304);
   values->stencil_mask_misc.alpha_to_coverage = __gen_unpack_uint(cl, 305, 305);
   values->stencil_mask_misc.alpha_to_coverage_invert = __gen_unpack_uint(cl, 306, 306);
   values->stencil_mask_misc.alpha_test_compare_function = (enum mali_func)__gen_unpack_uint(cl, 309, 311);
   values->stencil_mask_misc.force_seamless_cubemaps = __gen_unpack_uint(cl, 314, 314);
   values->stencil_mask_misc.depth_range_1 = __gen_unpack_uint(cl, 316, 316);
   values->stencil_mask_misc.depth_range_2 = __gen_unpack_uint(cl, 317, 317);
   values->stencil_mask_misc.single_sampled_lines = __gen_unpack_uint(cl, 318, 318);
   values->stencil_mask_misc.point_snap = __gen_unpack_uint(cl, 319, 319);
   values->stencil_front.reference_value = __gen_unpack_uint(cl, 320, 327);
   values->stencil_front.mask = __gen_unpack_uint(cl, 328, 335);
   values->stencil_front.compare_function = (enum mali_func)__gen_unpack_uint(cl, 336, 338);
   values->stencil_front.stencil_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 339, 341);
   values->stencil_front.depth_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 342, 344);
   values->stencil_front.depth_pass = (enum mali_stencil_op)__gen_unpack_uint(cl, 345, 347);
   values->stencil_back.reference_value = __gen_unpack_uint(cl, 352, 359);
   values->stencil_back.mask = __gen_unpack_uint(cl, 360, 367);
   values->stencil_back.compare_function = (enum mali_func)__gen_unpack_uint(cl, 368, 370);
   values->stencil_back.stencil_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 371, 373);
   values->stencil_back.depth_fail = (enum mali_stencil_op)__gen_unpack_uint(cl, 374, 376);
   values->stencil_back.depth_pass = (enum mali_stencil_op)__gen_unpack_uint(cl, 377, 379);
   values->preload.compute.pc = __gen_unpack_uint(cl, 390, 390);
   values->preload.compute.local_invocation_xy = __gen_unpack_uint(cl, 391, 391);
   values->preload.compute.local_invocation_z = __gen_unpack_uint(cl, 392, 392);
   values->preload.compute.work_group_x = __gen_unpack_uint(cl, 393, 393);
   values->preload.compute.work_group_y = __gen_unpack_uint(cl, 394, 394);
   values->preload.compute.work_group_z = __gen_unpack_uint(cl, 395, 395);
   values->preload.compute.global_invocation_x = __gen_unpack_uint(cl, 396, 396);
   values->preload.compute.global_invocation_y = __gen_unpack_uint(cl, 397, 397);
   values->preload.compute.global_invocation_z = __gen_unpack_uint(cl, 398, 398);
   values->preload.vertex.warp_limit = (enum mali_warp_limit)__gen_unpack_uint(cl, 384, 385);
   values->preload.vertex.pc = __gen_unpack_uint(cl, 390, 390);
   values->preload.vertex.position_result_address_lo = __gen_unpack_uint(cl, 394, 394);
   values->preload.vertex.position_result_address_hi = __gen_unpack_uint(cl, 395, 395);
   values->preload.vertex.vertex_id = __gen_unpack_uint(cl, 397, 397);
   values->preload.vertex.instance_id = __gen_unpack_uint(cl, 398, 398);
   values->preload.fragment.pc = __gen_unpack_uint(cl, 390, 390);
   values->preload.fragment.coverage = __gen_unpack_uint(cl, 391, 391);
   values->preload.fragment.primitive_id = __gen_unpack_uint(cl, 393, 393);
   values->preload.fragment.primitive_flags = __gen_unpack_uint(cl, 394, 394);
   values->preload.fragment.fragment_position = __gen_unpack_uint(cl, 395, 395);
   values->preload.fragment.sample_mask_id = __gen_unpack_uint(cl, 397, 397);
   values->preload.uniform_count = __gen_unpack_uint(cl, 399, 405);
   values->alpha_reference = __gen_unpack_float(cl, 384, 415);
   values->thread_balancing = __gen_unpack_uint(cl, 416, 431);
   values->secondary_preload.compute.pc = __gen_unpack_uint(cl, 422, 422);
   values->secondary_preload.compute.local_invocation_xy = __gen_unpack_uint(cl, 423, 423);
   values->secondary_preload.compute.local_invocation_z = __gen_unpack_uint(cl, 424, 424);
   values->secondary_preload.compute.work_group_x = __gen_unpack_uint(cl, 425, 425);
   values->secondary_preload.compute.work_group_y = __gen_unpack_uint(cl, 426, 426);
   values->secondary_preload.compute.work_group_z = __gen_unpack_uint(cl, 427, 427);
   values->secondary_preload.compute.global_invocation_x = __gen_unpack_uint(cl, 428, 428);
   values->secondary_preload.compute.global_invocation_y = __gen_unpack_uint(cl, 429, 429);
   values->secondary_preload.compute.global_invocation_z = __gen_unpack_uint(cl, 430, 430);
   values->secondary_preload.vertex.warp_limit = (enum mali_warp_limit)__gen_unpack_uint(cl, 416, 417);
   values->secondary_preload.vertex.pc = __gen_unpack_uint(cl, 422, 422);
   values->secondary_preload.vertex.position_result_address_lo = __gen_unpack_uint(cl, 426, 426);
   values->secondary_preload.vertex.position_result_address_hi = __gen_unpack_uint(cl, 427, 427);
   values->secondary_preload.vertex.vertex_id = __gen_unpack_uint(cl, 429, 429);
   values->secondary_preload.vertex.instance_id = __gen_unpack_uint(cl, 430, 430);
   values->secondary_preload.fragment.pc = __gen_unpack_uint(cl, 422, 422);
   values->secondary_preload.fragment.coverage = __gen_unpack_uint(cl, 423, 423);
   values->secondary_preload.fragment.primitive_id = __gen_unpack_uint(cl, 425, 425);
   values->secondary_preload.fragment.primitive_flags = __gen_unpack_uint(cl, 426, 426);
   values->secondary_preload.fragment.fragment_position = __gen_unpack_uint(cl, 427, 427);
   values->secondary_preload.fragment.sample_mask_id = __gen_unpack_uint(cl, 429, 429);
   values->secondary_preload.uniform_count = __gen_unpack_uint(cl, 431, 437);
   values->secondary_shader = __gen_unpack_uint(cl, 448, 511);
}

static inline void
MALI_RENDERER_STATE_print(FILE *fp, const struct MALI_RENDERER_STATE * values, unsigned indent)
{
   fprintf(fp, "%*sShader:\n", indent, "");
   MALI_SHADER_print(fp, &values->shader, indent + 2);
   fprintf(fp, "%*sProperties:\n", indent, "");
   MALI_RENDERER_PROPERTIES_print(fp, &values->properties, indent + 2);
   fprintf(fp, "%*sDepth units: %f\n", indent, "", values->depth_units);
   fprintf(fp, "%*sDepth factor: %f\n", indent, "", values->depth_factor);
   fprintf(fp, "%*sDepth bias clamp: %f\n", indent, "", values->depth_bias_clamp);
   fprintf(fp, "%*sMultisample, Misc:\n", indent, "");
   MALI_MULTISAMPLE_MISC_print(fp, &values->multisample_misc, indent + 2);
   fprintf(fp, "%*sStencil Mask, Misc:\n", indent, "");
   MALI_STENCIL_MASK_MISC_print(fp, &values->stencil_mask_misc, indent + 2);
   fprintf(fp, "%*sStencil front:\n", indent, "");
   MALI_STENCIL_print(fp, &values->stencil_front, indent + 2);
   fprintf(fp, "%*sStencil back:\n", indent, "");
   MALI_STENCIL_print(fp, &values->stencil_back, indent + 2);
   fprintf(fp, "%*sPreload:\n", indent, "");
   MALI_PRELOAD_print(fp, &values->preload, indent + 2);
   fprintf(fp, "%*sAlpha reference: %f\n", indent, "", values->alpha_reference);
   fprintf(fp, "%*sThread Balancing: %u\n", indent, "", values->thread_balancing);
   fprintf(fp, "%*sSecondary preload:\n", indent, "");
   MALI_PRELOAD_print(fp, &values->secondary_preload, indent + 2);
   fprintf(fp, "%*sSecondary shader: 0x%" PRIx64 "\n", indent, "", values->secondary_shader);
}

struct MALI_UNIFORM_BUFFER {
   uint32_t                             entries;
   uint64_t                             pointer;
};

#define MALI_UNIFORM_BUFFER_header              \
   0

static inline void
MALI_UNIFORM_BUFFER_pack(uint32_t * restrict cl,
                         const struct MALI_UNIFORM_BUFFER * restrict values)
{
   assert(values->entries >= 1);
   assert((values->pointer & 0xf) == 0);
   cl[ 0] = __gen_uint(values->entries - 1, 0, 11) |
            __gen_uint(values->pointer >> 4, 12, 63);
   cl[ 1] = __gen_uint(values->pointer >> 4, 12, 63) >> 32;
}


#define MALI_UNIFORM_BUFFER_LENGTH 8
#define MALI_UNIFORM_BUFFER_ALIGN 8
struct mali_uniform_buffer_packed { uint32_t opaque[2]; };
static inline void
MALI_UNIFORM_BUFFER_unpack(const uint8_t * restrict cl,
                           struct MALI_UNIFORM_BUFFER * restrict values)
{
   values->entries = __gen_unpack_uint(cl, 0, 11) + 1;
   values->pointer = __gen_unpack_uint(cl, 12, 63) << 4;
}

static inline void
MALI_UNIFORM_BUFFER_print(FILE *fp, const struct MALI_UNIFORM_BUFFER * values, unsigned indent)
{
   fprintf(fp, "%*sEntries: %u\n", indent, "", values->entries);
   fprintf(fp, "%*sPointer: 0x%" PRIx64 "\n", indent, "", values->pointer);
}

struct MALI_VIEWPORT {
   float                                minimum_x;
   float                                minimum_y;
   float                                maximum_x;
   float                                maximum_y;
   float                                minimum_z;
   float                                maximum_z;
   uint32_t                             scissor_minimum_x;
   uint32_t                             scissor_minimum_y;
   uint32_t                             scissor_maximum_x;
   uint32_t                             scissor_maximum_y;
};

#define MALI_VIEWPORT_header                    \
   .minimum_x = -INFINITY,  \
   .minimum_y = -INFINITY,  \
   .maximum_x = +INFINITY,  \
   .maximum_y = +INFINITY,  \
   .minimum_z = 0.0,  \
   .maximum_z = 1.0,  \
   .scissor_minimum_x = 0,  \
   .scissor_minimum_y = 0

static inline void
MALI_VIEWPORT_pack(uint32_t * restrict cl,
                   const struct MALI_VIEWPORT * restrict values)
{
   cl[ 0] = __gen_uint(fui(values->minimum_x), 0, 32);
   cl[ 1] = __gen_uint(fui(values->minimum_y), 0, 32);
   cl[ 2] = __gen_uint(fui(values->maximum_x), 0, 32);
   cl[ 3] = __gen_uint(fui(values->maximum_y), 0, 32);
   cl[ 4] = __gen_uint(fui(values->minimum_z), 0, 32);
   cl[ 5] = __gen_uint(fui(values->maximum_z), 0, 32);
   cl[ 6] = __gen_uint(values->scissor_minimum_x, 0, 15) |
            __gen_uint(values->scissor_minimum_y, 16, 31);
   cl[ 7] = __gen_uint(values->scissor_maximum_x, 0, 15) |
            __gen_uint(values->scissor_maximum_y, 16, 31);
}


#define MALI_VIEWPORT_LENGTH 32
#define MALI_VIEWPORT_ALIGN 32
struct mali_viewport_packed { uint32_t opaque[8]; };
static inline void
MALI_VIEWPORT_unpack(const uint8_t * restrict cl,
                     struct MALI_VIEWPORT * restrict values)
{
   values->minimum_x = __gen_unpack_float(cl, 0, 31);
   values->minimum_y = __gen_unpack_float(cl, 32, 63);
   values->maximum_x = __gen_unpack_float(cl, 64, 95);
   values->maximum_y = __gen_unpack_float(cl, 96, 127);
   values->minimum_z = __gen_unpack_float(cl, 128, 159);
   values->maximum_z = __gen_unpack_float(cl, 160, 191);
   values->scissor_minimum_x = __gen_unpack_uint(cl, 192, 207);
   values->scissor_minimum_y = __gen_unpack_uint(cl, 208, 223);
   values->scissor_maximum_x = __gen_unpack_uint(cl, 224, 239);
   values->scissor_maximum_y = __gen_unpack_uint(cl, 240, 255);
}

static inline void
MALI_VIEWPORT_print(FILE *fp, const struct MALI_VIEWPORT * values, unsigned indent)
{
   fprintf(fp, "%*sMinimum X: %f\n", indent, "", values->minimum_x);
   fprintf(fp, "%*sMinimum Y: %f\n", indent, "", values->minimum_y);
   fprintf(fp, "%*sMaximum X: %f\n", indent, "", values->maximum_x);
   fprintf(fp, "%*sMaximum Y: %f\n", indent, "", values->maximum_y);
   fprintf(fp, "%*sMinimum Z: %f\n", indent, "", values->minimum_z);
   fprintf(fp, "%*sMaximum Z: %f\n", indent, "", values->maximum_z);
   fprintf(fp, "%*sScissor Minimum X: %u\n", indent, "", values->scissor_minimum_x);
   fprintf(fp, "%*sScissor Minimum Y: %u\n", indent, "", values->scissor_minimum_y);
   fprintf(fp, "%*sScissor Maximum X: %u\n", indent, "", values->scissor_maximum_x);
   fprintf(fp, "%*sScissor Maximum Y: %u\n", indent, "", values->scissor_maximum_y);
}

struct MALI_LOCAL_STORAGE {
   uint32_t                             tls_size;
   uint32_t                             tls_initial_stack_pointer_offset;
   uint32_t                             wls_instances;
#define MALI_LOCAL_STORAGE_NO_WORKGROUP_MEM      2147483648
   uint32_t                             wls_size_base;
   uint32_t                             wls_size_scale;
   uint64_t                             tls_base_pointer;
   uint64_t                             wls_base_pointer;
};

#define MALI_LOCAL_STORAGE_header               \
   .wls_instances = MALI_LOCAL_STORAGE_NO_WORKGROUP_MEM

static inline void
MALI_LOCAL_STORAGE_pack(uint32_t * restrict cl,
                        const struct MALI_LOCAL_STORAGE * restrict values)
{
   assert(util_is_power_of_two_nonzero(values->wls_instances));
   cl[ 0] = __gen_uint(values->tls_size, 0, 4) |
            __gen_uint(values->tls_initial_stack_pointer_offset, 5, 31);
   cl[ 1] = __gen_uint(util_logbase2(values->wls_instances), 0, 4) |
            __gen_uint(values->wls_size_base, 5, 6) |
            __gen_uint(values->wls_size_scale, 8, 12);
   cl[ 2] = __gen_uint(values->tls_base_pointer, 0, 63);
   cl[ 3] = __gen_uint(values->tls_base_pointer, 0, 63) >> 32;
   cl[ 4] = __gen_uint(values->wls_base_pointer, 0, 63);
   cl[ 5] = __gen_uint(values->wls_base_pointer, 0, 63) >> 32;
   cl[ 6] = 0;
   cl[ 7] = 0;
}


#define MALI_LOCAL_STORAGE_LENGTH 32
#define MALI_LOCAL_STORAGE_ALIGN 64
struct mali_local_storage_packed { uint32_t opaque[8]; };
static inline void
MALI_LOCAL_STORAGE_unpack(const uint8_t * restrict cl,
                          struct MALI_LOCAL_STORAGE * restrict values)
{
   if (((const uint32_t *) cl)[1] & 0xffffe080) fprintf(stderr, "XXX: Invalid field of Local Storage unpacked at word 1\n");
   if (((const uint32_t *) cl)[6] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Local Storage unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Local Storage unpacked at word 7\n");
   values->tls_size = __gen_unpack_uint(cl, 0, 4);
   values->tls_initial_stack_pointer_offset = __gen_unpack_uint(cl, 5, 31);
   values->wls_instances = 1U << __gen_unpack_uint(cl, 32, 36);
   values->wls_size_base = __gen_unpack_uint(cl, 37, 38);
   values->wls_size_scale = __gen_unpack_uint(cl, 40, 44);
   values->tls_base_pointer = __gen_unpack_uint(cl, 64, 127);
   values->wls_base_pointer = __gen_unpack_uint(cl, 128, 191);
}

static inline void
MALI_LOCAL_STORAGE_print(FILE *fp, const struct MALI_LOCAL_STORAGE * values, unsigned indent)
{
   fprintf(fp, "%*sTLS Size: %u\n", indent, "", values->tls_size);
   fprintf(fp, "%*sTLS Initial Stack Pointer Offset: %u\n", indent, "", values->tls_initial_stack_pointer_offset);
   fprintf(fp, "%*sWLS Instances: %u\n", indent, "", values->wls_instances);
   fprintf(fp, "%*sWLS Size Base: %u\n", indent, "", values->wls_size_base);
   fprintf(fp, "%*sWLS Size Scale: %u\n", indent, "", values->wls_size_scale);
   fprintf(fp, "%*sTLS Base Pointer: 0x%" PRIx64 "\n", indent, "", values->tls_base_pointer);
   fprintf(fp, "%*sWLS Base Pointer: 0x%" PRIx64 "\n", indent, "", values->wls_base_pointer);
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

enum mali_color_format {
        MALI_COLOR_FORMAT_RAW8               =      0,
        MALI_COLOR_FORMAT_RAW16              =      1,
        MALI_COLOR_FORMAT_RAW24              =      2,
        MALI_COLOR_FORMAT_RAW32              =      3,
        MALI_COLOR_FORMAT_RAW48              =      4,
        MALI_COLOR_FORMAT_RAW64              =      5,
        MALI_COLOR_FORMAT_RAW96              =      6,
        MALI_COLOR_FORMAT_RAW128             =      7,
        MALI_COLOR_FORMAT_RAW192             =      8,
        MALI_COLOR_FORMAT_RAW256             =      9,
        MALI_COLOR_FORMAT_RAW384             =     10,
        MALI_COLOR_FORMAT_RAW512             =     11,
        MALI_COLOR_FORMAT_RAW768             =     12,
        MALI_COLOR_FORMAT_RAW1024            =     13,
        MALI_COLOR_FORMAT_RAW1536            =     14,
        MALI_COLOR_FORMAT_RAW2048            =     15,
        MALI_COLOR_FORMAT_R8                 =     16,
        MALI_COLOR_FORMAT_R8G8               =     17,
        MALI_COLOR_FORMAT_R8G8B8             =     18,
        MALI_COLOR_FORMAT_R8G8B8A8           =     19,
        MALI_COLOR_FORMAT_R4G4B4A4           =     20,
        MALI_COLOR_FORMAT_R5G6B5             =     21,
        MALI_COLOR_FORMAT_R8G8B8_FROM_R8G8B8A2 =     22,
        MALI_COLOR_FORMAT_R10G10B10A2        =     24,
        MALI_COLOR_FORMAT_A2B10G10R10        =     25,
        MALI_COLOR_FORMAT_R5G5B5A1           =     28,
        MALI_COLOR_FORMAT_A1B5G5R5           =     29,
        MALI_COLOR_FORMAT_NATIVE             =     31,
};

static inline const char *
mali_color_format_as_str(enum mali_color_format imm)
{
    switch (imm) {
    case MALI_COLOR_FORMAT_RAW8: return "RAW8";
    case MALI_COLOR_FORMAT_RAW16: return "RAW16";
    case MALI_COLOR_FORMAT_RAW24: return "RAW24";
    case MALI_COLOR_FORMAT_RAW32: return "RAW32";
    case MALI_COLOR_FORMAT_RAW48: return "RAW48";
    case MALI_COLOR_FORMAT_RAW64: return "RAW64";
    case MALI_COLOR_FORMAT_RAW96: return "RAW96";
    case MALI_COLOR_FORMAT_RAW128: return "RAW128";
    case MALI_COLOR_FORMAT_RAW192: return "RAW192";
    case MALI_COLOR_FORMAT_RAW256: return "RAW256";
    case MALI_COLOR_FORMAT_RAW384: return "RAW384";
    case MALI_COLOR_FORMAT_RAW512: return "RAW512";
    case MALI_COLOR_FORMAT_RAW768: return "RAW768";
    case MALI_COLOR_FORMAT_RAW1024: return "RAW1024";
    case MALI_COLOR_FORMAT_RAW1536: return "RAW1536";
    case MALI_COLOR_FORMAT_RAW2048: return "RAW2048";
    case MALI_COLOR_FORMAT_R8: return "R8";
    case MALI_COLOR_FORMAT_R8G8: return "R8G8";
    case MALI_COLOR_FORMAT_R8G8B8: return "R8G8B8";
    case MALI_COLOR_FORMAT_R8G8B8A8: return "R8G8B8A8";
    case MALI_COLOR_FORMAT_R4G4B4A4: return "R4G4B4A4";
    case MALI_COLOR_FORMAT_R5G6B5: return "R5G6B5";
    case MALI_COLOR_FORMAT_R8G8B8_FROM_R8G8B8A2: return "R8G8B8_FROM_R8G8B8A2";
    case MALI_COLOR_FORMAT_R10G10B10A2: return "R10G10B10A2";
    case MALI_COLOR_FORMAT_A2B10G10R10: return "A2B10G10R10";
    case MALI_COLOR_FORMAT_R5G5B5A1: return "R5G5B5A1";
    case MALI_COLOR_FORMAT_A1B5G5R5: return "A1B5G5R5";
    case MALI_COLOR_FORMAT_NATIVE: return "NATIVE";
    default: return "XXX: INVALID";
    }
}

enum mali_downsampling_accumulation_mode {
        MALI_DOWNSAMPLING_ACCUMULATION_MODE_UNSIGNED_NORMALIZED_INTEGER =      0,
        MALI_DOWNSAMPLING_ACCUMULATION_MODE_SIGNED_NORMALIZED_INTEGER =      1,
};

static inline const char *
mali_downsampling_accumulation_mode_as_str(enum mali_downsampling_accumulation_mode imm)
{
    switch (imm) {
    case MALI_DOWNSAMPLING_ACCUMULATION_MODE_UNSIGNED_NORMALIZED_INTEGER: return "Unsigned normalized integer";
    case MALI_DOWNSAMPLING_ACCUMULATION_MODE_SIGNED_NORMALIZED_INTEGER: return "Signed normalized integer";
    default: return "XXX: INVALID";
    }
}

enum mali_sample_layout {
        MALI_SAMPLE_LAYOUT_ORDERED_4X_GRID   =      0,
        MALI_SAMPLE_LAYOUT_ROTATED_4X_GRID   =      1,
        MALI_SAMPLE_LAYOUT_D3D_8X_GRID       =      2,
        MALI_SAMPLE_LAYOUT_D3D_16X_GRID      =      3,
};

static inline const char *
mali_sample_layout_as_str(enum mali_sample_layout imm)
{
    switch (imm) {
    case MALI_SAMPLE_LAYOUT_ORDERED_4X_GRID: return "Ordered 4x Grid";
    case MALI_SAMPLE_LAYOUT_ROTATED_4X_GRID: return "Rotated 4x Grid";
    case MALI_SAMPLE_LAYOUT_D3D_8X_GRID: return "D3D 8x Grid";
    case MALI_SAMPLE_LAYOUT_D3D_16X_GRID: return "D3D 16x Grid";
    default: return "XXX: INVALID";
    }
}

enum mali_zs_format {
        MALI_ZS_FORMAT_D16                   =      1,
        MALI_ZS_FORMAT_D24                   =      2,
        MALI_ZS_FORMAT_D24X8                 =      4,
        MALI_ZS_FORMAT_D24S8                 =      5,
        MALI_ZS_FORMAT_X8D24                 =      6,
        MALI_ZS_FORMAT_S8D24                 =      7,
        MALI_ZS_FORMAT_D32_X8X24             =     13,
        MALI_ZS_FORMAT_D32                   =     14,
        MALI_ZS_FORMAT_D32_S8X24             =     15,
};

static inline const char *
mali_zs_format_as_str(enum mali_zs_format imm)
{
    switch (imm) {
    case MALI_ZS_FORMAT_D16: return "D16";
    case MALI_ZS_FORMAT_D24: return "D24";
    case MALI_ZS_FORMAT_D24X8: return "D24X8";
    case MALI_ZS_FORMAT_D24S8: return "D24S8";
    case MALI_ZS_FORMAT_X8D24: return "X8D24";
    case MALI_ZS_FORMAT_S8D24: return "S8D24";
    case MALI_ZS_FORMAT_D32_X8X24: return "D32_X8X24";
    case MALI_ZS_FORMAT_D32: return "D32";
    case MALI_ZS_FORMAT_D32_S8X24: return "D32_S8X24";
    default: return "XXX: INVALID";
    }
}

enum mali_zs_preload_format {
        MALI_ZS_PRELOAD_FORMAT_D32_S8X24     =      4,
};

static inline const char *
mali_zs_preload_format_as_str(enum mali_zs_preload_format imm)
{
    switch (imm) {
    case MALI_ZS_PRELOAD_FORMAT_D32_S8X24: return "D32_S8X24";
    default: return "XXX: INVALID";
    }
}

enum mali_s_format {
        MALI_S_FORMAT_S8                     =      1,
        MALI_S_FORMAT_S8X8                   =      2,
        MALI_S_FORMAT_S8X24                  =      3,
        MALI_S_FORMAT_X24S8                  =      4,
        MALI_S_FORMAT_X8S8                   =      5,
        MALI_S_FORMAT_X32_S8X24              =      6,
};

static inline const char *
mali_s_format_as_str(enum mali_s_format imm)
{
    switch (imm) {
    case MALI_S_FORMAT_S8: return "S8";
    case MALI_S_FORMAT_S8X8: return "S8X8";
    case MALI_S_FORMAT_S8X24: return "S8X24";
    case MALI_S_FORMAT_X24S8: return "X24S8";
    case MALI_S_FORMAT_X8S8: return "X8S8";
    case MALI_S_FORMAT_X32_S8X24: return "X32_S8X24";
    default: return "XXX: INVALID";
    }
}

enum mali_tie_break_rule {
        MALI_TIE_BREAK_RULE_0_IN_180_OUT     =      0,
        MALI_TIE_BREAK_RULE_0_OUT_180_IN     =      1,
        MALI_TIE_BREAK_RULE_MINUS_180_IN_0_OUT =      2,
        MALI_TIE_BREAK_RULE_MINUS_180_OUT_0_IN =      3,
        MALI_TIE_BREAK_RULE_90_IN_270_OUT    =      4,
        MALI_TIE_BREAK_RULE_90_OUT_270_IN    =      5,
        MALI_TIE_BREAK_RULE_MINUS_90_IN_90_OUT =      6,
        MALI_TIE_BREAK_RULE_MINUS_90_OUT_90_IN =      7,
};

static inline const char *
mali_tie_break_rule_as_str(enum mali_tie_break_rule imm)
{
    switch (imm) {
    case MALI_TIE_BREAK_RULE_0_IN_180_OUT: return "0_IN_180_OUT";
    case MALI_TIE_BREAK_RULE_0_OUT_180_IN: return "0_OUT_180_IN";
    case MALI_TIE_BREAK_RULE_MINUS_180_IN_0_OUT: return "MINUS_180_IN_0_OUT";
    case MALI_TIE_BREAK_RULE_MINUS_180_OUT_0_IN: return "MINUS_180_OUT_0_IN";
    case MALI_TIE_BREAK_RULE_90_IN_270_OUT: return "90_IN_270_OUT";
    case MALI_TIE_BREAK_RULE_90_OUT_270_IN: return "90_OUT_270_IN";
    case MALI_TIE_BREAK_RULE_MINUS_90_IN_90_OUT: return "MINUS_90_IN_90_OUT";
    case MALI_TIE_BREAK_RULE_MINUS_90_OUT_90_IN: return "MINUS_90_OUT_90_IN";
    default: return "XXX: INVALID";
    }
}

struct MALI_RT_BUFFER {
   uint64_t                             base;
   uint32_t                             row_stride;
   uint32_t                             surface_stride;
};

#define MALI_RT_BUFFER_header                   \
   0

static inline void
MALI_RT_BUFFER_pack(uint32_t * restrict cl,
                    const struct MALI_RT_BUFFER * restrict values)
{
   cl[ 0] = __gen_uint(values->base, 0, 63);
   cl[ 1] = __gen_uint(values->base, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->row_stride, 0, 31);
   cl[ 3] = __gen_uint(values->surface_stride, 0, 31);
}


#define MALI_RT_BUFFER_LENGTH 16
struct mali_rt_buffer_packed { uint32_t opaque[4]; };
static inline void
MALI_RT_BUFFER_unpack(const uint8_t * restrict cl,
                      struct MALI_RT_BUFFER * restrict values)
{
   values->base = __gen_unpack_uint(cl, 0, 63);
   values->row_stride = __gen_unpack_uint(cl, 64, 95);
   values->surface_stride = __gen_unpack_uint(cl, 96, 127);
}

static inline void
MALI_RT_BUFFER_print(FILE *fp, const struct MALI_RT_BUFFER * values, unsigned indent)
{
   fprintf(fp, "%*sBase: 0x%" PRIx64 "\n", indent, "", values->base);
   fprintf(fp, "%*sRow Stride: %u\n", indent, "", values->row_stride);
   fprintf(fp, "%*sSurface Stride: %u\n", indent, "", values->surface_stride);
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

enum mali_z_internal_format {
        MALI_Z_INTERNAL_FORMAT_D16           =      0,
        MALI_Z_INTERNAL_FORMAT_D24           =      1,
        MALI_Z_INTERNAL_FORMAT_D32           =      2,
};

static inline const char *
mali_z_internal_format_as_str(enum mali_z_internal_format imm)
{
    switch (imm) {
    case MALI_Z_INTERNAL_FORMAT_D16: return "D16";
    case MALI_Z_INTERNAL_FORMAT_D24: return "D24";
    case MALI_Z_INTERNAL_FORMAT_D32: return "D32";
    default: return "XXX: INVALID";
    }
}

enum mali_fbd_tag {
        MALI_FBD_TAG_IS_MFBD                 =      1,
        MALI_FBD_TAG_HAS_ZS_RT               =      2,
        MALI_FBD_TAG_MASK                    =     63,
};

static inline const char *
mali_fbd_tag_as_str(enum mali_fbd_tag imm)
{
    switch (imm) {
    case MALI_FBD_TAG_IS_MFBD: return "IS_MFBD";
    case MALI_FBD_TAG_HAS_ZS_RT: return "HAS_ZS_RT";
    case MALI_FBD_TAG_MASK: return "MASK";
    default: return "XXX: INVALID";
    }
}

enum mali_pre_post_frame_shader_mode {
        MALI_PRE_POST_FRAME_SHADER_MODE_NEVER =      0,
        MALI_PRE_POST_FRAME_SHADER_MODE_ALWAYS =      1,
        MALI_PRE_POST_FRAME_SHADER_MODE_INTERSECT =      2,
        MALI_PRE_POST_FRAME_SHADER_MODE_EARLY_ZS_ALWAYS =      3,
};

static inline const char *
mali_pre_post_frame_shader_mode_as_str(enum mali_pre_post_frame_shader_mode imm)
{
    switch (imm) {
    case MALI_PRE_POST_FRAME_SHADER_MODE_NEVER: return "Never";
    case MALI_PRE_POST_FRAME_SHADER_MODE_ALWAYS: return "Always";
    case MALI_PRE_POST_FRAME_SHADER_MODE_INTERSECT: return "Intersect";
    case MALI_PRE_POST_FRAME_SHADER_MODE_EARLY_ZS_ALWAYS: return "Early ZS always";
    default: return "XXX: INVALID";
    }
}

struct MALI_FRAMEBUFFER_PARAMETERS {
   enum mali_pre_post_frame_shader_mode pre_frame_0;
   enum mali_pre_post_frame_shader_mode pre_frame_1;
   enum mali_pre_post_frame_shader_mode post_frame;
   uint64_t                             sample_locations;
   uint64_t                             frame_shader_dcds;
   uint32_t                             width;
   uint32_t                             height;
   uint32_t                             bound_min_x;
   uint32_t                             bound_min_y;
   uint32_t                             bound_max_x;
   uint32_t                             bound_max_y;
   uint32_t                             sample_count;
   enum mali_sample_pattern             sample_pattern;
   enum mali_tie_break_rule             tie_break_rule;
   uint32_t                             effective_tile_size;
   uint32_t                             x_downsampling_scale;
   uint32_t                             y_downsampling_scale;
   uint32_t                             render_target_count;
   uint32_t                             color_buffer_allocation;
   uint32_t                             s_clear;
   bool                                 s_write_enable;
   bool                                 s_preload_enable;
   bool                                 s_unload_enable;
   enum mali_z_internal_format          z_internal_format;
   bool                                 z_write_enable;
   bool                                 z_preload_enable;
   bool                                 z_unload_enable;
   bool                                 has_zs_crc_extension;
   bool                                 crc_read_enable;
   bool                                 crc_write_enable;
   float                                z_clear;
   uint64_t                             tiler;
};

#define MALI_FRAMEBUFFER_PARAMETERS_header      \
   .sample_count = 1

static inline void
MALI_FRAMEBUFFER_PARAMETERS_pack(uint32_t * restrict cl,
                                 const struct MALI_FRAMEBUFFER_PARAMETERS * restrict values)
{
   assert(values->width >= 1);
   assert(values->height >= 1);
   assert(util_is_power_of_two_nonzero(values->sample_count));
   assert(util_is_power_of_two_nonzero(values->effective_tile_size));
   assert(values->render_target_count >= 1);
   assert((values->color_buffer_allocation & 0x3ff) == 0);
   cl[ 0] = __gen_uint(values->pre_frame_0, 0, 2) |
            __gen_uint(values->pre_frame_1, 3, 5) |
            __gen_uint(values->post_frame, 6, 8);
   cl[ 1] = 0;
   cl[ 2] = 0;
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->sample_locations, 0, 63);
   cl[ 5] = __gen_uint(values->sample_locations, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->frame_shader_dcds, 0, 63);
   cl[ 7] = __gen_uint(values->frame_shader_dcds, 0, 63) >> 32;
   cl[ 8] = __gen_uint(values->width - 1, 0, 15) |
            __gen_uint(values->height - 1, 16, 31);
   cl[ 9] = __gen_uint(values->bound_min_x, 0, 15) |
            __gen_uint(values->bound_min_y, 16, 31);
   cl[10] = __gen_uint(values->bound_max_x, 0, 15) |
            __gen_uint(values->bound_max_y, 16, 31);
   cl[11] = __gen_uint(util_logbase2(values->sample_count), 0, 2) |
            __gen_uint(values->sample_pattern, 3, 5) |
            __gen_uint(values->tie_break_rule, 6, 8) |
            __gen_uint(util_logbase2(values->effective_tile_size), 9, 12) |
            __gen_uint(values->x_downsampling_scale, 13, 15) |
            __gen_uint(values->y_downsampling_scale, 16, 18) |
            __gen_uint(values->render_target_count - 1, 19, 22) |
            __gen_uint(values->color_buffer_allocation >> 10, 24, 31);
   cl[12] = __gen_uint(values->s_clear, 0, 7) |
            __gen_uint(values->s_write_enable, 8, 8) |
            __gen_uint(values->s_preload_enable, 9, 9) |
            __gen_uint(values->s_unload_enable, 10, 10) |
            __gen_uint(values->z_internal_format, 16, 17) |
            __gen_uint(values->z_write_enable, 18, 18) |
            __gen_uint(values->z_preload_enable, 19, 19) |
            __gen_uint(values->z_unload_enable, 20, 20) |
            __gen_uint(values->has_zs_crc_extension, 21, 21) |
            __gen_uint(values->crc_read_enable, 30, 30) |
            __gen_uint(values->crc_write_enable, 31, 31);
   cl[13] = __gen_uint(fui(values->z_clear), 0, 32);
   cl[14] = __gen_uint(values->tiler, 0, 63);
   cl[15] = __gen_uint(values->tiler, 0, 63) >> 32;
}


#define MALI_FRAMEBUFFER_PARAMETERS_LENGTH 64
struct mali_framebuffer_parameters_packed { uint32_t opaque[16]; };
static inline void
MALI_FRAMEBUFFER_PARAMETERS_unpack(const uint8_t * restrict cl,
                                   struct MALI_FRAMEBUFFER_PARAMETERS * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xfffffe00) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 3\n");
   if (((const uint32_t *) cl)[11] & 0x800000) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 11\n");
   if (((const uint32_t *) cl)[12] & 0x3fc0f800) fprintf(stderr, "XXX: Invalid field of Framebuffer Parameters unpacked at word 12\n");
   values->pre_frame_0 = (enum mali_pre_post_frame_shader_mode)__gen_unpack_uint(cl, 0, 2);
   values->pre_frame_1 = (enum mali_pre_post_frame_shader_mode)__gen_unpack_uint(cl, 3, 5);
   values->post_frame = (enum mali_pre_post_frame_shader_mode)__gen_unpack_uint(cl, 6, 8);
   values->sample_locations = __gen_unpack_uint(cl, 128, 191);
   values->frame_shader_dcds = __gen_unpack_uint(cl, 192, 255);
   values->width = __gen_unpack_uint(cl, 256, 271) + 1;
   values->height = __gen_unpack_uint(cl, 272, 287) + 1;
   values->bound_min_x = __gen_unpack_uint(cl, 288, 303);
   values->bound_min_y = __gen_unpack_uint(cl, 304, 319);
   values->bound_max_x = __gen_unpack_uint(cl, 320, 335);
   values->bound_max_y = __gen_unpack_uint(cl, 336, 351);
   values->sample_count = 1U << __gen_unpack_uint(cl, 352, 354);
   values->sample_pattern = (enum mali_sample_pattern)__gen_unpack_uint(cl, 355, 357);
   values->tie_break_rule = (enum mali_tie_break_rule)__gen_unpack_uint(cl, 358, 360);
   values->effective_tile_size = 1U << __gen_unpack_uint(cl, 361, 364);
   values->x_downsampling_scale = __gen_unpack_uint(cl, 365, 367);
   values->y_downsampling_scale = __gen_unpack_uint(cl, 368, 370);
   values->render_target_count = __gen_unpack_uint(cl, 371, 374) + 1;
   values->color_buffer_allocation = __gen_unpack_uint(cl, 376, 383) << 10;
   values->s_clear = __gen_unpack_uint(cl, 384, 391);
   values->s_write_enable = __gen_unpack_uint(cl, 392, 392);
   values->s_preload_enable = __gen_unpack_uint(cl, 393, 393);
   values->s_unload_enable = __gen_unpack_uint(cl, 394, 394);
   values->z_internal_format = (enum mali_z_internal_format)__gen_unpack_uint(cl, 400, 401);
   values->z_write_enable = __gen_unpack_uint(cl, 402, 402);
   values->z_preload_enable = __gen_unpack_uint(cl, 403, 403);
   values->z_unload_enable = __gen_unpack_uint(cl, 404, 404);
   values->has_zs_crc_extension = __gen_unpack_uint(cl, 405, 405);
   values->crc_read_enable = __gen_unpack_uint(cl, 414, 414);
   values->crc_write_enable = __gen_unpack_uint(cl, 415, 415);
   values->z_clear = __gen_unpack_float(cl, 416, 447);
   values->tiler = __gen_unpack_uint(cl, 448, 511);
}

static inline void
MALI_FRAMEBUFFER_PARAMETERS_print(FILE *fp, const struct MALI_FRAMEBUFFER_PARAMETERS * values, unsigned indent)
{
   fprintf(fp, "%*sPre Frame 0: %s\n", indent, "", mali_pre_post_frame_shader_mode_as_str(values->pre_frame_0));
   fprintf(fp, "%*sPre Frame 1: %s\n", indent, "", mali_pre_post_frame_shader_mode_as_str(values->pre_frame_1));
   fprintf(fp, "%*sPost Frame: %s\n", indent, "", mali_pre_post_frame_shader_mode_as_str(values->post_frame));
   fprintf(fp, "%*sSample Locations: 0x%" PRIx64 "\n", indent, "", values->sample_locations);
   fprintf(fp, "%*sFrame Shader DCDs: 0x%" PRIx64 "\n", indent, "", values->frame_shader_dcds);
   fprintf(fp, "%*sWidth: %u\n", indent, "", values->width);
   fprintf(fp, "%*sHeight: %u\n", indent, "", values->height);
   fprintf(fp, "%*sBound Min X: %u\n", indent, "", values->bound_min_x);
   fprintf(fp, "%*sBound Min Y: %u\n", indent, "", values->bound_min_y);
   fprintf(fp, "%*sBound Max X: %u\n", indent, "", values->bound_max_x);
   fprintf(fp, "%*sBound Max Y: %u\n", indent, "", values->bound_max_y);
   fprintf(fp, "%*sSample Count: %u\n", indent, "", values->sample_count);
   fprintf(fp, "%*sSample Pattern: %s\n", indent, "", mali_sample_pattern_as_str(values->sample_pattern));
   fprintf(fp, "%*sTie-Break Rule: %s\n", indent, "", mali_tie_break_rule_as_str(values->tie_break_rule));
   fprintf(fp, "%*sEffective Tile Size: %u\n", indent, "", values->effective_tile_size);
   fprintf(fp, "%*sX Downsampling Scale: %u\n", indent, "", values->x_downsampling_scale);
   fprintf(fp, "%*sY Downsampling Scale: %u\n", indent, "", values->y_downsampling_scale);
   fprintf(fp, "%*sRender Target Count: %u\n", indent, "", values->render_target_count);
   fprintf(fp, "%*sColor Buffer Allocation: %u\n", indent, "", values->color_buffer_allocation);
   fprintf(fp, "%*sS Clear: %u\n", indent, "", values->s_clear);
   fprintf(fp, "%*sS Write Enable: %s\n", indent, "", values->s_write_enable ? "true" : "false");
   fprintf(fp, "%*sS Preload Enable: %s\n", indent, "", values->s_preload_enable ? "true" : "false");
   fprintf(fp, "%*sS Unload Enable: %s\n", indent, "", values->s_unload_enable ? "true" : "false");
   fprintf(fp, "%*sZ Internal Format: %s\n", indent, "", mali_z_internal_format_as_str(values->z_internal_format));
   fprintf(fp, "%*sZ Write Enable: %s\n", indent, "", values->z_write_enable ? "true" : "false");
   fprintf(fp, "%*sZ Preload Enable: %s\n", indent, "", values->z_preload_enable ? "true" : "false");
   fprintf(fp, "%*sZ Unload Enable: %s\n", indent, "", values->z_unload_enable ? "true" : "false");
   fprintf(fp, "%*sHas ZS CRC Extension: %s\n", indent, "", values->has_zs_crc_extension ? "true" : "false");
   fprintf(fp, "%*sCRC Read Enable: %s\n", indent, "", values->crc_read_enable ? "true" : "false");
   fprintf(fp, "%*sCRC Write Enable: %s\n", indent, "", values->crc_write_enable ? "true" : "false");
   fprintf(fp, "%*sZ Clear: %f\n", indent, "", values->z_clear);
   fprintf(fp, "%*sTiler: 0x%" PRIx64 "\n", indent, "", values->tiler);
}

struct MALI_ZS_CRC_EXTENSION {
   uint64_t                             crc_base;
   uint32_t                             crc_row_stride;
   enum mali_zs_format                  zs_write_format;
   enum mali_block_format               zs_block_format;
   enum mali_msaa                       zs_msaa;
   bool                                 zs_big_endian;
   bool                                 zs_clean_pixel_write_enable;
   enum mali_s_format                   s_write_format;
   enum mali_block_format               s_block_format;
   enum mali_msaa                       s_msaa;
   uint64_t                             zs_writeback_base;
   uint32_t                             zs_writeback_row_stride;
   uint32_t                             zs_writeback_surface_stride;
   uint64_t                             s_writeback_base;
   uint32_t                             s_writeback_row_stride;
   uint32_t                             s_writeback_surface_stride;
   uint64_t                             zs_afbc_header;
   uint32_t                             zs_afbc_row_stride;
   uint64_t                             zs_afbc_body;
};

#define MALI_ZS_CRC_EXTENSION_header            \
   .zs_msaa = MALI_MSAA_SINGLE,  \
   .s_msaa = MALI_MSAA_SINGLE

static inline void
MALI_ZS_CRC_EXTENSION_pack(uint32_t * restrict cl,
                           const struct MALI_ZS_CRC_EXTENSION * restrict values)
{
   cl[ 0] = __gen_uint(values->crc_base, 0, 63);
   cl[ 1] = __gen_uint(values->crc_base, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->crc_row_stride, 0, 31);
   cl[ 3] = __gen_uint(values->zs_write_format, 0, 3) |
            __gen_uint(values->zs_block_format, 4, 5) |
            __gen_uint(values->zs_msaa, 6, 7) |
            __gen_uint(values->zs_big_endian, 8, 8) |
            __gen_uint(values->zs_clean_pixel_write_enable, 10, 10) |
            __gen_uint(values->s_write_format, 16, 19) |
            __gen_uint(values->s_block_format, 20, 21) |
            __gen_uint(values->s_msaa, 22, 23);
   cl[ 4] = __gen_uint(values->zs_writeback_base, 0, 63) |
            __gen_uint(values->zs_afbc_header, 0, 63);
   cl[ 5] = __gen_uint(values->zs_writeback_base, 0, 63) >> 32 |
            __gen_uint(values->zs_afbc_header, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->zs_writeback_row_stride, 0, 31) |
            __gen_uint(values->zs_afbc_row_stride, 0, 12);
   cl[ 7] = __gen_uint(values->zs_writeback_surface_stride, 0, 31);
   cl[ 8] = __gen_uint(values->s_writeback_base, 0, 63) |
            __gen_uint(values->zs_afbc_body, 0, 63);
   cl[ 9] = __gen_uint(values->s_writeback_base, 0, 63) >> 32 |
            __gen_uint(values->zs_afbc_body, 0, 63) >> 32;
   cl[10] = __gen_uint(values->s_writeback_row_stride, 0, 31);
   cl[11] = __gen_uint(values->s_writeback_surface_stride, 0, 31);
   cl[12] = 0;
   cl[13] = 0;
   cl[14] = 0;
   cl[15] = 0;
}


#define MALI_ZS_CRC_EXTENSION_LENGTH 64
#define MALI_ZS_CRC_EXTENSION_ALIGN 64
struct mali_zs_crc_extension_packed { uint32_t opaque[16]; };
static inline void
MALI_ZS_CRC_EXTENSION_unpack(const uint8_t * restrict cl,
                             struct MALI_ZS_CRC_EXTENSION * restrict values)
{
   if (((const uint32_t *) cl)[3] & 0xff00fa00) fprintf(stderr, "XXX: Invalid field of ZS CRC Extension unpacked at word 3\n");
   if (((const uint32_t *) cl)[12] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of ZS CRC Extension unpacked at word 12\n");
   if (((const uint32_t *) cl)[13] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of ZS CRC Extension unpacked at word 13\n");
   if (((const uint32_t *) cl)[14] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of ZS CRC Extension unpacked at word 14\n");
   if (((const uint32_t *) cl)[15] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of ZS CRC Extension unpacked at word 15\n");
   values->crc_base = __gen_unpack_uint(cl, 0, 63);
   values->crc_row_stride = __gen_unpack_uint(cl, 64, 95);
   values->zs_write_format = (enum mali_zs_format)__gen_unpack_uint(cl, 96, 99);
   values->zs_block_format = (enum mali_block_format)__gen_unpack_uint(cl, 100, 101);
   values->zs_msaa = (enum mali_msaa)__gen_unpack_uint(cl, 102, 103);
   values->zs_big_endian = __gen_unpack_uint(cl, 104, 104);
   values->zs_clean_pixel_write_enable = __gen_unpack_uint(cl, 106, 106);
   values->s_write_format = (enum mali_s_format)__gen_unpack_uint(cl, 112, 115);
   values->s_block_format = (enum mali_block_format)__gen_unpack_uint(cl, 116, 117);
   values->s_msaa = (enum mali_msaa)__gen_unpack_uint(cl, 118, 119);
   values->zs_writeback_base = __gen_unpack_uint(cl, 128, 191);
   values->zs_writeback_row_stride = __gen_unpack_uint(cl, 192, 223);
   values->zs_writeback_surface_stride = __gen_unpack_uint(cl, 224, 255);
   values->s_writeback_base = __gen_unpack_uint(cl, 256, 319);
   values->s_writeback_row_stride = __gen_unpack_uint(cl, 320, 351);
   values->s_writeback_surface_stride = __gen_unpack_uint(cl, 352, 383);
   values->zs_afbc_header = __gen_unpack_uint(cl, 128, 191);
   values->zs_afbc_row_stride = __gen_unpack_uint(cl, 192, 204);
   values->zs_afbc_body = __gen_unpack_uint(cl, 256, 319);
}

static inline void
MALI_ZS_CRC_EXTENSION_print(FILE *fp, const struct MALI_ZS_CRC_EXTENSION * values, unsigned indent)
{
   fprintf(fp, "%*sCRC Base: 0x%" PRIx64 "\n", indent, "", values->crc_base);
   fprintf(fp, "%*sCRC Row Stride: %u\n", indent, "", values->crc_row_stride);
   fprintf(fp, "%*sZS Write Format: %s\n", indent, "", mali_zs_format_as_str(values->zs_write_format));
   fprintf(fp, "%*sZS Block Format: %s\n", indent, "", mali_block_format_as_str(values->zs_block_format));
   fprintf(fp, "%*sZS MSAA: %s\n", indent, "", mali_msaa_as_str(values->zs_msaa));
   fprintf(fp, "%*sZS Big Endian: %s\n", indent, "", values->zs_big_endian ? "true" : "false");
   fprintf(fp, "%*sZS Clean Pixel Write Enable: %s\n", indent, "", values->zs_clean_pixel_write_enable ? "true" : "false");
   fprintf(fp, "%*sS Write Format: %s\n", indent, "", mali_s_format_as_str(values->s_write_format));
   fprintf(fp, "%*sS Block Format: %s\n", indent, "", mali_block_format_as_str(values->s_block_format));
   fprintf(fp, "%*sS MSAA: %s\n", indent, "", mali_msaa_as_str(values->s_msaa));
   fprintf(fp, "%*sZS Writeback Base: 0x%" PRIx64 "\n", indent, "", values->zs_writeback_base);
   fprintf(fp, "%*sZS Writeback Row Stride: %u\n", indent, "", values->zs_writeback_row_stride);
   fprintf(fp, "%*sZS Writeback Surface Stride: %u\n", indent, "", values->zs_writeback_surface_stride);
   fprintf(fp, "%*sS Writeback Base: 0x%" PRIx64 "\n", indent, "", values->s_writeback_base);
   fprintf(fp, "%*sS Writeback Row Stride: %u\n", indent, "", values->s_writeback_row_stride);
   fprintf(fp, "%*sS Writeback Surface Stride: %u\n", indent, "", values->s_writeback_surface_stride);
   fprintf(fp, "%*sZS AFBC Header: 0x%" PRIx64 "\n", indent, "", values->zs_afbc_header);
   fprintf(fp, "%*sZS AFBC Row Stride: %u\n", indent, "", values->zs_afbc_row_stride);
   fprintf(fp, "%*sZS AFBC Body: 0x%" PRIx64 "\n", indent, "", values->zs_afbc_body);
}

enum mali_rt_endianness {
        MALI_RT_ENDIANNESS_LITTLE_ENDIAN     =      0,
        MALI_RT_ENDIANNESS_BIG_ENDIAN_2B     =      1,
        MALI_RT_ENDIANNESS_BIG_ENDIAN_4B     =      2,
        MALI_RT_ENDIANNESS_BIG_ENDIAN_8B     =      3,
};

static inline const char *
mali_rt_endianness_as_str(enum mali_rt_endianness imm)
{
    switch (imm) {
    case MALI_RT_ENDIANNESS_LITTLE_ENDIAN: return "Little Endian";
    case MALI_RT_ENDIANNESS_BIG_ENDIAN_2B: return "Big Endian 2B";
    case MALI_RT_ENDIANNESS_BIG_ENDIAN_4B: return "Big Endian 4B";
    case MALI_RT_ENDIANNESS_BIG_ENDIAN_8B: return "Big Endian 8B";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_conv_k6 {
        MALI_YUV_CONV_K6_0                   =      0,
        MALI_YUV_CONV_K6_16                  =      1,
};

static inline const char *
mali_yuv_conv_k6_as_str(enum mali_yuv_conv_k6 imm)
{
    switch (imm) {
    case MALI_YUV_CONV_K6_0: return "0";
    case MALI_YUV_CONV_K6_16: return "16";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_conv_k7_clamp {
        MALI_YUV_CONV_K7_CLAMP_MINUS_128_TO_127 =      0,
        MALI_YUV_CONV_K7_CLAMP_MINUS_112_TO_111 =      1,
        MALI_YUV_CONV_K7_CLAMP_0_TO_255      =      2,
        MALI_YUV_CONV_K7_CLAMP_16_TO_239     =      3,
};

static inline const char *
mali_yuv_conv_k7_clamp_as_str(enum mali_yuv_conv_k7_clamp imm)
{
    switch (imm) {
    case MALI_YUV_CONV_K7_CLAMP_MINUS_128_TO_127: return "MINUS_128_TO_127";
    case MALI_YUV_CONV_K7_CLAMP_MINUS_112_TO_111: return "MINUS_112_TO_111";
    case MALI_YUV_CONV_K7_CLAMP_0_TO_255: return "0_TO_255";
    case MALI_YUV_CONV_K7_CLAMP_16_TO_239: return "16_TO_239";
    default: return "XXX: INVALID";
    }
}

enum mali_yuv_conv_k8 {
        MALI_YUV_CONV_K8_220                 =      0,
        MALI_YUV_CONV_K8_256                 =      1,
};

static inline const char *
mali_yuv_conv_k8_as_str(enum mali_yuv_conv_k8 imm)
{
    switch (imm) {
    case MALI_YUV_CONV_K8_220: return "220";
    case MALI_YUV_CONV_K8_256: return "256";
    default: return "XXX: INVALID";
    }
}

struct MALI_RENDER_TARGET_YUV_OVERLAY {
   enum mali_yuv_swizzle                swizzle;
   bool                                 full_range;
   enum mali_yuv_conversion_mode        conversion_mode;
   enum mali_yuv_cr_siting              cr_siting;
   bool                                 unsigned_cr_range;
   uint64_t                             plane_0_base;
   uint64_t                             plane_1_base;
   uint64_t                             plane_2_base;
   uint32_t                             plane_0_stride;
   uint32_t                             plane_1_2_stride;
};

#define MALI_RENDER_TARGET_YUV_OVERLAY_header   \
   0

static inline void
MALI_RENDER_TARGET_YUV_OVERLAY_pack(uint32_t * restrict cl,
                                    const struct MALI_RENDER_TARGET_YUV_OVERLAY * restrict values)
{
   cl[ 0] = 0;
   cl[ 1] = 0;
   cl[ 2] = __gen_uint(values->swizzle, 16, 18) |
            __gen_uint(values->full_range, 20, 20) |
            __gen_uint(values->conversion_mode, 21, 24) |
            __gen_uint(values->cr_siting, 25, 27) |
            __gen_uint(values->unsigned_cr_range, 28, 28);
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->plane_0_base, 0, 63);
   cl[ 5] = __gen_uint(values->plane_0_base, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->plane_1_base, 0, 63);
   cl[ 7] = __gen_uint(values->plane_1_base, 0, 63) >> 32;
   cl[ 8] = __gen_uint(values->plane_2_base, 0, 63);
   cl[ 9] = __gen_uint(values->plane_2_base, 0, 63) >> 32;
   cl[10] = __gen_uint(values->plane_0_stride, 0, 31);
   cl[11] = __gen_uint(values->plane_1_2_stride, 0, 31);
   cl[12] = 0;
   cl[13] = 0;
   cl[14] = 0;
   cl[15] = 0;
}


#define MALI_RENDER_TARGET_YUV_OVERLAY_LENGTH 64
struct mali_render_target_yuv_overlay_packed { uint32_t opaque[16]; };
static inline void
MALI_RENDER_TARGET_YUV_OVERLAY_unpack(const uint8_t * restrict cl,
                                      struct MALI_RENDER_TARGET_YUV_OVERLAY * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xe008ffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 3\n");
   if (((const uint32_t *) cl)[12] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 12\n");
   if (((const uint32_t *) cl)[13] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 13\n");
   if (((const uint32_t *) cl)[14] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 14\n");
   if (((const uint32_t *) cl)[15] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target YUV Overlay unpacked at word 15\n");
   values->swizzle = (enum mali_yuv_swizzle)__gen_unpack_uint(cl, 80, 82);
   values->full_range = __gen_unpack_uint(cl, 84, 84);
   values->conversion_mode = (enum mali_yuv_conversion_mode)__gen_unpack_uint(cl, 85, 88);
   values->cr_siting = (enum mali_yuv_cr_siting)__gen_unpack_uint(cl, 89, 91);
   values->unsigned_cr_range = __gen_unpack_uint(cl, 92, 92);
   values->plane_0_base = __gen_unpack_uint(cl, 128, 191);
   values->plane_1_base = __gen_unpack_uint(cl, 192, 255);
   values->plane_2_base = __gen_unpack_uint(cl, 256, 319);
   values->plane_0_stride = __gen_unpack_uint(cl, 320, 351);
   values->plane_1_2_stride = __gen_unpack_uint(cl, 352, 383);
}

static inline void
MALI_RENDER_TARGET_YUV_OVERLAY_print(FILE *fp, const struct MALI_RENDER_TARGET_YUV_OVERLAY * values, unsigned indent)
{
   fprintf(fp, "%*sSwizzle: %s\n", indent, "", mali_yuv_swizzle_as_str(values->swizzle));
   fprintf(fp, "%*sFull Range: %s\n", indent, "", values->full_range ? "true" : "false");
   fprintf(fp, "%*sConversion Mode: %s\n", indent, "", mali_yuv_conversion_mode_as_str(values->conversion_mode));
   fprintf(fp, "%*sCr Siting: %s\n", indent, "", mali_yuv_cr_siting_as_str(values->cr_siting));
   fprintf(fp, "%*sUnsigned Cr Range: %s\n", indent, "", values->unsigned_cr_range ? "true" : "false");
   fprintf(fp, "%*sPlane 0 Base: 0x%" PRIx64 "\n", indent, "", values->plane_0_base);
   fprintf(fp, "%*sPlane 1 Base: 0x%" PRIx64 "\n", indent, "", values->plane_1_base);
   fprintf(fp, "%*sPlane 2 Base: 0x%" PRIx64 "\n", indent, "", values->plane_2_base);
   fprintf(fp, "%*sPlane 0 Stride: %u\n", indent, "", values->plane_0_stride);
   fprintf(fp, "%*sPlane 1 2 Stride: %u\n", indent, "", values->plane_1_2_stride);
}

struct MALI_RENDER_TARGET_AFBC_OVERLAY {
   uint64_t                             header;
   uint32_t                             row_stride;
   uint32_t                             chunk_size;
   bool                                 afbc_split_block_enable;
   bool                                 afbc_wide_block_enable;
   bool                                 reverse_issue_order;
   bool                                 yuv_transform_enable;
   uint64_t                             body;
   uint32_t                             body_size;
};

#define MALI_RENDER_TARGET_AFBC_OVERLAY_header  \
   0

static inline void
MALI_RENDER_TARGET_AFBC_OVERLAY_pack(uint32_t * restrict cl,
                                     const struct MALI_RENDER_TARGET_AFBC_OVERLAY * restrict values)
{
   cl[ 0] = 0;
   cl[ 1] = 0;
   cl[ 2] = 0;
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->header, 0, 63);
   cl[ 5] = __gen_uint(values->header, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->row_stride, 0, 12);
   cl[ 7] = __gen_uint(values->chunk_size, 0, 11) |
            __gen_uint(values->afbc_split_block_enable, 18, 18) |
            __gen_uint(values->afbc_wide_block_enable, 19, 19) |
            __gen_uint(values->reverse_issue_order, 20, 20) |
            __gen_uint(values->yuv_transform_enable, 17, 17);
   cl[ 8] = __gen_uint(values->body, 0, 63);
   cl[ 9] = __gen_uint(values->body, 0, 63) >> 32;
   cl[10] = __gen_uint(values->body_size, 0, 31);
   cl[11] = 0;
   cl[12] = 0;
   cl[13] = 0;
   cl[14] = 0;
   cl[15] = 0;
}


#define MALI_RENDER_TARGET_AFBC_OVERLAY_LENGTH 64
struct mali_render_target_afbc_overlay_packed { uint32_t opaque[16]; };
static inline void
MALI_RENDER_TARGET_AFBC_OVERLAY_unpack(const uint8_t * restrict cl,
                                       struct MALI_RENDER_TARGET_AFBC_OVERLAY * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 3\n");
   if (((const uint32_t *) cl)[6] & 0xffffe000) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffe1f000) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 7\n");
   if (((const uint32_t *) cl)[11] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 11\n");
   if (((const uint32_t *) cl)[12] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 12\n");
   if (((const uint32_t *) cl)[13] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 13\n");
   if (((const uint32_t *) cl)[14] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 14\n");
   if (((const uint32_t *) cl)[15] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target AFBC Overlay unpacked at word 15\n");
   values->header = __gen_unpack_uint(cl, 128, 191);
   values->row_stride = __gen_unpack_uint(cl, 192, 204);
   values->chunk_size = __gen_unpack_uint(cl, 224, 235);
   values->afbc_split_block_enable = __gen_unpack_uint(cl, 242, 242);
   values->afbc_wide_block_enable = __gen_unpack_uint(cl, 243, 243);
   values->reverse_issue_order = __gen_unpack_uint(cl, 244, 244);
   values->yuv_transform_enable = __gen_unpack_uint(cl, 241, 241);
   values->body = __gen_unpack_uint(cl, 256, 319);
   values->body_size = __gen_unpack_uint(cl, 320, 351);
}

static inline void
MALI_RENDER_TARGET_AFBC_OVERLAY_print(FILE *fp, const struct MALI_RENDER_TARGET_AFBC_OVERLAY * values, unsigned indent)
{
   fprintf(fp, "%*sHeader: 0x%" PRIx64 "\n", indent, "", values->header);
   fprintf(fp, "%*sRow Stride: %u\n", indent, "", values->row_stride);
   fprintf(fp, "%*sChunk Size: %u\n", indent, "", values->chunk_size);
   fprintf(fp, "%*sAFBC Split Block Enable: %s\n", indent, "", values->afbc_split_block_enable ? "true" : "false");
   fprintf(fp, "%*sAFBC Wide Block Enable: %s\n", indent, "", values->afbc_wide_block_enable ? "true" : "false");
   fprintf(fp, "%*sReverse Issue Order: %s\n", indent, "", values->reverse_issue_order ? "true" : "false");
   fprintf(fp, "%*sYUV Transform Enable: %s\n", indent, "", values->yuv_transform_enable ? "true" : "false");
   fprintf(fp, "%*sBody: 0x%" PRIx64 "\n", indent, "", values->body);
   fprintf(fp, "%*sBody Size: %u\n", indent, "", values->body_size);
}

struct MALI_RT_CLEAR {
   uint32_t                             color_0;
   uint32_t                             color_1;
   uint32_t                             color_2;
   uint32_t                             color_3;
};

#define MALI_RT_CLEAR_header                    \
   0

static inline void
MALI_RT_CLEAR_pack(uint32_t * restrict cl,
                   const struct MALI_RT_CLEAR * restrict values)
{
   cl[ 0] = __gen_uint(values->color_0, 0, 31);
   cl[ 1] = __gen_uint(values->color_1, 0, 31);
   cl[ 2] = __gen_uint(values->color_2, 0, 31);
   cl[ 3] = __gen_uint(values->color_3, 0, 31);
}


#define MALI_RT_CLEAR_LENGTH 16
struct mali_rt_clear_packed { uint32_t opaque[4]; };
static inline void
MALI_RT_CLEAR_unpack(const uint8_t * restrict cl,
                     struct MALI_RT_CLEAR * restrict values)
{
   values->color_0 = __gen_unpack_uint(cl, 0, 31);
   values->color_1 = __gen_unpack_uint(cl, 32, 63);
   values->color_2 = __gen_unpack_uint(cl, 64, 95);
   values->color_3 = __gen_unpack_uint(cl, 96, 127);
}

static inline void
MALI_RT_CLEAR_print(FILE *fp, const struct MALI_RT_CLEAR * values, unsigned indent)
{
   fprintf(fp, "%*sColor 0: %u\n", indent, "", values->color_0);
   fprintf(fp, "%*sColor 1: %u\n", indent, "", values->color_1);
   fprintf(fp, "%*sColor 2: %u\n", indent, "", values->color_2);
   fprintf(fp, "%*sColor 3: %u\n", indent, "", values->color_3);
}

struct MALI_RENDER_TARGET {
   struct MALI_RENDER_TARGET_YUV_OVERLAY yuv;
   struct MALI_RENDER_TARGET_AFBC_OVERLAY afbc;
   uint32_t                             internal_buffer_offset;
   bool                                 yuv_enable;
   bool                                 dithered_clear;
   enum mali_color_buffer_internal_format internal_format;
   bool                                 write_enable;
   enum mali_color_format               writeback_format;
   enum mali_rt_endianness              writeback_endianness;
   enum mali_block_format               writeback_block_format;
   enum mali_msaa                       writeback_msaa;
   bool                                 srgb;
   bool                                 dithering_enable;
   uint32_t                             swizzle;
   bool                                 clean_pixel_write_enable;
   struct MALI_RT_BUFFER                rgb;
   struct MALI_RT_CLEAR                 clear;
};

#define MALI_RENDER_TARGET_header               \
   .yuv = { MALI_RENDER_TARGET_YUV_OVERLAY_header },  \
   .afbc = { MALI_RENDER_TARGET_AFBC_OVERLAY_header },  \
   .rgb = { MALI_RT_BUFFER_header },  \
   .clear = { MALI_RT_CLEAR_header }

static inline void
MALI_RENDER_TARGET_pack(uint32_t * restrict cl,
                        const struct MALI_RENDER_TARGET * restrict values)
{
   assert((values->internal_buffer_offset & 0xf) == 0);
   cl[ 0] = __gen_uint(values->internal_buffer_offset >> 4, 4, 15) |
            __gen_uint(values->yuv_enable, 24, 24) |
            __gen_uint(values->dithered_clear, 25, 25) |
            __gen_uint(values->internal_format, 26, 31);
   cl[ 1] = __gen_uint(values->write_enable, 0, 0) |
            __gen_uint(values->writeback_format, 3, 7) |
            __gen_uint(values->writeback_endianness, 8, 9) |
            __gen_uint(values->writeback_block_format, 10, 11) |
            __gen_uint(values->writeback_msaa, 12, 13) |
            __gen_uint(values->srgb, 14, 14) |
            __gen_uint(values->dithering_enable, 15, 15) |
            __gen_uint(values->swizzle, 16, 27) |
            __gen_uint(values->clean_pixel_write_enable, 31, 31);
   cl[ 2] = __gen_uint(values->yuv.swizzle, 16, 18) |
            __gen_uint(values->yuv.full_range, 20, 20) |
            __gen_uint(values->yuv.conversion_mode, 21, 24) |
            __gen_uint(values->yuv.cr_siting, 25, 27) |
            __gen_uint(values->yuv.unsigned_cr_range, 28, 28);
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->yuv.plane_0_base, 0, 63) |
            __gen_uint(values->afbc.header, 0, 63);
   cl[ 5] = __gen_uint(values->yuv.plane_0_base, 0, 63) >> 32 |
            __gen_uint(values->afbc.header, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->yuv.plane_1_base, 0, 63) |
            __gen_uint(values->afbc.row_stride, 0, 12);
   cl[ 7] = __gen_uint(values->yuv.plane_1_base, 0, 63) >> 32 |
            __gen_uint(values->afbc.chunk_size, 0, 11) |
            __gen_uint(values->afbc.afbc_split_block_enable, 18, 18) |
            __gen_uint(values->afbc.afbc_wide_block_enable, 19, 19) |
            __gen_uint(values->afbc.reverse_issue_order, 20, 20) |
            __gen_uint(values->afbc.yuv_transform_enable, 17, 17);
   cl[ 8] = __gen_uint(values->yuv.plane_2_base, 0, 63) |
            __gen_uint(values->afbc.body, 0, 63) |
            __gen_uint(values->rgb.base, 0, 63);
   cl[ 9] = __gen_uint(values->yuv.plane_2_base, 0, 63) >> 32 |
            __gen_uint(values->afbc.body, 0, 63) >> 32 |
            __gen_uint(values->rgb.base, 0, 63) >> 32;
   cl[10] = __gen_uint(values->yuv.plane_0_stride, 0, 31) |
            __gen_uint(values->afbc.body_size, 0, 31) |
            __gen_uint(values->rgb.row_stride, 0, 31);
   cl[11] = __gen_uint(values->yuv.plane_1_2_stride, 0, 31) |
            __gen_uint(values->rgb.surface_stride, 0, 31);
   cl[12] = __gen_uint(values->clear.color_0, 0, 31);
   cl[13] = __gen_uint(values->clear.color_1, 0, 31);
   cl[14] = __gen_uint(values->clear.color_2, 0, 31);
   cl[15] = __gen_uint(values->clear.color_3, 0, 31);
}


#define MALI_RENDER_TARGET_LENGTH 64
#define MALI_RENDER_TARGET_ALIGN 64
struct mali_render_target_packed { uint32_t opaque[16]; };
static inline void
MALI_RENDER_TARGET_unpack(const uint8_t * restrict cl,
                          struct MALI_RENDER_TARGET * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xff000f) fprintf(stderr, "XXX: Invalid field of Render Target unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0x70000006) fprintf(stderr, "XXX: Invalid field of Render Target unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xe008ffff) fprintf(stderr, "XXX: Invalid field of Render Target unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Render Target unpacked at word 3\n");
   values->yuv.swizzle = (enum mali_yuv_swizzle)__gen_unpack_uint(cl, 80, 82);
   values->yuv.full_range = __gen_unpack_uint(cl, 84, 84);
   values->yuv.conversion_mode = (enum mali_yuv_conversion_mode)__gen_unpack_uint(cl, 85, 88);
   values->yuv.cr_siting = (enum mali_yuv_cr_siting)__gen_unpack_uint(cl, 89, 91);
   values->yuv.unsigned_cr_range = __gen_unpack_uint(cl, 92, 92);
   values->yuv.plane_0_base = __gen_unpack_uint(cl, 128, 191);
   values->yuv.plane_1_base = __gen_unpack_uint(cl, 192, 255);
   values->yuv.plane_2_base = __gen_unpack_uint(cl, 256, 319);
   values->yuv.plane_0_stride = __gen_unpack_uint(cl, 320, 351);
   values->yuv.plane_1_2_stride = __gen_unpack_uint(cl, 352, 383);
   values->afbc.header = __gen_unpack_uint(cl, 128, 191);
   values->afbc.row_stride = __gen_unpack_uint(cl, 192, 204);
   values->afbc.chunk_size = __gen_unpack_uint(cl, 224, 235);
   values->afbc.afbc_split_block_enable = __gen_unpack_uint(cl, 242, 242);
   values->afbc.afbc_wide_block_enable = __gen_unpack_uint(cl, 243, 243);
   values->afbc.reverse_issue_order = __gen_unpack_uint(cl, 244, 244);
   values->afbc.yuv_transform_enable = __gen_unpack_uint(cl, 241, 241);
   values->afbc.body = __gen_unpack_uint(cl, 256, 319);
   values->afbc.body_size = __gen_unpack_uint(cl, 320, 351);
   values->internal_buffer_offset = __gen_unpack_uint(cl, 4, 15) << 4;
   values->yuv_enable = __gen_unpack_uint(cl, 24, 24);
   values->dithered_clear = __gen_unpack_uint(cl, 25, 25);
   values->internal_format = (enum mali_color_buffer_internal_format)__gen_unpack_uint(cl, 26, 31);
   values->write_enable = __gen_unpack_uint(cl, 32, 32);
   values->writeback_format = (enum mali_color_format)__gen_unpack_uint(cl, 35, 39);
   values->writeback_endianness = (enum mali_rt_endianness)__gen_unpack_uint(cl, 40, 41);
   values->writeback_block_format = (enum mali_block_format)__gen_unpack_uint(cl, 42, 43);
   values->writeback_msaa = (enum mali_msaa)__gen_unpack_uint(cl, 44, 45);
   values->srgb = __gen_unpack_uint(cl, 46, 46);
   values->dithering_enable = __gen_unpack_uint(cl, 47, 47);
   values->swizzle = __gen_unpack_uint(cl, 48, 59);
   values->clean_pixel_write_enable = __gen_unpack_uint(cl, 63, 63);
   values->rgb.base = __gen_unpack_uint(cl, 256, 319);
   values->rgb.row_stride = __gen_unpack_uint(cl, 320, 351);
   values->rgb.surface_stride = __gen_unpack_uint(cl, 352, 383);
   values->clear.color_0 = __gen_unpack_uint(cl, 384, 415);
   values->clear.color_1 = __gen_unpack_uint(cl, 416, 447);
   values->clear.color_2 = __gen_unpack_uint(cl, 448, 479);
   values->clear.color_3 = __gen_unpack_uint(cl, 480, 511);
}

static inline void
MALI_RENDER_TARGET_print(FILE *fp, const struct MALI_RENDER_TARGET * values, unsigned indent)
{
   fprintf(fp, "%*sYUV:\n", indent, "");
   MALI_RENDER_TARGET_YUV_OVERLAY_print(fp, &values->yuv, indent + 2);
   fprintf(fp, "%*sAFBC:\n", indent, "");
   MALI_RENDER_TARGET_AFBC_OVERLAY_print(fp, &values->afbc, indent + 2);
   fprintf(fp, "%*sInternal Buffer Offset: %u\n", indent, "", values->internal_buffer_offset);
   fprintf(fp, "%*sYUV Enable: %s\n", indent, "", values->yuv_enable ? "true" : "false");
   fprintf(fp, "%*sDithered Clear: %s\n", indent, "", values->dithered_clear ? "true" : "false");
   fprintf(fp, "%*sInternal Format: %s\n", indent, "", mali_color_buffer_internal_format_as_str(values->internal_format));
   fprintf(fp, "%*sWrite Enable: %s\n", indent, "", values->write_enable ? "true" : "false");
   fprintf(fp, "%*sWriteback Format: %s\n", indent, "", mali_color_format_as_str(values->writeback_format));
   fprintf(fp, "%*sWriteback Endianness: %s\n", indent, "", mali_rt_endianness_as_str(values->writeback_endianness));
   fprintf(fp, "%*sWriteback Block Format: %s\n", indent, "", mali_block_format_as_str(values->writeback_block_format));
   fprintf(fp, "%*sWriteback MSAA: %s\n", indent, "", mali_msaa_as_str(values->writeback_msaa));
   fprintf(fp, "%*ssRGB: %s\n", indent, "", values->srgb ? "true" : "false");
   fprintf(fp, "%*sDithering Enable: %s\n", indent, "", values->dithering_enable ? "true" : "false");
   fprintf(fp, "%*sSwizzle: %u\n", indent, "", values->swizzle);
   fprintf(fp, "%*sClean Pixel Write Enable: %s\n", indent, "", values->clean_pixel_write_enable ? "true" : "false");
   fprintf(fp, "%*sRGB:\n", indent, "");
   MALI_RT_BUFFER_print(fp, &values->rgb, indent + 2);
   fprintf(fp, "%*sClear:\n", indent, "");
   MALI_RT_CLEAR_print(fp, &values->clear, indent + 2);
}

struct MALI_TILER_HEAP {
   uint32_t                             size;
   uint64_t                             base;
   uint64_t                             bottom;
   uint64_t                             top;
};

#define MALI_TILER_HEAP_header                  \
   0

static inline void
MALI_TILER_HEAP_pack(uint32_t * restrict cl,
                     const struct MALI_TILER_HEAP * restrict values)
{
   cl[ 0] = 0;
   cl[ 1] = __gen_uint(ALIGN_POT(values->size, 4096), 0, 31);
   cl[ 2] = __gen_uint(values->base, 0, 63);
   cl[ 3] = __gen_uint(values->base, 0, 63) >> 32;
   cl[ 4] = __gen_uint(values->bottom, 0, 63);
   cl[ 5] = __gen_uint(values->bottom, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->top, 0, 63);
   cl[ 7] = __gen_uint(values->top, 0, 63) >> 32;
}


#define MALI_TILER_HEAP_LENGTH 32
#define MALI_TILER_HEAP_ALIGN 64
struct mali_tiler_heap_packed { uint32_t opaque[8]; };
static inline void
MALI_TILER_HEAP_unpack(const uint8_t * restrict cl,
                       struct MALI_TILER_HEAP * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Heap unpacked at word 0\n");
   values->size = __gen_unpack_uint(cl, 32, 63);
   assert(!(values->size & 0xfff));
   values->base = __gen_unpack_uint(cl, 64, 127);
   values->bottom = __gen_unpack_uint(cl, 128, 191);
   values->top = __gen_unpack_uint(cl, 192, 255);
}

static inline void
MALI_TILER_HEAP_print(FILE *fp, const struct MALI_TILER_HEAP * values, unsigned indent)
{
   fprintf(fp, "%*sSize: %u\n", indent, "", values->size);
   fprintf(fp, "%*sBase: 0x%" PRIx64 "\n", indent, "", values->base);
   fprintf(fp, "%*sBottom: 0x%" PRIx64 "\n", indent, "", values->bottom);
   fprintf(fp, "%*sTop: 0x%" PRIx64 "\n", indent, "", values->top);
}

struct MALI_TILER_WEIGHTS {
   uint32_t                             weight0;
   uint32_t                             weight1;
   uint32_t                             weight2;
   uint32_t                             weight3;
   uint32_t                             weight4;
   uint32_t                             weight5;
   uint32_t                             weight6;
   uint32_t                             weight7;
};

#define MALI_TILER_WEIGHTS_header               \
   0

static inline void
MALI_TILER_WEIGHTS_pack(uint32_t * restrict cl,
                        const struct MALI_TILER_WEIGHTS * restrict values)
{
   cl[ 0] = __gen_uint(values->weight0, 16, 31);
   cl[ 1] = __gen_uint(values->weight1, 16, 31);
   cl[ 2] = __gen_uint(values->weight2, 16, 31);
   cl[ 3] = __gen_uint(values->weight3, 16, 31);
   cl[ 4] = __gen_uint(values->weight4, 16, 31);
   cl[ 5] = __gen_uint(values->weight5, 16, 31);
   cl[ 6] = __gen_uint(values->weight6, 16, 31);
   cl[ 7] = __gen_uint(values->weight7, 16, 31);
}


#define MALI_TILER_WEIGHTS_LENGTH 32
struct mali_tiler_weights_packed { uint32_t opaque[8]; };
static inline void
MALI_TILER_WEIGHTS_unpack(const uint8_t * restrict cl,
                          struct MALI_TILER_WEIGHTS * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 3\n");
   if (((const uint32_t *) cl)[4] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 4\n");
   if (((const uint32_t *) cl)[5] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 5\n");
   if (((const uint32_t *) cl)[6] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Weights unpacked at word 7\n");
   values->weight0 = __gen_unpack_uint(cl, 16, 31);
   values->weight1 = __gen_unpack_uint(cl, 48, 63);
   values->weight2 = __gen_unpack_uint(cl, 80, 95);
   values->weight3 = __gen_unpack_uint(cl, 112, 127);
   values->weight4 = __gen_unpack_uint(cl, 144, 159);
   values->weight5 = __gen_unpack_uint(cl, 176, 191);
   values->weight6 = __gen_unpack_uint(cl, 208, 223);
   values->weight7 = __gen_unpack_uint(cl, 240, 255);
}

static inline void
MALI_TILER_WEIGHTS_print(FILE *fp, const struct MALI_TILER_WEIGHTS * values, unsigned indent)
{
   fprintf(fp, "%*sWeight0: %u\n", indent, "", values->weight0);
   fprintf(fp, "%*sWeight1: %u\n", indent, "", values->weight1);
   fprintf(fp, "%*sWeight2: %u\n", indent, "", values->weight2);
   fprintf(fp, "%*sWeight3: %u\n", indent, "", values->weight3);
   fprintf(fp, "%*sWeight4: %u\n", indent, "", values->weight4);
   fprintf(fp, "%*sWeight5: %u\n", indent, "", values->weight5);
   fprintf(fp, "%*sWeight6: %u\n", indent, "", values->weight6);
   fprintf(fp, "%*sWeight7: %u\n", indent, "", values->weight7);
}

struct MALI_TILER_STATE {
   uint32_t                             word0;
   uint32_t                             word1;
   uint32_t                             word2;
   uint32_t                             word3;
   uint32_t                             word4;
   uint32_t                             word5;
   uint32_t                             word6;
   uint32_t                             word7;
   uint32_t                             word8;
   uint32_t                             word9;
   uint32_t                             word10;
   uint32_t                             word11;
   uint32_t                             word12;
   uint32_t                             word13;
   uint32_t                             word14;
   uint32_t                             word15;
};

#define MALI_TILER_STATE_header                 \
   0

static inline void
MALI_TILER_STATE_pack(uint32_t * restrict cl,
                      const struct MALI_TILER_STATE * restrict values)
{
   cl[ 0] = __gen_uint(values->word0, 0, 31);
   cl[ 1] = __gen_uint(values->word1, 0, 31);
   cl[ 2] = __gen_uint(values->word2, 0, 31);
   cl[ 3] = __gen_uint(values->word3, 0, 31);
   cl[ 4] = __gen_uint(values->word4, 0, 31);
   cl[ 5] = __gen_uint(values->word5, 0, 31);
   cl[ 6] = __gen_uint(values->word6, 0, 31);
   cl[ 7] = __gen_uint(values->word7, 0, 31);
   cl[ 8] = __gen_uint(values->word8, 0, 31);
   cl[ 9] = __gen_uint(values->word9, 0, 31);
   cl[10] = __gen_uint(values->word10, 0, 31);
   cl[11] = __gen_uint(values->word11, 0, 31);
   cl[12] = __gen_uint(values->word12, 0, 31);
   cl[13] = __gen_uint(values->word13, 0, 31);
   cl[14] = __gen_uint(values->word14, 0, 31);
   cl[15] = __gen_uint(values->word15, 0, 31);
}


#define MALI_TILER_STATE_LENGTH 64
struct mali_tiler_state_packed { uint32_t opaque[16]; };
static inline void
MALI_TILER_STATE_unpack(const uint8_t * restrict cl,
                        struct MALI_TILER_STATE * restrict values)
{
   values->word0 = __gen_unpack_uint(cl, 0, 31);
   values->word1 = __gen_unpack_uint(cl, 32, 63);
   values->word2 = __gen_unpack_uint(cl, 64, 95);
   values->word3 = __gen_unpack_uint(cl, 96, 127);
   values->word4 = __gen_unpack_uint(cl, 128, 159);
   values->word5 = __gen_unpack_uint(cl, 160, 191);
   values->word6 = __gen_unpack_uint(cl, 192, 223);
   values->word7 = __gen_unpack_uint(cl, 224, 255);
   values->word8 = __gen_unpack_uint(cl, 256, 287);
   values->word9 = __gen_unpack_uint(cl, 288, 319);
   values->word10 = __gen_unpack_uint(cl, 320, 351);
   values->word11 = __gen_unpack_uint(cl, 352, 383);
   values->word12 = __gen_unpack_uint(cl, 384, 415);
   values->word13 = __gen_unpack_uint(cl, 416, 447);
   values->word14 = __gen_unpack_uint(cl, 448, 479);
   values->word15 = __gen_unpack_uint(cl, 480, 511);
}

static inline void
MALI_TILER_STATE_print(FILE *fp, const struct MALI_TILER_STATE * values, unsigned indent)
{
   fprintf(fp, "%*sWord0: %u\n", indent, "", values->word0);
   fprintf(fp, "%*sWord1: %u\n", indent, "", values->word1);
   fprintf(fp, "%*sWord2: %u\n", indent, "", values->word2);
   fprintf(fp, "%*sWord3: %u\n", indent, "", values->word3);
   fprintf(fp, "%*sWord4: %u\n", indent, "", values->word4);
   fprintf(fp, "%*sWord5: %u\n", indent, "", values->word5);
   fprintf(fp, "%*sWord6: %u\n", indent, "", values->word6);
   fprintf(fp, "%*sWord7: %u\n", indent, "", values->word7);
   fprintf(fp, "%*sWord8: %u\n", indent, "", values->word8);
   fprintf(fp, "%*sWord9: %u\n", indent, "", values->word9);
   fprintf(fp, "%*sWord10: %u\n", indent, "", values->word10);
   fprintf(fp, "%*sWord11: %u\n", indent, "", values->word11);
   fprintf(fp, "%*sWord12: %u\n", indent, "", values->word12);
   fprintf(fp, "%*sWord13: %u\n", indent, "", values->word13);
   fprintf(fp, "%*sWord14: %u\n", indent, "", values->word14);
   fprintf(fp, "%*sWord15: %u\n", indent, "", values->word15);
}

struct MALI_TILER_CONTEXT {
   uint64_t                             polygon_list;
   uint32_t                             hierarchy_mask;
   enum mali_sample_pattern             sample_pattern;
   bool                                 update_cost_table;
   uint32_t                             fb_width;
   uint32_t                             fb_height;
   uint64_t                             heap;
   struct MALI_TILER_WEIGHTS            weights;
   struct MALI_TILER_STATE              state;
};

#define MALI_TILER_CONTEXT_header               \
   .weights = { MALI_TILER_WEIGHTS_header },  \
   .state = { MALI_TILER_STATE_header }

static inline void
MALI_TILER_CONTEXT_pack(uint32_t * restrict cl,
                        const struct MALI_TILER_CONTEXT * restrict values)
{
   assert(values->fb_width >= 1);
   assert(values->fb_height >= 1);
   cl[ 0] = __gen_uint(values->polygon_list, 0, 63);
   cl[ 1] = __gen_uint(values->polygon_list, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->hierarchy_mask, 0, 12) |
            __gen_uint(values->sample_pattern, 13, 15) |
            __gen_uint(values->update_cost_table, 16, 16);
   cl[ 3] = __gen_uint(values->fb_width - 1, 0, 15) |
            __gen_uint(values->fb_height - 1, 16, 31);
   cl[ 4] = 0;
   cl[ 5] = 0;
   cl[ 6] = __gen_uint(values->heap, 0, 63);
   cl[ 7] = __gen_uint(values->heap, 0, 63) >> 32;
   cl[ 8] = __gen_uint(values->weights.weight0, 16, 31);
   cl[ 9] = __gen_uint(values->weights.weight1, 16, 31);
   cl[10] = __gen_uint(values->weights.weight2, 16, 31);
   cl[11] = __gen_uint(values->weights.weight3, 16, 31);
   cl[12] = __gen_uint(values->weights.weight4, 16, 31);
   cl[13] = __gen_uint(values->weights.weight5, 16, 31);
   cl[14] = __gen_uint(values->weights.weight6, 16, 31);
   cl[15] = __gen_uint(values->weights.weight7, 16, 31);
   cl[16] = 0;
   cl[17] = 0;
   cl[18] = 0;
   cl[19] = 0;
   cl[20] = 0;
   cl[21] = 0;
   cl[22] = 0;
   cl[23] = 0;
   cl[24] = 0;
   cl[25] = 0;
   cl[26] = 0;
   cl[27] = 0;
   cl[28] = 0;
   cl[29] = 0;
   cl[30] = 0;
   cl[31] = 0;
   cl[32] = __gen_uint(values->state.word0, 0, 31);
   cl[33] = __gen_uint(values->state.word1, 0, 31);
   cl[34] = __gen_uint(values->state.word2, 0, 31);
   cl[35] = __gen_uint(values->state.word3, 0, 31);
   cl[36] = __gen_uint(values->state.word4, 0, 31);
   cl[37] = __gen_uint(values->state.word5, 0, 31);
   cl[38] = __gen_uint(values->state.word6, 0, 31);
   cl[39] = __gen_uint(values->state.word7, 0, 31);
   cl[40] = __gen_uint(values->state.word8, 0, 31);
   cl[41] = __gen_uint(values->state.word9, 0, 31);
   cl[42] = __gen_uint(values->state.word10, 0, 31);
   cl[43] = __gen_uint(values->state.word11, 0, 31);
   cl[44] = __gen_uint(values->state.word12, 0, 31);
   cl[45] = __gen_uint(values->state.word13, 0, 31);
   cl[46] = __gen_uint(values->state.word14, 0, 31);
   cl[47] = __gen_uint(values->state.word15, 0, 31);
}


#define MALI_TILER_CONTEXT_LENGTH 192
#define MALI_TILER_CONTEXT_ALIGN 64
struct mali_tiler_context_packed { uint32_t opaque[48]; };
static inline void
MALI_TILER_CONTEXT_unpack(const uint8_t * restrict cl,
                          struct MALI_TILER_CONTEXT * restrict values)
{
   if (((const uint32_t *) cl)[2] & 0xfffe0000) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 2\n");
   if (((const uint32_t *) cl)[4] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 4\n");
   if (((const uint32_t *) cl)[5] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 5\n");
   if (((const uint32_t *) cl)[8] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 8\n");
   if (((const uint32_t *) cl)[9] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 9\n");
   if (((const uint32_t *) cl)[10] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 10\n");
   if (((const uint32_t *) cl)[11] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 11\n");
   if (((const uint32_t *) cl)[12] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 12\n");
   if (((const uint32_t *) cl)[13] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 13\n");
   if (((const uint32_t *) cl)[14] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 14\n");
   if (((const uint32_t *) cl)[15] & 0xffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 15\n");
   if (((const uint32_t *) cl)[16] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 16\n");
   if (((const uint32_t *) cl)[17] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 17\n");
   if (((const uint32_t *) cl)[18] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 18\n");
   if (((const uint32_t *) cl)[19] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 19\n");
   if (((const uint32_t *) cl)[20] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 20\n");
   if (((const uint32_t *) cl)[21] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 21\n");
   if (((const uint32_t *) cl)[22] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 22\n");
   if (((const uint32_t *) cl)[23] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 23\n");
   if (((const uint32_t *) cl)[24] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 24\n");
   if (((const uint32_t *) cl)[25] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 25\n");
   if (((const uint32_t *) cl)[26] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 26\n");
   if (((const uint32_t *) cl)[27] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 27\n");
   if (((const uint32_t *) cl)[28] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 28\n");
   if (((const uint32_t *) cl)[29] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 29\n");
   if (((const uint32_t *) cl)[30] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 30\n");
   if (((const uint32_t *) cl)[31] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Context unpacked at word 31\n");
   values->polygon_list = __gen_unpack_uint(cl, 0, 63);
   values->hierarchy_mask = __gen_unpack_uint(cl, 64, 76);
   values->sample_pattern = (enum mali_sample_pattern)__gen_unpack_uint(cl, 77, 79);
   values->update_cost_table = __gen_unpack_uint(cl, 80, 80);
   values->fb_width = __gen_unpack_uint(cl, 96, 111) + 1;
   values->fb_height = __gen_unpack_uint(cl, 112, 127) + 1;
   values->heap = __gen_unpack_uint(cl, 192, 255);
   values->weights.weight0 = __gen_unpack_uint(cl, 272, 287);
   values->weights.weight1 = __gen_unpack_uint(cl, 304, 319);
   values->weights.weight2 = __gen_unpack_uint(cl, 336, 351);
   values->weights.weight3 = __gen_unpack_uint(cl, 368, 383);
   values->weights.weight4 = __gen_unpack_uint(cl, 400, 415);
   values->weights.weight5 = __gen_unpack_uint(cl, 432, 447);
   values->weights.weight6 = __gen_unpack_uint(cl, 464, 479);
   values->weights.weight7 = __gen_unpack_uint(cl, 496, 511);
   values->state.word0 = __gen_unpack_uint(cl, 1024, 1055);
   values->state.word1 = __gen_unpack_uint(cl, 1056, 1087);
   values->state.word2 = __gen_unpack_uint(cl, 1088, 1119);
   values->state.word3 = __gen_unpack_uint(cl, 1120, 1151);
   values->state.word4 = __gen_unpack_uint(cl, 1152, 1183);
   values->state.word5 = __gen_unpack_uint(cl, 1184, 1215);
   values->state.word6 = __gen_unpack_uint(cl, 1216, 1247);
   values->state.word7 = __gen_unpack_uint(cl, 1248, 1279);
   values->state.word8 = __gen_unpack_uint(cl, 1280, 1311);
   values->state.word9 = __gen_unpack_uint(cl, 1312, 1343);
   values->state.word10 = __gen_unpack_uint(cl, 1344, 1375);
   values->state.word11 = __gen_unpack_uint(cl, 1376, 1407);
   values->state.word12 = __gen_unpack_uint(cl, 1408, 1439);
   values->state.word13 = __gen_unpack_uint(cl, 1440, 1471);
   values->state.word14 = __gen_unpack_uint(cl, 1472, 1503);
   values->state.word15 = __gen_unpack_uint(cl, 1504, 1535);
}

static inline void
MALI_TILER_CONTEXT_print(FILE *fp, const struct MALI_TILER_CONTEXT * values, unsigned indent)
{
   fprintf(fp, "%*sPolygon List: 0x%" PRIx64 "\n", indent, "", values->polygon_list);
   fprintf(fp, "%*sHierarchy Mask: %u\n", indent, "", values->hierarchy_mask);
   fprintf(fp, "%*sSample Pattern: %s\n", indent, "", mali_sample_pattern_as_str(values->sample_pattern));
   fprintf(fp, "%*sUpdate Cost Table: %s\n", indent, "", values->update_cost_table ? "true" : "false");
   fprintf(fp, "%*sFB Width: %u\n", indent, "", values->fb_width);
   fprintf(fp, "%*sFB Height: %u\n", indent, "", values->fb_height);
   fprintf(fp, "%*sHeap: 0x%" PRIx64 "\n", indent, "", values->heap);
   fprintf(fp, "%*sWeights:\n", indent, "");
   MALI_TILER_WEIGHTS_print(fp, &values->weights, indent + 2);
   fprintf(fp, "%*sState:\n", indent, "");
   MALI_TILER_STATE_print(fp, &values->state, indent + 2);
}

struct MALI_FRAMEBUFFER_PADDING {
   int dummy;
};

#define MALI_FRAMEBUFFER_PADDING_header         \
   0

static inline void
MALI_FRAMEBUFFER_PADDING_pack(uint32_t * restrict cl,
                              const struct MALI_FRAMEBUFFER_PADDING * restrict values)
{
   cl[ 0] = 0;
   cl[ 1] = 0;
   cl[ 2] = 0;
   cl[ 3] = 0;
   cl[ 4] = 0;
   cl[ 5] = 0;
   cl[ 6] = 0;
   cl[ 7] = 0;
   cl[ 8] = 0;
   cl[ 9] = 0;
   cl[10] = 0;
   cl[11] = 0;
   cl[12] = 0;
   cl[13] = 0;
   cl[14] = 0;
   cl[15] = 0;
}


#define MALI_FRAMEBUFFER_PADDING_LENGTH 64
struct mali_framebuffer_padding_packed { uint32_t opaque[16]; };
static inline void
MALI_FRAMEBUFFER_PADDING_unpack(const uint8_t * restrict cl,
                                struct MALI_FRAMEBUFFER_PADDING * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 3\n");
   if (((const uint32_t *) cl)[4] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 4\n");
   if (((const uint32_t *) cl)[5] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 5\n");
   if (((const uint32_t *) cl)[6] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 7\n");
   if (((const uint32_t *) cl)[8] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 8\n");
   if (((const uint32_t *) cl)[9] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 9\n");
   if (((const uint32_t *) cl)[10] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 10\n");
   if (((const uint32_t *) cl)[11] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 11\n");
   if (((const uint32_t *) cl)[12] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 12\n");
   if (((const uint32_t *) cl)[13] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 13\n");
   if (((const uint32_t *) cl)[14] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 14\n");
   if (((const uint32_t *) cl)[15] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Framebuffer Padding unpacked at word 15\n");
}

static inline void
MALI_FRAMEBUFFER_PADDING_print(FILE *fp, const struct MALI_FRAMEBUFFER_PADDING * values, unsigned indent)
{
}

struct mali_framebuffer_packed {
   uint32_t opaque[32];
};

#define MALI_FRAMEBUFFER_LENGTH 128
#define MALI_FRAMEBUFFER_ALIGN 64
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_TYPE struct MALI_FRAMEBUFFER_PARAMETERS
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_header MALI_FRAMEBUFFER_PARAMETERS_header
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_pack MALI_FRAMEBUFFER_PARAMETERS_pack
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_unpack MALI_FRAMEBUFFER_PARAMETERS_unpack
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_print MALI_FRAMEBUFFER_PARAMETERS_print
#define MALI_FRAMEBUFFER_SECTION_PARAMETERS_OFFSET 0
#define MALI_FRAMEBUFFER_SECTION_PADDING_TYPE struct MALI_FRAMEBUFFER_PADDING
#define MALI_FRAMEBUFFER_SECTION_PADDING_header MALI_FRAMEBUFFER_PADDING_header
#define MALI_FRAMEBUFFER_SECTION_PADDING_pack MALI_FRAMEBUFFER_PADDING_pack
#define MALI_FRAMEBUFFER_SECTION_PADDING_unpack MALI_FRAMEBUFFER_PADDING_unpack
#define MALI_FRAMEBUFFER_SECTION_PADDING_print MALI_FRAMEBUFFER_PADDING_print
#define MALI_FRAMEBUFFER_SECTION_PADDING_OFFSET 64

struct MALI_JOB_HEADER {
   uint32_t                             exception_status;
   uint32_t                             first_incomplete_task;
   uint64_t                             fault_pointer;
   bool                                 is_64b;
   enum mali_job_type                   type;
   bool                                 barrier;
   bool                                 invalidate_cache;
   bool                                 suppress_prefetch;
   bool                                 enable_texture_mapper;
   bool                                 relax_dependency_1;
   bool                                 relax_dependency_2;
   uint32_t                             index;
   uint32_t                             dependency_1;
   uint32_t                             dependency_2;
   uint64_t                             next;
};

#define MALI_JOB_HEADER_header                  \
   .is_64b = true

static inline void
MALI_JOB_HEADER_pack(uint32_t * restrict cl,
                     const struct MALI_JOB_HEADER * restrict values)
{
   cl[ 0] = __gen_uint(values->exception_status, 0, 31);
   cl[ 1] = __gen_uint(values->first_incomplete_task, 0, 31);
   cl[ 2] = __gen_uint(values->fault_pointer, 0, 63);
   cl[ 3] = __gen_uint(values->fault_pointer, 0, 63) >> 32;
   cl[ 4] = __gen_uint(values->is_64b, 0, 0) |
            __gen_uint(values->type, 1, 7) |
            __gen_uint(values->barrier, 8, 8) |
            __gen_uint(values->invalidate_cache, 9, 9) |
            __gen_uint(values->suppress_prefetch, 11, 11) |
            __gen_uint(values->enable_texture_mapper, 12, 12) |
            __gen_uint(values->relax_dependency_1, 14, 14) |
            __gen_uint(values->relax_dependency_2, 15, 15) |
            __gen_uint(values->index, 16, 31);
   cl[ 5] = __gen_uint(values->dependency_1, 0, 15) |
            __gen_uint(values->dependency_2, 16, 31);
   cl[ 6] = __gen_uint(values->next, 0, 63);
   cl[ 7] = __gen_uint(values->next, 0, 63) >> 32;
}


#define MALI_JOB_HEADER_LENGTH 32
#define MALI_JOB_HEADER_ALIGN 64
struct mali_job_header_packed { uint32_t opaque[8]; };
static inline void
MALI_JOB_HEADER_unpack(const uint8_t * restrict cl,
                       struct MALI_JOB_HEADER * restrict values)
{
   if (((const uint32_t *) cl)[4] & 0x2400) fprintf(stderr, "XXX: Invalid field of Job Header unpacked at word 4\n");
   values->exception_status = __gen_unpack_uint(cl, 0, 31);
   values->first_incomplete_task = __gen_unpack_uint(cl, 32, 63);
   values->fault_pointer = __gen_unpack_uint(cl, 64, 127);
   values->is_64b = __gen_unpack_uint(cl, 128, 128);
   values->type = (enum mali_job_type)__gen_unpack_uint(cl, 129, 135);
   values->barrier = __gen_unpack_uint(cl, 136, 136);
   values->invalidate_cache = __gen_unpack_uint(cl, 137, 137);
   values->suppress_prefetch = __gen_unpack_uint(cl, 139, 139);
   values->enable_texture_mapper = __gen_unpack_uint(cl, 140, 140);
   values->relax_dependency_1 = __gen_unpack_uint(cl, 142, 142);
   values->relax_dependency_2 = __gen_unpack_uint(cl, 143, 143);
   values->index = __gen_unpack_uint(cl, 144, 159);
   values->dependency_1 = __gen_unpack_uint(cl, 160, 175);
   values->dependency_2 = __gen_unpack_uint(cl, 176, 191);
   values->next = __gen_unpack_uint(cl, 192, 255);
}

static inline void
MALI_JOB_HEADER_print(FILE *fp, const struct MALI_JOB_HEADER * values, unsigned indent)
{
   fprintf(fp, "%*sException Status: %u\n", indent, "", values->exception_status);
   fprintf(fp, "%*sFirst Incomplete Task: %u\n", indent, "", values->first_incomplete_task);
   fprintf(fp, "%*sFault Pointer: 0x%" PRIx64 "\n", indent, "", values->fault_pointer);
   fprintf(fp, "%*sIs 64b: %s\n", indent, "", values->is_64b ? "true" : "false");
   fprintf(fp, "%*sType: %s\n", indent, "", mali_job_type_as_str(values->type));
   fprintf(fp, "%*sBarrier: %s\n", indent, "", values->barrier ? "true" : "false");
   fprintf(fp, "%*sInvalidate Cache: %s\n", indent, "", values->invalidate_cache ? "true" : "false");
   fprintf(fp, "%*sSuppress Prefetch: %s\n", indent, "", values->suppress_prefetch ? "true" : "false");
   fprintf(fp, "%*sEnable Texture Mapper: %s\n", indent, "", values->enable_texture_mapper ? "true" : "false");
   fprintf(fp, "%*sRelax Dependency 1: %s\n", indent, "", values->relax_dependency_1 ? "true" : "false");
   fprintf(fp, "%*sRelax Dependency 2: %s\n", indent, "", values->relax_dependency_2 ? "true" : "false");
   fprintf(fp, "%*sIndex: %u\n", indent, "", values->index);
   fprintf(fp, "%*sDependency 1: %u\n", indent, "", values->dependency_1);
   fprintf(fp, "%*sDependency 2: %u\n", indent, "", values->dependency_2);
   fprintf(fp, "%*sNext: 0x%" PRIx64 "\n", indent, "", values->next);
}

struct MALI_FRAGMENT_JOB_PAYLOAD {
   uint32_t                             bound_min_x;
   uint32_t                             bound_min_y;
   uint32_t                             bound_max_x;
   uint32_t                             bound_max_y;
   bool                                 has_tile_enable_map;
   uint64_t                             framebuffer;
   uint64_t                             tile_enable_map;
   uint32_t                             tile_enable_map_row_stride;
};

#define MALI_FRAGMENT_JOB_PAYLOAD_header        \
   0

static inline void
MALI_FRAGMENT_JOB_PAYLOAD_pack(uint32_t * restrict cl,
                               const struct MALI_FRAGMENT_JOB_PAYLOAD * restrict values)
{
   cl[ 0] = __gen_uint(values->bound_min_x, 0, 11) |
            __gen_uint(values->bound_min_y, 16, 27);
   cl[ 1] = __gen_uint(values->bound_max_x, 0, 11) |
            __gen_uint(values->bound_max_y, 16, 27) |
            __gen_uint(values->has_tile_enable_map, 31, 31);
   cl[ 2] = __gen_uint(values->framebuffer, 0, 63);
   cl[ 3] = __gen_uint(values->framebuffer, 0, 63) >> 32;
   cl[ 4] = __gen_uint(values->tile_enable_map, 0, 63);
   cl[ 5] = __gen_uint(values->tile_enable_map, 0, 63) >> 32;
   cl[ 6] = __gen_uint(values->tile_enable_map_row_stride, 0, 7);
   cl[ 7] = 0;
}


#define MALI_FRAGMENT_JOB_PAYLOAD_LENGTH 32
struct mali_fragment_job_payload_packed { uint32_t opaque[8]; };
static inline void
MALI_FRAGMENT_JOB_PAYLOAD_unpack(const uint8_t * restrict cl,
                                 struct MALI_FRAGMENT_JOB_PAYLOAD * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xf000f000) fprintf(stderr, "XXX: Invalid field of Fragment Job Payload unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0x7000f000) fprintf(stderr, "XXX: Invalid field of Fragment Job Payload unpacked at word 1\n");
   if (((const uint32_t *) cl)[6] & 0xffffff00) fprintf(stderr, "XXX: Invalid field of Fragment Job Payload unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Fragment Job Payload unpacked at word 7\n");
   values->bound_min_x = __gen_unpack_uint(cl, 0, 11);
   values->bound_min_y = __gen_unpack_uint(cl, 16, 27);
   values->bound_max_x = __gen_unpack_uint(cl, 32, 43);
   values->bound_max_y = __gen_unpack_uint(cl, 48, 59);
   values->has_tile_enable_map = __gen_unpack_uint(cl, 63, 63);
   values->framebuffer = __gen_unpack_uint(cl, 64, 127);
   values->tile_enable_map = __gen_unpack_uint(cl, 128, 191);
   values->tile_enable_map_row_stride = __gen_unpack_uint(cl, 192, 199);
}

static inline void
MALI_FRAGMENT_JOB_PAYLOAD_print(FILE *fp, const struct MALI_FRAGMENT_JOB_PAYLOAD * values, unsigned indent)
{
   fprintf(fp, "%*sBound Min X: %u\n", indent, "", values->bound_min_x);
   fprintf(fp, "%*sBound Min Y: %u\n", indent, "", values->bound_min_y);
   fprintf(fp, "%*sBound Max X: %u\n", indent, "", values->bound_max_x);
   fprintf(fp, "%*sBound Max Y: %u\n", indent, "", values->bound_max_y);
   fprintf(fp, "%*sHas Tile Enable Map: %s\n", indent, "", values->has_tile_enable_map ? "true" : "false");
   fprintf(fp, "%*sFramebuffer: 0x%" PRIx64 "\n", indent, "", values->framebuffer);
   fprintf(fp, "%*sTile Enable Map: 0x%" PRIx64 "\n", indent, "", values->tile_enable_map);
   fprintf(fp, "%*sTile Enable Map Row Stride: %u\n", indent, "", values->tile_enable_map_row_stride);
}

struct mali_fragment_job_packed {
   uint32_t opaque[16];
};

#define MALI_FRAGMENT_JOB_LENGTH 64
#define MALI_FRAGMENT_JOB_ALIGN 64
#define MALI_FRAGMENT_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_FRAGMENT_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_FRAGMENT_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_FRAGMENT_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_FRAGMENT_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_FRAGMENT_JOB_SECTION_HEADER_OFFSET 0
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_TYPE struct MALI_FRAGMENT_JOB_PAYLOAD
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_header MALI_FRAGMENT_JOB_PAYLOAD_header
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_pack MALI_FRAGMENT_JOB_PAYLOAD_pack
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_unpack MALI_FRAGMENT_JOB_PAYLOAD_unpack
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_print MALI_FRAGMENT_JOB_PAYLOAD_print
#define MALI_FRAGMENT_JOB_SECTION_PAYLOAD_OFFSET 32

enum mali_write_value_type {
        MALI_WRITE_VALUE_TYPE_CYCLE_COUNTER  =      1,
        MALI_WRITE_VALUE_TYPE_SYSTEM_TIMESTAMP =      2,
        MALI_WRITE_VALUE_TYPE_ZERO           =      3,
        MALI_WRITE_VALUE_TYPE_IMMEDIATE_8    =      4,
        MALI_WRITE_VALUE_TYPE_IMMEDIATE_16   =      5,
        MALI_WRITE_VALUE_TYPE_IMMEDIATE_32   =      6,
        MALI_WRITE_VALUE_TYPE_IMMEDIATE_64   =      7,
};

static inline const char *
mali_write_value_type_as_str(enum mali_write_value_type imm)
{
    switch (imm) {
    case MALI_WRITE_VALUE_TYPE_CYCLE_COUNTER: return "Cycle Counter";
    case MALI_WRITE_VALUE_TYPE_SYSTEM_TIMESTAMP: return "System Timestamp";
    case MALI_WRITE_VALUE_TYPE_ZERO: return "Zero";
    case MALI_WRITE_VALUE_TYPE_IMMEDIATE_8: return "Immediate 8";
    case MALI_WRITE_VALUE_TYPE_IMMEDIATE_16: return "Immediate 16";
    case MALI_WRITE_VALUE_TYPE_IMMEDIATE_32: return "Immediate 32";
    case MALI_WRITE_VALUE_TYPE_IMMEDIATE_64: return "Immediate 64";
    default: return "XXX: INVALID";
    }
}

struct MALI_WRITE_VALUE_JOB_PAYLOAD {
   uint64_t                             address;
   enum mali_write_value_type           type;
   uint64_t                             immediate_value;
};

#define MALI_WRITE_VALUE_JOB_PAYLOAD_header     \
   0

static inline void
MALI_WRITE_VALUE_JOB_PAYLOAD_pack(uint32_t * restrict cl,
                                  const struct MALI_WRITE_VALUE_JOB_PAYLOAD * restrict values)
{
   cl[ 0] = __gen_uint(values->address, 0, 63);
   cl[ 1] = __gen_uint(values->address, 0, 63) >> 32;
   cl[ 2] = __gen_uint(values->type, 0, 31);
   cl[ 3] = 0;
   cl[ 4] = __gen_uint(values->immediate_value, 0, 63);
   cl[ 5] = __gen_uint(values->immediate_value, 0, 63) >> 32;
}


#define MALI_WRITE_VALUE_JOB_PAYLOAD_LENGTH 24
struct mali_write_value_job_payload_packed { uint32_t opaque[6]; };
static inline void
MALI_WRITE_VALUE_JOB_PAYLOAD_unpack(const uint8_t * restrict cl,
                                    struct MALI_WRITE_VALUE_JOB_PAYLOAD * restrict values)
{
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Write Value Job Payload unpacked at word 3\n");
   values->address = __gen_unpack_uint(cl, 0, 63);
   values->type = (enum mali_write_value_type)__gen_unpack_uint(cl, 64, 95);
   values->immediate_value = __gen_unpack_uint(cl, 128, 191);
}

static inline void
MALI_WRITE_VALUE_JOB_PAYLOAD_print(FILE *fp, const struct MALI_WRITE_VALUE_JOB_PAYLOAD * values, unsigned indent)
{
   fprintf(fp, "%*sAddress: 0x%" PRIx64 "\n", indent, "", values->address);
   fprintf(fp, "%*sType: %s\n", indent, "", mali_write_value_type_as_str(values->type));
   fprintf(fp, "%*sImmediate Value: 0x%" PRIx64 "\n", indent, "", values->immediate_value);
}

struct MALI_CACHE_FLUSH_JOB_PAYLOAD {
   bool                                 clean_shader_core_ls;
   bool                                 invalidate_shader_core_ls;
   bool                                 invalidate_shader_core_other;
   bool                                 job_manager_clean;
   bool                                 job_manager_invalidate;
   bool                                 tiler_clean;
   bool                                 tiler_invalidate;
   bool                                 l2_clean;
   bool                                 l2_invalidate;
};

#define MALI_CACHE_FLUSH_JOB_PAYLOAD_header     \
   0

static inline void
MALI_CACHE_FLUSH_JOB_PAYLOAD_pack(uint32_t * restrict cl,
                                  const struct MALI_CACHE_FLUSH_JOB_PAYLOAD * restrict values)
{
   cl[ 0] = __gen_uint(values->clean_shader_core_ls, 0, 0) |
            __gen_uint(values->invalidate_shader_core_ls, 1, 1) |
            __gen_uint(values->invalidate_shader_core_other, 2, 2) |
            __gen_uint(values->job_manager_clean, 16, 16) |
            __gen_uint(values->job_manager_invalidate, 17, 17) |
            __gen_uint(values->tiler_clean, 24, 24) |
            __gen_uint(values->tiler_invalidate, 25, 25);
   cl[ 1] = __gen_uint(values->l2_clean, 0, 0) |
            __gen_uint(values->l2_invalidate, 1, 1);
}


#define MALI_CACHE_FLUSH_JOB_PAYLOAD_LENGTH 8
struct mali_cache_flush_job_payload_packed { uint32_t opaque[2]; };
static inline void
MALI_CACHE_FLUSH_JOB_PAYLOAD_unpack(const uint8_t * restrict cl,
                                    struct MALI_CACHE_FLUSH_JOB_PAYLOAD * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xfcfcfff8) fprintf(stderr, "XXX: Invalid field of Cache Flush Job Payload unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xfffffffc) fprintf(stderr, "XXX: Invalid field of Cache Flush Job Payload unpacked at word 1\n");
   values->clean_shader_core_ls = __gen_unpack_uint(cl, 0, 0);
   values->invalidate_shader_core_ls = __gen_unpack_uint(cl, 1, 1);
   values->invalidate_shader_core_other = __gen_unpack_uint(cl, 2, 2);
   values->job_manager_clean = __gen_unpack_uint(cl, 16, 16);
   values->job_manager_invalidate = __gen_unpack_uint(cl, 17, 17);
   values->tiler_clean = __gen_unpack_uint(cl, 24, 24);
   values->tiler_invalidate = __gen_unpack_uint(cl, 25, 25);
   values->l2_clean = __gen_unpack_uint(cl, 32, 32);
   values->l2_invalidate = __gen_unpack_uint(cl, 33, 33);
}

static inline void
MALI_CACHE_FLUSH_JOB_PAYLOAD_print(FILE *fp, const struct MALI_CACHE_FLUSH_JOB_PAYLOAD * values, unsigned indent)
{
   fprintf(fp, "%*sClean Shader Core LS: %s\n", indent, "", values->clean_shader_core_ls ? "true" : "false");
   fprintf(fp, "%*sInvalidate Shader Core LS: %s\n", indent, "", values->invalidate_shader_core_ls ? "true" : "false");
   fprintf(fp, "%*sInvalidate Shader Core Other: %s\n", indent, "", values->invalidate_shader_core_other ? "true" : "false");
   fprintf(fp, "%*sJob Manager Clean: %s\n", indent, "", values->job_manager_clean ? "true" : "false");
   fprintf(fp, "%*sJob Manager Invalidate: %s\n", indent, "", values->job_manager_invalidate ? "true" : "false");
   fprintf(fp, "%*sTiler Clean: %s\n", indent, "", values->tiler_clean ? "true" : "false");
   fprintf(fp, "%*sTiler Invalidate: %s\n", indent, "", values->tiler_invalidate ? "true" : "false");
   fprintf(fp, "%*sL2 Clean: %s\n", indent, "", values->l2_clean ? "true" : "false");
   fprintf(fp, "%*sL2 Invalidate: %s\n", indent, "", values->l2_invalidate ? "true" : "false");
}

struct mali_write_value_job_packed {
   uint32_t opaque[14];
};

#define MALI_WRITE_VALUE_JOB_LENGTH 56
#define MALI_WRITE_VALUE_JOB_ALIGN 64
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_WRITE_VALUE_JOB_SECTION_HEADER_OFFSET 0
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_TYPE struct MALI_WRITE_VALUE_JOB_PAYLOAD
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_header MALI_WRITE_VALUE_JOB_PAYLOAD_header
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_pack MALI_WRITE_VALUE_JOB_PAYLOAD_pack
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_unpack MALI_WRITE_VALUE_JOB_PAYLOAD_unpack
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_print MALI_WRITE_VALUE_JOB_PAYLOAD_print
#define MALI_WRITE_VALUE_JOB_SECTION_PAYLOAD_OFFSET 32

struct mali_cache_flush_job_packed {
   uint32_t opaque[10];
};

#define MALI_CACHE_FLUSH_JOB_LENGTH 40
#define MALI_CACHE_FLUSH_JOB_ALIGN 64
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_CACHE_FLUSH_JOB_SECTION_HEADER_OFFSET 0
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_TYPE struct MALI_CACHE_FLUSH_JOB_PAYLOAD
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_header MALI_CACHE_FLUSH_JOB_PAYLOAD_header
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_pack MALI_CACHE_FLUSH_JOB_PAYLOAD_pack
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_unpack MALI_CACHE_FLUSH_JOB_PAYLOAD_unpack
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_print MALI_CACHE_FLUSH_JOB_PAYLOAD_print
#define MALI_CACHE_FLUSH_JOB_SECTION_PAYLOAD_OFFSET 32

struct MALI_COMPUTE_JOB_PARAMETERS {
   uint32_t                             job_task_split;
};

#define MALI_COMPUTE_JOB_PARAMETERS_header      \
   0

static inline void
MALI_COMPUTE_JOB_PARAMETERS_pack(uint32_t * restrict cl,
                                 const struct MALI_COMPUTE_JOB_PARAMETERS * restrict values)
{
   cl[ 0] = __gen_uint(values->job_task_split, 26, 29);
   cl[ 1] = 0;
   cl[ 2] = 0;
   cl[ 3] = 0;
   cl[ 4] = 0;
   cl[ 5] = 0;
}


#define MALI_COMPUTE_JOB_PARAMETERS_LENGTH 24
struct mali_compute_job_parameters_packed { uint32_t opaque[6]; };
static inline void
MALI_COMPUTE_JOB_PARAMETERS_unpack(const uint8_t * restrict cl,
                                   struct MALI_COMPUTE_JOB_PARAMETERS * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xc3ffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 3\n");
   if (((const uint32_t *) cl)[4] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 4\n");
   if (((const uint32_t *) cl)[5] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Compute Job Parameters unpacked at word 5\n");
   values->job_task_split = __gen_unpack_uint(cl, 26, 29);
}

static inline void
MALI_COMPUTE_JOB_PARAMETERS_print(FILE *fp, const struct MALI_COMPUTE_JOB_PARAMETERS * values, unsigned indent)
{
   fprintf(fp, "%*sJob Task Split: %u\n", indent, "", values->job_task_split);
}

struct mali_compute_job_packed {
   uint32_t opaque[48];
};

#define MALI_COMPUTE_JOB_LENGTH 192
#define MALI_COMPUTE_JOB_ALIGN 64
#define MALI_COMPUTE_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_COMPUTE_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_COMPUTE_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_COMPUTE_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_COMPUTE_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_COMPUTE_JOB_SECTION_HEADER_OFFSET 0
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_TYPE struct MALI_INVOCATION
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_header MALI_INVOCATION_header
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_pack MALI_INVOCATION_pack
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_unpack MALI_INVOCATION_unpack
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_print MALI_INVOCATION_print
#define MALI_COMPUTE_JOB_SECTION_INVOCATION_OFFSET 32
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_TYPE struct MALI_COMPUTE_JOB_PARAMETERS
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_header MALI_COMPUTE_JOB_PARAMETERS_header
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_pack MALI_COMPUTE_JOB_PARAMETERS_pack
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_unpack MALI_COMPUTE_JOB_PARAMETERS_unpack
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_print MALI_COMPUTE_JOB_PARAMETERS_print
#define MALI_COMPUTE_JOB_SECTION_PARAMETERS_OFFSET 40
#define MALI_COMPUTE_JOB_SECTION_DRAW_TYPE struct MALI_DRAW
#define MALI_COMPUTE_JOB_SECTION_DRAW_header MALI_DRAW_header
#define MALI_COMPUTE_JOB_SECTION_DRAW_pack MALI_DRAW_pack
#define MALI_COMPUTE_JOB_SECTION_DRAW_unpack MALI_DRAW_unpack
#define MALI_COMPUTE_JOB_SECTION_DRAW_print MALI_DRAW_print
#define MALI_COMPUTE_JOB_SECTION_DRAW_OFFSET 64

struct MALI_PRIMITIVE_SIZE {
   float                                constant;
   uint64_t                             size_array;
};

#define MALI_PRIMITIVE_SIZE_header              \
   0

static inline void
MALI_PRIMITIVE_SIZE_pack(uint32_t * restrict cl,
                         const struct MALI_PRIMITIVE_SIZE * restrict values)
{
   cl[ 0] = __gen_uint(fui(values->constant), 0, 32) |
            __gen_uint(values->size_array, 0, 63);
   cl[ 1] = __gen_uint(values->size_array, 0, 63) >> 32;
}


#define MALI_PRIMITIVE_SIZE_LENGTH 8
struct mali_primitive_size_packed { uint32_t opaque[2]; };
static inline void
MALI_PRIMITIVE_SIZE_unpack(const uint8_t * restrict cl,
                           struct MALI_PRIMITIVE_SIZE * restrict values)
{
   values->constant = __gen_unpack_float(cl, 0, 31);
   values->size_array = __gen_unpack_uint(cl, 0, 63);
}

static inline void
MALI_PRIMITIVE_SIZE_print(FILE *fp, const struct MALI_PRIMITIVE_SIZE * values, unsigned indent)
{
   fprintf(fp, "%*sConstant: %f\n", indent, "", values->constant);
   fprintf(fp, "%*sSize Array: 0x%" PRIx64 "\n", indent, "", values->size_array);
}

struct MALI_TILER_POINTER {
   uint64_t                             address;
};

#define MALI_TILER_POINTER_header               \
   0

static inline void
MALI_TILER_POINTER_pack(uint32_t * restrict cl,
                        const struct MALI_TILER_POINTER * restrict values)
{
   cl[ 0] = __gen_uint(values->address, 0, 63);
   cl[ 1] = __gen_uint(values->address, 0, 63) >> 32;
}


#define MALI_TILER_POINTER_LENGTH 8
struct mali_tiler_pointer_packed { uint32_t opaque[2]; };
static inline void
MALI_TILER_POINTER_unpack(const uint8_t * restrict cl,
                          struct MALI_TILER_POINTER * restrict values)
{
   values->address = __gen_unpack_uint(cl, 0, 63);
}

static inline void
MALI_TILER_POINTER_print(FILE *fp, const struct MALI_TILER_POINTER * values, unsigned indent)
{
   fprintf(fp, "%*sAddress: 0x%" PRIx64 "\n", indent, "", values->address);
}

struct MALI_TILER_JOB_PADDING {
   int dummy;
};

#define MALI_TILER_JOB_PADDING_header           \
   0

static inline void
MALI_TILER_JOB_PADDING_pack(uint32_t * restrict cl,
                            const struct MALI_TILER_JOB_PADDING * restrict values)
{
   cl[ 0] = 0;
   cl[ 1] = 0;
   cl[ 2] = 0;
   cl[ 3] = 0;
   cl[ 4] = 0;
   cl[ 5] = 0;
   cl[ 6] = 0;
   cl[ 7] = 0;
   cl[ 8] = 0;
   cl[ 9] = 0;
   cl[10] = 0;
   cl[11] = 0;
}


#define MALI_TILER_JOB_PADDING_LENGTH 48
struct mali_tiler_job_padding_packed { uint32_t opaque[12]; };
static inline void
MALI_TILER_JOB_PADDING_unpack(const uint8_t * restrict cl,
                              struct MALI_TILER_JOB_PADDING * restrict values)
{
   if (((const uint32_t *) cl)[0] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 0\n");
   if (((const uint32_t *) cl)[1] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 1\n");
   if (((const uint32_t *) cl)[2] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 2\n");
   if (((const uint32_t *) cl)[3] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 3\n");
   if (((const uint32_t *) cl)[4] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 4\n");
   if (((const uint32_t *) cl)[5] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 5\n");
   if (((const uint32_t *) cl)[6] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 6\n");
   if (((const uint32_t *) cl)[7] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 7\n");
   if (((const uint32_t *) cl)[8] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 8\n");
   if (((const uint32_t *) cl)[9] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 9\n");
   if (((const uint32_t *) cl)[10] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 10\n");
   if (((const uint32_t *) cl)[11] & 0xffffffff) fprintf(stderr, "XXX: Invalid field of Tiler Job Padding unpacked at word 11\n");
}

static inline void
MALI_TILER_JOB_PADDING_print(FILE *fp, const struct MALI_TILER_JOB_PADDING * values, unsigned indent)
{
}

struct mali_tiler_job_packed {
   uint32_t opaque[64];
};

#define MALI_TILER_JOB_LENGTH 256
#define MALI_TILER_JOB_ALIGN 64
#define MALI_TILER_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_TILER_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_TILER_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_TILER_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_TILER_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_TILER_JOB_SECTION_HEADER_OFFSET 0
#define MALI_TILER_JOB_SECTION_INVOCATION_TYPE struct MALI_INVOCATION
#define MALI_TILER_JOB_SECTION_INVOCATION_header MALI_INVOCATION_header
#define MALI_TILER_JOB_SECTION_INVOCATION_pack MALI_INVOCATION_pack
#define MALI_TILER_JOB_SECTION_INVOCATION_unpack MALI_INVOCATION_unpack
#define MALI_TILER_JOB_SECTION_INVOCATION_print MALI_INVOCATION_print
#define MALI_TILER_JOB_SECTION_INVOCATION_OFFSET 32
#define MALI_TILER_JOB_SECTION_PRIMITIVE_TYPE struct MALI_PRIMITIVE
#define MALI_TILER_JOB_SECTION_PRIMITIVE_header MALI_PRIMITIVE_header
#define MALI_TILER_JOB_SECTION_PRIMITIVE_pack MALI_PRIMITIVE_pack
#define MALI_TILER_JOB_SECTION_PRIMITIVE_unpack MALI_PRIMITIVE_unpack
#define MALI_TILER_JOB_SECTION_PRIMITIVE_print MALI_PRIMITIVE_print
#define MALI_TILER_JOB_SECTION_PRIMITIVE_OFFSET 40
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_TYPE struct MALI_PRIMITIVE_SIZE
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_header MALI_PRIMITIVE_SIZE_header
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_pack MALI_PRIMITIVE_SIZE_pack
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_unpack MALI_PRIMITIVE_SIZE_unpack
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_print MALI_PRIMITIVE_SIZE_print
#define MALI_TILER_JOB_SECTION_PRIMITIVE_SIZE_OFFSET 64
#define MALI_TILER_JOB_SECTION_TILER_TYPE struct MALI_TILER_POINTER
#define MALI_TILER_JOB_SECTION_TILER_header MALI_TILER_POINTER_header
#define MALI_TILER_JOB_SECTION_TILER_pack MALI_TILER_POINTER_pack
#define MALI_TILER_JOB_SECTION_TILER_unpack MALI_TILER_POINTER_unpack
#define MALI_TILER_JOB_SECTION_TILER_print MALI_TILER_POINTER_print
#define MALI_TILER_JOB_SECTION_TILER_OFFSET 72
#define MALI_TILER_JOB_SECTION_PADDING_TYPE struct MALI_TILER_JOB_PADDING
#define MALI_TILER_JOB_SECTION_PADDING_header MALI_TILER_JOB_PADDING_header
#define MALI_TILER_JOB_SECTION_PADDING_pack MALI_TILER_JOB_PADDING_pack
#define MALI_TILER_JOB_SECTION_PADDING_unpack MALI_TILER_JOB_PADDING_unpack
#define MALI_TILER_JOB_SECTION_PADDING_print MALI_TILER_JOB_PADDING_print
#define MALI_TILER_JOB_SECTION_PADDING_OFFSET 80
#define MALI_TILER_JOB_SECTION_DRAW_TYPE struct MALI_DRAW
#define MALI_TILER_JOB_SECTION_DRAW_header MALI_DRAW_header
#define MALI_TILER_JOB_SECTION_DRAW_pack MALI_DRAW_pack
#define MALI_TILER_JOB_SECTION_DRAW_unpack MALI_DRAW_unpack
#define MALI_TILER_JOB_SECTION_DRAW_print MALI_DRAW_print
#define MALI_TILER_JOB_SECTION_DRAW_OFFSET 128

struct mali_indexed_vertex_job_packed {
   uint32_t opaque[96];
};

#define MALI_INDEXED_VERTEX_JOB_LENGTH 384
#define MALI_INDEXED_VERTEX_JOB_ALIGN 64
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_TYPE struct MALI_JOB_HEADER
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_header MALI_JOB_HEADER_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_pack MALI_JOB_HEADER_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_unpack MALI_JOB_HEADER_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_print MALI_JOB_HEADER_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_HEADER_OFFSET 0
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_TYPE struct MALI_INVOCATION
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_header MALI_INVOCATION_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_pack MALI_INVOCATION_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_unpack MALI_INVOCATION_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_print MALI_INVOCATION_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_INVOCATION_OFFSET 32
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_TYPE struct MALI_PRIMITIVE
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_header MALI_PRIMITIVE_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_pack MALI_PRIMITIVE_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_unpack MALI_PRIMITIVE_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_print MALI_PRIMITIVE_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_OFFSET 40
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_TYPE struct MALI_PRIMITIVE_SIZE
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_header MALI_PRIMITIVE_SIZE_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_pack MALI_PRIMITIVE_SIZE_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_unpack MALI_PRIMITIVE_SIZE_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_print MALI_PRIMITIVE_SIZE_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_PRIMITIVE_SIZE_OFFSET 64
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_TYPE struct MALI_TILER_POINTER
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_header MALI_TILER_POINTER_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_pack MALI_TILER_POINTER_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_unpack MALI_TILER_POINTER_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_print MALI_TILER_POINTER_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_TILER_OFFSET 72
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_TYPE struct MALI_TILER_JOB_PADDING
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_header MALI_TILER_JOB_PADDING_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_pack MALI_TILER_JOB_PADDING_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_unpack MALI_TILER_JOB_PADDING_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_print MALI_TILER_JOB_PADDING_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_PADDING_OFFSET 80
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_TYPE struct MALI_DRAW
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_header MALI_DRAW_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_pack MALI_DRAW_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_unpack MALI_DRAW_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_print MALI_DRAW_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_FRAGMENT_DRAW_OFFSET 128
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_TYPE struct MALI_DRAW
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_header MALI_DRAW_header
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_pack MALI_DRAW_pack
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_unpack MALI_DRAW_unpack
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_print MALI_DRAW_print
#define MALI_INDEXED_VERTEX_JOB_SECTION_VERTEX_DRAW_OFFSET 256

#include "panfrost-job.h"
#endif
