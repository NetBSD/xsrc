#include "ir/lima_ir.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_search.h"
#include "nir_search_helpers.h"

/* What follows is NIR algebraic transform code for the following 2
 * transforms:
 *    ('fsin', 'a') => ('fsin', ('fmul', 'a', 0.15915494309189535))
 *    ('fcos', 'a') => ('fcos', ('fmul', 'a', 0.15915494309189535))
 */


   static const nir_search_variable search0_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsin,
   { &search0_0.value },
   NULL,
};

   /* replace0_0_0 -> search0_0 in the cache */

static const nir_search_constant replace0_0_1 = {
   { nir_search_value_constant, -1 },
   nir_type_float, { 0x3fc45f306dc9c883 /* 0.15915494309189535 */ },
};
static const nir_search_expression replace0_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 1,
   nir_op_fmul,
   { &search0_0.value, &replace0_0_1.value },
   NULL,
};
static const nir_search_expression replace0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 1,
   nir_op_fsin,
   { &replace0_0.value },
   NULL,
};

   /* search1_0 -> search0_0 in the cache */
static const nir_search_expression search1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fcos,
   { &search0_0.value },
   NULL,
};

   /* replace1_0_0 -> search0_0 in the cache */

/* replace1_0_1 -> replace0_0_1 in the cache */
/* replace1_0 -> replace0_0 in the cache */
static const nir_search_expression replace1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 1,
   nir_op_fcos,
   { &replace0_0.value },
   NULL,
};


static const struct transform lima_nir_scale_trig_state2_xforms[] = {
  { &search0, &replace0.value, 0 },
};
static const struct transform lima_nir_scale_trig_state3_xforms[] = {
  { &search1, &replace1.value, 0 },
};

static const struct per_op_table lima_nir_scale_trig_table[nir_num_search_ops] = {
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

const struct transform *lima_nir_scale_trig_transforms[] = {
   NULL,
   NULL,
   lima_nir_scale_trig_state2_xforms,
   lima_nir_scale_trig_state3_xforms,
};

const uint16_t lima_nir_scale_trig_transform_counts[] = {
   0,
   0,
   (uint16_t)ARRAY_SIZE(lima_nir_scale_trig_state2_xforms),
   (uint16_t)ARRAY_SIZE(lima_nir_scale_trig_state3_xforms),
};

bool
lima_nir_scale_trig(nir_shader *shader)
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
                                        lima_nir_scale_trig_transforms,
                                        lima_nir_scale_trig_transform_counts,
                                        lima_nir_scale_trig_table);
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
 *    ('ftrunc', 'a') => ('fmul', ('fsign', 'a'), ('ffloor', ('fmax', 'a', ('fneg', 'a'))))
 */


   static const nir_search_variable search2_0 = {
   { nir_search_value_variable, -1 },
   0, /* a */
   false,
   nir_type_invalid,
   NULL,
   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
};
static const nir_search_expression search2 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_ftrunc,
   { &search2_0.value },
   NULL,
};

   /* replace2_0_0 -> search2_0 in the cache */
static const nir_search_expression replace2_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fsign,
   { &search2_0.value },
   NULL,
};

/* replace2_1_0_0 -> search2_0 in the cache */

/* replace2_1_0_1_0 -> search2_0 in the cache */
static const nir_search_expression replace2_1_0_1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 0,
   nir_op_fneg,
   { &search2_0.value },
   NULL,
};
static const nir_search_expression replace2_1_0 = {
   { nir_search_value_expression, -1 },
   false, false,
   1, 1,
   nir_op_fmax,
   { &search2_0.value, &replace2_1_0_1.value },
   NULL,
};
static const nir_search_expression replace2_1 = {
   { nir_search_value_expression, -1 },
   false, false,
   -1, 1,
   nir_op_ffloor,
   { &replace2_1_0.value },
   NULL,
};
static const nir_search_expression replace2 = {
   { nir_search_value_expression, -1 },
   false, false,
   0, 2,
   nir_op_fmul,
   { &replace2_0.value, &replace2_1.value },
   NULL,
};


static const struct transform lima_nir_lower_ftrunc_state2_xforms[] = {
  { &search2, &replace2.value, 0 },
};

static const struct per_op_table lima_nir_lower_ftrunc_table[nir_num_search_ops] = {
   [nir_op_ftrunc] = {
      .filter = (uint16_t []) {
         0,
         0,
         0,
      },
      
      .num_filtered_states = 1,
      .table = (uint16_t []) {
      
         2,
      },
   },
};

const struct transform *lima_nir_lower_ftrunc_transforms[] = {
   NULL,
   NULL,
   lima_nir_lower_ftrunc_state2_xforms,
};

const uint16_t lima_nir_lower_ftrunc_transform_counts[] = {
   0,
   0,
   (uint16_t)ARRAY_SIZE(lima_nir_lower_ftrunc_state2_xforms),
};

bool
lima_nir_lower_ftrunc(nir_shader *shader)
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
                                        lima_nir_lower_ftrunc_transforms,
                                        lima_nir_lower_ftrunc_transform_counts,
                                        lima_nir_lower_ftrunc_table);
      }
   }

   return progress;
}

