/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "nir_to_dxil.h"

#include "dxil_container.h"
#include "dxil_dump.h"
#include "dxil_enums.h"
#include "dxil_function.h"
#include "dxil_module.h"
#include "dxil_nir.h"
#include "dxil_signature.h"

#include "nir/nir_builder.h"
#include "util/u_debug.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"

#include "git_sha1.h"

#include "vulkan/vulkan_core.h"

#include <stdint.h>

int debug_dxil = 0;

static const struct debug_named_value
dxil_debug_options[] = {
   { "verbose", DXIL_DEBUG_VERBOSE, NULL },
   { "dump_blob",  DXIL_DEBUG_DUMP_BLOB , "Write shader blobs" },
   { "trace",  DXIL_DEBUG_TRACE , "Trace instruction conversion" },
   { "dump_module", DXIL_DEBUG_DUMP_MODULE, "dump module tree to stderr"},
   DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(debug_dxil, "DXIL_DEBUG", dxil_debug_options, 0)

#define NIR_INSTR_UNSUPPORTED(instr) \
   if (debug_dxil & DXIL_DEBUG_VERBOSE) \
   do { \
      fprintf(stderr, "Unsupported instruction:"); \
      nir_print_instr(instr, stderr); \
      fprintf(stderr, "\n"); \
   } while (0)

#define TRACE_CONVERSION(instr) \
   if (debug_dxil & DXIL_DEBUG_TRACE) \
      do { \
         fprintf(stderr, "Convert '"); \
         nir_print_instr(instr, stderr); \
         fprintf(stderr, "'\n"); \
      } while (0)

static const nir_shader_compiler_options
nir_options = {
   .lower_ineg = true,
   .lower_fneg = true,
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_isign = true,
   .lower_fsign = true,
   .lower_iabs = true,
   .lower_fmod = true,
   .lower_fpow = true,
   .lower_scmp = true,
   .lower_ldexp = true,
   .lower_flrp16 = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_bitfield_extract_to_shifts = true,
   .lower_extract_word = true,
   .lower_extract_byte = true,
   .lower_insert_word = true,
   .lower_insert_byte = true,
   .lower_all_io_to_elements = true,
   .lower_all_io_to_temps = true,
   .lower_hadd = true,
   .lower_uadd_sat = true,
   .lower_iadd_sat = true,
   .lower_uadd_carry = true,
   .lower_mul_high = true,
   .lower_rotate = true,
   .lower_pack_64_2x32_split = true,
   .lower_pack_32_2x16_split = true,
   .lower_unpack_64_2x32_split = true,
   .lower_unpack_32_2x16_split = true,
   .has_fsub = true,
   .has_isub = true,
   .use_scoped_barrier = true,
   .vertex_id_zero_based = true,
   .lower_base_vertex = true,
   .has_cs_global_id = true,
   .has_txs = true,
};

const nir_shader_compiler_options*
dxil_get_nir_compiler_options(void)
{
   return &nir_options;
}

static bool
emit_llvm_ident(struct dxil_module *m)
{
   const struct dxil_mdnode *compiler = dxil_get_metadata_string(m, "Mesa version " PACKAGE_VERSION MESA_GIT_SHA1);
   if (!compiler)
      return false;

   const struct dxil_mdnode *llvm_ident = dxil_get_metadata_node(m, &compiler, 1);
   return llvm_ident &&
          dxil_add_metadata_named_node(m, "llvm.ident", &llvm_ident, 1);
}

static bool
emit_named_version(struct dxil_module *m, const char *name,
                   int major, int minor)
{
   const struct dxil_mdnode *major_node = dxil_get_metadata_int32(m, major);
   const struct dxil_mdnode *minor_node = dxil_get_metadata_int32(m, minor);
   const struct dxil_mdnode *version_nodes[] = { major_node, minor_node };
   const struct dxil_mdnode *version = dxil_get_metadata_node(m, version_nodes,
                                                     ARRAY_SIZE(version_nodes));
   return dxil_add_metadata_named_node(m, name, &version, 1);
}

static const char *
get_shader_kind_str(enum dxil_shader_kind kind)
{
   switch (kind) {
   case DXIL_PIXEL_SHADER:
      return "ps";
   case DXIL_VERTEX_SHADER:
      return "vs";
   case DXIL_GEOMETRY_SHADER:
      return "gs";
   case DXIL_HULL_SHADER:
      return "hs";
   case DXIL_DOMAIN_SHADER:
      return "ds";
   case DXIL_COMPUTE_SHADER:
      return "cs";
   default:
      unreachable("invalid shader kind");
   }
}

static bool
emit_dx_shader_model(struct dxil_module *m)
{
   const struct dxil_mdnode *type_node = dxil_get_metadata_string(m, get_shader_kind_str(m->shader_kind));
   const struct dxil_mdnode *major_node = dxil_get_metadata_int32(m, m->major_version);
   const struct dxil_mdnode *minor_node = dxil_get_metadata_int32(m, m->minor_version);
   const struct dxil_mdnode *shader_model[] = { type_node, major_node,
                                                minor_node };
   const struct dxil_mdnode *dx_shader_model = dxil_get_metadata_node(m, shader_model, ARRAY_SIZE(shader_model));

   return dxil_add_metadata_named_node(m, "dx.shaderModel",
                                       &dx_shader_model, 1);
}

enum {
   DXIL_TYPED_BUFFER_ELEMENT_TYPE_TAG = 0,
   DXIL_STRUCTURED_BUFFER_ELEMENT_STRIDE_TAG = 1
};

enum dxil_intr {
   DXIL_INTR_LOAD_INPUT = 4,
   DXIL_INTR_STORE_OUTPUT = 5,
   DXIL_INTR_FABS = 6,
   DXIL_INTR_SATURATE = 7,

   DXIL_INTR_ISFINITE = 10,
   DXIL_INTR_ISNORMAL = 11,

   DXIL_INTR_FCOS = 12,
   DXIL_INTR_FSIN = 13,

   DXIL_INTR_FEXP2 = 21,
   DXIL_INTR_FRC = 22,
   DXIL_INTR_FLOG2 = 23,

   DXIL_INTR_SQRT = 24,
   DXIL_INTR_RSQRT = 25,
   DXIL_INTR_ROUND_NE = 26,
   DXIL_INTR_ROUND_NI = 27,
   DXIL_INTR_ROUND_PI = 28,
   DXIL_INTR_ROUND_Z = 29,

   DXIL_INTR_COUNTBITS = 31,
   DXIL_INTR_FIRSTBIT_HI = 33,

   DXIL_INTR_FMAX = 35,
   DXIL_INTR_FMIN = 36,
   DXIL_INTR_IMAX = 37,
   DXIL_INTR_IMIN = 38,
   DXIL_INTR_UMAX = 39,
   DXIL_INTR_UMIN = 40,

   DXIL_INTR_FMA = 47,

   DXIL_INTR_CREATE_HANDLE = 57,
   DXIL_INTR_CBUFFER_LOAD_LEGACY = 59,

   DXIL_INTR_SAMPLE = 60,
   DXIL_INTR_SAMPLE_BIAS = 61,
   DXIL_INTR_SAMPLE_LEVEL = 62,
   DXIL_INTR_SAMPLE_GRAD = 63,
   DXIL_INTR_SAMPLE_CMP = 64,
   DXIL_INTR_SAMPLE_CMP_LVL_ZERO = 65,

   DXIL_INTR_TEXTURE_LOAD = 66,
   DXIL_INTR_TEXTURE_STORE = 67,

   DXIL_INTR_BUFFER_LOAD = 68,
   DXIL_INTR_BUFFER_STORE = 69,

   DXIL_INTR_TEXTURE_SIZE = 72,

   DXIL_INTR_ATOMIC_BINOP = 78,
   DXIL_INTR_ATOMIC_CMPXCHG = 79,
   DXIL_INTR_BARRIER = 80,
   DXIL_INTR_TEXTURE_LOD = 81,

   DXIL_INTR_DISCARD = 82,
   DXIL_INTR_DDX_COARSE = 83,
   DXIL_INTR_DDY_COARSE = 84,
   DXIL_INTR_DDX_FINE = 85,
   DXIL_INTR_DDY_FINE = 86,

   DXIL_INTR_SAMPLE_INDEX = 90,

   DXIL_INTR_THREAD_ID = 93,
   DXIL_INTR_GROUP_ID = 94,
   DXIL_INTR_THREAD_ID_IN_GROUP = 95,
   DXIL_INTR_FLATTENED_THREAD_ID_IN_GROUP = 96,

   DXIL_INTR_EMIT_STREAM = 97,
   DXIL_INTR_CUT_STREAM = 98,

   DXIL_INTR_MAKE_DOUBLE = 101,
   DXIL_INTR_SPLIT_DOUBLE = 102,

   DXIL_INTR_PRIMITIVE_ID = 108,

   DXIL_INTR_LEGACY_F32TOF16 = 130,
   DXIL_INTR_LEGACY_F16TOF32 = 131,

   DXIL_INTR_ATTRIBUTE_AT_VERTEX = 137,
};

enum dxil_atomic_op {
   DXIL_ATOMIC_ADD = 0,
   DXIL_ATOMIC_AND = 1,
   DXIL_ATOMIC_OR = 2,
   DXIL_ATOMIC_XOR = 3,
   DXIL_ATOMIC_IMIN = 4,
   DXIL_ATOMIC_IMAX = 5,
   DXIL_ATOMIC_UMIN = 6,
   DXIL_ATOMIC_UMAX = 7,
   DXIL_ATOMIC_EXCHANGE = 8,
};

typedef struct {
   unsigned id;
   unsigned binding;
   unsigned size;
   unsigned space;
} resource_array_layout;

static void
fill_resource_metadata(struct dxil_module *m, const struct dxil_mdnode **fields,
                       const struct dxil_type *struct_type,
                       const char *name, const resource_array_layout *layout)
{
   const struct dxil_type *pointer_type = dxil_module_get_pointer_type(m, struct_type);
   const struct dxil_value *pointer_undef = dxil_module_get_undef(m, pointer_type);

   fields[0] = dxil_get_metadata_int32(m, layout->id); // resource ID
   fields[1] = dxil_get_metadata_value(m, pointer_type, pointer_undef); // global constant symbol
   fields[2] = dxil_get_metadata_string(m, name ? name : ""); // name
   fields[3] = dxil_get_metadata_int32(m, layout->space); // space ID
   fields[4] = dxil_get_metadata_int32(m, layout->binding); // lower bound
   fields[5] = dxil_get_metadata_int32(m, layout->size); // range size
}

static const struct dxil_mdnode *
emit_srv_metadata(struct dxil_module *m, const struct dxil_type *elem_type,
                  const char *name, const resource_array_layout *layout,
                  enum dxil_component_type comp_type,
                  enum dxil_resource_kind res_kind)
{
   const struct dxil_mdnode *fields[9];

   const struct dxil_mdnode *metadata_tag_nodes[2];

   fill_resource_metadata(m, fields, elem_type, name, layout);
   fields[6] = dxil_get_metadata_int32(m, res_kind); // resource shape
   fields[7] = dxil_get_metadata_int1(m, 0); // sample count
   if (res_kind != DXIL_RESOURCE_KIND_RAW_BUFFER &&
       res_kind != DXIL_RESOURCE_KIND_STRUCTURED_BUFFER) {
      metadata_tag_nodes[0] = dxil_get_metadata_int32(m, DXIL_TYPED_BUFFER_ELEMENT_TYPE_TAG);
      metadata_tag_nodes[1] = dxil_get_metadata_int32(m, comp_type);
      fields[8] = dxil_get_metadata_node(m, metadata_tag_nodes, ARRAY_SIZE(metadata_tag_nodes)); // metadata
   } else if (res_kind == DXIL_RESOURCE_KIND_RAW_BUFFER)
      fields[8] = NULL;
   else
      unreachable("Structured buffers not supported yet");

   return dxil_get_metadata_node(m, fields, ARRAY_SIZE(fields));
}

static const struct dxil_mdnode *
emit_uav_metadata(struct dxil_module *m, const struct dxil_type *struct_type,
                  const char *name, const resource_array_layout *layout,
                  enum dxil_component_type comp_type,
                  enum dxil_resource_kind res_kind)
{
   const struct dxil_mdnode *fields[11];

   const struct dxil_mdnode *metadata_tag_nodes[2];

   fill_resource_metadata(m, fields, struct_type, name, layout);
   fields[6] = dxil_get_metadata_int32(m, res_kind); // resource shape
   fields[7] = dxil_get_metadata_int1(m, false); // globally-coherent
   fields[8] = dxil_get_metadata_int1(m, false); // has counter
   fields[9] = dxil_get_metadata_int1(m, false); // is ROV
   if (res_kind != DXIL_RESOURCE_KIND_RAW_BUFFER &&
       res_kind != DXIL_RESOURCE_KIND_STRUCTURED_BUFFER) {
      metadata_tag_nodes[0] = dxil_get_metadata_int32(m, DXIL_TYPED_BUFFER_ELEMENT_TYPE_TAG);
      metadata_tag_nodes[1] = dxil_get_metadata_int32(m, comp_type);
      fields[10] = dxil_get_metadata_node(m, metadata_tag_nodes, ARRAY_SIZE(metadata_tag_nodes)); // metadata
   } else if (res_kind == DXIL_RESOURCE_KIND_RAW_BUFFER)
      fields[10] = NULL;
   else
      unreachable("Structured buffers not supported yet");

   return dxil_get_metadata_node(m, fields, ARRAY_SIZE(fields));
}

static const struct dxil_mdnode *
emit_cbv_metadata(struct dxil_module *m, const struct dxil_type *struct_type,
                  const char *name, const resource_array_layout *layout,
                  unsigned size)
{
   const struct dxil_mdnode *fields[8];

   fill_resource_metadata(m, fields, struct_type, name, layout);
   fields[6] = dxil_get_metadata_int32(m, size); // constant buffer size
   fields[7] = NULL; // metadata

   return dxil_get_metadata_node(m, fields, ARRAY_SIZE(fields));
}

static const struct dxil_mdnode *
emit_sampler_metadata(struct dxil_module *m, const struct dxil_type *struct_type,
                      nir_variable *var, const resource_array_layout *layout)
{
   const struct dxil_mdnode *fields[8];
   const struct glsl_type *type = glsl_without_array(var->type);

   fill_resource_metadata(m, fields, struct_type, var->name, layout);
   fields[6] = dxil_get_metadata_int32(m, DXIL_SAMPLER_KIND_DEFAULT); // sampler kind
   enum dxil_sampler_kind sampler_kind = glsl_sampler_type_is_shadow(type) ?
          DXIL_SAMPLER_KIND_COMPARISON : DXIL_SAMPLER_KIND_DEFAULT;
   fields[6] = dxil_get_metadata_int32(m, sampler_kind); // sampler kind
   fields[7] = NULL; // metadata

   return dxil_get_metadata_node(m, fields, ARRAY_SIZE(fields));
}


#define MAX_SRVS 128
#define MAX_UAVS 64
#define MAX_CBVS 64 // ??
#define MAX_SAMPLERS 64 // ??

struct dxil_def {
   const struct dxil_value *chans[NIR_MAX_VEC_COMPONENTS];
};

struct ntd_context {
   void *ralloc_ctx;
   const struct nir_to_dxil_options *opts;
   struct nir_shader *shader;

   struct dxil_module mod;

   struct util_dynarray srv_metadata_nodes;
   const struct dxil_value *srv_handles[MAX_SRVS];

   struct util_dynarray uav_metadata_nodes;
   const struct dxil_value *uav_handles[MAX_UAVS];

   struct util_dynarray cbv_metadata_nodes;
   const struct dxil_value *cbv_handles[MAX_CBVS];

   struct util_dynarray sampler_metadata_nodes;
   const struct dxil_value *sampler_handles[MAX_SAMPLERS];

   struct util_dynarray resources;

   const struct dxil_mdnode *shader_property_nodes[6];
   size_t num_shader_property_nodes;

   struct dxil_def *defs;
   unsigned num_defs;
   struct hash_table *phis;

   const struct dxil_value *sharedvars;
   const struct dxil_value *scratchvars;
   struct hash_table *consts;

   nir_variable *ps_front_face;
   nir_variable *system_value[SYSTEM_VALUE_MAX];
};

static const char*
unary_func_name(enum dxil_intr intr)
{
   switch (intr) {
   case DXIL_INTR_COUNTBITS:
   case DXIL_INTR_FIRSTBIT_HI:
      return "dx.op.unaryBits";
   case DXIL_INTR_ISFINITE:
   case DXIL_INTR_ISNORMAL:
      return "dx.op.isSpecialFloat";
   default:
      return "dx.op.unary";
   }
}

static const struct dxil_value *
emit_unary_call(struct ntd_context *ctx, enum overload_type overload,
                enum dxil_intr intr,
                const struct dxil_value *op0)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod,
                                                    unary_func_name(intr),
                                                    overload);
   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, intr);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     op0
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_binary_call(struct ntd_context *ctx, enum overload_type overload,
                 enum dxil_intr intr,
                 const struct dxil_value *op0, const struct dxil_value *op1)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.binary", overload);
   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, intr);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     op0,
     op1
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_tertiary_call(struct ntd_context *ctx, enum overload_type overload,
                   enum dxil_intr intr,
                   const struct dxil_value *op0,
                   const struct dxil_value *op1,
                   const struct dxil_value *op2)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.tertiary", overload);
   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, intr);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     op0,
     op1,
     op2
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_threadid_call(struct ntd_context *ctx, const struct dxil_value *comp)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.threadId", DXIL_I32);
   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
       DXIL_INTR_THREAD_ID);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     comp
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_threadidingroup_call(struct ntd_context *ctx,
                          const struct dxil_value *comp)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.threadIdInGroup", DXIL_I32);

   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
       DXIL_INTR_THREAD_ID_IN_GROUP);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     comp
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_flattenedthreadidingroup_call(struct ntd_context *ctx)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.flattenedThreadIdInGroup", DXIL_I32);

   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
      DXIL_INTR_FLATTENED_THREAD_ID_IN_GROUP);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_groupid_call(struct ntd_context *ctx, const struct dxil_value *comp)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.groupId", DXIL_I32);

   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
       DXIL_INTR_GROUP_ID);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
     opcode,
     comp
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_bufferload_call(struct ntd_context *ctx,
                     const struct dxil_value *handle,
                     const struct dxil_value *coord[2],
                     enum overload_type overload)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.bufferLoad", overload);
   if (!func)
      return NULL;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
      DXIL_INTR_BUFFER_LOAD);
   const struct dxil_value *args[] = { opcode, handle, coord[0], coord[1] };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_bufferstore_call(struct ntd_context *ctx,
                      const struct dxil_value *handle,
                      const struct dxil_value *coord[2],
                      const struct dxil_value *value[4],
                      const struct dxil_value *write_mask,
                      enum overload_type overload)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.bufferStore", overload);

   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
      DXIL_INTR_BUFFER_STORE);
   const struct dxil_value *args[] = {
      opcode, handle, coord[0], coord[1],
      value[0], value[1], value[2], value[3],
      write_mask
   };

   return dxil_emit_call_void(&ctx->mod, func,
                              args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_textureload_call(struct ntd_context *ctx,
                      const struct dxil_value *handle,
                      const struct dxil_value *coord[3],
                      enum overload_type overload)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.textureLoad", overload);
   if (!func)
      return NULL;
   const struct dxil_type *int_type = dxil_module_get_int_type(&ctx->mod, 32);
   const struct dxil_value *int_undef = dxil_module_get_undef(&ctx->mod, int_type);

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
      DXIL_INTR_TEXTURE_LOAD);
   const struct dxil_value *args[] = { opcode, handle,
      /*lod_or_sample*/ int_undef,
      coord[0], coord[1], coord[2],
      /* offsets */ int_undef, int_undef, int_undef};

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_texturestore_call(struct ntd_context *ctx,
                       const struct dxil_value *handle,
                       const struct dxil_value *coord[3],
                       const struct dxil_value *value[4],
                       const struct dxil_value *write_mask,
                       enum overload_type overload)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.textureStore", overload);

   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod,
      DXIL_INTR_TEXTURE_STORE);
   const struct dxil_value *args[] = {
      opcode, handle, coord[0], coord[1], coord[2],
      value[0], value[1], value[2], value[3],
      write_mask
   };

   return dxil_emit_call_void(&ctx->mod, func,
                              args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_atomic_binop(struct ntd_context *ctx,
                  const struct dxil_value *handle,
                  enum dxil_atomic_op atomic_op,
                  const struct dxil_value *coord[3],
                  const struct dxil_value *value)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.atomicBinOp", DXIL_I32);

   if (!func)
      return false;

   const struct dxil_value *opcode =
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_ATOMIC_BINOP);
   const struct dxil_value *atomic_op_value =
      dxil_module_get_int32_const(&ctx->mod, atomic_op);
   const struct dxil_value *args[] = {
      opcode, handle, atomic_op_value,
      coord[0], coord[1], coord[2], value
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_atomic_cmpxchg(struct ntd_context *ctx,
                    const struct dxil_value *handle,
                    const struct dxil_value *coord[3],
                    const struct dxil_value *cmpval,
                    const struct dxil_value *newval)
{
   const struct dxil_func *func =
      dxil_get_function(&ctx->mod, "dx.op.atomicCompareExchange", DXIL_I32);

   if (!func)
      return false;

   const struct dxil_value *opcode =
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_ATOMIC_CMPXCHG);
   const struct dxil_value *args[] = {
      opcode, handle, coord[0], coord[1], coord[2], cmpval, newval
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_createhandle_call(struct ntd_context *ctx,
                       enum dxil_resource_class resource_class,
                       unsigned resource_range_id,
                       const struct dxil_value *resource_range_index,
                       bool non_uniform_resource_index)
{
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_CREATE_HANDLE);
   const struct dxil_value *resource_class_value = dxil_module_get_int8_const(&ctx->mod, resource_class);
   const struct dxil_value *resource_range_id_value = dxil_module_get_int32_const(&ctx->mod, resource_range_id);
   const struct dxil_value *non_uniform_resource_index_value = dxil_module_get_int1_const(&ctx->mod, non_uniform_resource_index);
   if (!opcode || !resource_class_value || !resource_range_id_value ||
       !non_uniform_resource_index_value)
      return NULL;

   const struct dxil_value *args[] = {
      opcode,
      resource_class_value,
      resource_range_id_value,
      resource_range_index,
      non_uniform_resource_index_value
   };

   const struct dxil_func *func =
         dxil_get_function(&ctx->mod, "dx.op.createHandle", DXIL_NONE);

   if (!func)
         return NULL;

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_createhandle_call_const_index(struct ntd_context *ctx,
                                   enum dxil_resource_class resource_class,
                                   unsigned resource_range_id,
                                   unsigned resource_range_index,
                                   bool non_uniform_resource_index)
{

   const struct dxil_value *resource_range_index_value = dxil_module_get_int32_const(&ctx->mod, resource_range_index);
   if (!resource_range_index_value)
      return NULL;

   return emit_createhandle_call(ctx, resource_class, resource_range_id,
                                 resource_range_index_value,
                                 non_uniform_resource_index);
}

static void
add_resource(struct ntd_context *ctx, enum dxil_resource_type type,
             const resource_array_layout *layout)
{
   struct dxil_resource *resource = util_dynarray_grow(&ctx->resources, struct dxil_resource, 1);
   resource->resource_type = type;
   resource->space = layout->space;
   resource->lower_bound = layout->binding;
   if (layout->size == 0 || (uint64_t)layout->size + layout->binding >= UINT_MAX)
      resource->upper_bound = UINT_MAX;
   else
      resource->upper_bound = layout->binding + layout->size - 1;
}

static unsigned
get_resource_id(struct ntd_context *ctx, enum dxil_resource_class class,
                unsigned space, unsigned binding)
{
   unsigned offset = 0;
   unsigned count = 0;

   unsigned num_srvs = util_dynarray_num_elements(&ctx->srv_metadata_nodes, const struct dxil_mdnode *);
   unsigned num_uavs = util_dynarray_num_elements(&ctx->uav_metadata_nodes, const struct dxil_mdnode *);
   unsigned num_cbvs = util_dynarray_num_elements(&ctx->cbv_metadata_nodes, const struct dxil_mdnode *);
   unsigned num_samplers = util_dynarray_num_elements(&ctx->sampler_metadata_nodes, const struct dxil_mdnode *);

   switch (class) {
   case DXIL_RESOURCE_CLASS_UAV:
      offset = num_srvs + num_samplers + num_cbvs;
      count = num_uavs;
      break;
   case DXIL_RESOURCE_CLASS_SRV:
      offset = num_samplers + num_cbvs;
      count = num_srvs;
      break;
   case DXIL_RESOURCE_CLASS_SAMPLER:
      offset = num_cbvs;
      count = num_samplers;
      break;
   case DXIL_RESOURCE_CLASS_CBV:
      offset = 0;
      count = num_cbvs;
      break;
   }

   assert(offset + count <= util_dynarray_num_elements(&ctx->resources, struct dxil_resource));
   for (unsigned i = offset; i < offset + count; ++i) {
      const struct dxil_resource *resource = util_dynarray_element(&ctx->resources, struct dxil_resource, i);
      if (resource->space == space &&
          resource->lower_bound <= binding &&
          resource->upper_bound >= binding) {
         return i - offset;
      }
   }

   unreachable("Resource access for undeclared range");
   return 0;
}

static bool
emit_srv(struct ntd_context *ctx, nir_variable *var, unsigned count)
{
   unsigned id = util_dynarray_num_elements(&ctx->srv_metadata_nodes, const struct dxil_mdnode *);
   unsigned binding = var->data.binding;
   resource_array_layout layout = {id, binding, count, var->data.descriptor_set};

   enum dxil_component_type comp_type;
   enum dxil_resource_kind res_kind;
   enum dxil_resource_type res_type;
   if (var->data.mode == nir_var_mem_ssbo) {
      comp_type = DXIL_COMP_TYPE_INVALID;
      res_kind = DXIL_RESOURCE_KIND_RAW_BUFFER;
      res_type = DXIL_RES_SRV_RAW;
   } else {
      comp_type = dxil_get_comp_type(var->type);
      res_kind = dxil_get_resource_kind(var->type);
      res_type = DXIL_RES_SRV_TYPED;
   }
   const struct dxil_type *res_type_as_type = dxil_module_get_res_type(&ctx->mod, res_kind, comp_type, false /* readwrite */);
   const struct dxil_mdnode *srv_meta = emit_srv_metadata(&ctx->mod, res_type_as_type, var->name,
                                                          &layout, comp_type, res_kind);

   if (!srv_meta)
      return false;

   util_dynarray_append(&ctx->srv_metadata_nodes, const struct dxil_mdnode *, srv_meta);
   add_resource(ctx, res_type, &layout);
   if (res_type == DXIL_RES_SRV_RAW)
      ctx->mod.raw_and_structured_buffers = true;

   if (!ctx->opts->vulkan_environment) {
      for (unsigned i = 0; i < count; ++i) {
         const struct dxil_value *handle =
            emit_createhandle_call_const_index(ctx, DXIL_RESOURCE_CLASS_SRV,
                                               id, binding + i, false);
         if (!handle)
            return false;

         int idx = var->data.binding + i;
         ctx->srv_handles[idx] = handle;
      }
   }

   return true;
}

static bool
emit_globals(struct ntd_context *ctx, unsigned size)
{
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_mem_ssbo)
      size++;

   if (!size)
      return true;

   const struct dxil_type *struct_type = dxil_module_get_res_type(&ctx->mod,
      DXIL_RESOURCE_KIND_RAW_BUFFER, DXIL_COMP_TYPE_INVALID, true /* readwrite */);
   if (!struct_type)
      return false;

   const struct dxil_type *array_type =
      dxil_module_get_array_type(&ctx->mod, struct_type, size);
   if (!array_type)
      return false;

   resource_array_layout layout = {0, 0, size, 0};
   const struct dxil_mdnode *uav_meta =
      emit_uav_metadata(&ctx->mod, array_type,
                                   "globals", &layout,
                                   DXIL_COMP_TYPE_INVALID,
                                   DXIL_RESOURCE_KIND_RAW_BUFFER);
   if (!uav_meta)
      return false;

   util_dynarray_append(&ctx->uav_metadata_nodes, const struct dxil_mdnode *, uav_meta);
   if (util_dynarray_num_elements(&ctx->uav_metadata_nodes, const struct dxil_mdnode *) > 8)
      ctx->mod.feats.use_64uavs = 1;
   /* Handles to UAVs used for kernel globals are created on-demand */
   add_resource(ctx, DXIL_RES_UAV_RAW, &layout);
   ctx->mod.raw_and_structured_buffers = true;
   return true;
}

static bool
emit_uav(struct ntd_context *ctx, unsigned binding, unsigned space, unsigned count,
         enum dxil_component_type comp_type, enum dxil_resource_kind res_kind, const char *name)
{
   unsigned id = util_dynarray_num_elements(&ctx->uav_metadata_nodes, const struct dxil_mdnode *);
   resource_array_layout layout = { id, binding, count, space };

   const struct dxil_type *res_type = dxil_module_get_res_type(&ctx->mod, res_kind, comp_type, true /* readwrite */);
   const struct dxil_mdnode *uav_meta = emit_uav_metadata(&ctx->mod, res_type, name,
                                                          &layout, comp_type, res_kind);

   if (!uav_meta)
      return false;

   util_dynarray_append(&ctx->uav_metadata_nodes, const struct dxil_mdnode *, uav_meta);
   if (util_dynarray_num_elements(&ctx->uav_metadata_nodes, const struct dxil_mdnode *) > 8)
      ctx->mod.feats.use_64uavs = 1;

   add_resource(ctx, res_kind == DXIL_RESOURCE_KIND_RAW_BUFFER ? DXIL_RES_UAV_RAW : DXIL_RES_UAV_TYPED, &layout);
   if (res_kind == DXIL_RESOURCE_KIND_RAW_BUFFER)
      ctx->mod.raw_and_structured_buffers = true;

   if (!ctx->opts->vulkan_environment) {
      for (unsigned i = 0; i < count; ++i) {
         const struct dxil_value *handle = emit_createhandle_call_const_index(ctx, DXIL_RESOURCE_CLASS_UAV,
                                                                              id, binding + i, false);
         if (!handle)
            return false;

         ctx->uav_handles[binding + i] = handle;
      }
   }

   return true;
}

static bool
emit_uav_var(struct ntd_context *ctx, nir_variable *var, unsigned count)
{
   unsigned binding = var->data.binding;
   unsigned space = var->data.descriptor_set;
   enum dxil_component_type comp_type = dxil_get_comp_type(var->type);
   enum dxil_resource_kind res_kind = dxil_get_resource_kind(var->type);
   const char *name = var->name;

   return emit_uav(ctx, binding, space, count, comp_type, res_kind, name);
}

static unsigned get_dword_size(const struct glsl_type *type)
{
   if (glsl_type_is_array(type)) {
      type = glsl_without_array(type);
   }
   assert(glsl_type_is_struct(type) || glsl_type_is_interface(type));
   return glsl_get_explicit_size(type, false);
}

static void
var_fill_const_array_with_vector_or_scalar(struct ntd_context *ctx,
                                           const struct nir_constant *c,
                                           const struct glsl_type *type,
                                           void *const_vals,
                                           unsigned int offset)
{
   assert(glsl_type_is_vector_or_scalar(type));
   unsigned int components = glsl_get_vector_elements(type);
   unsigned bit_size = glsl_get_bit_size(type);
   unsigned int increment = bit_size / 8;

   for (unsigned int comp = 0; comp < components; comp++) {
      uint8_t *dst = (uint8_t *)const_vals + offset;

      switch (bit_size) {
      case 64:
         memcpy(dst, &c->values[comp].u64, sizeof(c->values[0].u64));
         break;
      case 32:
         memcpy(dst, &c->values[comp].u32, sizeof(c->values[0].u32));
         break;
      case 16:
         memcpy(dst, &c->values[comp].u16, sizeof(c->values[0].u16));
         break;
      case 8:
         assert(glsl_base_type_is_integer(glsl_get_base_type(type)));
         memcpy(dst, &c->values[comp].u8, sizeof(c->values[0].u8));
         break;
      default:
         unreachable("unexpeted bit-size");
      }

      offset += increment;
   }
}

static void
var_fill_const_array(struct ntd_context *ctx, const struct nir_constant *c,
                     const struct glsl_type *type, void *const_vals,
                     unsigned int offset)
{
   assert(!glsl_type_is_interface(type));

   if (glsl_type_is_vector_or_scalar(type)) {
      var_fill_const_array_with_vector_or_scalar(ctx, c, type,
                                                 const_vals,
                                                 offset);
   } else if (glsl_type_is_array(type)) {
      assert(!glsl_type_is_unsized_array(type));
      const struct glsl_type *without = glsl_without_array(type);
      unsigned stride = glsl_get_explicit_stride(without);

      for (unsigned elt = 0; elt < glsl_get_length(type); elt++) {
         var_fill_const_array(ctx, c->elements[elt], without,
                              const_vals, offset + (elt * stride));
         offset += glsl_get_cl_size(without);
      }
   } else if (glsl_type_is_struct(type)) {
      for (unsigned int elt = 0; elt < glsl_get_length(type); elt++) {
         const struct glsl_type *elt_type = glsl_get_struct_field(type, elt);
         unsigned field_offset = glsl_get_struct_field_offset(type, elt);

         var_fill_const_array(ctx, c->elements[elt],
                              elt_type, const_vals,
                              offset + field_offset);
      }
   } else
      unreachable("unknown GLSL type in var_fill_const_array");
}

static bool
emit_global_consts(struct ntd_context *ctx)
{
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_shader_temp) {
      assert(var->constant_initializer);

      unsigned int num_members = DIV_ROUND_UP(glsl_get_cl_size(var->type), 4);
      uint32_t *const_ints = ralloc_array(ctx->ralloc_ctx, uint32_t, num_members);
      var_fill_const_array(ctx, var->constant_initializer, var->type,
                                 const_ints, 0);
      const struct dxil_value **const_vals =
         ralloc_array(ctx->ralloc_ctx, const struct dxil_value *, num_members);
      if (!const_vals)
         return false;
      for (int i = 0; i < num_members; i++)
         const_vals[i] = dxil_module_get_int32_const(&ctx->mod, const_ints[i]);

      const struct dxil_type *elt_type = dxil_module_get_int_type(&ctx->mod, 32);
      if (!elt_type)
         return false;
      const struct dxil_type *type =
         dxil_module_get_array_type(&ctx->mod, elt_type, num_members);
      if (!type)
         return false;
      const struct dxil_value *agg_vals =
         dxil_module_get_array_const(&ctx->mod, type, const_vals);
      if (!agg_vals)
         return false;

      const struct dxil_value *gvar = dxil_add_global_ptr_var(&ctx->mod, var->name, type,
                                                              DXIL_AS_DEFAULT, 4,
                                                              agg_vals);
      if (!gvar)
         return false;

      if (!_mesa_hash_table_insert(ctx->consts, var, (void *)gvar))
         return false;
   }

   return true;
}

static bool
emit_cbv(struct ntd_context *ctx, unsigned binding, unsigned space,
         unsigned size, unsigned count, char *name)
{
   unsigned idx = util_dynarray_num_elements(&ctx->cbv_metadata_nodes, const struct dxil_mdnode *);

   const struct dxil_type *float32 = dxil_module_get_float_type(&ctx->mod, 32);
   const struct dxil_type *array_type = dxil_module_get_array_type(&ctx->mod, float32, size);
   const struct dxil_type *buffer_type = dxil_module_get_struct_type(&ctx->mod, name,
                                                                     &array_type, 1);
   const struct dxil_type *final_type = count != 1 ? dxil_module_get_array_type(&ctx->mod, buffer_type, count) : buffer_type;
   resource_array_layout layout = {idx, binding, count, space};
   const struct dxil_mdnode *cbv_meta = emit_cbv_metadata(&ctx->mod, final_type,
                                                          name, &layout, 4 * size);

   if (!cbv_meta)
      return false;

   util_dynarray_append(&ctx->cbv_metadata_nodes, const struct dxil_mdnode *, cbv_meta);
   add_resource(ctx, DXIL_RES_CBV, &layout);

   if (!ctx->opts->vulkan_environment) {
      for (unsigned i = 0; i < count; ++i) {
         const struct dxil_value *handle = emit_createhandle_call_const_index(ctx, DXIL_RESOURCE_CLASS_CBV,
                                                                              idx, binding + i, false);
         if (!handle)
            return false;

         assert(!ctx->cbv_handles[binding + i]);
         ctx->cbv_handles[binding + i] = handle;
      }
   }

   return true;
}

static bool
emit_ubo_var(struct ntd_context *ctx, nir_variable *var)
{
   unsigned count = 1;
   if (glsl_type_is_array(var->type))
      count = glsl_get_length(var->type);
   return emit_cbv(ctx, var->data.binding, var->data.descriptor_set, get_dword_size(var->type), count, var->name);
}

static bool
emit_sampler(struct ntd_context *ctx, nir_variable *var, unsigned count)
{
   unsigned id = util_dynarray_num_elements(&ctx->sampler_metadata_nodes, const struct dxil_mdnode *);
   unsigned binding = var->data.binding;
   resource_array_layout layout = {id, binding, count, var->data.descriptor_set};
   const struct dxil_type *int32_type = dxil_module_get_int_type(&ctx->mod, 32);
   const struct dxil_type *sampler_type = dxil_module_get_struct_type(&ctx->mod, "struct.SamplerState", &int32_type, 1);
   const struct dxil_mdnode *sampler_meta = emit_sampler_metadata(&ctx->mod, sampler_type, var, &layout);

   if (!sampler_meta)
      return false;

   util_dynarray_append(&ctx->sampler_metadata_nodes, const struct dxil_mdnode *, sampler_meta);
   add_resource(ctx, DXIL_RES_SAMPLER, &layout);

   if (!ctx->opts->vulkan_environment) {
      for (unsigned i = 0; i < count; ++i) {
         const struct dxil_value *handle =
            emit_createhandle_call_const_index(ctx, DXIL_RESOURCE_CLASS_SAMPLER,
                                               id, binding + i, false);
         if (!handle)
            return false;

         unsigned idx = var->data.binding + i;
         ctx->sampler_handles[idx] = handle;
      }
   }

   return true;
}

static const struct dxil_mdnode *
emit_gs_state(struct ntd_context *ctx)
{
   const struct dxil_mdnode *gs_state_nodes[5];
   const nir_shader *s = ctx->shader;

   gs_state_nodes[0] = dxil_get_metadata_int32(&ctx->mod, dxil_get_input_primitive(s->info.gs.input_primitive));
   gs_state_nodes[1] = dxil_get_metadata_int32(&ctx->mod, s->info.gs.vertices_out);
   gs_state_nodes[2] = dxil_get_metadata_int32(&ctx->mod, s->info.gs.active_stream_mask);
   gs_state_nodes[3] = dxil_get_metadata_int32(&ctx->mod, dxil_get_primitive_topology(s->info.gs.output_primitive));
   gs_state_nodes[4] = dxil_get_metadata_int32(&ctx->mod, s->info.gs.invocations);

   for (unsigned i = 0; i < ARRAY_SIZE(gs_state_nodes); ++i) {
      if (!gs_state_nodes[i])
         return NULL;
   }

   return dxil_get_metadata_node(&ctx->mod, gs_state_nodes, ARRAY_SIZE(gs_state_nodes));
}

static const struct dxil_mdnode *
emit_threads(struct ntd_context *ctx)
{
   const nir_shader *s = ctx->shader;
   const struct dxil_mdnode *threads_x = dxil_get_metadata_int32(&ctx->mod, MAX2(s->info.workgroup_size[0], 1));
   const struct dxil_mdnode *threads_y = dxil_get_metadata_int32(&ctx->mod, MAX2(s->info.workgroup_size[1], 1));
   const struct dxil_mdnode *threads_z = dxil_get_metadata_int32(&ctx->mod, MAX2(s->info.workgroup_size[2], 1));
   if (!threads_x || !threads_y || !threads_z)
      return false;

   const struct dxil_mdnode *threads_nodes[] = { threads_x, threads_y, threads_z };
   return dxil_get_metadata_node(&ctx->mod, threads_nodes, ARRAY_SIZE(threads_nodes));
}

static int64_t
get_module_flags(struct ntd_context *ctx)
{
   /* See the DXIL documentation for the definition of these flags:
    *
    * https://github.com/Microsoft/DirectXShaderCompiler/blob/master/docs/DXIL.rst#shader-flags
    */

   uint64_t flags = 0;
   if (ctx->mod.feats.doubles)
      flags |= (1 << 2);
   if (ctx->mod.raw_and_structured_buffers)
      flags |= (1 << 4);
   if (ctx->mod.feats.min_precision)
      flags |= (1 << 5);
   if (ctx->mod.feats.dx11_1_double_extensions)
      flags |= (1 << 6);
   if (ctx->mod.feats.inner_coverage)
      flags |= (1 << 10);
   if (ctx->mod.feats.typed_uav_load_additional_formats)
      flags |= (1 << 13);
   if (ctx->mod.feats.use_64uavs)
      flags |= (1 << 15);
   if (ctx->mod.feats.cs_4x_raw_sb)
      flags |= (1 << 17);
   if (ctx->mod.feats.wave_ops)
      flags |= (1 << 19);
   if (ctx->mod.feats.int64_ops)
      flags |= (1 << 20);
   if (ctx->mod.feats.stencil_ref)
      flags |= (1 << 11);
   if (ctx->mod.feats.native_low_precision)
      flags |= (1 << 23) | (1 << 5);

   if (ctx->opts->disable_math_refactoring)
      flags |= (1 << 1);

   return flags;
}

static const struct dxil_mdnode *
emit_entrypoint(struct ntd_context *ctx,
                const struct dxil_func *func, const char *name,
                const struct dxil_mdnode *signatures,
                const struct dxil_mdnode *resources,
                const struct dxil_mdnode *shader_props)
{
   const struct dxil_mdnode *func_md = dxil_get_metadata_func(&ctx->mod, func);
   const struct dxil_mdnode *name_md = dxil_get_metadata_string(&ctx->mod, name);
   const struct dxil_mdnode *nodes[] = {
      func_md,
      name_md,
      signatures,
      resources,
      shader_props
   };
   return dxil_get_metadata_node(&ctx->mod, nodes,
                                 ARRAY_SIZE(nodes));
}

static const struct dxil_mdnode *
emit_resources(struct ntd_context *ctx)
{
   bool emit_resources = false;
   const struct dxil_mdnode *resources_nodes[] = {
      NULL, NULL, NULL, NULL
   };

#define ARRAY_AND_SIZE(arr) arr.data, util_dynarray_num_elements(&arr, const struct dxil_mdnode *)

   if (ctx->srv_metadata_nodes.size) {
      resources_nodes[0] = dxil_get_metadata_node(&ctx->mod, ARRAY_AND_SIZE(ctx->srv_metadata_nodes));
      emit_resources = true;
   }

   if (ctx->uav_metadata_nodes.size) {
      resources_nodes[1] = dxil_get_metadata_node(&ctx->mod, ARRAY_AND_SIZE(ctx->uav_metadata_nodes));
      emit_resources = true;
   }

   if (ctx->cbv_metadata_nodes.size) {
      resources_nodes[2] = dxil_get_metadata_node(&ctx->mod, ARRAY_AND_SIZE(ctx->cbv_metadata_nodes));
      emit_resources = true;
   }

   if (ctx->sampler_metadata_nodes.size) {
      resources_nodes[3] = dxil_get_metadata_node(&ctx->mod, ARRAY_AND_SIZE(ctx->sampler_metadata_nodes));
      emit_resources = true;
   }

#undef ARRAY_AND_SIZE

   return emit_resources ?
      dxil_get_metadata_node(&ctx->mod, resources_nodes, ARRAY_SIZE(resources_nodes)): NULL;
}

static boolean
emit_tag(struct ntd_context *ctx, enum dxil_shader_tag tag,
         const struct dxil_mdnode *value_node)
{
   const struct dxil_mdnode *tag_node = dxil_get_metadata_int32(&ctx->mod, tag);
   if (!tag_node || !value_node)
      return false;
   assert(ctx->num_shader_property_nodes <= ARRAY_SIZE(ctx->shader_property_nodes) - 2);
   ctx->shader_property_nodes[ctx->num_shader_property_nodes++] = tag_node;
   ctx->shader_property_nodes[ctx->num_shader_property_nodes++] = value_node;

   return true;
}

static bool
emit_metadata(struct ntd_context *ctx)
{
   unsigned dxilMinor = ctx->mod.minor_version;
   if (!emit_llvm_ident(&ctx->mod) ||
       !emit_named_version(&ctx->mod, "dx.version", 1, dxilMinor) ||
       !emit_named_version(&ctx->mod, "dx.valver", 1, 4) ||
       !emit_dx_shader_model(&ctx->mod))
      return false;

   const struct dxil_type *void_type = dxil_module_get_void_type(&ctx->mod);
   const struct dxil_type *main_func_type = dxil_module_add_function_type(&ctx->mod, void_type, NULL, 0);
   const struct dxil_func *main_func = dxil_add_function_def(&ctx->mod, "main", main_func_type);
   if (!main_func)
      return false;

   const struct dxil_mdnode *resources_node = emit_resources(ctx);

   const struct dxil_mdnode *main_entrypoint = dxil_get_metadata_func(&ctx->mod, main_func);
   const struct dxil_mdnode *node27 = dxil_get_metadata_node(&ctx->mod, NULL, 0);

   const struct dxil_mdnode *node4 = dxil_get_metadata_int32(&ctx->mod, 0);
   const struct dxil_mdnode *nodes_4_27_27[] = {
      node4, node27, node27
   };
   const struct dxil_mdnode *node28 = dxil_get_metadata_node(&ctx->mod, nodes_4_27_27,
                                                      ARRAY_SIZE(nodes_4_27_27));

   const struct dxil_mdnode *node29 = dxil_get_metadata_node(&ctx->mod, &node28, 1);

   const struct dxil_mdnode *node3 = dxil_get_metadata_int32(&ctx->mod, 1);
   const struct dxil_mdnode *main_type_annotation_nodes[] = {
      node3, main_entrypoint, node29
   };
   const struct dxil_mdnode *main_type_annotation = dxil_get_metadata_node(&ctx->mod, main_type_annotation_nodes,
                                                                           ARRAY_SIZE(main_type_annotation_nodes));

   if (ctx->mod.shader_kind == DXIL_GEOMETRY_SHADER) {
      if (!emit_tag(ctx, DXIL_SHADER_TAG_GS_STATE, emit_gs_state(ctx)))
         return false;
   } else if (ctx->mod.shader_kind == DXIL_COMPUTE_SHADER) {
      if (!emit_tag(ctx, DXIL_SHADER_TAG_NUM_THREADS, emit_threads(ctx)))
         return false;
   }

   uint64_t flags = get_module_flags(ctx);
   if (flags != 0) {
      if (!emit_tag(ctx, DXIL_SHADER_TAG_FLAGS, dxil_get_metadata_int64(&ctx->mod, flags)))
         return false;
   }
   const struct dxil_mdnode *shader_properties = NULL;
   if (ctx->num_shader_property_nodes > 0) {
      shader_properties = dxil_get_metadata_node(&ctx->mod, ctx->shader_property_nodes,
                                                 ctx->num_shader_property_nodes);
      if (!shader_properties)
         return false;
   }

   const struct dxil_mdnode *signatures = get_signatures(&ctx->mod, ctx->shader,
                                                         ctx->opts->vulkan_environment);

   const struct dxil_mdnode *dx_entry_point = emit_entrypoint(ctx, main_func,
       "main", signatures, resources_node, shader_properties);
   if (!dx_entry_point)
      return false;

   if (resources_node) {
      const struct dxil_mdnode *dx_resources = resources_node;
      dxil_add_metadata_named_node(&ctx->mod, "dx.resources",
                                       &dx_resources, 1);
   }

   const struct dxil_mdnode *dx_type_annotations[] = { main_type_annotation };
   return dxil_add_metadata_named_node(&ctx->mod, "dx.typeAnnotations",
                                       dx_type_annotations,
                                       ARRAY_SIZE(dx_type_annotations)) &&
          dxil_add_metadata_named_node(&ctx->mod, "dx.entryPoints",
                                       &dx_entry_point, 1);
}

static const struct dxil_value *
bitcast_to_int(struct ntd_context *ctx, unsigned bit_size,
               const struct dxil_value *value)
{
   const struct dxil_type *type = dxil_module_get_int_type(&ctx->mod, bit_size);
   if (!type)
      return NULL;

   return dxil_emit_cast(&ctx->mod, DXIL_CAST_BITCAST, type, value);
}

static const struct dxil_value *
bitcast_to_float(struct ntd_context *ctx, unsigned bit_size,
                 const struct dxil_value *value)
{
   const struct dxil_type *type = dxil_module_get_float_type(&ctx->mod, bit_size);
   if (!type)
      return NULL;

   return dxil_emit_cast(&ctx->mod, DXIL_CAST_BITCAST, type, value);
}

static void
store_ssa_def(struct ntd_context *ctx, nir_ssa_def *ssa, unsigned chan,
              const struct dxil_value *value)
{
   assert(ssa->index < ctx->num_defs);
   assert(chan < ssa->num_components);
   /* We pre-defined the dest value because of a phi node, so bitcast while storing if the
    * base type differs */
   if (ctx->defs[ssa->index].chans[chan]) {
      const struct dxil_type *expect_type = dxil_value_get_type(ctx->defs[ssa->index].chans[chan]);
      const struct dxil_type *value_type = dxil_value_get_type(value);
      if (dxil_type_to_nir_type(expect_type) != dxil_type_to_nir_type(value_type))
         value = dxil_emit_cast(&ctx->mod, DXIL_CAST_BITCAST, expect_type, value);
   }
   ctx->defs[ssa->index].chans[chan] = value;
}

static void
store_dest_value(struct ntd_context *ctx, nir_dest *dest, unsigned chan,
                 const struct dxil_value *value)
{
   assert(dest->is_ssa);
   assert(value);
   store_ssa_def(ctx, &dest->ssa, chan, value);
}

static void
store_dest(struct ntd_context *ctx, nir_dest *dest, unsigned chan,
           const struct dxil_value *value, nir_alu_type type)
{
   switch (nir_alu_type_get_base_type(type)) {
   case nir_type_float:
      if (nir_dest_bit_size(*dest) == 64)
         ctx->mod.feats.doubles = true;
      FALLTHROUGH;
   case nir_type_uint:
   case nir_type_int:
      if (nir_dest_bit_size(*dest) == 16)
         ctx->mod.feats.native_low_precision = true;
      if (nir_dest_bit_size(*dest) == 64)
         ctx->mod.feats.int64_ops = true;
      FALLTHROUGH;
   case nir_type_bool:
      store_dest_value(ctx, dest, chan, value);
      break;
   default:
      unreachable("unexpected nir_alu_type");
   }
}

static void
store_alu_dest(struct ntd_context *ctx, nir_alu_instr *alu, unsigned chan,
               const struct dxil_value *value)
{
   assert(!alu->dest.saturate);
   store_dest(ctx, &alu->dest.dest, chan, value,
              nir_op_infos[alu->op].output_type);
}

static const struct dxil_value *
get_src_ssa(struct ntd_context *ctx, const nir_ssa_def *ssa, unsigned chan)
{
   assert(ssa->index < ctx->num_defs);
   assert(chan < ssa->num_components);
   assert(ctx->defs[ssa->index].chans[chan]);
   return ctx->defs[ssa->index].chans[chan];
}

static const struct dxil_value *
get_src(struct ntd_context *ctx, nir_src *src, unsigned chan,
        nir_alu_type type)
{
   assert(src->is_ssa);
   const struct dxil_value *value = get_src_ssa(ctx, src->ssa, chan);

   const int bit_size = nir_src_bit_size(*src);

   switch (nir_alu_type_get_base_type(type)) {
   case nir_type_int:
   case nir_type_uint: {
      assert(bit_size != 64 || ctx->mod.feats.int64_ops);
      const struct dxil_type *expect_type =  dxil_module_get_int_type(&ctx->mod, bit_size);
      /* nohing to do */
      if (dxil_value_type_equal_to(value, expect_type))
         return value;
      assert(dxil_value_type_bitsize_equal_to(value, bit_size));
      return bitcast_to_int(ctx,  bit_size, value);
      }

   case nir_type_float:
      assert(nir_src_bit_size(*src) >= 16);
      assert(nir_src_bit_size(*src) != 64 || (ctx->mod.feats.doubles &&
                                              ctx->mod.feats.int64_ops));
      if (dxil_value_type_equal_to(value, dxil_module_get_float_type(&ctx->mod, bit_size)))
         return value;
      assert(dxil_value_type_bitsize_equal_to(value, bit_size));
      return bitcast_to_float(ctx, bit_size, value);

   case nir_type_bool:
      if (!dxil_value_type_bitsize_equal_to(value, 1)) {
         return dxil_emit_cast(&ctx->mod, DXIL_CAST_TRUNC,
                               dxil_module_get_int_type(&ctx->mod, 1), value);
      }
      return value;

   default:
      unreachable("unexpected nir_alu_type");
   }
}

static const struct dxil_type *
get_alu_src_type(struct ntd_context *ctx, nir_alu_instr *alu, unsigned src)
{
   assert(!alu->src[src].abs);
   assert(!alu->src[src].negate);
   nir_ssa_def *ssa_src = alu->src[src].src.ssa;
   unsigned chan = alu->src[src].swizzle[0];
   const struct dxil_value *value = get_src_ssa(ctx, ssa_src, chan);
   return dxil_value_get_type(value);
}

static const struct dxil_value *
get_alu_src(struct ntd_context *ctx, nir_alu_instr *alu, unsigned src)
{
   assert(!alu->src[src].abs);
   assert(!alu->src[src].negate);

   unsigned chan = alu->src[src].swizzle[0];
   return get_src(ctx, &alu->src[src].src, chan,
                  nir_op_infos[alu->op].input_types[src]);
}

static bool
emit_binop(struct ntd_context *ctx, nir_alu_instr *alu,
           enum dxil_bin_opcode opcode,
           const struct dxil_value *op0, const struct dxil_value *op1)
{
   bool is_float_op = nir_alu_type_get_base_type(nir_op_infos[alu->op].output_type) == nir_type_float;

   enum dxil_opt_flags flags = 0;
   if (is_float_op && !alu->exact)
      flags |= DXIL_UNSAFE_ALGEBRA;

   const struct dxil_value *v = dxil_emit_binop(&ctx->mod, opcode, op0, op1, flags);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_shift(struct ntd_context *ctx, nir_alu_instr *alu,
           enum dxil_bin_opcode opcode,
           const struct dxil_value *op0, const struct dxil_value *op1)
{
   unsigned op0_bit_size = nir_src_bit_size(alu->src[0].src);
   unsigned op1_bit_size = nir_src_bit_size(alu->src[1].src);
   if (op0_bit_size != op1_bit_size) {
      const struct dxil_type *type =
         dxil_module_get_int_type(&ctx->mod, op0_bit_size);
      enum dxil_cast_opcode cast_op =
         op1_bit_size < op0_bit_size ? DXIL_CAST_ZEXT : DXIL_CAST_TRUNC;
      op1 = dxil_emit_cast(&ctx->mod, cast_op, type, op1);
   }

   const struct dxil_value *v =
      dxil_emit_binop(&ctx->mod, opcode, op0, op1, 0);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_cmp(struct ntd_context *ctx, nir_alu_instr *alu,
         enum dxil_cmp_pred pred,
         const struct dxil_value *op0, const struct dxil_value *op1)
{
   const struct dxil_value *v = dxil_emit_cmp(&ctx->mod, pred, op0, op1);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static enum dxil_cast_opcode
get_cast_op(nir_alu_instr *alu)
{
   unsigned dst_bits = nir_dest_bit_size(alu->dest.dest);
   unsigned src_bits = nir_src_bit_size(alu->src[0].src);

   switch (alu->op) {
   /* bool -> int */
   case nir_op_b2i16:
   case nir_op_b2i32:
   case nir_op_b2i64:
      return DXIL_CAST_ZEXT;

   /* float -> float */
   case nir_op_f2f16_rtz:
   case nir_op_f2f32:
   case nir_op_f2f64:
      assert(dst_bits != src_bits);
      if (dst_bits < src_bits)
         return DXIL_CAST_FPTRUNC;
      else
         return DXIL_CAST_FPEXT;

   /* int -> int */
   case nir_op_i2i16:
   case nir_op_i2i32:
   case nir_op_i2i64:
      assert(dst_bits != src_bits);
      if (dst_bits < src_bits)
         return DXIL_CAST_TRUNC;
      else
         return DXIL_CAST_SEXT;

   /* uint -> uint */
   case nir_op_u2u16:
   case nir_op_u2u32:
   case nir_op_u2u64:
      assert(dst_bits != src_bits);
      if (dst_bits < src_bits)
         return DXIL_CAST_TRUNC;
      else
         return DXIL_CAST_ZEXT;

   /* float -> int */
   case nir_op_f2i16:
   case nir_op_f2i32:
   case nir_op_f2i64:
      return DXIL_CAST_FPTOSI;

   /* float -> uint */
   case nir_op_f2u16:
   case nir_op_f2u32:
   case nir_op_f2u64:
      return DXIL_CAST_FPTOUI;

   /* int -> float */
   case nir_op_i2f16:
   case nir_op_i2f32:
   case nir_op_i2f64:
      return DXIL_CAST_SITOFP;

   /* uint -> float */
   case nir_op_u2f16:
   case nir_op_u2f32:
   case nir_op_u2f64:
      return DXIL_CAST_UITOFP;

   default:
      unreachable("unexpected cast op");
   }
}

static const struct dxil_type *
get_cast_dest_type(struct ntd_context *ctx, nir_alu_instr *alu)
{
   unsigned dst_bits = nir_dest_bit_size(alu->dest.dest);
   switch (nir_alu_type_get_base_type(nir_op_infos[alu->op].output_type)) {
   case nir_type_bool:
      assert(dst_bits == 1);
      FALLTHROUGH;
   case nir_type_int:
   case nir_type_uint:
      return dxil_module_get_int_type(&ctx->mod, dst_bits);

   case nir_type_float:
      return dxil_module_get_float_type(&ctx->mod, dst_bits);

   default:
      unreachable("unknown nir_alu_type");
   }
}

static bool
is_double(nir_alu_type alu_type, unsigned bit_size)
{
   return nir_alu_type_get_base_type(alu_type) == nir_type_float &&
          bit_size == 64;
}

static bool
emit_cast(struct ntd_context *ctx, nir_alu_instr *alu,
          const struct dxil_value *value)
{
   enum dxil_cast_opcode opcode = get_cast_op(alu);
   const struct dxil_type *type = get_cast_dest_type(ctx, alu);
   if (!type)
      return false;

   const nir_op_info *info = &nir_op_infos[alu->op];
   switch (opcode) {
   case DXIL_CAST_UITOFP:
   case DXIL_CAST_SITOFP:
      if (is_double(info->output_type, nir_dest_bit_size(alu->dest.dest)))
         ctx->mod.feats.dx11_1_double_extensions = true;
      break;
   case DXIL_CAST_FPTOUI:
   case DXIL_CAST_FPTOSI:
      if (is_double(info->input_types[0], nir_src_bit_size(alu->src[0].src)))
         ctx->mod.feats.dx11_1_double_extensions = true;
      break;
   default:
      break;
   }

   const struct dxil_value *v = dxil_emit_cast(&ctx->mod, opcode, type,
                                               value);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static enum overload_type
get_overload(nir_alu_type alu_type, unsigned bit_size)
{
   switch (nir_alu_type_get_base_type(alu_type)) {
   case nir_type_int:
   case nir_type_uint:
      switch (bit_size) {
      case 16: return DXIL_I16;
      case 32: return DXIL_I32;
      case 64: return DXIL_I64;
      default:
         unreachable("unexpected bit_size");
      }
   case nir_type_float:
      switch (bit_size) {
      case 16: return DXIL_F16;
      case 32: return DXIL_F32;
      case 64: return DXIL_F64;
      default:
         unreachable("unexpected bit_size");
      }
   default:
      unreachable("unexpected output type");
   }
}

static bool
emit_unary_intin(struct ntd_context *ctx, nir_alu_instr *alu,
                 enum dxil_intr intr, const struct dxil_value *op)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   unsigned src_bits = nir_src_bit_size(alu->src[0].src);
   enum overload_type overload = get_overload(info->input_types[0], src_bits);

   const struct dxil_value *v = emit_unary_call(ctx, overload, intr, op);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_binary_intin(struct ntd_context *ctx, nir_alu_instr *alu,
                  enum dxil_intr intr,
                  const struct dxil_value *op0, const struct dxil_value *op1)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   assert(info->output_type == info->input_types[0]);
   assert(info->output_type == info->input_types[1]);
   unsigned dst_bits = nir_dest_bit_size(alu->dest.dest);
   assert(nir_src_bit_size(alu->src[0].src) == dst_bits);
   assert(nir_src_bit_size(alu->src[1].src) == dst_bits);
   enum overload_type overload = get_overload(info->output_type, dst_bits);

   const struct dxil_value *v = emit_binary_call(ctx, overload, intr,
                                                 op0, op1);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_tertiary_intin(struct ntd_context *ctx, nir_alu_instr *alu,
                    enum dxil_intr intr,
                    const struct dxil_value *op0,
                    const struct dxil_value *op1,
                    const struct dxil_value *op2)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   assert(info->output_type == info->input_types[0]);
   assert(info->output_type == info->input_types[1]);
   assert(info->output_type == info->input_types[2]);

   unsigned dst_bits = nir_dest_bit_size(alu->dest.dest);
   assert(nir_src_bit_size(alu->src[0].src) == dst_bits);
   assert(nir_src_bit_size(alu->src[1].src) == dst_bits);
   assert(nir_src_bit_size(alu->src[2].src) == dst_bits);

   enum overload_type overload = get_overload(info->output_type, dst_bits);

   const struct dxil_value *v = emit_tertiary_call(ctx, overload, intr,
                                                   op0, op1, op2);
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool emit_select(struct ntd_context *ctx, nir_alu_instr *alu,
                        const struct dxil_value *sel,
                        const struct dxil_value *val_true,
                        const struct dxil_value *val_false)
{
   assert(sel);
   assert(val_true);
   assert(val_false);

   const struct dxil_value *v = dxil_emit_select(&ctx->mod, sel, val_true, val_false);
   if (!v)
      return false;

   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_b2f16(struct ntd_context *ctx, nir_alu_instr *alu, const struct dxil_value *val)
{
   assert(val);

   struct dxil_module *m = &ctx->mod;

   const struct dxil_value *c1 = dxil_module_get_float16_const(m, 0x3C00);
   const struct dxil_value *c0 = dxil_module_get_float16_const(m, 0);

   if (!c0 || !c1)
      return false;

   return emit_select(ctx, alu, val, c1, c0);
}

static bool
emit_b2f32(struct ntd_context *ctx, nir_alu_instr *alu, const struct dxil_value *val)
{
   assert(val);

   struct dxil_module *m = &ctx->mod;

   const struct dxil_value *c1 = dxil_module_get_float_const(m, 1.0f);
   const struct dxil_value *c0 = dxil_module_get_float_const(m, 0.0f);

   if (!c0 || !c1)
      return false;

   return emit_select(ctx, alu, val, c1, c0);
}

static bool
emit_f2b32(struct ntd_context *ctx, nir_alu_instr *alu, const struct dxil_value *val)
{
   assert(val);

   const struct dxil_value *zero = dxil_module_get_float_const(&ctx->mod, 0.0f);
   return emit_cmp(ctx, alu, DXIL_FCMP_UNE, val, zero);
}

static bool
emit_ufind_msb(struct ntd_context *ctx, nir_alu_instr *alu,
               const struct dxil_value *val)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   unsigned dst_bits = nir_dest_bit_size(alu->dest.dest);
   unsigned src_bits = nir_src_bit_size(alu->src[0].src);
   enum overload_type overload = get_overload(info->output_type, src_bits);

   const struct dxil_value *v = emit_unary_call(ctx, overload,
                                                DXIL_INTR_FIRSTBIT_HI, val);
   if (!v)
      return false;

   const struct dxil_value *size = dxil_module_get_int32_const(&ctx->mod,
      src_bits - 1);
   const struct dxil_value *zero = dxil_module_get_int_const(&ctx->mod, 0,
                                                             src_bits);
   if (!size || !zero)
      return false;

   v = dxil_emit_binop(&ctx->mod, DXIL_BINOP_SUB, size, v, 0);
   const struct dxil_value *cnd = dxil_emit_cmp(&ctx->mod, DXIL_ICMP_NE,
                                                val, zero);
   if (!v || !cnd)
      return false;

   const struct dxil_value *minus_one =
      dxil_module_get_int_const(&ctx->mod, -1, dst_bits);
   if (!minus_one)
      return false;

   v = dxil_emit_select(&ctx->mod, cnd, v, minus_one);
   if (!v)
      return false;

   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_f16tof32(struct ntd_context *ctx, nir_alu_instr *alu, const struct dxil_value *val)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod,
                                                    "dx.op.legacyF16ToF32",
                                                    DXIL_NONE);
   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_LEGACY_F16TOF32);
   if (!opcode)
      return false;

   const struct dxil_value *args[] = {
     opcode,
     val
   };

   const struct dxil_value *v = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_f32tof16(struct ntd_context *ctx, nir_alu_instr *alu, const struct dxil_value *val)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod,
                                                    "dx.op.legacyF32ToF16",
                                                    DXIL_NONE);
   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_LEGACY_F32TOF16);
   if (!opcode)
      return false;

   const struct dxil_value *args[] = {
     opcode,
     val
   };

   const struct dxil_value *v = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   if (!v)
      return false;
   store_alu_dest(ctx, alu, 0, v);
   return true;
}

static bool
emit_vec(struct ntd_context *ctx, nir_alu_instr *alu, unsigned num_inputs)
{
   const struct dxil_type *type = get_alu_src_type(ctx, alu, 0);
   nir_alu_type t = dxil_type_to_nir_type(type);

   for (unsigned i = 0; i < num_inputs; i++) {
      const struct dxil_value *src =
         get_src(ctx, &alu->src[i].src, alu->src[i].swizzle[0], t);
      if (!src)
         return false;

      store_alu_dest(ctx, alu, i, src);
   }
   return true;
}

static bool
emit_make_double(struct ntd_context *ctx, nir_alu_instr *alu)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.makeDouble", DXIL_F64);
   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_MAKE_DOUBLE);
   if (!opcode)
      return false;

   const struct dxil_value *args[3] = {
      opcode,
      get_src(ctx, &alu->src[0].src, 0, nir_type_uint32),
      get_src(ctx, &alu->src[0].src, 1, nir_type_uint32),
   };
   if (!args[1] || !args[2])
      return false;

   const struct dxil_value *v = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   if (!v)
      return false;
   store_dest(ctx, &alu->dest.dest, 0, v, nir_type_float64);
   return true;
}

static bool
emit_split_double(struct ntd_context *ctx, nir_alu_instr *alu)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.splitDouble", DXIL_F64);
   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_SPLIT_DOUBLE);
   if (!opcode)
      return false;

   const struct dxil_value *args[] = {
      opcode,
      get_src(ctx, &alu->src[0].src, 0, nir_type_float64)
   };
   if (!args[1])
      return false;

   const struct dxil_value *v = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   if (!v)
      return false;

   const struct dxil_value *hi = dxil_emit_extractval(&ctx->mod, v, 0);
   const struct dxil_value *lo = dxil_emit_extractval(&ctx->mod, v, 1);
   if (!hi || !lo)
      return false;

   store_dest_value(ctx, &alu->dest.dest, 0, hi);
   store_dest_value(ctx, &alu->dest.dest, 1, lo);
   return true;
}

static bool
emit_alu(struct ntd_context *ctx, nir_alu_instr *alu)
{
   /* handle vec-instructions first; they are the only ones that produce
    * vector results.
    */
   switch (alu->op) {
   case nir_op_vec2:
   case nir_op_vec3:
   case nir_op_vec4:
   case nir_op_vec8:
   case nir_op_vec16:
      return emit_vec(ctx, alu, nir_op_infos[alu->op].num_inputs);
   case nir_op_mov: {
         assert(nir_dest_num_components(alu->dest.dest) == 1);
         store_ssa_def(ctx, &alu->dest.dest.ssa, 0, get_src_ssa(ctx,
                        alu->src->src.ssa, alu->src->swizzle[0]));
         return true;
      }
   case nir_op_pack_double_2x32_dxil:
      return emit_make_double(ctx, alu);
   case nir_op_unpack_double_2x32_dxil:
      return emit_split_double(ctx, alu);
   default:
      /* silence warnings */
      ;
   }

   /* other ops should be scalar */
   assert(alu->dest.write_mask == 1);
   const struct dxil_value *src[4];
   assert(nir_op_infos[alu->op].num_inputs <= 4);
   for (unsigned i = 0; i < nir_op_infos[alu->op].num_inputs; i++) {
      src[i] = get_alu_src(ctx, alu, i);
      if (!src[i])
         return false;
   }

   switch (alu->op) {
   case nir_op_iadd:
   case nir_op_fadd: return emit_binop(ctx, alu, DXIL_BINOP_ADD, src[0], src[1]);

   case nir_op_isub:
   case nir_op_fsub: return emit_binop(ctx, alu, DXIL_BINOP_SUB, src[0], src[1]);

   case nir_op_imul:
   case nir_op_fmul: return emit_binop(ctx, alu, DXIL_BINOP_MUL, src[0], src[1]);

   case nir_op_idiv:
   case nir_op_fdiv: return emit_binop(ctx, alu, DXIL_BINOP_SDIV, src[0], src[1]);

   case nir_op_udiv: return emit_binop(ctx, alu, DXIL_BINOP_UDIV, src[0], src[1]);
   case nir_op_irem: return emit_binop(ctx, alu, DXIL_BINOP_SREM, src[0], src[1]);
   case nir_op_imod: return emit_binop(ctx, alu, DXIL_BINOP_UREM, src[0], src[1]);
   case nir_op_umod: return emit_binop(ctx, alu, DXIL_BINOP_UREM, src[0], src[1]);
   case nir_op_ishl: return emit_shift(ctx, alu, DXIL_BINOP_SHL, src[0], src[1]);
   case nir_op_ishr: return emit_shift(ctx, alu, DXIL_BINOP_ASHR, src[0], src[1]);
   case nir_op_ushr: return emit_shift(ctx, alu, DXIL_BINOP_LSHR, src[0], src[1]);
   case nir_op_iand: return emit_binop(ctx, alu, DXIL_BINOP_AND, src[0], src[1]);
   case nir_op_ior:  return emit_binop(ctx, alu, DXIL_BINOP_OR, src[0], src[1]);
   case nir_op_ixor: return emit_binop(ctx, alu, DXIL_BINOP_XOR, src[0], src[1]);
   case nir_op_inot: {
      unsigned bit_size = alu->dest.dest.ssa.bit_size;
      intmax_t val = bit_size == 1 ? 1 : -1;
      const struct dxil_value *negative_one = dxil_module_get_int_const(&ctx->mod, val, bit_size);
      return emit_binop(ctx, alu, DXIL_BINOP_XOR, src[0], negative_one);
   }
   case nir_op_ieq:  return emit_cmp(ctx, alu, DXIL_ICMP_EQ, src[0], src[1]);
   case nir_op_ine:  return emit_cmp(ctx, alu, DXIL_ICMP_NE, src[0], src[1]);
   case nir_op_ige:  return emit_cmp(ctx, alu, DXIL_ICMP_SGE, src[0], src[1]);
   case nir_op_uge:  return emit_cmp(ctx, alu, DXIL_ICMP_UGE, src[0], src[1]);
   case nir_op_ilt:  return emit_cmp(ctx, alu, DXIL_ICMP_SLT, src[0], src[1]);
   case nir_op_ult:  return emit_cmp(ctx, alu, DXIL_ICMP_ULT, src[0], src[1]);
   case nir_op_feq:  return emit_cmp(ctx, alu, DXIL_FCMP_OEQ, src[0], src[1]);
   case nir_op_fneu: return emit_cmp(ctx, alu, DXIL_FCMP_UNE, src[0], src[1]);
   case nir_op_flt:  return emit_cmp(ctx, alu, DXIL_FCMP_OLT, src[0], src[1]);
   case nir_op_fge:  return emit_cmp(ctx, alu, DXIL_FCMP_OGE, src[0], src[1]);
   case nir_op_bcsel: return emit_select(ctx, alu, src[0], src[1], src[2]);
   case nir_op_ftrunc: return emit_unary_intin(ctx, alu, DXIL_INTR_ROUND_Z, src[0]);
   case nir_op_fabs: return emit_unary_intin(ctx, alu, DXIL_INTR_FABS, src[0]);
   case nir_op_fcos: return emit_unary_intin(ctx, alu, DXIL_INTR_FCOS, src[0]);
   case nir_op_fsin: return emit_unary_intin(ctx, alu, DXIL_INTR_FSIN, src[0]);
   case nir_op_fceil: return emit_unary_intin(ctx, alu, DXIL_INTR_ROUND_PI, src[0]);
   case nir_op_fexp2: return emit_unary_intin(ctx, alu, DXIL_INTR_FEXP2, src[0]);
   case nir_op_flog2: return emit_unary_intin(ctx, alu, DXIL_INTR_FLOG2, src[0]);
   case nir_op_ffloor: return emit_unary_intin(ctx, alu, DXIL_INTR_ROUND_NI, src[0]);
   case nir_op_ffract: return emit_unary_intin(ctx, alu, DXIL_INTR_FRC, src[0]);
   case nir_op_fisnormal: return emit_unary_intin(ctx, alu, DXIL_INTR_ISNORMAL, src[0]);
   case nir_op_fisfinite: return emit_unary_intin(ctx, alu, DXIL_INTR_ISFINITE, src[0]);

   case nir_op_fddx:
   case nir_op_fddx_coarse: return emit_unary_intin(ctx, alu, DXIL_INTR_DDX_COARSE, src[0]);
   case nir_op_fddx_fine: return emit_unary_intin(ctx, alu, DXIL_INTR_DDX_FINE, src[0]);
   case nir_op_fddy:
   case nir_op_fddy_coarse: return emit_unary_intin(ctx, alu, DXIL_INTR_DDY_COARSE, src[0]);
   case nir_op_fddy_fine: return emit_unary_intin(ctx, alu, DXIL_INTR_DDY_FINE, src[0]);

   case nir_op_fround_even: return emit_unary_intin(ctx, alu, DXIL_INTR_ROUND_NE, src[0]);
   case nir_op_frcp: {
         const struct dxil_value *one = dxil_module_get_float_const(&ctx->mod, 1.0f);
         return emit_binop(ctx, alu, DXIL_BINOP_SDIV, one, src[0]);
      }
   case nir_op_fsat: return emit_unary_intin(ctx, alu, DXIL_INTR_SATURATE, src[0]);
   case nir_op_bit_count: return emit_unary_intin(ctx, alu, DXIL_INTR_COUNTBITS, src[0]);
   case nir_op_ufind_msb: return emit_ufind_msb(ctx, alu, src[0]);
   case nir_op_imax: return emit_binary_intin(ctx, alu, DXIL_INTR_IMAX, src[0], src[1]);
   case nir_op_imin: return emit_binary_intin(ctx, alu, DXIL_INTR_IMIN, src[0], src[1]);
   case nir_op_umax: return emit_binary_intin(ctx, alu, DXIL_INTR_UMAX, src[0], src[1]);
   case nir_op_umin: return emit_binary_intin(ctx, alu, DXIL_INTR_UMIN, src[0], src[1]);
   case nir_op_frsq: return emit_unary_intin(ctx, alu, DXIL_INTR_RSQRT, src[0]);
   case nir_op_fsqrt: return emit_unary_intin(ctx, alu, DXIL_INTR_SQRT, src[0]);
   case nir_op_fmax: return emit_binary_intin(ctx, alu, DXIL_INTR_FMAX, src[0], src[1]);
   case nir_op_fmin: return emit_binary_intin(ctx, alu, DXIL_INTR_FMIN, src[0], src[1]);
   case nir_op_ffma: return emit_tertiary_intin(ctx, alu, DXIL_INTR_FMA, src[0], src[1], src[2]);

   case nir_op_unpack_half_2x16_split_x: return emit_f16tof32(ctx, alu, src[0]);
   case nir_op_pack_half_2x16_split: return emit_f32tof16(ctx, alu, src[0]);

   case nir_op_b2i16:
   case nir_op_i2i16:
   case nir_op_f2i16:
   case nir_op_f2u16:
   case nir_op_u2u16:
   case nir_op_u2f16:
   case nir_op_i2f16:
   case nir_op_f2f16_rtz:
   case nir_op_b2i32:
   case nir_op_f2f32:
   case nir_op_f2i32:
   case nir_op_f2u32:
   case nir_op_i2f32:
   case nir_op_i2i32:
   case nir_op_u2f32:
   case nir_op_u2u32:
   case nir_op_b2i64:
   case nir_op_f2f64:
   case nir_op_f2i64:
   case nir_op_f2u64:
   case nir_op_i2f64:
   case nir_op_i2i64:
   case nir_op_u2f64:
   case nir_op_u2u64:
      return emit_cast(ctx, alu, src[0]);

   case nir_op_f2b32: return emit_f2b32(ctx, alu, src[0]);
   case nir_op_b2f16: return emit_b2f16(ctx, alu, src[0]);
   case nir_op_b2f32: return emit_b2f32(ctx, alu, src[0]);
   default:
      NIR_INSTR_UNSUPPORTED(&alu->instr);
      assert("Unimplemented ALU instruction");
      return false;
   }
}

static const struct dxil_value *
load_ubo(struct ntd_context *ctx, const struct dxil_value *handle,
         const struct dxil_value *offset, enum overload_type overload)
{
   assert(handle && offset);

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_CBUFFER_LOAD_LEGACY);
   if (!opcode)
      return NULL;

   const struct dxil_value *args[] = {
      opcode, handle, offset
   };

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.cbufferLoadLegacy", overload);
   if (!func)
      return NULL;
   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_barrier(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *opcode, *mode;
   const struct dxil_func *func;
   uint32_t flags = 0;

   if (nir_intrinsic_execution_scope(intr) == NIR_SCOPE_WORKGROUP)
      flags |= DXIL_BARRIER_MODE_SYNC_THREAD_GROUP;

   nir_variable_mode modes = nir_intrinsic_memory_modes(intr);
   nir_scope mem_scope = nir_intrinsic_memory_scope(intr);

   /* Currently vtn uses uniform to indicate image memory, which DXIL considers global */
   if (modes & nir_var_uniform)
      modes |= nir_var_mem_global;

   if (modes & (nir_var_mem_ssbo | nir_var_mem_global)) {
      if (mem_scope > NIR_SCOPE_WORKGROUP)
         flags |= DXIL_BARRIER_MODE_UAV_FENCE_GLOBAL;
      else
         flags |= DXIL_BARRIER_MODE_UAV_FENCE_THREAD_GROUP;
   }

   if (modes & nir_var_mem_shared)
      flags |= DXIL_BARRIER_MODE_GROUPSHARED_MEM_FENCE;

   func = dxil_get_function(&ctx->mod, "dx.op.barrier", DXIL_NONE);
   if (!func)
      return false;

   opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_BARRIER);
   if (!opcode)
      return false;

   mode = dxil_module_get_int32_const(&ctx->mod, flags);
   if (!mode)
      return false;

   const struct dxil_value *args[] = { opcode, mode };

   return dxil_emit_call_void(&ctx->mod, func,
                              args, ARRAY_SIZE(args));
}

static bool
emit_load_global_invocation_id(struct ntd_context *ctx,
                                    nir_intrinsic_instr *intr)
{
   assert(intr->dest.is_ssa);
   nir_component_mask_t comps = nir_ssa_def_components_read(&intr->dest.ssa);

   for (int i = 0; i < nir_intrinsic_dest_components(intr); i++) {
      if (comps & (1 << i)) {
         const struct dxil_value *idx = dxil_module_get_int32_const(&ctx->mod, i);
         if (!idx)
            return false;
         const struct dxil_value *globalid = emit_threadid_call(ctx, idx);

         if (!globalid)
            return false;

         store_dest_value(ctx, &intr->dest, i, globalid);
      }
   }
   return true;
}

static bool
emit_load_local_invocation_id(struct ntd_context *ctx,
                              nir_intrinsic_instr *intr)
{
   assert(intr->dest.is_ssa);
   nir_component_mask_t comps = nir_ssa_def_components_read(&intr->dest.ssa);

   for (int i = 0; i < nir_intrinsic_dest_components(intr); i++) {
      if (comps & (1 << i)) {
         const struct dxil_value
            *idx = dxil_module_get_int32_const(&ctx->mod, i);
         if (!idx)
            return false;
         const struct dxil_value
            *threadidingroup = emit_threadidingroup_call(ctx, idx);
         if (!threadidingroup)
            return false;
         store_dest_value(ctx, &intr->dest, i, threadidingroup);
      }
   }
   return true;
}

static bool
emit_load_local_invocation_index(struct ntd_context *ctx,
                                 nir_intrinsic_instr *intr)
{
   assert(intr->dest.is_ssa);

   const struct dxil_value
      *flattenedthreadidingroup = emit_flattenedthreadidingroup_call(ctx);
   if (!flattenedthreadidingroup)
      return false;
   store_dest_value(ctx, &intr->dest, 0, flattenedthreadidingroup);
   
   return true;
}

static bool
emit_load_local_workgroup_id(struct ntd_context *ctx,
                              nir_intrinsic_instr *intr)
{
   assert(intr->dest.is_ssa);
   nir_component_mask_t comps = nir_ssa_def_components_read(&intr->dest.ssa);

   for (int i = 0; i < nir_intrinsic_dest_components(intr); i++) {
      if (comps & (1 << i)) {
         const struct dxil_value *idx = dxil_module_get_int32_const(&ctx->mod, i);
         if (!idx)
            return false;
         const struct dxil_value *groupid = emit_groupid_call(ctx, idx);
         if (!groupid)
            return false;
         store_dest_value(ctx, &intr->dest, i, groupid);
      }
   }
   return true;
}

static bool
emit_load_unary_external_function(struct ntd_context *ctx,
                                  nir_intrinsic_instr *intr, const char *name,
                                  int32_t dxil_intr)
{
   const struct dxil_func *func =
      dxil_get_function(&ctx->mod, name, DXIL_I32);
   if (!func)
      return false;

   const struct dxil_value *opcode =
      dxil_module_get_int32_const(&ctx->mod, dxil_intr);
   if (!opcode)
      return false;

   const struct dxil_value *args[] = {opcode};

   const struct dxil_value *value =
      dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   store_dest_value(ctx, &intr->dest, 0, value);

   return true;
}

static const struct dxil_value *
get_int32_undef(struct dxil_module *m)
{
   const struct dxil_type *int32_type =
      dxil_module_get_int_type(m, 32);
   if (!int32_type)
      return NULL;

   return dxil_module_get_undef(m, int32_type);
}

static const struct dxil_value *
emit_gep_for_index(struct ntd_context *ctx, const nir_variable *var,
                   const struct dxil_value *index)
{
   assert(var->data.mode == nir_var_shader_temp);

   struct hash_entry *he = _mesa_hash_table_search(ctx->consts, var);
   assert(he != NULL);
   const struct dxil_value *ptr = he->data;

   const struct dxil_value *zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return NULL;

   const struct dxil_value *ops[] = { ptr, zero, index };
   return dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
}

static const struct dxil_value *
get_ubo_ssbo_handle(struct ntd_context *ctx, nir_src *src, enum dxil_resource_class class, unsigned base_binding)
{
   /* This source might be one of:
    * 1. Constant resource index - just look it up in precomputed handle arrays
    *    If it's null in that array, create a handle, and store the result
    * 2. A handle from load_vulkan_descriptor - just get the stored SSA value
    * 3. Dynamic resource index - create a handle for it here
    */
   assert(src->ssa->num_components == 1 && src->ssa->bit_size == 32);
   nir_const_value *const_block_index = nir_src_as_const_value(*src);
   const struct dxil_value **handle_entry = NULL;
   if (const_block_index) {
      assert(!ctx->opts->vulkan_environment);
      switch (class) {
      case DXIL_RESOURCE_CLASS_CBV:
         handle_entry = &ctx->cbv_handles[const_block_index->u32];
         break;
      case DXIL_RESOURCE_CLASS_UAV:
         handle_entry = &ctx->uav_handles[const_block_index->u32];
         break;
      case DXIL_RESOURCE_CLASS_SRV:
         handle_entry = &ctx->srv_handles[const_block_index->u32];
         break;
      default:
         unreachable("Unexpected resource class");
      }
   }

   if (handle_entry && *handle_entry)
      return *handle_entry;

   const struct dxil_value *value = get_src_ssa(ctx, src->ssa, 0);
   if (ctx->opts->vulkan_environment) {
      return value;
   }

   const struct dxil_value *handle = emit_createhandle_call(ctx, class, 
      get_resource_id(ctx, class, 0, base_binding), value, !const_block_index);
   if (handle_entry)
      *handle_entry = handle;

   return handle;
}

static bool
emit_load_ssbo(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);

   nir_variable *var = nir_get_binding_variable(ctx->shader, nir_chase_binding(intr->src[0]));
   enum dxil_resource_class class = DXIL_RESOURCE_CLASS_UAV;
   if (var && var->data.access & ACCESS_NON_WRITEABLE)
      class = DXIL_RESOURCE_CLASS_SRV;

   const struct dxil_value *handle = get_ubo_ssbo_handle(ctx, &intr->src[0], class, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);
   if (!int32_undef || !handle || !offset)
      return false;

   assert(nir_src_bit_size(intr->src[0]) == 32);
   assert(nir_intrinsic_dest_components(intr) <= 4);

   const struct dxil_value *coord[2] = {
      offset,
      int32_undef
   };

   const struct dxil_value *load = emit_bufferload_call(ctx, handle, coord, DXIL_I32);
   if (!load)
      return false;

   for (int i = 0; i < nir_intrinsic_dest_components(intr); i++) {
      const struct dxil_value *val =
         dxil_emit_extractval(&ctx->mod, load, i);
      if (!val)
         return false;
      store_dest_value(ctx, &intr->dest, i, val);
   }
   return true;
}

static bool
emit_store_ssbo(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[1], DXIL_RESOURCE_CLASS_UAV, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[2], 0, nir_type_uint);
   if (!handle || !offset)
      return false;

   assert(nir_src_bit_size(intr->src[0]) == 32);
   unsigned num_components = nir_src_num_components(intr->src[0]);
   assert(num_components <= 4);
   const struct dxil_value *value[4];
   for (unsigned i = 0; i < num_components; ++i) {
      value[i] = get_src(ctx, &intr->src[0], i, nir_type_uint);
      if (!value[i])
         return false;
   }

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[2] = {
      offset,
      int32_undef
   };

   for (int i = num_components; i < 4; ++i)
      value[i] = int32_undef;

   const struct dxil_value *write_mask =
      dxil_module_get_int8_const(&ctx->mod, (1u << num_components) - 1);
   if (!write_mask)
      return false;

   return emit_bufferstore_call(ctx, handle, coord, value, write_mask, DXIL_I32);
}

static bool
emit_store_ssbo_masked(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *value =
      get_src(ctx, &intr->src[0], 0, nir_type_uint);
   const struct dxil_value *mask =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);
   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[2], DXIL_RESOURCE_CLASS_UAV, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[3], 0, nir_type_uint);
   if (!value || !mask || !handle || !offset)
      return false;

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[3] = {
      offset, int32_undef, int32_undef
   };

   return
      emit_atomic_binop(ctx, handle, DXIL_ATOMIC_AND, coord, mask) != NULL &&
      emit_atomic_binop(ctx, handle, DXIL_ATOMIC_OR, coord, value) != NULL;
}

static bool
emit_store_shared(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *zero, *index;

   /* All shared mem accesses should have been lowered to scalar 32bit
    * accesses.
    */
   assert(nir_src_bit_size(intr->src[0]) == 32);
   assert(nir_src_num_components(intr->src[0]) == 1);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   if (intr->intrinsic == nir_intrinsic_store_shared_dxil)
      index = get_src(ctx, &intr->src[1], 0, nir_type_uint);
   else
      index = get_src(ctx, &intr->src[2], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->sharedvars, zero, index };
   const struct dxil_value *ptr, *value;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   value = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!value)
      return false;

   if (intr->intrinsic == nir_intrinsic_store_shared_dxil)
      return dxil_emit_store(&ctx->mod, value, ptr, 4, false);

   const struct dxil_value *mask = get_src(ctx, &intr->src[1], 0, nir_type_uint);
   if (!mask)
      return false;

   if (!dxil_emit_atomicrmw(&ctx->mod, mask, ptr, DXIL_RMWOP_AND, false,
                            DXIL_ATOMIC_ORDERING_ACQREL,
                            DXIL_SYNC_SCOPE_CROSSTHREAD))
      return false;

   if (!dxil_emit_atomicrmw(&ctx->mod, value, ptr, DXIL_RMWOP_OR, false,
                            DXIL_ATOMIC_ORDERING_ACQREL,
                            DXIL_SYNC_SCOPE_CROSSTHREAD))
      return false;

   return true;
}

static bool
emit_store_scratch(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *zero, *index;

   /* All scratch mem accesses should have been lowered to scalar 32bit
    * accesses.
    */
   assert(nir_src_bit_size(intr->src[0]) == 32);
   assert(nir_src_num_components(intr->src[0]) == 1);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   index = get_src(ctx, &intr->src[1], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->scratchvars, zero, index };
   const struct dxil_value *ptr, *value;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   value = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!value)
      return false;

   return dxil_emit_store(&ctx->mod, value, ptr, 4, false);
}

static bool
emit_load_ubo(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[0], DXIL_RESOURCE_CLASS_CBV, 0);
   if (!handle)
      return false;

   const struct dxil_value *offset;
   nir_const_value *const_offset = nir_src_as_const_value(intr->src[1]);
   if (const_offset) {
      offset = dxil_module_get_int32_const(&ctx->mod, const_offset->i32 >> 4);
   } else {
      const struct dxil_value *offset_src = get_src(ctx, &intr->src[1], 0, nir_type_uint);
      const struct dxil_value *c4 = dxil_module_get_int32_const(&ctx->mod, 4);
      if (!offset_src || !c4)
         return false;

      offset = dxil_emit_binop(&ctx->mod, DXIL_BINOP_ASHR, offset_src, c4, 0);
   }

   const struct dxil_value *agg = load_ubo(ctx, handle, offset, DXIL_F32);

   if (!agg)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); ++i) {
      const struct dxil_value *retval = dxil_emit_extractval(&ctx->mod, agg, i);
      store_dest(ctx, &intr->dest, i, retval,
                 nir_dest_bit_size(intr->dest) > 1 ? nir_type_float : nir_type_bool);
   }
   return true;
}

static bool
emit_load_ubo_dxil(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   assert(nir_dest_num_components(intr->dest) <= 4);
   assert(nir_dest_bit_size(intr->dest) == 32);

   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[0], DXIL_RESOURCE_CLASS_CBV, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);

   if (!handle || !offset)
      return false;

   const struct dxil_value *agg = load_ubo(ctx, handle, offset, DXIL_I32);
   if (!agg)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); i++)
      store_dest_value(ctx, &intr->dest, i,
                       dxil_emit_extractval(&ctx->mod, agg, i));

   return true;
}

static bool
emit_store_output(struct ntd_context *ctx, nir_intrinsic_instr *intr,
                  nir_variable *output)
{
   nir_alu_type out_type = nir_get_nir_type_for_glsl_base_type(glsl_get_base_type(output->type));
   enum overload_type overload = DXIL_F32;
   if (output->data.compact)
      out_type = nir_type_float;
   else
      overload = get_overload(out_type, glsl_get_bit_size(output->type));
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.storeOutput", overload);

   if (!func)
      return false;

   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_STORE_OUTPUT);
   const struct dxil_value *output_id = dxil_module_get_int32_const(&ctx->mod, (int)output->data.driver_location);
   const struct dxil_value *row = dxil_module_get_int32_const(&ctx->mod, 0);

   bool success = true;
   if (output->data.compact) {
      nir_deref_instr *array_deref = nir_instr_as_deref(intr->src[0].ssa->parent_instr);
      unsigned array_index = nir_src_as_uint(array_deref->arr.index);

      const struct dxil_value *col = dxil_module_get_int8_const(&ctx->mod, array_index);
      const struct dxil_value *value = get_src(ctx, &intr->src[1], 0, out_type);
      if (!col || !value)
         return false;

      const struct dxil_value *args[] = {
         opcode, output_id, row, col, value
      };
      success = dxil_emit_call_void(&ctx->mod, func, args, ARRAY_SIZE(args));
   } else {
      uint32_t writemask = nir_intrinsic_write_mask(intr);
      for (unsigned i = 0; i < nir_src_num_components(intr->src[1]) && success; ++i) {
         if (writemask & (1 << i)) {
            const struct dxil_value *col = dxil_module_get_int8_const(&ctx->mod, i);
            const struct dxil_value *value = get_src(ctx, &intr->src[1], i, out_type);
            if (!col || !value)
               return false;

            const struct dxil_value *args[] = {
               opcode, output_id, row, col, value
            };
            success &= dxil_emit_call_void(&ctx->mod, func, args, ARRAY_SIZE(args));
         }
      }
   }
   return success;
}

static bool
emit_store_deref(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   nir_variable *var = nir_deref_instr_get_variable(deref);

   switch (var->data.mode) {
   case nir_var_shader_out:
      return emit_store_output(ctx, intr, var);

   default:
      unreachable("unsupported nir_variable_mode");
   }
}

static bool
emit_load_input_array(struct ntd_context *ctx, nir_intrinsic_instr *intr, nir_variable *var, nir_src *index)
{
   assert(var);
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_LOAD_INPUT);
   const struct dxil_value *input_id = dxil_module_get_int32_const(&ctx->mod, var->data.driver_location);
   const struct dxil_value *vertex_id;
   const struct dxil_value *row;

   if (ctx->mod.shader_kind == DXIL_GEOMETRY_SHADER) {
      vertex_id = get_src(ctx, index, 0, nir_type_int);
      row = dxil_module_get_int32_const(&ctx->mod, 0);
   } else {
      const struct dxil_type *int32_type = dxil_module_get_int_type(&ctx->mod, 32);
      vertex_id = dxil_module_get_undef(&ctx->mod, int32_type);
      row = get_src(ctx, index, 0, nir_type_int);
   }

   if (!opcode || !input_id || !vertex_id || !row)
      return false;

   nir_alu_type out_type = nir_get_nir_type_for_glsl_base_type(glsl_get_base_type(glsl_get_array_element(var->type)));
   enum overload_type overload = get_overload(out_type, glsl_get_bit_size(glsl_get_array_element(var->type)));

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.loadInput", overload);

   if (!func)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); ++i) {
      const struct dxil_value *comp = dxil_module_get_int8_const(&ctx->mod, i);
      if (!comp)
         return false;

      const struct dxil_value *args[] = {
         opcode, input_id, row, comp, vertex_id
      };

      const struct dxil_value *retval = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
      if (!retval)
         return false;
      store_dest(ctx, &intr->dest, i, retval, out_type);
   }
   return true;
}

static bool
emit_load_compact_input_array(struct ntd_context *ctx, nir_intrinsic_instr *intr, nir_variable *var, nir_deref_instr *deref)
{
   assert(var);
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_LOAD_INPUT);
   const struct dxil_value *input_id = dxil_module_get_int32_const(&ctx->mod, var->data.driver_location);
   const struct dxil_value *row = dxil_module_get_int32_const(&ctx->mod, 0);
   const struct dxil_value *vertex_id;

   nir_src *col = &deref->arr.index;
   nir_src_is_const(*col);

   if (ctx->mod.shader_kind == DXIL_GEOMETRY_SHADER) {
      nir_deref_instr *deref_parent = nir_deref_instr_parent(deref);
      assert(deref_parent->deref_type == nir_deref_type_array);

      vertex_id = get_src(ctx, &deref_parent->arr.index, 0, nir_type_int);
   } else {
      const struct dxil_type *int32_type = dxil_module_get_int_type(&ctx->mod, 32);
      vertex_id = dxil_module_get_undef(&ctx->mod, int32_type);
   }

   if (!opcode || !input_id || !row || !vertex_id)
      return false;

   nir_alu_type out_type = nir_type_float;
   enum overload_type overload = get_overload(out_type, 32);

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.loadInput", overload);

   if (!func)
      return false;

   const struct dxil_value *comp = dxil_module_get_int8_const(&ctx->mod, nir_src_as_int(*col));
   if (!comp)
      return false;

   const struct dxil_value *args[] = {
      opcode, input_id, row, comp, vertex_id
   };

   const struct dxil_value *retval = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
   if (!retval)
      return false;
   store_dest(ctx, &intr->dest, 0, retval, out_type);
   return true;
}

static bool
emit_load_input_interpolated(struct ntd_context *ctx, nir_intrinsic_instr *intr, nir_variable *var)
{
   assert(var);
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_LOAD_INPUT);
   const struct dxil_value *input_id = dxil_module_get_int32_const(&ctx->mod, var->data.driver_location);
   const struct dxil_value *row = dxil_module_get_int32_const(&ctx->mod, 0);
   const struct dxil_type *int32_type = dxil_module_get_int_type(&ctx->mod, 32);
   const struct dxil_value *vertex_id = dxil_module_get_undef(&ctx->mod, int32_type);

   if (!opcode || !input_id || !row || !int32_type || !vertex_id)
      return false;

   nir_alu_type out_type = nir_get_nir_type_for_glsl_base_type(glsl_get_base_type(var->type));
   enum overload_type overload = get_overload(out_type, glsl_get_bit_size(var->type));

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.loadInput", overload);

   if (!func)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); ++i) {
      const struct dxil_value *comp = dxil_module_get_int8_const(&ctx->mod, i);

      const struct dxil_value *args[] = {
         opcode, input_id, row, comp, vertex_id
      };

      const struct dxil_value *retval = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
      if (!retval)
         return false;
      store_dest(ctx, &intr->dest, i, retval, out_type);
   }
   return true;
}

static bool
emit_load_input_flat(struct ntd_context *ctx, nir_intrinsic_instr *intr, nir_variable* var)
{
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_ATTRIBUTE_AT_VERTEX);
   const struct dxil_value *input_id = dxil_module_get_int32_const(&ctx->mod, (int)var->data.driver_location);
   const struct dxil_value *row = dxil_module_get_int32_const(&ctx->mod, 0);
   const struct dxil_value *vertex_id = dxil_module_get_int8_const(&ctx->mod, ctx->opts->provoking_vertex);

   nir_alu_type out_type = nir_get_nir_type_for_glsl_base_type(glsl_get_base_type(var->type));
   enum overload_type overload = get_overload(out_type, glsl_get_bit_size(var->type));

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.attributeAtVertex", overload);
   if (!func)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); ++i) {
      const struct dxil_value *comp = dxil_module_get_int8_const(&ctx->mod, i);
      const struct dxil_value *args[] = {
         opcode, input_id, row, comp, vertex_id
      };

      const struct dxil_value *retval = dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
      if (!retval)
         return false;

      store_dest(ctx, &intr->dest, i, retval, out_type);
   }
   return true;
}

static bool
emit_load_input(struct ntd_context *ctx, nir_intrinsic_instr *intr,
                nir_variable *input)
{
   if (ctx->mod.shader_kind != DXIL_PIXEL_SHADER ||
       input->data.interpolation != INTERP_MODE_FLAT ||
       !ctx->opts->interpolate_at_vertex ||
       ctx->opts->provoking_vertex == 0 ||
       glsl_type_is_integer(input->type))
      return emit_load_input_interpolated(ctx, intr, input);
   else
      return emit_load_input_flat(ctx, intr, input);
}

static bool
emit_load_ptr(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   struct nir_variable *var =
      nir_deref_instr_get_variable(nir_src_as_deref(intr->src[0]));

   const struct dxil_value *index =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ptr = emit_gep_for_index(ctx, var, index);
   if (!ptr)
      return false;

   const struct dxil_value *retval =
      dxil_emit_load(&ctx->mod, ptr, 4, false);
   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, nir_type_uint);
   return true;
}

static bool
emit_load_shared(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *zero, *index;
   unsigned bit_size = nir_dest_bit_size(intr->dest);
   unsigned align = bit_size / 8;

   /* All shared mem accesses should have been lowered to scalar 32bit
    * accesses.
    */
   assert(bit_size == 32);
   assert(nir_dest_num_components(intr->dest) == 1);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   index = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->sharedvars, zero, index };
   const struct dxil_value *ptr, *retval;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   retval = dxil_emit_load(&ctx->mod, ptr, align, false);
   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, nir_type_uint);
   return true;
}

static bool
emit_load_scratch(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *zero, *index;
   unsigned bit_size = nir_dest_bit_size(intr->dest);
   unsigned align = bit_size / 8;

   /* All scratch mem accesses should have been lowered to scalar 32bit
    * accesses.
    */
   assert(bit_size == 32);
   assert(nir_dest_num_components(intr->dest) == 1);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   index = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->scratchvars, zero, index };
   const struct dxil_value *ptr, *retval;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   retval = dxil_emit_load(&ctx->mod, ptr, align, false);
   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, nir_type_uint);
   return true;
}

static bool
emit_load_deref(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   assert(intr->src[0].is_ssa);
   nir_deref_instr *deref = nir_instr_as_deref(intr->src[0].ssa->parent_instr);
   nir_variable *var = nir_deref_instr_get_variable(deref);

   switch (var->data.mode) {
   case nir_var_shader_in:
      if (glsl_type_is_array(var->type)) {
         if (var->data.compact)
            return emit_load_compact_input_array(ctx, intr, var, deref);
         else
            return emit_load_input_array(ctx, intr, var, &deref->arr.index);
      }
      return emit_load_input(ctx, intr, var);

   default:
      unreachable("unsupported nir_variable_mode");
   }
}

static bool
emit_discard_if_with_value(struct ntd_context *ctx, const struct dxil_value *value)
{
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_DISCARD);
   if (!opcode)
      return false;

   const struct dxil_value *args[] = {
     opcode,
     value
   };

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.discard", DXIL_NONE);
   if (!func)
      return false;

   return dxil_emit_call_void(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_discard_if(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *value = get_src(ctx, &intr->src[0], 0, nir_type_bool);
   if (!value)
      return false;

   return emit_discard_if_with_value(ctx, value);
}

static bool
emit_discard(struct ntd_context *ctx)
{
   const struct dxil_value *value = dxil_module_get_int1_const(&ctx->mod, true);
   return emit_discard_if_with_value(ctx, value);
}

static bool
emit_emit_vertex(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_EMIT_STREAM);
   const struct dxil_value *stream_id = dxil_module_get_int8_const(&ctx->mod, nir_intrinsic_stream_id(intr));
   if (!opcode || !stream_id)
      return false;

   const struct dxil_value *args[] = {
     opcode,
     stream_id
   };

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.emitStream", DXIL_NONE);
   if (!func)
      return false;

   return dxil_emit_call_void(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_end_primitive(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *opcode = dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_CUT_STREAM);
   const struct dxil_value *stream_id = dxil_module_get_int8_const(&ctx->mod, nir_intrinsic_stream_id(intr));
   if (!opcode || !stream_id)
      return false;

   const struct dxil_value *args[] = {
     opcode,
     stream_id
   };

   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.cutStream", DXIL_NONE);
   if (!func)
      return false;

   return dxil_emit_call_void(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_image_store(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *handle;
   bool is_array = false;
   if (ctx->opts->vulkan_environment) {
      assert(intr->intrinsic == nir_intrinsic_image_deref_store);
      handle = get_src_ssa(ctx, intr->src[0].ssa, 0);
      is_array = glsl_sampler_type_is_array(nir_src_as_deref(intr->src[0])->type);
   } else {
      assert(intr->intrinsic == nir_intrinsic_image_store);
      int binding = nir_src_as_int(intr->src[0]);
      is_array = nir_intrinsic_image_array(intr);
      handle = ctx->uav_handles[binding];
   }
   if (!handle)
      return false;

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[3] = { int32_undef, int32_undef, int32_undef };
   enum glsl_sampler_dim image_dim = intr->intrinsic == nir_intrinsic_image_store ?
      nir_intrinsic_image_dim(intr) :
      glsl_get_sampler_dim(nir_src_as_deref(intr->src[0])->type);
   unsigned num_coords = glsl_get_sampler_dim_coordinate_components(image_dim);
   if (is_array)
      ++num_coords;

   assert(num_coords <= nir_src_num_components(intr->src[1]));
   for (unsigned i = 0; i < num_coords; ++i) {
      coord[i] = get_src(ctx, &intr->src[1], i, nir_type_uint);
      if (!coord[i])
         return false;
   }

   nir_alu_type in_type = nir_intrinsic_src_type(intr);
   enum overload_type overload = get_overload(in_type, 32);

   assert(nir_src_bit_size(intr->src[3]) == 32);
   unsigned num_components = nir_src_num_components(intr->src[3]);
   assert(num_components <= 4);
   const struct dxil_value *value[4];
   for (unsigned i = 0; i < num_components; ++i) {
      value[i] = get_src(ctx, &intr->src[3], i, in_type);
      if (!value[i])
         return false;
   }

   for (int i = num_components; i < 4; ++i)
      value[i] = int32_undef;

   const struct dxil_value *write_mask =
      dxil_module_get_int8_const(&ctx->mod, (1u << num_components) - 1);
   if (!write_mask)
      return false;

   if (image_dim == GLSL_SAMPLER_DIM_BUF) {
      coord[1] = int32_undef;
      return emit_bufferstore_call(ctx, handle, coord, value, write_mask, overload);
   } else
      return emit_texturestore_call(ctx, handle, coord, value, write_mask, overload);
}

static bool
emit_image_load(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *handle;
   bool is_array = false;
   if (ctx->opts->vulkan_environment) {
      assert(intr->intrinsic == nir_intrinsic_image_deref_load);
      handle = get_src_ssa(ctx, intr->src[0].ssa, 0);
      is_array = glsl_sampler_type_is_array(nir_src_as_deref(intr->src[0])->type);
   } else {
      assert(intr->intrinsic == nir_intrinsic_image_load);
      int binding = nir_src_as_int(intr->src[0]);
      is_array = nir_intrinsic_image_array(intr);
      handle = ctx->uav_handles[binding];
   }
   if (!handle)
      return false;

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[3] = { int32_undef, int32_undef, int32_undef };
   enum glsl_sampler_dim image_dim = intr->intrinsic == nir_intrinsic_image_load ?
      nir_intrinsic_image_dim(intr) :
      glsl_get_sampler_dim(nir_src_as_deref(intr->src[0])->type);
   unsigned num_coords = glsl_get_sampler_dim_coordinate_components(image_dim);
   if (is_array)
      ++num_coords;

   assert(num_coords <= nir_src_num_components(intr->src[1]));
   for (unsigned i = 0; i < num_coords; ++i) {
      coord[i] = get_src(ctx, &intr->src[1], i, nir_type_uint);
      if (!coord[i])
         return false;
   }

   nir_alu_type out_type = nir_intrinsic_dest_type(intr);
   enum overload_type overload = get_overload(out_type, 32);

   const struct dxil_value *load_result;
   if (image_dim == GLSL_SAMPLER_DIM_BUF) {
      coord[1] = int32_undef;
      load_result = emit_bufferload_call(ctx, handle, coord, overload);
   } else
      load_result = emit_textureload_call(ctx, handle, coord, overload);

   if (!load_result)
      return false;

   assert(nir_dest_bit_size(intr->dest) == 32);
   unsigned num_components = nir_dest_num_components(intr->dest);
   assert(num_components <= 4);
   for (unsigned i = 0; i < num_components; ++i) {
      const struct dxil_value *component = dxil_emit_extractval(&ctx->mod, load_result, i);
      if (!component)
         return false;
      store_dest(ctx, &intr->dest, i, component, out_type);
   }

   if (num_components > 1)
      ctx->mod.feats.typed_uav_load_additional_formats = true;

   return true;
}

struct texop_parameters {
   const struct dxil_value *tex;
   const struct dxil_value *sampler;
   const struct dxil_value *bias, *lod_or_sample, *min_lod;
   const struct dxil_value *coord[4], *offset[3], *dx[3], *dy[3];
   const struct dxil_value *cmp;
   enum overload_type overload;
};

static const struct dxil_value *
emit_texture_size(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.getDimensions", DXIL_NONE);
   if (!func)
      return false;

   const struct dxil_value *args[] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_TEXTURE_SIZE),
      params->tex,
      params->lod_or_sample
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_image_size(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *handle;
   if (ctx->opts->vulkan_environment) {
      assert(intr->intrinsic == nir_intrinsic_image_deref_size);
      handle = get_src_ssa(ctx, intr->src[0].ssa, 0);
   }
   else {
      assert(intr->intrinsic == nir_intrinsic_image_size);
      int binding = nir_src_as_int(intr->src[0]);
      handle = ctx->uav_handles[binding];
   }
   if (!handle)
      return false;

   const struct dxil_value *lod = get_src(ctx, &intr->src[1], 0, nir_type_uint);
   if (!lod)
      return false;

   struct texop_parameters params = {
      .tex = handle,
      .lod_or_sample = lod
   };
   const struct dxil_value *dimensions = emit_texture_size(ctx, &params);
   if (!dimensions)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(intr->dest); ++i) {
      const struct dxil_value *retval = dxil_emit_extractval(&ctx->mod, dimensions, i);
      store_dest(ctx, &intr->dest, i, retval, nir_type_uint);
   }

   return true;
}

static bool
emit_get_ssbo_size(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value* handle = NULL;
   if (ctx->opts->vulkan_environment) {
      handle = get_src_ssa(ctx, intr->src[0].ssa, 0);
   } else {
      int binding = nir_src_as_int(intr->src[0]);
      handle = ctx->uav_handles[binding];
   }
   
   if (!handle)
     return false;

   struct texop_parameters params = {
      .tex = handle,
      .lod_or_sample = dxil_module_get_undef(
                        &ctx->mod, dxil_module_get_int_type(&ctx->mod, 32))
   };

   const struct dxil_value *dimensions = emit_texture_size(ctx, &params);
   if (!dimensions)
      return false;

   const struct dxil_value *retval = dxil_emit_extractval(&ctx->mod, dimensions, 0);
   store_dest(ctx, &intr->dest, 0, retval, nir_type_uint);

   return true;
}

static bool
emit_ssbo_atomic(struct ntd_context *ctx, nir_intrinsic_instr *intr,
                   enum dxil_atomic_op op, nir_alu_type type)
{
   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[0], DXIL_RESOURCE_CLASS_UAV, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);
   const struct dxil_value *value =
      get_src(ctx, &intr->src[2], 0, type);

   if (!value || !handle || !offset)
      return false;

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[3] = {
      offset, int32_undef, int32_undef
   };

   const struct dxil_value *retval =
      emit_atomic_binop(ctx, handle, op, coord, value);

   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, type);
   return true;
}

static bool
emit_ssbo_atomic_comp_swap(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value* handle = get_ubo_ssbo_handle(ctx, &intr->src[0], DXIL_RESOURCE_CLASS_UAV, 0);
   const struct dxil_value *offset =
      get_src(ctx, &intr->src[1], 0, nir_type_uint);
   const struct dxil_value *cmpval =
      get_src(ctx, &intr->src[2], 0, nir_type_int);
   const struct dxil_value *newval =
      get_src(ctx, &intr->src[3], 0, nir_type_int);

   if (!cmpval || !newval || !handle || !offset)
      return false;

   const struct dxil_value *int32_undef = get_int32_undef(&ctx->mod);
   if (!int32_undef)
      return false;

   const struct dxil_value *coord[3] = {
      offset, int32_undef, int32_undef
   };

   const struct dxil_value *retval =
      emit_atomic_cmpxchg(ctx, handle, coord, cmpval, newval);

   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, nir_type_int);
   return true;
}

static bool
emit_shared_atomic(struct ntd_context *ctx, nir_intrinsic_instr *intr,
                   enum dxil_rmw_op op, nir_alu_type type)
{
   const struct dxil_value *zero, *index;

   assert(nir_src_bit_size(intr->src[1]) == 32);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   index = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->sharedvars, zero, index };
   const struct dxil_value *ptr, *value, *retval;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   value = get_src(ctx, &intr->src[1], 0, type);
   if (!value)
      return false;

   retval = dxil_emit_atomicrmw(&ctx->mod, value, ptr, op, false,
                                DXIL_ATOMIC_ORDERING_ACQREL,
                                DXIL_SYNC_SCOPE_CROSSTHREAD);
   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, type);
   return true;
}

static bool
emit_shared_atomic_comp_swap(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   const struct dxil_value *zero, *index;

   assert(nir_src_bit_size(intr->src[1]) == 32);

   zero = dxil_module_get_int32_const(&ctx->mod, 0);
   if (!zero)
      return false;

   index = get_src(ctx, &intr->src[0], 0, nir_type_uint);
   if (!index)
      return false;

   const struct dxil_value *ops[] = { ctx->sharedvars, zero, index };
   const struct dxil_value *ptr, *cmpval, *newval, *retval;

   ptr = dxil_emit_gep_inbounds(&ctx->mod, ops, ARRAY_SIZE(ops));
   if (!ptr)
      return false;

   cmpval = get_src(ctx, &intr->src[1], 0, nir_type_uint);
   newval = get_src(ctx, &intr->src[2], 0, nir_type_uint);
   if (!cmpval || !newval)
      return false;

   retval = dxil_emit_cmpxchg(&ctx->mod, cmpval, newval, ptr, false,
                              DXIL_ATOMIC_ORDERING_ACQREL,
                              DXIL_SYNC_SCOPE_CROSSTHREAD);
   if (!retval)
      return false;

   store_dest(ctx, &intr->dest, 0, retval, nir_type_uint);
   return true;
}

static bool
emit_vulkan_resource_index(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   unsigned int binding = nir_intrinsic_binding(intr);

   bool const_index = nir_src_is_const(intr->src[0]);
   if (const_index) {
      binding += nir_src_as_const_value(intr->src[0])->u32;
   }

   const struct dxil_value *index_value = dxil_module_get_int32_const(&ctx->mod, binding);
   if (!index_value)
      return false;

   if (!const_index) {
      const struct dxil_value *offset = get_src(ctx, &intr->src[0], 0, nir_type_uint32);
      if (!offset)
         return false;

      index_value = dxil_emit_binop(&ctx->mod, DXIL_BINOP_ADD, index_value, offset, 0);
      if (!index_value)
         return false;
   }

   store_dest(ctx, &intr->dest, 0, index_value, nir_type_uint32);
   store_dest(ctx, &intr->dest, 1, dxil_module_get_int32_const(&ctx->mod, 0), nir_type_uint32);
   return true;
}

static bool
emit_load_vulkan_descriptor(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   nir_intrinsic_instr* index = nir_src_as_intrinsic(intr->src[0]);
   /* We currently do not support reindex */
   assert(index && index->intrinsic == nir_intrinsic_vulkan_resource_index);

   unsigned binding = nir_intrinsic_binding(index);
   unsigned space = nir_intrinsic_desc_set(index);

   /* The descriptor_set field for variables is only 5 bits. We shouldn't have intrinsics trying to go beyond that. */
   assert(space < 32);

   nir_variable *var = nir_get_binding_variable(ctx->shader, nir_chase_binding(intr->src[0]));

   const struct dxil_value *handle = NULL;
   enum dxil_resource_class resource_class;

   switch (nir_intrinsic_desc_type(intr)) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      resource_class = DXIL_RESOURCE_CLASS_CBV;
      break;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      if (var->data.access & ACCESS_NON_WRITEABLE)
         resource_class = DXIL_RESOURCE_CLASS_SRV;
      else
         resource_class = DXIL_RESOURCE_CLASS_UAV;
      break;
   default:
      unreachable("unknown descriptor type");
      return false;
   }

   const struct dxil_value *index_value = get_src(ctx, &intr->src[0], 0, nir_type_uint32);
   if (!index_value)
      return false;

   handle = emit_createhandle_call(ctx, resource_class,
      get_resource_id(ctx, resource_class, space, binding),
      index_value, false);

   store_dest_value(ctx, &intr->dest, 0, handle);
   store_dest(ctx, &intr->dest, 1, get_src(ctx, &intr->src[0], 1, nir_type_uint32), nir_type_uint32);

   return true;
}

static bool
emit_intrinsic(struct ntd_context *ctx, nir_intrinsic_instr *intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_load_global_invocation_id:
   case nir_intrinsic_load_global_invocation_id_zero_base:
      return emit_load_global_invocation_id(ctx, intr);
   case nir_intrinsic_load_local_invocation_id:
      return emit_load_local_invocation_id(ctx, intr);
   case nir_intrinsic_load_local_invocation_index:
      return emit_load_local_invocation_index(ctx, intr);
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_workgroup_id_zero_base:
      return emit_load_local_workgroup_id(ctx, intr);
   case nir_intrinsic_load_ssbo:
      return emit_load_ssbo(ctx, intr);
   case nir_intrinsic_store_ssbo:
      return emit_store_ssbo(ctx, intr);
   case nir_intrinsic_store_ssbo_masked_dxil:
      return emit_store_ssbo_masked(ctx, intr);
   case nir_intrinsic_store_deref:
      return emit_store_deref(ctx, intr);
   case nir_intrinsic_store_shared_dxil:
   case nir_intrinsic_store_shared_masked_dxil:
      return emit_store_shared(ctx, intr);
   case nir_intrinsic_store_scratch_dxil:
      return emit_store_scratch(ctx, intr);
   case nir_intrinsic_load_deref:
      return emit_load_deref(ctx, intr);
   case nir_intrinsic_load_ptr_dxil:
      return emit_load_ptr(ctx, intr);
   case nir_intrinsic_load_ubo:
      return emit_load_ubo(ctx, intr);
   case nir_intrinsic_load_ubo_dxil:
      return emit_load_ubo_dxil(ctx, intr);
   case nir_intrinsic_load_front_face:
      return emit_load_input_interpolated(ctx, intr,
                                          ctx->system_value[SYSTEM_VALUE_FRONT_FACE]);
   case nir_intrinsic_load_vertex_id_zero_base:
      return emit_load_input_interpolated(ctx, intr,
                                          ctx->system_value[SYSTEM_VALUE_VERTEX_ID_ZERO_BASE]);
   case nir_intrinsic_load_instance_id:
      return emit_load_input_interpolated(ctx, intr,
                                          ctx->system_value[SYSTEM_VALUE_INSTANCE_ID]);
   case nir_intrinsic_load_primitive_id:
      return emit_load_unary_external_function(ctx, intr, "dx.op.primitiveID",
                                               DXIL_INTR_PRIMITIVE_ID);
   case nir_intrinsic_load_sample_id:
      return emit_load_unary_external_function(ctx, intr, "dx.op.sampleIndex",
                                               DXIL_INTR_SAMPLE_INDEX);
   case nir_intrinsic_load_shared_dxil:
      return emit_load_shared(ctx, intr);
   case nir_intrinsic_load_scratch_dxil:
      return emit_load_scratch(ctx, intr);
   case nir_intrinsic_discard_if:
      return emit_discard_if(ctx, intr);
   case nir_intrinsic_discard:
      return emit_discard(ctx);
   case nir_intrinsic_emit_vertex:
      return emit_emit_vertex(ctx, intr);
   case nir_intrinsic_end_primitive:
      return emit_end_primitive(ctx, intr);
   case nir_intrinsic_scoped_barrier:
      return emit_barrier(ctx, intr);
   case nir_intrinsic_ssbo_atomic_add:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_ADD, nir_type_int);
   case nir_intrinsic_ssbo_atomic_imin:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_IMIN, nir_type_int);
   case nir_intrinsic_ssbo_atomic_umin:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_UMIN, nir_type_uint);
   case nir_intrinsic_ssbo_atomic_imax:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_IMAX, nir_type_int);
   case nir_intrinsic_ssbo_atomic_umax:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_UMAX, nir_type_uint);
   case nir_intrinsic_ssbo_atomic_and:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_AND, nir_type_uint);
   case nir_intrinsic_ssbo_atomic_or:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_OR, nir_type_uint);
   case nir_intrinsic_ssbo_atomic_xor:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_XOR, nir_type_uint);
   case nir_intrinsic_ssbo_atomic_exchange:
      return emit_ssbo_atomic(ctx, intr, DXIL_ATOMIC_EXCHANGE, nir_type_int);
   case nir_intrinsic_ssbo_atomic_comp_swap:
      return emit_ssbo_atomic_comp_swap(ctx, intr);
   case nir_intrinsic_shared_atomic_add_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_ADD, nir_type_int);
   case nir_intrinsic_shared_atomic_imin_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_MIN, nir_type_int);
   case nir_intrinsic_shared_atomic_umin_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_UMIN, nir_type_uint);
   case nir_intrinsic_shared_atomic_imax_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_MAX, nir_type_int);
   case nir_intrinsic_shared_atomic_umax_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_UMAX, nir_type_uint);
   case nir_intrinsic_shared_atomic_and_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_AND, nir_type_uint);
   case nir_intrinsic_shared_atomic_or_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_OR, nir_type_uint);
   case nir_intrinsic_shared_atomic_xor_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_XOR, nir_type_uint);
   case nir_intrinsic_shared_atomic_exchange_dxil:
      return emit_shared_atomic(ctx, intr, DXIL_RMWOP_XCHG, nir_type_int);
   case nir_intrinsic_shared_atomic_comp_swap_dxil:
      return emit_shared_atomic_comp_swap(ctx, intr);
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_deref_store:
      return emit_image_store(ctx, intr);
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_deref_load:
      return emit_image_load(ctx, intr);
   case nir_intrinsic_image_size:
   case nir_intrinsic_image_deref_size:
      return emit_image_size(ctx, intr);
   case nir_intrinsic_get_ssbo_size:
      return emit_get_ssbo_size(ctx, intr);

   case nir_intrinsic_vulkan_resource_index:
      return emit_vulkan_resource_index(ctx, intr);
   case nir_intrinsic_load_vulkan_descriptor:
      return emit_load_vulkan_descriptor(ctx, intr);

   case nir_intrinsic_load_num_workgroups:
   case nir_intrinsic_load_workgroup_size:
   default:
      NIR_INSTR_UNSUPPORTED(&intr->instr);
      assert("Unimplemented intrinsic instruction");
      return false;
   }
}

static bool
emit_load_const(struct ntd_context *ctx, nir_load_const_instr *load_const)
{
   for (int i = 0; i < load_const->def.num_components; ++i) {
      const struct dxil_value *value;
      switch (load_const->def.bit_size) {
      case 1:
         value = dxil_module_get_int1_const(&ctx->mod,
                                            load_const->value[i].b);
         break;
      case 16:
         ctx->mod.feats.native_low_precision = true;
         value = dxil_module_get_int16_const(&ctx->mod,
                                             load_const->value[i].u16);
         break;
      case 32:
         value = dxil_module_get_int32_const(&ctx->mod,
                                             load_const->value[i].u32);
         break;
      case 64:
         ctx->mod.feats.int64_ops = true;
         value = dxil_module_get_int64_const(&ctx->mod,
                                             load_const->value[i].u64);
         break;
      default:
         unreachable("unexpected bit_size");
      }
      if (!value)
         return false;

      store_ssa_def(ctx, &load_const->def, i, value);
   }
   return true;
}

static bool
emit_deref(struct ntd_context* ctx, nir_deref_instr* instr)
{
   assert(instr->deref_type == nir_deref_type_var ||
          instr->deref_type == nir_deref_type_array);

   /* In the non-Vulkan environment, there's nothing to emit. Any references to
    * derefs will emit the necessary logic to handle scratch/shared GEP addressing
    */
   if (!ctx->opts->vulkan_environment)
      return true;

   /* In the Vulkan environment, we don't have cached handles for textures or
    * samplers, so let's use the opportunity of walking through the derefs to
    * emit those.
    */
   nir_variable *var = nir_deref_instr_get_variable(instr);
   assert(var);

   if (!glsl_type_is_sampler(glsl_without_array(var->type)) &&
       !glsl_type_is_image(glsl_without_array(var->type)))
      return true;

   const struct glsl_type *type = instr->type;
   const struct dxil_value *binding;

   if (instr->deref_type == nir_deref_type_var) {
      binding = dxil_module_get_int32_const(&ctx->mod, var->data.binding);
   } else {
      const struct dxil_value *base = get_src(ctx, &instr->parent, 0, nir_type_uint32);
      const struct dxil_value *offset = get_src(ctx, &instr->arr.index, 0, nir_type_uint32);
      if (!base || !offset)
         return false;

      binding = dxil_emit_binop(&ctx->mod, DXIL_BINOP_ADD, base, offset, 0);
   }

   if (!binding)
      return false;

   /* Haven't finished chasing the deref chain yet, just store the value */
   if (glsl_type_is_array(type)) {
      store_dest(ctx, &instr->dest, 0, binding, nir_type_uint32);
      return true;
   }

   assert(glsl_type_is_sampler(type) || glsl_type_is_image(type));
   enum dxil_resource_class res_class;
   if (glsl_type_is_image(type))
      res_class = DXIL_RESOURCE_CLASS_UAV;
   else if (glsl_get_sampler_result_type(type) == GLSL_TYPE_VOID)
      res_class = DXIL_RESOURCE_CLASS_SAMPLER;
   else
      res_class = DXIL_RESOURCE_CLASS_SRV;
   
   const struct dxil_value *handle = emit_createhandle_call(ctx, res_class,
      get_resource_id(ctx, res_class, var->data.descriptor_set, var->data.binding), binding, false);
   if (!handle)
      return false;

   store_dest_value(ctx, &instr->dest, 0, handle);
   return true;
}

static bool
emit_cond_branch(struct ntd_context *ctx, const struct dxil_value *cond,
                 int true_block, int false_block)
{
   assert(cond);
   assert(true_block >= 0);
   assert(false_block >= 0);
   return dxil_emit_branch(&ctx->mod, cond, true_block, false_block);
}

static bool
emit_branch(struct ntd_context *ctx, int block)
{
   assert(block >= 0);
   return dxil_emit_branch(&ctx->mod, NULL, block, -1);
}

static bool
emit_jump(struct ntd_context *ctx, nir_jump_instr *instr)
{
   switch (instr->type) {
   case nir_jump_break:
   case nir_jump_continue:
      assert(instr->instr.block->successors[0]);
      assert(!instr->instr.block->successors[1]);
      return emit_branch(ctx, instr->instr.block->successors[0]->index);

   default:
      unreachable("Unsupported jump type\n");
   }
}

struct phi_block {
   unsigned num_components;
   struct dxil_instr *comp[NIR_MAX_VEC_COMPONENTS];
};

static bool
emit_phi(struct ntd_context *ctx, nir_phi_instr *instr)
{
   unsigned bit_size = nir_dest_bit_size(instr->dest);
   const struct dxil_type *type = dxil_module_get_int_type(&ctx->mod,
                                                           bit_size);

   struct phi_block *vphi = ralloc(ctx->phis, struct phi_block);
   vphi->num_components = nir_dest_num_components(instr->dest);

   for (unsigned i = 0; i < vphi->num_components; ++i) {
      struct dxil_instr *phi = vphi->comp[i] = dxil_emit_phi(&ctx->mod, type);
      if (!phi)
         return false;
      store_dest_value(ctx, &instr->dest, i, dxil_instr_get_return_value(phi));
   }
   _mesa_hash_table_insert(ctx->phis, instr, vphi);
   return true;
}

static void
fixup_phi(struct ntd_context *ctx, nir_phi_instr *instr,
          struct phi_block *vphi)
{
   const struct dxil_value *values[128];
   unsigned blocks[128];
   for (unsigned i = 0; i < vphi->num_components; ++i) {
      size_t num_incoming = 0;
      nir_foreach_phi_src(src, instr) {
         assert(src->src.is_ssa);
         const struct dxil_value *val = get_src_ssa(ctx, src->src.ssa, i);
         assert(num_incoming < ARRAY_SIZE(values));
         values[num_incoming] = val;
         assert(num_incoming < ARRAY_SIZE(blocks));
         blocks[num_incoming] = src->pred->index;
         ++num_incoming;
      }
      dxil_phi_set_incoming(vphi->comp[i], values, blocks, num_incoming);
   }
}

static unsigned
get_n_src(struct ntd_context *ctx, const struct dxil_value **values,
          unsigned max_components, nir_tex_src *src, nir_alu_type type)
{
   unsigned num_components = nir_src_num_components(src->src);
   unsigned i = 0;

   assert(num_components <= max_components);

   for (i = 0; i < num_components; ++i) {
      values[i] = get_src(ctx, &src->src, i, type);
      if (!values[i])
         return 0;
   }

   return num_components;
}

#define PAD_SRC(ctx, array, components, undef) \
   for (unsigned i = components; i < ARRAY_SIZE(array); ++i) { \
      array[i] = undef; \
   }

static const struct dxil_value *
emit_sample(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.sample", params->overload);
   if (!func)
      return NULL;

   const struct dxil_value *args[11] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_SAMPLE),
      params->tex, params->sampler,
      params->coord[0], params->coord[1], params->coord[2], params->coord[3],
      params->offset[0], params->offset[1], params->offset[2],
      params->min_lod
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_sample_bias(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.sampleBias", params->overload);
   if (!func)
      return NULL;

   assert(params->bias != NULL);

   const struct dxil_value *args[12] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_SAMPLE_BIAS),
      params->tex, params->sampler,
      params->coord[0], params->coord[1], params->coord[2], params->coord[3],
      params->offset[0], params->offset[1], params->offset[2],
      params->bias, params->min_lod
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_sample_level(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.sampleLevel", params->overload);
   if (!func)
      return NULL;

   assert(params->lod_or_sample != NULL);

   const struct dxil_value *args[11] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_SAMPLE_LEVEL),
      params->tex, params->sampler,
      params->coord[0], params->coord[1], params->coord[2], params->coord[3],
      params->offset[0], params->offset[1], params->offset[2],
      params->lod_or_sample
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_sample_cmp(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func;
   enum dxil_intr opcode;
   int numparam;

   if (ctx->mod.shader_kind == DXIL_PIXEL_SHADER)  {
      func = dxil_get_function(&ctx->mod, "dx.op.sampleCmp", DXIL_F32);
      opcode = DXIL_INTR_SAMPLE_CMP;
      numparam = 12;
   } else {
      func = dxil_get_function(&ctx->mod, "dx.op.sampleCmpLevelZero", DXIL_F32);
      opcode = DXIL_INTR_SAMPLE_CMP_LVL_ZERO;
      numparam = 11;
   }

   if (!func)
      return NULL;

   const struct dxil_value *args[12] = {
      dxil_module_get_int32_const(&ctx->mod, opcode),
      params->tex, params->sampler,
      params->coord[0], params->coord[1], params->coord[2], params->coord[3],
      params->offset[0], params->offset[1], params->offset[2],
      params->cmp, params->min_lod
   };

   return dxil_emit_call(&ctx->mod, func, args, numparam);
}

static const struct dxil_value *
emit_sample_grad(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.sampleGrad", params->overload);
   if (!func)
      return false;

   const struct dxil_value *args[17] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_SAMPLE_GRAD),
      params->tex, params->sampler,
      params->coord[0], params->coord[1], params->coord[2], params->coord[3],
      params->offset[0], params->offset[1], params->offset[2],
      params->dx[0], params->dx[1], params->dx[2],
      params->dy[0], params->dy[1], params->dy[2],
      params->min_lod
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_texel_fetch(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.textureLoad", params->overload);
   if (!func)
      return false;

   if (!params->lod_or_sample)
      params->lod_or_sample = dxil_module_get_undef(&ctx->mod, dxil_module_get_int_type(&ctx->mod, 32));

   const struct dxil_value *args[] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_TEXTURE_LOAD),
      params->tex,
      params->lod_or_sample, params->coord[0], params->coord[1], params->coord[2],
      params->offset[0], params->offset[1], params->offset[2]
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static const struct dxil_value *
emit_texture_lod(struct ntd_context *ctx, struct texop_parameters *params)
{
   const struct dxil_func *func = dxil_get_function(&ctx->mod, "dx.op.calculateLOD", DXIL_F32);
   if (!func)
      return false;

   const struct dxil_value *args[] = {
      dxil_module_get_int32_const(&ctx->mod, DXIL_INTR_TEXTURE_LOD),
      params->tex,
      params->sampler,
      params->coord[0],
      params->coord[1],
      params->coord[2],
      dxil_module_get_int1_const(&ctx->mod, 1)
   };

   return dxil_emit_call(&ctx->mod, func, args, ARRAY_SIZE(args));
}

static bool
emit_tex(struct ntd_context *ctx, nir_tex_instr *instr)
{
   struct texop_parameters params;
   memset(&params, 0, sizeof(struct texop_parameters));
   if (!ctx->opts->vulkan_environment) {
      params.tex = ctx->srv_handles[instr->texture_index];
      params.sampler = ctx->sampler_handles[instr->sampler_index];
   }

   const struct dxil_type *int_type = dxil_module_get_int_type(&ctx->mod, 32);
   const struct dxil_type *float_type = dxil_module_get_float_type(&ctx->mod, 32);
   const struct dxil_value *int_undef = dxil_module_get_undef(&ctx->mod, int_type);
   const struct dxil_value *float_undef = dxil_module_get_undef(&ctx->mod, float_type);

   unsigned coord_components = 0, offset_components = 0, dx_components = 0, dy_components = 0;
   params.overload = get_overload(instr->dest_type, 32);

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      nir_alu_type type = nir_tex_instr_src_type(instr, i);

      switch (instr->src[i].src_type) {
      case nir_tex_src_coord:
         coord_components = get_n_src(ctx, params.coord, ARRAY_SIZE(params.coord),
                                      &instr->src[i], type);
         if (!coord_components)
            return false;
         break;

      case nir_tex_src_offset:
         offset_components = get_n_src(ctx, params.offset, ARRAY_SIZE(params.offset),
                                       &instr->src[i],  nir_type_int);
         if (!offset_components)
            return false;
         break;

      case nir_tex_src_bias:
         assert(instr->op == nir_texop_txb);
         assert(nir_src_num_components(instr->src[i].src) == 1);
         params.bias = get_src(ctx, &instr->src[i].src, 0, nir_type_float);
         if (!params.bias)
            return false;
         break;

      case nir_tex_src_lod:
         assert(nir_src_num_components(instr->src[i].src) == 1);
         /* Buffers don't have a LOD */
         if (instr->sampler_dim != GLSL_SAMPLER_DIM_BUF)
            params.lod_or_sample = get_src(ctx, &instr->src[i].src, 0, type);
         else
            params.lod_or_sample = int_undef;
         if (!params.lod_or_sample)
            return false;
         break;

      case nir_tex_src_min_lod:
         assert(nir_src_num_components(instr->src[i].src) == 1);
         params.min_lod = get_src(ctx, &instr->src[i].src, 0, type);
         if (!params.min_lod)
            return false;
         break;

      case nir_tex_src_comparator:
         assert(nir_src_num_components(instr->src[i].src) == 1);
         params.cmp = get_src(ctx, &instr->src[i].src, 0, nir_type_float);
         if (!params.cmp)
            return false;
         break;

      case nir_tex_src_ddx:
         dx_components = get_n_src(ctx, params.dx, ARRAY_SIZE(params.dx),
                                   &instr->src[i], nir_type_float);
         if (!dx_components)
            return false;
         break;

      case nir_tex_src_ddy:
         dy_components = get_n_src(ctx, params.dy, ARRAY_SIZE(params.dy),
                                   &instr->src[i], nir_type_float);
         if (!dy_components)
            return false;
         break;

      case nir_tex_src_ms_index:
         params.lod_or_sample = get_src(ctx, &instr->src[i].src, 0, nir_type_int);
         if (!params.lod_or_sample)
            return false;
         break;

      case nir_tex_src_texture_deref:
         assert(ctx->opts->vulkan_environment);
         params.tex = get_src_ssa(ctx, instr->src[i].src.ssa, 0);
         break;

      case nir_tex_src_sampler_deref:
         assert(ctx->opts->vulkan_environment);
         params.sampler = get_src_ssa(ctx, instr->src[i].src.ssa, 0);
         break;

      case nir_tex_src_projector:
         unreachable("Texture projector should have been lowered");

      default:
         fprintf(stderr, "texture source: %d\n", instr->src[i].src_type);
         unreachable("unknown texture source");
      }
   }

   assert(params.tex != NULL);
   assert(instr->op == nir_texop_txf ||
          instr->op == nir_texop_txf_ms ||
          nir_tex_instr_is_query(instr) ||
          params.sampler != NULL);

   PAD_SRC(ctx, params.coord, coord_components, float_undef);
   PAD_SRC(ctx, params.offset, offset_components, int_undef);
   if (!params.min_lod) params.min_lod = float_undef;

   const struct dxil_value *sample = NULL;
   switch (instr->op) {
   case nir_texop_txb:
      sample = emit_sample_bias(ctx, &params);
      break;

   case nir_texop_tex:
      if (params.cmp != NULL) {
         sample = emit_sample_cmp(ctx, &params);
         break;
      } else if (ctx->mod.shader_kind == DXIL_PIXEL_SHADER) {
         sample = emit_sample(ctx, &params);
         break;
      }
      params.lod_or_sample = dxil_module_get_float_const(&ctx->mod, 0);
      FALLTHROUGH;
   case nir_texop_txl:
      sample = emit_sample_level(ctx, &params);
      break;

   case nir_texop_txd:
      PAD_SRC(ctx, params.dx, dx_components, float_undef);
      PAD_SRC(ctx, params.dy, dy_components,float_undef);
      sample = emit_sample_grad(ctx, &params);
      break;

   case nir_texop_txf:
   case nir_texop_txf_ms:
      if (instr->sampler_dim == GLSL_SAMPLER_DIM_BUF) {
         params.coord[1] = int_undef;
         sample = emit_bufferload_call(ctx, params.tex, params.coord, params.overload);
      } else {
         PAD_SRC(ctx, params.coord, coord_components, int_undef);
         sample = emit_texel_fetch(ctx, &params);
      }
      break;

   case nir_texop_txs:
      sample = emit_texture_size(ctx, &params);
      break;

   case nir_texop_lod:
      sample = emit_texture_lod(ctx, &params);
      store_dest(ctx, &instr->dest, 0, sample, nir_alu_type_get_base_type(instr->dest_type));
      return true;

   case nir_texop_query_levels:
      params.lod_or_sample = dxil_module_get_int_const(&ctx->mod, 0, 32);
      sample = emit_texture_size(ctx, &params);
      const struct dxil_value *retval = dxil_emit_extractval(&ctx->mod, sample, 3);
      store_dest(ctx, &instr->dest, 0, retval, nir_alu_type_get_base_type(instr->dest_type));
      return true;

   default:
      fprintf(stderr, "texture op: %d\n", instr->op);
      unreachable("unknown texture op");
   }

   if (!sample)
      return false;

   for (unsigned i = 0; i < nir_dest_num_components(instr->dest); ++i) {
      const struct dxil_value *retval = dxil_emit_extractval(&ctx->mod, sample, i);
      store_dest(ctx, &instr->dest, i, retval, nir_alu_type_get_base_type(instr->dest_type));
   }

   return true;
}

static bool
emit_undefined(struct ntd_context *ctx, nir_ssa_undef_instr *undef)
{
   for (unsigned i = 0; i < undef->def.num_components; ++i)
      store_ssa_def(ctx, &undef->def, i, dxil_module_get_int32_const(&ctx->mod, 0));
   return true;
}

static bool emit_instr(struct ntd_context *ctx, struct nir_instr* instr)
{
   switch (instr->type) {
   case nir_instr_type_alu:
      return emit_alu(ctx, nir_instr_as_alu(instr));
   case nir_instr_type_intrinsic:
      return emit_intrinsic(ctx, nir_instr_as_intrinsic(instr));
   case nir_instr_type_load_const:
      return emit_load_const(ctx, nir_instr_as_load_const(instr));
   case nir_instr_type_deref:
      return emit_deref(ctx, nir_instr_as_deref(instr));
   case nir_instr_type_jump:
      return emit_jump(ctx, nir_instr_as_jump(instr));
   case nir_instr_type_phi:
      return emit_phi(ctx, nir_instr_as_phi(instr));
   case nir_instr_type_tex:
      return emit_tex(ctx, nir_instr_as_tex(instr));
   case nir_instr_type_ssa_undef:
      return emit_undefined(ctx, nir_instr_as_ssa_undef(instr));
   default:
      NIR_INSTR_UNSUPPORTED(instr);
      unreachable("Unimplemented instruction type");
      return false;
   }
}


static bool
emit_block(struct ntd_context *ctx, struct nir_block *block)
{
   assert(block->index < ctx->mod.num_basic_block_ids);
   ctx->mod.basic_block_ids[block->index] = ctx->mod.curr_block;

   nir_foreach_instr(instr, block) {
      TRACE_CONVERSION(instr);

      if (!emit_instr(ctx, instr))  {
         return false;
      }
   }
   return true;
}

static bool
emit_cf_list(struct ntd_context *ctx, struct exec_list *list);

static bool
emit_if(struct ntd_context *ctx, struct nir_if *if_stmt)
{
   assert(nir_src_num_components(if_stmt->condition) == 1);
   const struct dxil_value *cond = get_src(ctx, &if_stmt->condition, 0,
                                           nir_type_bool);
   if (!cond)
      return false;

   /* prepare blocks */
   nir_block *then_block = nir_if_first_then_block(if_stmt);
   assert(nir_if_last_then_block(if_stmt)->successors[0]);
   assert(!nir_if_last_then_block(if_stmt)->successors[1]);
   int then_succ = nir_if_last_then_block(if_stmt)->successors[0]->index;

   nir_block *else_block = NULL;
   int else_succ = -1;
   if (!exec_list_is_empty(&if_stmt->else_list)) {
      else_block = nir_if_first_else_block(if_stmt);
      assert(nir_if_last_else_block(if_stmt)->successors[0]);
      assert(!nir_if_last_else_block(if_stmt)->successors[1]);
      else_succ = nir_if_last_else_block(if_stmt)->successors[0]->index;
   }

   if (!emit_cond_branch(ctx, cond, then_block->index,
                         else_block ? else_block->index : then_succ))
      return false;

   /* handle then-block */
   if (!emit_cf_list(ctx, &if_stmt->then_list) ||
       (!nir_block_ends_in_jump(nir_if_last_then_block(if_stmt)) &&
        !emit_branch(ctx, then_succ)))
      return false;

   if (else_block) {
      /* handle else-block */
      if (!emit_cf_list(ctx, &if_stmt->else_list) ||
          (!nir_block_ends_in_jump(nir_if_last_else_block(if_stmt)) &&
           !emit_branch(ctx, else_succ)))
         return false;
   }

   return true;
}

static bool
emit_loop(struct ntd_context *ctx, nir_loop *loop)
{
   nir_block *first_block = nir_loop_first_block(loop);

   assert(nir_loop_last_block(loop)->successors[0]);
   assert(!nir_loop_last_block(loop)->successors[1]);

   if (!emit_branch(ctx, first_block->index))
      return false;

   if (!emit_cf_list(ctx, &loop->body))
      return false;

   if (!emit_branch(ctx, first_block->index))
      return false;

   return true;
}

static bool
emit_cf_list(struct ntd_context *ctx, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         if (!emit_block(ctx, nir_cf_node_as_block(node)))
            return false;
         break;

      case nir_cf_node_if:
         if (!emit_if(ctx, nir_cf_node_as_if(node)))
            return false;
         break;

      case nir_cf_node_loop:
         if (!emit_loop(ctx, nir_cf_node_as_loop(node)))
            return false;
         break;

      default:
         unreachable("unsupported cf-list node");
         break;
      }
   }
   return true;
}

static void
insert_sorted_by_binding(struct exec_list *var_list, nir_variable *new_var)
{
   nir_foreach_variable_in_list(var, var_list) {
      if (var->data.binding > new_var->data.binding) {
         exec_node_insert_node_before(&var->node, &new_var->node);
         return;
      }
   }
   exec_list_push_tail(var_list, &new_var->node);
}


static void
sort_uniforms_by_binding_and_remove_structs(nir_shader *s)
{
   struct exec_list new_list;
   exec_list_make_empty(&new_list);

   nir_foreach_variable_with_modes_safe(var, s, nir_var_uniform) {
      exec_node_remove(&var->node);
      const struct glsl_type *type = glsl_without_array(var->type);
      if (!glsl_type_is_struct(type))
         insert_sorted_by_binding(&new_list, var);
   }
   exec_list_append(&s->variables, &new_list);
}

static void
prepare_phi_values(struct ntd_context *ctx)
{
   /* PHI nodes are difficult to get right when tracking the types:
    * Since the incoming sources are linked to blocks, we can't bitcast
    * on the fly while loading. So scan the shader and insert a typed dummy
    * value for each phi source, and when storing we convert if the incoming
    * value has a different type then the one expected by the phi node.
    * We choose int as default, because it supports more bit sizes.
    */
   nir_foreach_function(function, ctx->shader) {
      if (function->impl) {
         nir_foreach_block(block, function->impl) {
            nir_foreach_instr(instr, block) {
               if (instr->type == nir_instr_type_phi) {
                  nir_phi_instr *ir = nir_instr_as_phi(instr);
                  unsigned bitsize = nir_dest_bit_size(ir->dest);
                  const struct dxil_value *dummy = dxil_module_get_int_const(&ctx->mod, 0, bitsize);
                  nir_foreach_phi_src(src, ir) {
                     for(unsigned int i = 0; i < ir->dest.ssa.num_components; ++i)
                        store_ssa_def(ctx, src->src.ssa, i, dummy);
                  }
               }
            }
         }
      }
   }
}

static bool
emit_cbvs(struct ntd_context *ctx)
{
   if (ctx->shader->info.stage == MESA_SHADER_KERNEL || ctx->opts->vulkan_environment) {
      nir_foreach_variable_with_modes(var, ctx->shader, nir_var_mem_ubo) {
         if (!emit_ubo_var(ctx, var))
            return false;
      }
   } else {
      for (int i = ctx->opts->ubo_binding_offset; i < ctx->shader->info.num_ubos; ++i) {
         char name[64];
         snprintf(name, sizeof(name), "__ubo%d", i);
         if (!emit_cbv(ctx, i, 0, 16384 /*4096 vec4's*/, 1, name))
            return false;
      }
   }

   return true;
}

static bool
emit_scratch(struct ntd_context *ctx)
{
   if (ctx->shader->scratch_size) {
      /*
       * We always allocate an u32 array, no matter the actual variable types.
       * According to the DXIL spec, the minimum load/store granularity is
       * 32-bit, anything smaller requires using a read-extract/read-write-modify
       * approach.
       */
      unsigned size = ALIGN_POT(ctx->shader->scratch_size, sizeof(uint32_t));
      const struct dxil_type *int32 = dxil_module_get_int_type(&ctx->mod, 32);
      const struct dxil_value *array_length = dxil_module_get_int32_const(&ctx->mod, size / sizeof(uint32_t));
      if (!int32 || !array_length)
         return false;

      const struct dxil_type *type = dxil_module_get_array_type(
         &ctx->mod, int32, size / sizeof(uint32_t));
      if (!type)
         return false;

      ctx->scratchvars = dxil_emit_alloca(&ctx->mod, type, int32, array_length, 4);
      if (!ctx->scratchvars)
         return false;
   }

   return true;
}

/* The validator complains if we don't have ops that reference a global variable. */
static bool
shader_has_shared_ops(struct nir_shader *s)
{
   nir_foreach_function(func, s) {
      if (!func->impl)
         continue;
      nir_foreach_block(block, func->impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;
            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            switch (intrin->intrinsic) {
            case nir_intrinsic_load_shared_dxil:
            case nir_intrinsic_store_shared_dxil:
            case nir_intrinsic_shared_atomic_add_dxil:
            case nir_intrinsic_shared_atomic_and_dxil:
            case nir_intrinsic_shared_atomic_comp_swap_dxil:
            case nir_intrinsic_shared_atomic_exchange_dxil:
            case nir_intrinsic_shared_atomic_imax_dxil:
            case nir_intrinsic_shared_atomic_imin_dxil:
            case nir_intrinsic_shared_atomic_or_dxil:
            case nir_intrinsic_shared_atomic_umax_dxil:
            case nir_intrinsic_shared_atomic_umin_dxil:
            case nir_intrinsic_shared_atomic_xor_dxil:
               return true;
            default: break;
            }
         }
      }
   }
   return false;
}

static bool
emit_module(struct ntd_context *ctx, const struct nir_to_dxil_options *opts)
{
   /* The validator forces us to emit resources in a specific order:
    * CBVs, Samplers, SRVs, UAVs. While we are at it also remove
    * stale struct uniforms, they are lowered but might not have been removed */
   sort_uniforms_by_binding_and_remove_structs(ctx->shader);

   /* CBVs */
   if (!emit_cbvs(ctx))
      return false;

   /* Samplers */
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_uniform) {
      unsigned count = glsl_type_get_sampler_count(var->type);
      const struct glsl_type *without_array = glsl_without_array(var->type);
      if (var->data.mode == nir_var_uniform && glsl_type_is_sampler(without_array) &&
          glsl_get_sampler_result_type(without_array) == GLSL_TYPE_VOID) {
         if (!emit_sampler(ctx, var, count))
            return false;
      }
   }

   /* SRVs */
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_uniform) {
      unsigned count = glsl_type_get_sampler_count(var->type);
      const struct glsl_type *without_array = glsl_without_array(var->type);
      if (var->data.mode == nir_var_uniform && glsl_type_is_sampler(without_array) &&
          glsl_get_sampler_result_type(without_array) != GLSL_TYPE_VOID) {
         if (!emit_srv(ctx, var, count))
            return false;
      }
   }
   /* Handle read-only SSBOs as SRVs */
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_mem_ssbo) {
      if ((var->data.access & ACCESS_NON_WRITEABLE) != 0) {
         unsigned count = 1;
         if (glsl_type_is_array(var->type))
            count = glsl_get_length(var->type);
         if (!emit_srv(ctx, var, count))
            return false;
      }
   }

   if (ctx->shader->info.shared_size && shader_has_shared_ops(ctx->shader)) {
      const struct dxil_type *type;
      unsigned size;

     /*
      * We always allocate an u32 array, no matter the actual variable types.
      * According to the DXIL spec, the minimum load/store granularity is
      * 32-bit, anything smaller requires using a read-extract/read-write-modify
      * approach. Non-atomic 64-bit accesses are allowed, but the
      * GEP(cast(gvar, u64[] *), offset) and cast(GEP(gvar, offset), u64 *))
      * sequences don't seem to be accepted by the DXIL validator when the
      * pointer is in the groupshared address space, making the 32-bit -> 64-bit
      * pointer cast impossible.
      */
      size = ALIGN_POT(ctx->shader->info.shared_size, sizeof(uint32_t));
      type = dxil_module_get_array_type(&ctx->mod,
                                        dxil_module_get_int_type(&ctx->mod, 32),
                                        size / sizeof(uint32_t));
      ctx->sharedvars = dxil_add_global_ptr_var(&ctx->mod, "shared", type,
                                                DXIL_AS_GROUPSHARED,
                                                ffs(sizeof(uint64_t)),
                                                NULL);
   }

   if (!emit_scratch(ctx))
      return false;

   /* UAVs */
   if (ctx->shader->info.stage == MESA_SHADER_KERNEL) {
      if (!emit_globals(ctx, opts->num_kernel_globals))
         return false;

      ctx->consts = _mesa_pointer_hash_table_create(ctx->ralloc_ctx);
      if (!ctx->consts)
         return false;
      if (!emit_global_consts(ctx))
         return false;
   } else {
      /* Handle read/write SSBOs as UAVs */
      nir_foreach_variable_with_modes(var, ctx->shader, nir_var_mem_ssbo) {
         if ((var->data.access & ACCESS_NON_WRITEABLE) == 0) {
            unsigned count = 1;
            if (glsl_type_is_array(var->type))
               count = glsl_get_length(var->type);
            if (!emit_uav(ctx, var->data.binding, var->data.descriptor_set,
                        count, DXIL_COMP_TYPE_INVALID,
                        DXIL_RESOURCE_KIND_RAW_BUFFER, var->name))
               return false;
            
         }
      }
   }

   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_uniform) {
      if (var->data.mode == nir_var_uniform && glsl_type_is_image(glsl_without_array(var->type))) {
         if (!emit_uav_var(ctx, var, glsl_type_get_image_count(var->type)))
            return false;
      }
   }

   nir_function_impl *entry = nir_shader_get_entrypoint(ctx->shader);
   nir_metadata_require(entry, nir_metadata_block_index);

   assert(entry->num_blocks > 0);
   ctx->mod.basic_block_ids = rzalloc_array(ctx->ralloc_ctx, int,
                                            entry->num_blocks);
   if (!ctx->mod.basic_block_ids)
      return false;

   for (int i = 0; i < entry->num_blocks; ++i)
      ctx->mod.basic_block_ids[i] = -1;
   ctx->mod.num_basic_block_ids = entry->num_blocks;

   ctx->defs = rzalloc_array(ctx->ralloc_ctx, struct dxil_def,
                             entry->ssa_alloc);
   if (!ctx->defs)
      return false;
   ctx->num_defs = entry->ssa_alloc;

   ctx->phis = _mesa_pointer_hash_table_create(ctx->ralloc_ctx);
   if (!ctx->phis)
      return false;

   prepare_phi_values(ctx);

   if (!emit_cf_list(ctx, &entry->body))
      return false;

   hash_table_foreach(ctx->phis, entry) {
      fixup_phi(ctx, (nir_phi_instr *)entry->key,
                (struct phi_block *)entry->data);
   }

   if (!dxil_emit_ret_void(&ctx->mod))
      return false;

   if (ctx->shader->info.stage == MESA_SHADER_FRAGMENT) {
      nir_foreach_variable_with_modes(var, ctx->shader, nir_var_shader_out) {
         if (var->data.location == FRAG_RESULT_STENCIL) {
            ctx->mod.feats.stencil_ref = true;
         }
      }
   }

   if (ctx->mod.feats.native_low_precision)
      ctx->mod.minor_version = MAX2(ctx->mod.minor_version, 2);

   return emit_metadata(ctx) &&
          dxil_emit_module(&ctx->mod);
}

static unsigned int
get_dxil_shader_kind(struct nir_shader *s)
{
   switch (s->info.stage) {
   case MESA_SHADER_VERTEX:
      return DXIL_VERTEX_SHADER;
   case MESA_SHADER_GEOMETRY:
      return DXIL_GEOMETRY_SHADER;
   case MESA_SHADER_FRAGMENT:
      return DXIL_PIXEL_SHADER;
   case MESA_SHADER_KERNEL:
   case MESA_SHADER_COMPUTE:
      return DXIL_COMPUTE_SHADER;
   default:
      unreachable("unknown shader stage in nir_to_dxil");
      return DXIL_COMPUTE_SHADER;
   }
}

static unsigned
lower_bit_size_callback(const nir_instr* instr, void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;
   const nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (nir_op_infos[alu->op].is_conversion)
      return 0;

   unsigned num_inputs = nir_op_infos[alu->op].num_inputs;
   const struct nir_to_dxil_options *opts = (const struct nir_to_dxil_options*)data;
   unsigned min_bit_size = opts->lower_int16 ? 32 : 16;

   unsigned ret = 0;
   for (unsigned i = 0; i < num_inputs; i++) {
      unsigned bit_size = nir_src_bit_size(alu->src[i].src);
      if (bit_size != 1 && bit_size < min_bit_size)
         ret = min_bit_size;
   }

   return ret;
}

static void
optimize_nir(struct nir_shader *s, const struct nir_to_dxil_options *opts)
{
   bool progress;
   do {
      progress = false;
      NIR_PASS_V(s, nir_lower_vars_to_ssa);
      NIR_PASS(progress, s, nir_lower_indirect_derefs, nir_var_function_temp, UINT32_MAX);
      NIR_PASS(progress, s, nir_lower_alu_to_scalar, NULL, NULL);
      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, nir_opt_copy_prop_vars);
      NIR_PASS(progress, s, nir_lower_bit_size, lower_bit_size_callback, (void*)opts);
      NIR_PASS(progress, s, dxil_nir_lower_8bit_conv);
      if (opts->lower_int16)
         NIR_PASS(progress, s, dxil_nir_lower_16bit_conv);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_if, true);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_peephole_select, 8, true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, dxil_nir_lower_x2b);
      if (s->options->lower_int64_options)
         NIR_PASS(progress, s, nir_lower_int64);
      NIR_PASS(progress, s, nir_lower_alu);
      NIR_PASS(progress, s, dxil_nir_lower_inot);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_undef);
      NIR_PASS(progress, s, nir_lower_undef_to_zero);
      NIR_PASS(progress, s, nir_opt_deref);
      NIR_PASS(progress, s, dxil_nir_lower_upcast_phis, opts->lower_int16 ? 32 : 16);
      NIR_PASS(progress, s, nir_lower_64bit_phis);
      NIR_PASS_V(s, nir_lower_system_values);
   } while (progress);

   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_algebraic_late);
   } while (progress);
}

static
void dxil_fill_validation_state(struct ntd_context *ctx,
                                struct dxil_validation_state *state)
{
   state->num_resources = util_dynarray_num_elements(&ctx->resources, struct dxil_resource);
   state->resources = (struct dxil_resource*)ctx->resources.data;
   state->state.psv0.max_expected_wave_lane_count = UINT_MAX;
   state->state.shader_stage = (uint8_t)ctx->mod.shader_kind;
   state->state.sig_input_elements = (uint8_t)ctx->mod.num_sig_inputs;
   state->state.sig_output_elements = (uint8_t)ctx->mod.num_sig_outputs;
   //state->state.sig_patch_const_or_prim_elements = 0;

   switch (ctx->mod.shader_kind) {
   case DXIL_VERTEX_SHADER:
      state->state.psv0.vs.output_position_present = ctx->mod.info.has_out_position;
      break;
   case DXIL_PIXEL_SHADER:
      /* TODO: handle depth outputs */
      state->state.psv0.ps.depth_output = ctx->mod.info.has_out_depth;
      state->state.psv0.ps.sample_frequency =
         ctx->mod.info.has_per_sample_input;
      break;
   case DXIL_COMPUTE_SHADER:
      break;
   case DXIL_GEOMETRY_SHADER:
      state->state.max_vertex_count = ctx->shader->info.gs.vertices_out;
      state->state.psv0.gs.input_primitive = dxil_get_input_primitive(ctx->shader->info.gs.input_primitive);
      state->state.psv0.gs.output_toplology = dxil_get_primitive_topology(ctx->shader->info.gs.output_primitive);
      state->state.psv0.gs.output_stream_mask = ctx->shader->info.gs.active_stream_mask;
      state->state.psv0.gs.output_position_present = ctx->mod.info.has_out_position;
      break;
   default:
      assert(0 && "Shader type not (yet) supported");
   }
}

static nir_variable *
add_sysvalue(struct ntd_context *ctx,
              uint8_t value, char *name,
              int driver_location)
{

   nir_variable *var = rzalloc(ctx->shader, nir_variable);
   if (!var)
      return NULL;
   var->data.driver_location = driver_location;
   var->data.location = value;
   var->type = glsl_uint_type();
   var->name = name;
   var->data.mode = nir_var_system_value;
   var->data.interpolation = INTERP_MODE_FLAT;
   return var;
}

static bool
append_input_or_sysvalue(struct ntd_context *ctx,
                         int input_loc,  int sv_slot,
                         char *name, int driver_location)
{
   if (input_loc >= 0) {
      /* Check inputs whether a variable is available the corresponds
       * to the sysvalue */
      nir_foreach_variable_with_modes(var, ctx->shader, nir_var_shader_in) {
         if (var->data.location == input_loc) {
            ctx->system_value[sv_slot] = var;
            return true;
         }
      }
   }

   ctx->system_value[sv_slot] = add_sysvalue(ctx, sv_slot, name, driver_location);
   if (!ctx->system_value[sv_slot])
      return false;

   nir_shader_add_variable(ctx->shader, ctx->system_value[sv_slot]);
   return true;
}

struct sysvalue_name {
   gl_system_value value;
   int slot;
   char *name;
} possible_sysvalues[] = {
   {SYSTEM_VALUE_VERTEX_ID_ZERO_BASE, -1, "SV_VertexID"},
   {SYSTEM_VALUE_INSTANCE_ID, -1, "SV_InstanceID"},
   {SYSTEM_VALUE_FRONT_FACE, VARYING_SLOT_FACE, "SV_IsFrontFace"},
   {SYSTEM_VALUE_PRIMITIVE_ID, VARYING_SLOT_PRIMITIVE_ID, "SV_PrimitiveID"},
   {SYSTEM_VALUE_SAMPLE_ID, -1, "SV_SampleIndex"},
};

static bool
allocate_sysvalues(struct ntd_context *ctx)
{
   unsigned driver_location = 0;
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_shader_in)
      driver_location++;
   nir_foreach_variable_with_modes(var, ctx->shader, nir_var_system_value)
      driver_location++;

   for (unsigned i = 0; i < ARRAY_SIZE(possible_sysvalues); ++i) {
      struct sysvalue_name *info = &possible_sysvalues[i];
      if (BITSET_TEST(ctx->shader->info.system_values_read, info->value)) {
         if (!append_input_or_sysvalue(ctx, info->slot,
                                       info->value, info->name,
                                       driver_location++))
            return false;
      }
   }
   return true;
}

bool
nir_to_dxil(struct nir_shader *s, const struct nir_to_dxil_options *opts,
            struct blob *blob)
{
   assert(opts);
   bool retval = true;
   debug_dxil = (int)debug_get_option_debug_dxil();
   blob_init(blob);

   struct ntd_context *ctx = calloc(1, sizeof(*ctx));
   if (!ctx)
      return false;

   ctx->opts = opts;
   ctx->shader = s;

   ctx->ralloc_ctx = ralloc_context(NULL);
   if (!ctx->ralloc_ctx) {
      retval = false;
      goto out;
   }

   util_dynarray_init(&ctx->srv_metadata_nodes, ctx->ralloc_ctx);
   util_dynarray_init(&ctx->uav_metadata_nodes, ctx->ralloc_ctx);
   util_dynarray_init(&ctx->cbv_metadata_nodes, ctx->ralloc_ctx);
   util_dynarray_init(&ctx->sampler_metadata_nodes, ctx->ralloc_ctx);
   util_dynarray_init(&ctx->resources, ctx->ralloc_ctx);
   dxil_module_init(&ctx->mod, ctx->ralloc_ctx);
   ctx->mod.shader_kind = get_dxil_shader_kind(s);
   ctx->mod.major_version = 6;
   ctx->mod.minor_version = 1;

   NIR_PASS_V(s, nir_lower_pack);
   NIR_PASS_V(s, nir_lower_frexp);
   NIR_PASS_V(s, nir_lower_flrp, 16 | 32 | 64, true);

   optimize_nir(s, opts);

   NIR_PASS_V(s, nir_remove_dead_variables,
              nir_var_function_temp | nir_var_shader_temp, NULL);

   if (!allocate_sysvalues(ctx))
      return false;

   if (debug_dxil & DXIL_DEBUG_VERBOSE)
      nir_print_shader(s, stderr);

   if (!emit_module(ctx, opts)) {
      debug_printf("D3D12: dxil_container_add_module failed\n");
      retval = false;
      goto out;
   }

   if (debug_dxil & DXIL_DEBUG_DUMP_MODULE) {
      struct dxil_dumper *dumper = dxil_dump_create();
      dxil_dump_module(dumper, &ctx->mod);
      fprintf(stderr, "\n");
      dxil_dump_buf_to_file(dumper, stderr);
      fprintf(stderr, "\n\n");
      dxil_dump_free(dumper);
   }

   struct dxil_container container;
   dxil_container_init(&container);
   if (!dxil_container_add_features(&container, &ctx->mod.feats)) {
      debug_printf("D3D12: dxil_container_add_features failed\n");
      retval = false;
      goto out;
   }

   if (!dxil_container_add_io_signature(&container,
                                        DXIL_ISG1,
                                        ctx->mod.num_sig_inputs,
                                        ctx->mod.inputs)) {
      debug_printf("D3D12: failed to write input signature\n");
      retval = false;
      goto out;
   }

   if (!dxil_container_add_io_signature(&container,
                                        DXIL_OSG1,
                                        ctx->mod.num_sig_outputs,
                                        ctx->mod.outputs)) {
      debug_printf("D3D12: failed to write output signature\n");
      retval = false;
      goto out;
   }

   struct dxil_validation_state validation_state;
   memset(&validation_state, 0, sizeof(validation_state));
   dxil_fill_validation_state(ctx, &validation_state);

   if (!dxil_container_add_state_validation(&container,&ctx->mod,
                                            &validation_state)) {
      debug_printf("D3D12: failed to write state-validation\n");
      retval = false;
      goto out;
   }

   if (!dxil_container_add_module(&container, &ctx->mod)) {
      debug_printf("D3D12: failed to write module\n");
      retval = false;
      goto out;
   }

   if (!dxil_container_write(&container, blob)) {
      debug_printf("D3D12: dxil_container_write failed\n");
      retval = false;
      goto out;
   }
   dxil_container_finish(&container);

   if (debug_dxil & DXIL_DEBUG_DUMP_BLOB) {
      static int shader_id = 0;
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "shader_%s_%d.blob",
               get_shader_kind_str(ctx->mod.shader_kind), shader_id++);
      debug_printf("Try to write blob to %s\n", buffer);
      FILE *f = fopen(buffer, "wb");
      if (f) {
         fwrite(blob->data, 1, blob->size, f);
         fclose(f);
      }
   }

out:
   dxil_module_release(&ctx->mod);
   ralloc_free(ctx->ralloc_ctx);
   free(ctx);
   return retval;
}

enum dxil_sysvalue_type
nir_var_to_dxil_sysvalue_type(nir_variable *var, uint64_t other_stage_mask)
{
   switch (var->data.location) {
   case VARYING_SLOT_FACE:
      return DXIL_GENERATED_SYSVALUE;
   case VARYING_SLOT_POS:
   case VARYING_SLOT_PRIMITIVE_ID:
   case VARYING_SLOT_CLIP_DIST0:
   case VARYING_SLOT_CLIP_DIST1:
   case VARYING_SLOT_PSIZ:
      if (!((1ull << var->data.location) & other_stage_mask))
         return DXIL_SYSVALUE;
      FALLTHROUGH;
   default:
      return DXIL_NO_SYSVALUE;
   }
}
