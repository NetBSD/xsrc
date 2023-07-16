#include "bifrost_nir.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 6
 * transforms:
 *    ('fmul', 'a', 2.0) => ('fadd', 'a', 'a')
 *    ('fmin', ('fmax', 'a', -1.0), 1.0) => ('fsat_signed_mali', 'a')
 *    ('fmax', ('fmin', 'a', 1.0), -1.0) => ('fsat_signed_mali', 'a')
 *    ('fmax', 'a', 0.0) => ('fclamp_pos_mali', 'a')
 *    ('fabs', ('fddx', 'a')) => ('fabs', ('fddx_must_abs_mali', 'a'))
 *    ('fabs', ('fddy', 'b')) => ('fabs', ('fddy_must_abs_mali', 'b'))
 */


   static const nir_search_variable search0_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};

static const nir_search_constant search0_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x4000000000000000 /* 2.0 */ },
};
static const nir_search_expression search0 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 1,
   nir_op_fmul,
   { &search0_0.value, &search0_1.value },
   NULL,
};

   /* replace0_0 -> search0_0 in the cache */

/* replace0_1 -> search0_0 in the cache */
static const nir_search_expression replace0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fadd,
   { &search0_0.value, &search0_0.value },
   NULL,
};

   /* search1_0_0 -> search0_0 in the cache */

static const nir_search_constant search1_0_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0xbff0000000000000 /* -1.0 */ },
};
static const nir_search_expression search1_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   1, 1,
   nir_op_fmax,
   { &search0_0.value, &search1_0_1.value },
   NULL,
};

static const nir_search_constant search1_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x3ff0000000000000 /* 1.0 */ },
};
static const nir_search_expression search1 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 2,
   nir_op_fmin,
   { &search1_0.value, &search1_1.value },
   NULL,
};

   /* replace1_0 -> search0_0 in the cache */
static const nir_search_expression replace1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsat_signed_mali,
   { &search0_0.value },
   NULL,
};

   /* search2_0_0 -> search0_0 in the cache */

/* search2_0_1 -> search1_1 in the cache */
static const nir_search_expression search2_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   1, 1,
   nir_op_fmin,
   { &search0_0.value, &search1_1.value },
   NULL,
};

/* search2_1 -> search1_0_1 in the cache */
static const nir_search_expression search2 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 2,
   nir_op_fmax,
   { &search2_0.value, &search1_0_1.value },
   NULL,
};

   /* replace2_0 -> search0_0 in the cache */
/* replace2 -> replace1 in the cache */

   /* search3_0 -> search0_0 in the cache */

static const nir_search_constant search3_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x0 /* 0.0 */ },
};
static const nir_search_expression search3 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 1,
   nir_op_fmax,
   { &search0_0.value, &search3_1.value },
   NULL,
};

   /* replace3_0 -> search0_0 in the cache */
static const nir_search_expression replace3 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fclamp_pos_mali,
   { &search0_0.value },
   NULL,
};

   /* search4_0_0 -> search0_0 in the cache */
static const nir_search_expression search4_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fddx,
   { &search0_0.value },
   NULL,
};
static const nir_search_expression search4 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fabs,
   { &search4_0.value },
   NULL,
};

   /* replace4_0_0 -> search0_0 in the cache */
static const nir_search_expression replace4_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fddx_must_abs_mali,
   { &search0_0.value },
   NULL,
};
static const nir_search_expression replace4 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fabs,
   { &replace4_0.value },
   NULL,
};

   static const nir_search_variable search5_0_0 = {
   { nir_search_value_variable, -1 },
   0, /* b */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search5_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fddy,
   { &search5_0_0.value },
   NULL,
};
static const nir_search_expression search5 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fabs,
   { &search5_0.value },
   NULL,
};

   /* replace5_0_0 -> search5_0_0 in the cache */
static const nir_search_expression replace5_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fddy_must_abs_mali,
   { &search5_0_0.value },
   NULL,
};
static const nir_search_expression replace5 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fabs,
   { &replace5_0.value },
   NULL,
};


static const struct transform bifrost_nir_lower_algebraic_late_state2_xforms[] = {
  { &search0, &replace0.value, 0 },
};
static const struct transform bifrost_nir_lower_algebraic_late_state4_xforms[] = {
  { &search3, &replace3.value, 0 },
};
static const struct transform bifrost_nir_lower_algebraic_late_state7_xforms[] = {
  { &search2, &replace1.value, 0 },
  { &search3, &replace3.value, 0 },
};
static const struct transform bifrost_nir_lower_algebraic_late_state8_xforms[] = {
  { &search1, &replace1.value, 0 },
};
static const struct transform bifrost_nir_lower_algebraic_late_state9_xforms[] = {
  { &search4, &replace4.value, 0 },
};
static const struct transform bifrost_nir_lower_algebraic_late_state10_xforms[] = {
  { &search5, &replace5.value, 0 },
};

static const struct per_op_table bifrost_nir_lower_algebraic_late_table[nir_num_search_ops] = {
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
      },
      
      .num_filtered_states = 2,
      .table = (uint16_t []) {
      
         0,
         2,
         2,
         2,
      },
   },
   [nir_op_fmin] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         0,
         2,
         0,
         0,
         2,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 3,
      .table = (uint16_t []) {
      
         0,
         3,
         0,
         3,
         3,
         8,
         0,
         8,
         0,
      },
   },
   [nir_op_fmax] = {
      .filter = (uint16_t []) {
         0,
         1,
         0,
         2,
         0,
         0,
         0,
         0,
         2,
         0,
         0,
      },
      
      .num_filtered_states = 3,
      .table = (uint16_t []) {
      
         0,
         4,
         0,
         4,
         4,
         7,
         0,
         7,
         0,
      },
   },
   [nir_op_fabs] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
         0,
         0,
         1,
         2,
         0,
         0,
         0,
         0,
      },
      
      .num_filtered_states = 3,
      .table = (uint16_t []) {
      
         0,
         9,
         10,
      },
   },
   [nir_op_fddx] = {
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
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         5,
      },
   },
   [nir_op_fddy] = {
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
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         6,
      },
   },
};

const struct transform *bifrost_nir_lower_algebraic_late_transforms[] = {
   NULL,
   NULL,
   bifrost_nir_lower_algebraic_late_state2_xforms,
   NULL,
   bifrost_nir_lower_algebraic_late_state4_xforms,
   NULL,
   NULL,
   bifrost_nir_lower_algebraic_late_state7_xforms,
   bifrost_nir_lower_algebraic_late_state8_xforms,
   bifrost_nir_lower_algebraic_late_state9_xforms,
   bifrost_nir_lower_algebraic_late_state10_xforms,
};

const uint16_t bifrost_nir_lower_algebraic_late_transform_counts[] = {
   0,
   0,
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state2_xforms),
   0,
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state4_xforms),
   0,
   0,
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state7_xforms),
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state8_xforms),
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state9_xforms),
   (uint16_t)ARRAY_SIZE(bifrost_nir_lower_algebraic_late_state10_xforms),
};

bool
bifrost_nir_lower_algebraic_late(nir_shader *shader)
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
                                        bifrost_nir_lower_algebraic_late_transforms,
                                        bifrost_nir_lower_algebraic_late_transform_counts,
                                        bifrost_nir_lower_algebraic_late_table);
      }
   }

   return progress;
}

