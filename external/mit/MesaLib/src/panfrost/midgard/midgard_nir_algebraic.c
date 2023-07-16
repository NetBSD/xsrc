#include "midgard_nir.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 1
 * transforms:
 *    ('~fadd', ('fadd', 'a', 'b'), 'a') => ('fadd', ('fadd', 'a', 'a'), 'b')
 */


   static const nir_search_variable search0_0_0 = {
   { nir_search_value_variable, -2 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static const nir_search_variable search0_0_1 = {
   { nir_search_value_variable, -2 },
   1, /* b */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search0_0 = {
   { nir_search_value_expression, -2 },
   false, false,
   1, 1,
   nir_op_fadd,
   { &search0_0_0.value, &search0_0_1.value },
   NULL,
};

/* search0_1 -> search0_0_0 in the cache */
static const nir_search_expression search0 = {
   { nir_search_value_expression, -2 },
   true, false,
   0, 2,
   nir_op_fadd,
   { &search0_0.value, &search0_0_0.value },
   NULL,
};

   /* replace0_0_0 -> search0_0_0 in the cache */

/* replace0_0_1 -> search0_0_0 in the cache */
static const nir_search_expression replace0_0 = {
   { nir_search_value_expression, -2 },
   false, false,
   -1, 0,
   nir_op_fadd,
   { &search0_0_0.value, &search0_0_0.value },
   NULL,
};

/* replace0_1 -> search0_0_1 in the cache */
static const nir_search_expression replace0 = {
   { nir_search_value_expression, -2 },
   false, false,
   0, 1,
   nir_op_fadd,
   { &replace0_0.value, &search0_0_1.value },
   NULL,
};


static const struct transform midgard_nir_lower_algebraic_early_state3_xforms[] = {
  { &search0, &replace0.value, 0 },
};

static const struct per_op_table midgard_nir_lower_algebraic_early_table[nir_num_search_ops] = {
   [nir_op_fadd] = {
      .filter = (uint16_t []) {
         0,
         0,
         1,
         1,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         2,
         3,
         3,
         3,
      },
   },
};

const struct transform *midgard_nir_lower_algebraic_early_transforms[] = {
   NULL,
   NULL,
   NULL,
   midgard_nir_lower_algebraic_early_state3_xforms,
};

const uint16_t midgard_nir_lower_algebraic_early_transform_counts[] = {
   0,
   0,
   0,
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_early_state3_xforms),
};

bool
midgard_nir_lower_algebraic_early(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[1];
   const nir_shader_compiler_options *options = shader->options;
   const shader_info *info = &shader->info;
   (void) options;
   (void) info;

   condition_flags[0] = true;

   nir_foreach_function(function, shader) {
      if (function->impl) {
         progress |= nir_algebraic_impl(function->impl, condition_flags,
                                        midgard_nir_lower_algebraic_early_transforms,
                                        midgard_nir_lower_algebraic_early_transform_counts,
                                        midgard_nir_lower_algebraic_early_table);
      }
   }

   return progress;
}


#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 54
 * transforms:
 *    ('ineg', 'a') => ('isub', 0, 'a')
 *    ('fsub', 'a', 'b') => ('fadd', 'a', ('fneg', 'b'))
 *    ('b32csel', 'a', 'b@32', 0) => ('iand', 'a', 'b')
 *    ('b32csel', 'a', 0, 'b@32') => ('iand', ('inot', 'a'), 'b')
 *    ('~fmin', ('fmax', 'a', -1.0), 1.0) => ('fsat_signed_mali', 'a')
 *    ('~fmax', ('fmin', 'a', 1.0), -1.0) => ('fsat_signed_mali', 'a')
 *    ('fmax', 'a', 0.0) => ('fclamp_pos_mali', 'a')
 *    ('ishl', 'a@16', 'b') => ('u2u16', ('ishl', ('u2u32', 'a'), 'b'))
 *    ('ishr', 'a@16', 'b') => ('i2i16', ('ishr', ('i2i32', 'a'), 'b'))
 *    ('ushr', 'a@16', 'b') => ('u2u16', ('ushr', ('u2u32', 'a'), 'b'))
 *    ('ishl', 'a@8', 'b') => ('u2u8', ('u2u16', ('ishl', ('u2u32', ('u2u16', 'a')), 'b')))
 *    ('ishr', 'a@8', 'b') => ('i2i8', ('i2i16', ('ishr', ('i2i32', ('i2i16', 'a')), 'b')))
 *    ('ushr', 'a@8', 'b') => ('u2u8', ('u2u16', ('ushr', ('u2u32', ('u2u16', 'a')), 'b')))
 *    ('fmul', 'a', 2.0) => ('fadd', 'a', 'a')
 *    ('ufind_msb', 'a@8') => ('isub', 31, ('uclz', ('u2u32', 'a')))
 *    ('ufind_msb', 'a@16') => ('isub', 31, ('uclz', ('u2u32', 'a')))
 *    ('ufind_msb', 'a@32') => ('isub', 31, ('uclz', 'a'))
 *    ('u2u8', 'a@32') => ('u2u8', ('u2u16', 'a'))
 *    ('u2u8', 'a@64') => ('u2u8', ('u2u16', ('u2u32', 'a')))
 *    ('u2u16', 'a@64') => ('u2u16', ('u2u32', 'a'))
 *    ('u2u32', 'a@8') => ('u2u32', ('u2u16', 'a'))
 *    ('u2u64', 'a@8') => ('u2u64', ('u2u32', ('u2u16', 'a')))
 *    ('u2u64', 'a@16') => ('u2u64', ('u2u32', 'a'))
 *    ('i2i8', 'a@32') => ('i2i8', ('i2i16', 'a'))
 *    ('i2i8', 'a@64') => ('i2i8', ('i2i16', ('i2i32', 'a')))
 *    ('i2i16', 'a@64') => ('i2i16', ('i2i32', 'a'))
 *    ('i2i32', 'a@8') => ('i2i32', ('i2i16', 'a'))
 *    ('i2i64', 'a@8') => ('i2i64', ('i2i32', ('i2i16', 'a')))
 *    ('i2i64', 'a@16') => ('i2i64', ('i2i32', 'a'))
 *    ('f2f16', 'a@64') => ('f2f16', ('f2f32', 'a'))
 *    ('f2f64', 'a@16') => ('f2f64', ('f2f32', 'a'))
 *    ('i2f16', 'a@64') => ('f2f16', ('f2f32', ('i2f64', 'a')))
 *    ('i2f32', 'a@8') => ('i2f32', ('i2i32', ('i2i16', 'a')))
 *    ('i2f64', 'a@8') => ('i2f64', ('i2i64', ('i2i32', ('i2i16', 'a'))))
 *    ('i2f64', 'a@16') => ('i2f64', ('i2i64', ('i2i32', 'a')))
 *    ('u2f16', 'a@64') => ('f2f16', ('f2f32', ('u2f64', 'a')))
 *    ('u2f32', 'a@8') => ('u2f32', ('u2u32', ('u2u16', 'a')))
 *    ('u2f64', 'a@8') => ('u2f64', ('u2u64', ('u2u32', ('u2u16', 'a'))))
 *    ('u2f64', 'a@16') => ('u2f64', ('u2u64', ('u2u32', 'a')))
 *    ('f2i8', 'a@32') => ('i2i8', ('i2i16', ('f2i32', 'a')))
 *    ('f2i8', 'a@64') => ('i2i8', ('i2i16', ('i2i32', ('f2i64', 'a'))))
 *    ('f2i16', 'a@64') => ('i2i16', ('i2i32', ('f2i64', 'a')))
 *    ('f2i64', 'a@16') => ('f2i64', ('f2f64', ('f2f32', 'a')))
 *    ('f2u8', 'a@32') => ('u2u8', ('u2u16', ('f2u32', 'a')))
 *    ('f2u8', 'a@64') => ('u2u8', ('u2u16', ('u2u32', ('f2u64', 'a'))))
 *    ('f2u16', 'a@64') => ('u2u16', ('u2u32', ('f2u64', 'a')))
 *    ('f2u64', 'a@16') => ('f2u64', ('f2f64', ('f2f32', 'a')))
 *    ('fge', 'a', '#b') => ('inot', ('flt', 'a', 'b'))
 *    ('fge32', 'a', '#b') => ('inot', ('flt32', 'a', 'b'))
 *    ('ige32', 'a', '#b') => ('inot', ('ilt32', 'a', 'b'))
 *    ('uge32', 'a', '#b') => ('inot', ('ult32', 'a', 'b'))
 *    ('flt32', '#a', 'b') => ('inot', ('fge32', 'a', 'b'))
 *    ('ilt32', '#a', 'b') => ('inot', ('ige32', 'a', 'b'))
 *    ('ult32', '#a', 'b') => ('inot', ('uge32', 'a', 'b'))
 */


   static const nir_search_variable search1_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_ineg,
   { &search1_0.value },
   NULL,
};

   static const nir_search_constant replace1_0 = {
   { nir_search_value_constant, -1 },
   nir_type_int, { 0x0 /* 0 */ },
};

/* replace1_1 -> search1_0 in the cache */
static const nir_search_expression replace1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_isub,
   { &replace1_0.value, &search1_0.value },
   NULL,
};

   static const nir_search_variable search2_0 = {
   { nir_search_value_variable, -2 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static const nir_search_variable search2_1 = {
   { nir_search_value_variable, -2 },
   1, /* b */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search2 = {
   { nir_search_value_expression, -2 },
   false, false,
   -1, 0,
   nir_op_fsub,
   { &search2_0.value, &search2_1.value },
   NULL,
};

   /* replace2_0 -> search2_0 in the cache */

/* replace2_1_0 -> search2_1 in the cache */
static const nir_search_expression replace2_1 = {
   { nir_search_value_expression, -2 },
   false, false,
   -1, 0,
   nir_op_fneg,
   { &search2_1.value },
   NULL,
};
static const nir_search_expression replace2 = {
   { nir_search_value_expression, -2 },
   false, false,
   0, 1,
   nir_op_fadd,
   { &search2_0.value, &replace2_1.value },
   NULL,
};

   static const nir_search_variable search3_0 = {
   { nir_search_value_variable, 32 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static const nir_search_variable search3_1 = {
   { nir_search_value_variable, 32 },
   1, /* b */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static const nir_search_constant search3_2 = {
   { nir_search_value_constant, 32 },
   nir_type_int, { 0x0 /* 0 */ },
};
static const nir_search_expression search3 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_b32csel,
   { &search3_0.value, &search3_1.value, &search3_2.value },
   NULL,
};

   /* replace3_0 -> search3_0 in the cache */

/* replace3_1 -> search3_1 in the cache */
static const nir_search_expression replace3 = {
   { nir_search_value_expression, 32 },
   false, false,
   0, 1,
   nir_op_iand,
   { &search3_0.value, &search3_1.value },
   NULL,
};

   /* search4_0 -> search3_0 in the cache */

/* search4_1 -> search3_2 in the cache */

/* search4_2 -> search3_1 in the cache */
static const nir_search_expression search4 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_b32csel,
   { &search3_0.value, &search3_2.value, &search3_1.value },
   NULL,
};

   /* replace4_0_0 -> search3_0 in the cache */
static const nir_search_expression replace4_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &search3_0.value },
   NULL,
};

/* replace4_1 -> search3_1 in the cache */
static const nir_search_expression replace4 = {
   { nir_search_value_expression, 32 },
   false, false,
   0, 1,
   nir_op_iand,
   { &replace4_0.value, &search3_1.value },
   NULL,
};

   /* search5_0_0 -> search1_0 in the cache */

static const nir_search_constant search5_0_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0xbff0000000000000 /* -1.0 */ },
};
static const nir_search_expression search5_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   1, 1,
   nir_op_fmax,
   { &search1_0.value, &search5_0_1.value },
   NULL,
};

static const nir_search_constant search5_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x3ff0000000000000 /* 1.0 */ },
};
static const nir_search_expression search5 = {
   { nir_search_value_expression, -1 },
   true, false,
   0, 2,
   nir_op_fmin,
   { &search5_0.value, &search5_1.value },
   NULL,
};

   /* replace5_0 -> search1_0 in the cache */
static const nir_search_expression replace5 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsat_signed_mali,
   { &search1_0.value },
   NULL,
};

   /* search6_0_0 -> search1_0 in the cache */

/* search6_0_1 -> search5_1 in the cache */
static const nir_search_expression search6_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   1, 1,
   nir_op_fmin,
   { &search1_0.value, &search5_1.value },
   NULL,
};

/* search6_1 -> search5_0_1 in the cache */
static const nir_search_expression search6 = {
   { nir_search_value_expression, -1 },
   true, false,
   0, 2,
   nir_op_fmax,
   { &search6_0.value, &search5_0_1.value },
   NULL,
};

   /* replace6_0 -> search1_0 in the cache */
/* replace6 -> replace5 in the cache */

   /* search7_0 -> search1_0 in the cache */

static const nir_search_constant search7_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x0 /* 0.0 */ },
};
static const nir_search_expression search7 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 1,
   nir_op_fmax,
   { &search1_0.value, &search7_1.value },
   NULL,
};

   /* replace7_0 -> search1_0 in the cache */
static const nir_search_expression replace7 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fclamp_pos_mali,
   { &search1_0.value },
   NULL,
};

   static const nir_search_variable search8_0 = {
   { nir_search_value_variable, 16 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

/* search8_1 -> search3_1 in the cache */
static const nir_search_expression search8 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_ishl,
   { &search8_0.value, &search3_1.value },
   NULL,
};

   /* replace8_0_0_0 -> search8_0 in the cache */
static const nir_search_expression replace8_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2u32,
   { &search8_0.value },
   NULL,
};

/* replace8_0_1 -> search3_1 in the cache */
static const nir_search_expression replace8_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ishl,
   { &replace8_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace8 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace8_0.value },
   NULL,
};

   /* search9_0 -> search8_0 in the cache */

/* search9_1 -> search3_1 in the cache */
static const nir_search_expression search9 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_ishr,
   { &search8_0.value, &search3_1.value },
   NULL,
};

   /* replace9_0_0_0 -> search8_0 in the cache */
static const nir_search_expression replace9_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2i32,
   { &search8_0.value },
   NULL,
};

/* replace9_0_1 -> search3_1 in the cache */
static const nir_search_expression replace9_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ishr,
   { &replace9_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace9 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &replace9_0.value },
   NULL,
};

   /* search10_0 -> search8_0 in the cache */

/* search10_1 -> search3_1 in the cache */
static const nir_search_expression search10 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_ushr,
   { &search8_0.value, &search3_1.value },
   NULL,
};

   /* replace10_0_0_0 -> search8_0 in the cache */
/* replace10_0_0 -> replace8_0_0 in the cache */

/* replace10_0_1 -> search3_1 in the cache */
static const nir_search_expression replace10_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ushr,
   { &replace8_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace10 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace10_0.value },
   NULL,
};

   static const nir_search_variable search11_0 = {
   { nir_search_value_variable, 8 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

/* search11_1 -> search3_1 in the cache */
static const nir_search_expression search11 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_ishl,
   { &search11_0.value, &search3_1.value },
   NULL,
};

   /* replace11_0_0_0_0_0 -> search11_0 in the cache */
static const nir_search_expression replace11_0_0_0_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &search11_0.value },
   NULL,
};
static const nir_search_expression replace11_0_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2u32,
   { &replace11_0_0_0_0.value },
   NULL,
};

/* replace11_0_0_1 -> search3_1 in the cache */
static const nir_search_expression replace11_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ishl,
   { &replace11_0_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace11_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace11_0_0.value },
   NULL,
};
static const nir_search_expression replace11 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace11_0.value },
   NULL,
};

   /* search12_0 -> search11_0 in the cache */

/* search12_1 -> search3_1 in the cache */
static const nir_search_expression search12 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_ishr,
   { &search11_0.value, &search3_1.value },
   NULL,
};

   /* replace12_0_0_0_0_0 -> search11_0 in the cache */
static const nir_search_expression replace12_0_0_0_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &search11_0.value },
   NULL,
};
static const nir_search_expression replace12_0_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2i32,
   { &replace12_0_0_0_0.value },
   NULL,
};

/* replace12_0_0_1 -> search3_1 in the cache */
static const nir_search_expression replace12_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ishr,
   { &replace12_0_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace12_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &replace12_0_0.value },
   NULL,
};
static const nir_search_expression replace12 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &replace12_0.value },
   NULL,
};

   /* search13_0 -> search11_0 in the cache */

/* search13_1 -> search3_1 in the cache */
static const nir_search_expression search13 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_ushr,
   { &search11_0.value, &search3_1.value },
   NULL,
};

   /* replace13_0_0_0_0_0 -> search11_0 in the cache */
/* replace13_0_0_0_0 -> replace11_0_0_0_0 in the cache */
/* replace13_0_0_0 -> replace11_0_0_0 in the cache */

/* replace13_0_0_1 -> search3_1 in the cache */
static const nir_search_expression replace13_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ushr,
   { &replace11_0_0_0.value, &search3_1.value },
   NULL,
};
static const nir_search_expression replace13_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace13_0_0.value },
   NULL,
};
static const nir_search_expression replace13 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace13_0.value },
   NULL,
};

   /* search14_0 -> search1_0 in the cache */

static const nir_search_constant search14_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x4000000000000000 /* 2.0 */ },
};
static const nir_search_expression search14 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 1,
   nir_op_fmul,
   { &search1_0.value, &search14_1.value },
   NULL,
};

   /* replace14_0 -> search1_0 in the cache */

/* replace14_1 -> search1_0 in the cache */
static const nir_search_expression replace14 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fadd,
   { &search1_0.value, &search1_0.value },
   NULL,
};

   /* search15_0 -> search11_0 in the cache */
static const nir_search_expression search15 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ufind_msb,
   { &search11_0.value },
   NULL,
};

   static const nir_search_constant replace15_0 = {
   { nir_search_value_constant, 32 },
   nir_type_int, { 0x1f /* 31 */ },
};

/* replace15_1_0_0 -> search11_0 in the cache */
static const nir_search_expression replace15_1_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2u32,
   { &search11_0.value },
   NULL,
};
static const nir_search_expression replace15_1 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_uclz,
   { &replace15_1_0.value },
   NULL,
};
static const nir_search_expression replace15 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_isub,
   { &replace15_0.value, &replace15_1.value },
   NULL,
};

   /* search16_0 -> search8_0 in the cache */
static const nir_search_expression search16 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ufind_msb,
   { &search8_0.value },
   NULL,
};

   /* replace16_0 -> replace15_0 in the cache */

/* replace16_1_0_0 -> search8_0 in the cache */
/* replace16_1_0 -> replace8_0_0 in the cache */
static const nir_search_expression replace16_1 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_uclz,
   { &replace8_0_0.value },
   NULL,
};
static const nir_search_expression replace16 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_isub,
   { &replace15_0.value, &replace16_1.value },
   NULL,
};

   /* search17_0 -> search3_0 in the cache */
static const nir_search_expression search17 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ufind_msb,
   { &search3_0.value },
   NULL,
};

   /* replace17_0 -> replace15_0 in the cache */

/* replace17_1_0 -> search3_0 in the cache */
static const nir_search_expression replace17_1 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_uclz,
   { &search3_0.value },
   NULL,
};
static const nir_search_expression replace17 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_isub,
   { &replace15_0.value, &replace17_1.value },
   NULL,
};

   /* search18_0 -> search3_0 in the cache */
static const nir_search_expression search18 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &search3_0.value },
   NULL,
};

   /* replace18_0_0 -> search3_0 in the cache */
static const nir_search_expression replace18_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &search3_0.value },
   NULL,
};
static const nir_search_expression replace18 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace18_0.value },
   NULL,
};

   static const nir_search_variable search19_0 = {
   { nir_search_value_variable, 64 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search19 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &search19_0.value },
   NULL,
};

   /* replace19_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace19_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2u32,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace19_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace19_0_0.value },
   NULL,
};
static const nir_search_expression replace19 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace19_0.value },
   NULL,
};

   /* search20_0 -> search19_0 in the cache */
static const nir_search_expression search20 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &search19_0.value },
   NULL,
};

   /* replace20_0_0 -> search19_0 in the cache */
/* replace20_0 -> replace19_0_0 in the cache */
/* replace20 -> replace19_0 in the cache */

   /* search21_0 -> search11_0 in the cache */
/* search21 -> replace15_1_0 in the cache */

   /* replace21_0_0 -> search11_0 in the cache */
/* replace21_0 -> replace11_0_0_0_0 in the cache */
/* replace21 -> replace11_0_0_0 in the cache */

   /* search22_0 -> search11_0 in the cache */
static const nir_search_expression search22 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2u64,
   { &search11_0.value },
   NULL,
};

   /* replace22_0_0_0 -> search11_0 in the cache */
/* replace22_0_0 -> replace11_0_0_0_0 in the cache */
/* replace22_0 -> replace11_0_0_0 in the cache */
static const nir_search_expression replace22 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2u64,
   { &replace11_0_0_0.value },
   NULL,
};

   /* search23_0 -> search8_0 in the cache */
static const nir_search_expression search23 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2u64,
   { &search8_0.value },
   NULL,
};

   /* replace23_0_0 -> search8_0 in the cache */
/* replace23_0 -> replace8_0_0 in the cache */
static const nir_search_expression replace23 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2u64,
   { &replace8_0_0.value },
   NULL,
};

   /* search24_0 -> search3_0 in the cache */
static const nir_search_expression search24 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &search3_0.value },
   NULL,
};

   /* replace24_0_0 -> search3_0 in the cache */
static const nir_search_expression replace24_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &search3_0.value },
   NULL,
};
static const nir_search_expression replace24 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &replace24_0.value },
   NULL,
};

   /* search25_0 -> search19_0 in the cache */
static const nir_search_expression search25 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &search19_0.value },
   NULL,
};

   /* replace25_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace25_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2i32,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace25_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &replace25_0_0.value },
   NULL,
};
static const nir_search_expression replace25 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &replace25_0.value },
   NULL,
};

   /* search26_0 -> search19_0 in the cache */
static const nir_search_expression search26 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &search19_0.value },
   NULL,
};

   /* replace26_0_0 -> search19_0 in the cache */
/* replace26_0 -> replace25_0_0 in the cache */
/* replace26 -> replace25_0 in the cache */

   /* search27_0 -> search11_0 in the cache */
static const nir_search_expression search27 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2i32,
   { &search11_0.value },
   NULL,
};

   /* replace27_0_0 -> search11_0 in the cache */
/* replace27_0 -> replace12_0_0_0_0 in the cache */
/* replace27 -> replace12_0_0_0 in the cache */

   /* search28_0 -> search11_0 in the cache */
static const nir_search_expression search28 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2i64,
   { &search11_0.value },
   NULL,
};

   /* replace28_0_0_0 -> search11_0 in the cache */
/* replace28_0_0 -> replace12_0_0_0_0 in the cache */
/* replace28_0 -> replace12_0_0_0 in the cache */
static const nir_search_expression replace28 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2i64,
   { &replace12_0_0_0.value },
   NULL,
};

   /* search29_0 -> search8_0 in the cache */
static const nir_search_expression search29 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2i64,
   { &search8_0.value },
   NULL,
};

   /* replace29_0_0 -> search8_0 in the cache */
/* replace29_0 -> replace9_0_0 in the cache */
static const nir_search_expression replace29 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2i64,
   { &replace9_0_0.value },
   NULL,
};

   /* search30_0 -> search19_0 in the cache */
static const nir_search_expression search30 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2f16,
   { &search19_0.value },
   NULL,
};

   /* replace30_0_0 -> search19_0 in the cache */
static const nir_search_expression replace30_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2f32,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace30 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2f16,
   { &replace30_0.value },
   NULL,
};

   /* search31_0 -> search8_0 in the cache */
static const nir_search_expression search31 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2f64,
   { &search8_0.value },
   NULL,
};

   /* replace31_0_0 -> search8_0 in the cache */
static const nir_search_expression replace31_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2f32,
   { &search8_0.value },
   NULL,
};
static const nir_search_expression replace31 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2f64,
   { &replace31_0.value },
   NULL,
};

   /* search32_0 -> search19_0 in the cache */
static const nir_search_expression search32 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2f16,
   { &search19_0.value },
   NULL,
};

   /* replace32_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace32_0_0 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2f64,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace32_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2f32,
   { &replace32_0_0.value },
   NULL,
};
static const nir_search_expression replace32 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2f16,
   { &replace32_0.value },
   NULL,
};

   /* search33_0 -> search11_0 in the cache */
static const nir_search_expression search33 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2f32,
   { &search11_0.value },
   NULL,
};

   /* replace33_0_0_0 -> search11_0 in the cache */
/* replace33_0_0 -> replace12_0_0_0_0 in the cache */
/* replace33_0 -> replace12_0_0_0 in the cache */
static const nir_search_expression replace33 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2f32,
   { &replace12_0_0_0.value },
   NULL,
};

   /* search34_0 -> search11_0 in the cache */
static const nir_search_expression search34 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2f64,
   { &search11_0.value },
   NULL,
};

   /* replace34_0_0_0_0 -> search11_0 in the cache */
/* replace34_0_0_0 -> replace12_0_0_0_0 in the cache */
/* replace34_0_0 -> replace12_0_0_0 in the cache */
/* replace34_0 -> replace28 in the cache */
static const nir_search_expression replace34 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2f64,
   { &replace28.value },
   NULL,
};

   /* search35_0 -> search8_0 in the cache */
static const nir_search_expression search35 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2f64,
   { &search8_0.value },
   NULL,
};

   /* replace35_0_0_0 -> search8_0 in the cache */
/* replace35_0_0 -> replace9_0_0 in the cache */
/* replace35_0 -> replace29 in the cache */
static const nir_search_expression replace35 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_i2f64,
   { &replace29.value },
   NULL,
};

   /* search36_0 -> search19_0 in the cache */
static const nir_search_expression search36 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2f16,
   { &search19_0.value },
   NULL,
};

   /* replace36_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace36_0_0 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2f64,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace36_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2f32,
   { &replace36_0_0.value },
   NULL,
};
static const nir_search_expression replace36 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2f16,
   { &replace36_0.value },
   NULL,
};

   /* search37_0 -> search11_0 in the cache */
static const nir_search_expression search37 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2f32,
   { &search11_0.value },
   NULL,
};

   /* replace37_0_0_0 -> search11_0 in the cache */
/* replace37_0_0 -> replace11_0_0_0_0 in the cache */
/* replace37_0 -> replace11_0_0_0 in the cache */
static const nir_search_expression replace37 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2f32,
   { &replace11_0_0_0.value },
   NULL,
};

   /* search38_0 -> search11_0 in the cache */
static const nir_search_expression search38 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2f64,
   { &search11_0.value },
   NULL,
};

   /* replace38_0_0_0_0 -> search11_0 in the cache */
/* replace38_0_0_0 -> replace11_0_0_0_0 in the cache */
/* replace38_0_0 -> replace11_0_0_0 in the cache */
/* replace38_0 -> replace22 in the cache */
static const nir_search_expression replace38 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2f64,
   { &replace22.value },
   NULL,
};

   /* search39_0 -> search8_0 in the cache */
static const nir_search_expression search39 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2f64,
   { &search8_0.value },
   NULL,
};

   /* replace39_0_0_0 -> search8_0 in the cache */
/* replace39_0_0 -> replace8_0_0 in the cache */
/* replace39_0 -> replace23 in the cache */
static const nir_search_expression replace39 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_u2f64,
   { &replace23.value },
   NULL,
};

   /* search40_0 -> search3_0 in the cache */
static const nir_search_expression search40 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_f2i8,
   { &search3_0.value },
   NULL,
};

   /* replace40_0_0_0 -> search3_0 in the cache */
static const nir_search_expression replace40_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2i32,
   { &search3_0.value },
   NULL,
};
static const nir_search_expression replace40_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &replace40_0_0.value },
   NULL,
};
static const nir_search_expression replace40 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &replace40_0.value },
   NULL,
};

   /* search41_0 -> search19_0 in the cache */
static const nir_search_expression search41 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_f2i8,
   { &search19_0.value },
   NULL,
};

   /* replace41_0_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace41_0_0_0 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2i64,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace41_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_i2i32,
   { &replace41_0_0_0.value },
   NULL,
};
static const nir_search_expression replace41_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_i2i16,
   { &replace41_0_0.value },
   NULL,
};
static const nir_search_expression replace41 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_i2i8,
   { &replace41_0.value },
   NULL,
};

   /* search42_0 -> search19_0 in the cache */
static const nir_search_expression search42 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2i16,
   { &search19_0.value },
   NULL,
};

   /* replace42_0_0_0 -> search19_0 in the cache */
/* replace42_0_0 -> replace41_0_0_0 in the cache */
/* replace42_0 -> replace41_0_0 in the cache */
/* replace42 -> replace41_0 in the cache */

   /* search43_0 -> search8_0 in the cache */
static const nir_search_expression search43 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2i64,
   { &search8_0.value },
   NULL,
};

   /* replace43_0_0_0 -> search8_0 in the cache */
/* replace43_0_0 -> replace31_0 in the cache */
/* replace43_0 -> replace31 in the cache */
static const nir_search_expression replace43 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2i64,
   { &replace31.value },
   NULL,
};

   /* search44_0 -> search3_0 in the cache */
static const nir_search_expression search44 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_f2u8,
   { &search3_0.value },
   NULL,
};

   /* replace44_0_0_0 -> search3_0 in the cache */
static const nir_search_expression replace44_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_f2u32,
   { &search3_0.value },
   NULL,
};
static const nir_search_expression replace44_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace44_0_0.value },
   NULL,
};
static const nir_search_expression replace44 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace44_0.value },
   NULL,
};

   /* search45_0 -> search19_0 in the cache */
static const nir_search_expression search45 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_f2u8,
   { &search19_0.value },
   NULL,
};

   /* replace45_0_0_0_0 -> search19_0 in the cache */
static const nir_search_expression replace45_0_0_0 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2u64,
   { &search19_0.value },
   NULL,
};
static const nir_search_expression replace45_0_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_u2u32,
   { &replace45_0_0_0.value },
   NULL,
};
static const nir_search_expression replace45_0 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_u2u16,
   { &replace45_0_0.value },
   NULL,
};
static const nir_search_expression replace45 = {
   { nir_search_value_expression, 8 },
   false, false,
   -1, 0,
   nir_op_u2u8,
   { &replace45_0.value },
   NULL,
};

   /* search46_0 -> search19_0 in the cache */
static const nir_search_expression search46 = {
   { nir_search_value_expression, 16 },
   false, false,
   -1, 0,
   nir_op_f2u16,
   { &search19_0.value },
   NULL,
};

   /* replace46_0_0_0 -> search19_0 in the cache */
/* replace46_0_0 -> replace45_0_0_0 in the cache */
/* replace46_0 -> replace45_0_0 in the cache */
/* replace46 -> replace45_0 in the cache */

   /* search47_0 -> search8_0 in the cache */
static const nir_search_expression search47 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2u64,
   { &search8_0.value },
   NULL,
};

   /* replace47_0_0_0 -> search8_0 in the cache */
/* replace47_0_0 -> replace31_0 in the cache */
/* replace47_0 -> replace31 in the cache */
static const nir_search_expression replace47 = {
   { nir_search_value_expression, 64 },
   false, false,
   -1, 0,
   nir_op_f2u64,
   { &replace31.value },
   NULL,
};

   /* search48_0 -> search2_0 in the cache */

static const nir_search_variable search48_1 = {
   { nir_search_value_variable, -2 },
   1, /* b */
   true,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search48 = {
   { nir_search_value_expression, 1 },
   false, false,
   -1, 0,
   nir_op_fge,
   { &search2_0.value, &search48_1.value },
   NULL,
};

   /* replace48_0_0 -> search2_0 in the cache */

/* replace48_0_1 -> search2_1 in the cache */
static const nir_search_expression replace48_0 = {
   { nir_search_value_expression, 1 },
   false, false,
   -1, 0,
   nir_op_flt,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace48 = {
   { nir_search_value_expression, 1 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace48_0.value },
   NULL,
};

   /* search49_0 -> search2_0 in the cache */

/* search49_1 -> search48_1 in the cache */
static const nir_search_expression search49 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_fge32,
   { &search2_0.value, &search48_1.value },
   NULL,
};

   /* replace49_0_0 -> search2_0 in the cache */

/* replace49_0_1 -> search2_1 in the cache */
static const nir_search_expression replace49_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_flt32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace49 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace49_0.value },
   NULL,
};

   /* search50_0 -> search2_0 in the cache */

/* search50_1 -> search48_1 in the cache */
static const nir_search_expression search50 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ige32,
   { &search2_0.value, &search48_1.value },
   NULL,
};

   /* replace50_0_0 -> search2_0 in the cache */

/* replace50_0_1 -> search2_1 in the cache */
static const nir_search_expression replace50_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ilt32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace50 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace50_0.value },
   NULL,
};

   /* search51_0 -> search2_0 in the cache */

/* search51_1 -> search48_1 in the cache */
static const nir_search_expression search51 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_uge32,
   { &search2_0.value, &search48_1.value },
   NULL,
};

   /* replace51_0_0 -> search2_0 in the cache */

/* replace51_0_1 -> search2_1 in the cache */
static const nir_search_expression replace51_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ult32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace51 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace51_0.value },
   NULL,
};

   static const nir_search_variable search52_0 = {
   { nir_search_value_variable, -2 },
   0, /* a */
   true,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

/* search52_1 -> search2_1 in the cache */
static const nir_search_expression search52 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_flt32,
   { &search52_0.value, &search2_1.value },
   NULL,
};

   /* replace52_0_0 -> search2_0 in the cache */

/* replace52_0_1 -> search2_1 in the cache */
static const nir_search_expression replace52_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_fge32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace52 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace52_0.value },
   NULL,
};

   /* search53_0 -> search52_0 in the cache */

/* search53_1 -> search2_1 in the cache */
static const nir_search_expression search53 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ilt32,
   { &search52_0.value, &search2_1.value },
   NULL,
};

   /* replace53_0_0 -> search2_0 in the cache */

/* replace53_0_1 -> search2_1 in the cache */
static const nir_search_expression replace53_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ige32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace53 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace53_0.value },
   NULL,
};

   /* search54_0 -> search52_0 in the cache */

/* search54_1 -> search2_1 in the cache */
static const nir_search_expression search54 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_ult32,
   { &search52_0.value, &search2_1.value },
   NULL,
};

   /* replace54_0_0 -> search2_0 in the cache */

/* replace54_0_1 -> search2_1 in the cache */
static const nir_search_expression replace54_0 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_uge32,
   { &search2_0.value, &search2_1.value },
   NULL,
};
static const nir_search_expression replace54 = {
   { nir_search_value_expression, 32 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &replace54_0.value },
   NULL,
};


static const struct transform midgard_nir_lower_algebraic_late_state2_xforms[] = {
  { &search1, &replace1.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state3_xforms[] = {
  { &search2, &replace2.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state4_xforms[] = {
  { &search3, &replace3.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state5_xforms[] = {
  { &search4, &replace4.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state6_xforms[] = {
  { &search3, &replace3.value, 0 },
  { &search4, &replace4.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state8_xforms[] = {
  { &search7, &replace7.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state9_xforms[] = {
  { &search8, &replace8.value, 0 },
  { &search11, &replace11.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state10_xforms[] = {
  { &search9, &replace9.value, 0 },
  { &search12, &replace12.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state11_xforms[] = {
  { &search10, &replace10.value, 0 },
  { &search13, &replace13.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state12_xforms[] = {
  { &search14, &replace14.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state13_xforms[] = {
  { &search15, &replace15.value, 0 },
  { &search16, &replace16.value, 0 },
  { &search17, &replace17.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state14_xforms[] = {
  { &search18, &replace18.value, 0 },
  { &search19, &replace19.value, 0 },
  { &search20, &replace19_0.value, 0 },
  { &replace15_1_0, &replace11_0_0_0.value, 0 },
  { &search22, &replace22.value, 0 },
  { &search23, &replace23.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state15_xforms[] = {
  { &search24, &replace24.value, 0 },
  { &search25, &replace25.value, 0 },
  { &search26, &replace25_0.value, 0 },
  { &search27, &replace12_0_0_0.value, 0 },
  { &search28, &replace28.value, 0 },
  { &search29, &replace29.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state16_xforms[] = {
  { &search30, &replace30.value, 0 },
  { &search31, &replace31.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state17_xforms[] = {
  { &search32, &replace32.value, 0 },
  { &search33, &replace33.value, 0 },
  { &search34, &replace34.value, 0 },
  { &search35, &replace35.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state18_xforms[] = {
  { &search36, &replace36.value, 0 },
  { &search37, &replace37.value, 0 },
  { &search38, &replace38.value, 0 },
  { &search39, &replace39.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state19_xforms[] = {
  { &search40, &replace40.value, 0 },
  { &search41, &replace41.value, 0 },
  { &search42, &replace41_0.value, 0 },
  { &search43, &replace43.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state20_xforms[] = {
  { &search44, &replace44.value, 0 },
  { &search45, &replace45.value, 0 },
  { &search46, &replace45_0.value, 0 },
  { &search47, &replace47.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state21_xforms[] = {
  { &search48, &replace48.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state22_xforms[] = {
  { &search49, &replace49.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state23_xforms[] = {
  { &search50, &replace50.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state24_xforms[] = {
  { &search51, &replace51.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state25_xforms[] = {
  { &search52, &replace52.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state26_xforms[] = {
  { &search53, &replace53.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state27_xforms[] = {
  { &search54, &replace54.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state28_xforms[] = {
  { &search6, &replace5.value, 0 },
  { &search7, &replace7.value, 0 },
};
static const struct transform midgard_nir_lower_algebraic_late_state29_xforms[] = {
  { &search5, &replace5.value, 0 },
};

static const struct per_op_table midgard_nir_lower_algebraic_late_table[nir_num_search_ops] = {
   [nir_op_ineg] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         2,
      },
   },
   [nir_op_fsub] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         3,
      },
   },
   [nir_op_b32csel] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         4,
         5,
         6,
         0,
         4,
         5,
         6,
      },
   },
   [nir_op_fmin] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         2,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         2,
         0,
      },
      
      .num_filtered_states = 3,
      .table = (uint16_t []) {
      
         0,
         7,
         0,
         7,
         7,
         29,
         0,
         29,
         0,
      },
   },
   [nir_op_fmax] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         2,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         2,
      },
      
      .num_filtered_states = 3,
      .table = (uint16_t []) {
      
         0,
         8,
         0,
         8,
         8,
         28,
         0,
         28,
         0,
      },
   },
   [nir_op_ishl] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         9,
      },
   },
   [nir_op_ishr] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         10,
      },
   },
   [nir_op_ushr] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         11,
      },
   },
   [nir_op_fmul] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         12,
         12,
         12,
      },
   },
   [nir_op_ufind_msb] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         13,
      },
   },
   [nir_search_op_u2u] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         14,
      },
   },
   [nir_search_op_i2i] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         15,
      },
   },
   [nir_search_op_f2f] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         16,
      },
   },
   [nir_search_op_i2f] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         17,
      },
   },
   [nir_search_op_u2f] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         18,
      },
   },
   [nir_search_op_f2i] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         19,
      },
   },
   [nir_search_op_f2u] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         20,
      },
   },
   [nir_op_fge] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         21,
         0,
         21,
      },
   },
   [nir_op_fge32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         22,
         0,
         22,
      },
   },
   [nir_op_ige32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         23,
         0,
         23,
      },
   },
   [nir_op_uge32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         24,
         0,
         24,
      },
   },
   [nir_op_flt32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         0,
         25,
         25,
      },
   },
   [nir_op_ilt32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         0,
         26,
         26,
      },
   },
   [nir_op_ult32] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         0,
         27,
         27,
      },
   },
};

const struct transform *midgard_nir_lower_algebraic_late_transforms[] = {
   NULL,
   NULL,
   midgard_nir_lower_algebraic_late_state2_xforms,
   midgard_nir_lower_algebraic_late_state3_xforms,
   midgard_nir_lower_algebraic_late_state4_xforms,
   midgard_nir_lower_algebraic_late_state5_xforms,
   midgard_nir_lower_algebraic_late_state6_xforms,
   NULL,
   midgard_nir_lower_algebraic_late_state8_xforms,
   midgard_nir_lower_algebraic_late_state9_xforms,
   midgard_nir_lower_algebraic_late_state10_xforms,
   midgard_nir_lower_algebraic_late_state11_xforms,
   midgard_nir_lower_algebraic_late_state12_xforms,
   midgard_nir_lower_algebraic_late_state13_xforms,
   midgard_nir_lower_algebraic_late_state14_xforms,
   midgard_nir_lower_algebraic_late_state15_xforms,
   midgard_nir_lower_algebraic_late_state16_xforms,
   midgard_nir_lower_algebraic_late_state17_xforms,
   midgard_nir_lower_algebraic_late_state18_xforms,
   midgard_nir_lower_algebraic_late_state19_xforms,
   midgard_nir_lower_algebraic_late_state20_xforms,
   midgard_nir_lower_algebraic_late_state21_xforms,
   midgard_nir_lower_algebraic_late_state22_xforms,
   midgard_nir_lower_algebraic_late_state23_xforms,
   midgard_nir_lower_algebraic_late_state24_xforms,
   midgard_nir_lower_algebraic_late_state25_xforms,
   midgard_nir_lower_algebraic_late_state26_xforms,
   midgard_nir_lower_algebraic_late_state27_xforms,
   midgard_nir_lower_algebraic_late_state28_xforms,
   midgard_nir_lower_algebraic_late_state29_xforms,
};

const uint16_t midgard_nir_lower_algebraic_late_transform_counts[] = {
   0,
   0,
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state2_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state3_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state4_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state5_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state6_xforms),
   0,
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state8_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state9_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state10_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state11_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state12_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state13_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state14_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state15_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state16_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state17_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state18_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state19_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state20_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state21_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state22_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state23_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state24_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state25_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state26_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state27_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state28_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_lower_algebraic_late_state29_xforms),
};

bool
midgard_nir_lower_algebraic_late(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[1];
   const nir_shader_compiler_options *options = shader->options;
   const shader_info *info = &shader->info;
   (void) options;
   (void) info;

   condition_flags[0] = true;

   nir_foreach_function(function, shader) {
      if (function->impl) {
         progress |= nir_algebraic_impl(function->impl, condition_flags,
                                        midgard_nir_lower_algebraic_late_transforms,
                                        midgard_nir_lower_algebraic_late_transform_counts,
                                        midgard_nir_lower_algebraic_late_table);
      }
   }

   return progress;
}


#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 2
 * transforms:
 *    ('fsin', 'a') => ('fsin', ('fdiv', 'a', 3.141592653589793))
 *    ('fcos', 'a') => ('fcos', ('fdiv', 'a', 3.141592653589793))
 */


   static const nir_search_variable search55_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search55 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsin,
   { &search55_0.value },
   NULL,
};

   /* replace55_0_0 -> search55_0 in the cache */

static const nir_search_constant replace55_0_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x400921fb54442d18 /* 3.141592653589793 */ },
};
static const nir_search_expression replace55_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fdiv,
   { &search55_0.value, &replace55_0_1.value },
   NULL,
};
static const nir_search_expression replace55 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsin,
   { &replace55_0.value },
   NULL,
};

   /* search56_0 -> search55_0 in the cache */
static const nir_search_expression search56 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fcos,
   { &search55_0.value },
   NULL,
};

   /* replace56_0_0 -> search55_0 in the cache */

/* replace56_0_1 -> replace55_0_1 in the cache */
/* replace56_0 -> replace55_0 in the cache */
static const nir_search_expression replace56 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fcos,
   { &replace55_0.value },
   NULL,
};


static const struct transform midgard_nir_scale_trig_state2_xforms[] = {
  { &search55, &replace55.value, 0 },
};
static const struct transform midgard_nir_scale_trig_state3_xforms[] = {
  { &search56, &replace56.value, 0 },
};

static const struct per_op_table midgard_nir_scale_trig_table[nir_num_search_ops] = {
   [nir_op_fsin] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         2,
      },
   },
   [nir_op_fcos] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         3,
      },
   },
};

const struct transform *midgard_nir_scale_trig_transforms[] = {
   NULL,
   NULL,
   midgard_nir_scale_trig_state2_xforms,
   midgard_nir_scale_trig_state3_xforms,
};

const uint16_t midgard_nir_scale_trig_transform_counts[] = {
   0,
   0,
   (uint16_t)ARRAY_SIZE(midgard_nir_scale_trig_state2_xforms),
   (uint16_t)ARRAY_SIZE(midgard_nir_scale_trig_state3_xforms),
};

bool
midgard_nir_scale_trig(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[1];
   const nir_shader_compiler_options *options = shader->options;
   const shader_info *info = &shader->info;
   (void) options;
   (void) info;

   condition_flags[0] = true;

   nir_foreach_function(function, shader) {
      if (function->impl) {
         progress |= nir_algebraic_impl(function->impl, condition_flags,
                                        midgard_nir_scale_trig_transforms,
                                        midgard_nir_scale_trig_transform_counts,
                                        midgard_nir_scale_trig_table);
      }
   }

   return progress;
}


#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 1
 * transforms:
 *    ('inot', ('inot', 'a')) => a
 */


   static const nir_search_variable search57_0_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search57_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &search57_0_0.value },
   NULL,
};
static const nir_search_expression search57 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_inot,
   { &search57_0.value },
   NULL,
};

   /* replace57 -> search57_0_0 in the cache */


static const struct transform midgard_nir_cancel_inot_state3_xforms[] = {
  { &search57, &search57_0_0.value, 0 },
};

static const struct per_op_table midgard_nir_cancel_inot_table[nir_num_search_ops] = {
   [nir_op_inot] = {
      .filter = (uint16_t []) {
         0,
         0,
         1,
         1,
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         2,
         3,
      },
   },
};

const struct transform *midgard_nir_cancel_inot_transforms[] = {
   NULL,
   NULL,
   NULL,
   midgard_nir_cancel_inot_state3_xforms,
};

const uint16_t midgard_nir_cancel_inot_transform_counts[] = {
   0,
   0,
   0,
   (uint16_t)ARRAY_SIZE(midgard_nir_cancel_inot_state3_xforms),
};

bool
midgard_nir_cancel_inot(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[1];
   const nir_shader_compiler_options *options = shader->options;
   const shader_info *info = &shader->info;
   (void) options;
   (void) info;

   condition_flags[0] = true;

   nir_foreach_function(function, shader) {
      if (function->impl) {
         progress |= nir_algebraic_impl(function->impl, condition_flags,
                                        midgard_nir_cancel_inot_transforms,
                                        midgard_nir_cancel_inot_transform_counts,
                                        midgard_nir_cancel_inot_table);
      }
   }

   return progress;
}

