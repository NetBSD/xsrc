/*
 * Copyright © 2018 Intel Corporation
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

#include "nir.h"
#include "gl_nir.h"
#include "gl_nir_linker.h"
#include "linker_util.h"
#include "main/mtypes.h"
#include "main/shaderobj.h"
#include "ir_uniform.h" /* for gl_uniform_storage */

/**
 * This file included general link methods, using NIR, instead of IR as
 * the counter-part glsl/linker.cpp
 */

static bool
can_remove_uniform(nir_variable *var, UNUSED void *data)
{
   /* Section 2.11.6 (Uniform Variables) of the OpenGL ES 3.0.3 spec
    * says:
    *
    *     "All members of a named uniform block declared with a shared or
    *     std140 layout qualifier are considered active, even if they are not
    *     referenced in any shader in the program. The uniform block itself is
    *     also considered active, even if no member of the block is
    *     referenced."
    *
    * Although the spec doesn't state it std430 layouts are expect to behave
    * the same way. If the variable is in a uniform block with one of those
    * layouts, do not eliminate it.
    */
   if (nir_variable_is_in_block(var) &&
       (glsl_get_ifc_packing(var->interface_type) !=
        GLSL_INTERFACE_PACKING_PACKED))
      return false;

   if (glsl_get_base_type(glsl_without_array(var->type)) ==
       GLSL_TYPE_SUBROUTINE)
      return false;

   /* Uniform initializers could get used by another stage */
   if (var->constant_initializer)
      return false;

   return true;
}

/**
 * Built-in / reserved GL variables names start with "gl_"
 */
static inline bool
is_gl_identifier(const char *s)
{
   return s && s[0] == 'g' && s[1] == 'l' && s[2] == '_';
}

static bool
inout_has_same_location(const nir_variable *var, unsigned stage)
{
   if (!var->data.patch &&
       ((var->data.mode == nir_var_shader_out &&
         stage == MESA_SHADER_TESS_CTRL) ||
        (var->data.mode == nir_var_shader_in &&
         (stage == MESA_SHADER_TESS_CTRL || stage == MESA_SHADER_TESS_EVAL ||
          stage == MESA_SHADER_GEOMETRY))))
      return true;
   else
      return false;
}

/**
 * Create gl_shader_variable from nir_variable.
 */
static struct gl_shader_variable *
create_shader_variable(struct gl_shader_program *shProg,
                       const nir_variable *in,
                       const char *name, const struct glsl_type *type,
                       const struct glsl_type *interface_type,
                       bool use_implicit_location, int location,
                       const struct glsl_type *outermost_struct_type)
{
   /* Allocate zero-initialized memory to ensure that bitfield padding
    * is zero.
    */
   struct gl_shader_variable *out = rzalloc(shProg,
                                            struct gl_shader_variable);
   if (!out)
      return NULL;

   /* Since gl_VertexID may be lowered to gl_VertexIDMESA, but applications
    * expect to see gl_VertexID in the program resource list.  Pretend.
    */
   if (in->data.mode == nir_var_system_value &&
       in->data.location == SYSTEM_VALUE_VERTEX_ID_ZERO_BASE) {
      out->name = ralloc_strdup(shProg, "gl_VertexID");
   } else if ((in->data.mode == nir_var_shader_out &&
               in->data.location == VARYING_SLOT_TESS_LEVEL_OUTER) ||
              (in->data.mode == nir_var_system_value &&
               in->data.location == SYSTEM_VALUE_TESS_LEVEL_OUTER)) {
      out->name = ralloc_strdup(shProg, "gl_TessLevelOuter");
      type = glsl_array_type(glsl_float_type(), 4, 0);
   } else if ((in->data.mode == nir_var_shader_out &&
               in->data.location == VARYING_SLOT_TESS_LEVEL_INNER) ||
              (in->data.mode == nir_var_system_value &&
               in->data.location == SYSTEM_VALUE_TESS_LEVEL_INNER)) {
      out->name = ralloc_strdup(shProg, "gl_TessLevelInner");
      type = glsl_array_type(glsl_float_type(), 2, 0);
   } else {
      out->name = ralloc_strdup(shProg, name);
   }

   if (!out->name)
      return NULL;

   /* The ARB_program_interface_query spec says:
    *
    *     "Not all active variables are assigned valid locations; the
    *     following variables will have an effective location of -1:
    *
    *      * uniforms declared as atomic counters;
    *
    *      * members of a uniform block;
    *
    *      * built-in inputs, outputs, and uniforms (starting with "gl_"); and
    *
    *      * inputs or outputs not declared with a "location" layout
    *        qualifier, except for vertex shader inputs and fragment shader
    *        outputs."
    */
   if (glsl_get_base_type(in->type) == GLSL_TYPE_ATOMIC_UINT ||
       is_gl_identifier(in->name) ||
       !(in->data.explicit_location || use_implicit_location)) {
      out->location = -1;
   } else {
      out->location = location;
   }

   out->type = type;
   out->outermost_struct_type = outermost_struct_type;
   out->interface_type = interface_type;
   out->component = in->data.location_frac;
   out->index = in->data.index;
   out->patch = in->data.patch;
   out->mode = in->data.mode;
   out->interpolation = in->data.interpolation;
   out->precision = in->data.precision;
   out->explicit_location = in->data.explicit_location;

   return out;
}

static bool
add_shader_variable(const struct gl_context *ctx,
                    struct gl_shader_program *shProg,
                    struct set *resource_set,
                    unsigned stage_mask,
                    GLenum programInterface, nir_variable *var,
                    const char *name, const struct glsl_type *type,
                    bool use_implicit_location, int location,
                    bool inouts_share_location,
                    const struct glsl_type *outermost_struct_type)
{
   const struct glsl_type *interface_type = var->interface_type;

   if (outermost_struct_type == NULL) {
      if (var->data.from_named_ifc_block) {
         const char *interface_name = glsl_get_type_name(interface_type);

         if (glsl_type_is_array(interface_type)) {
            /* Issue #16 of the ARB_program_interface_query spec says:
             *
             * "* If a variable is a member of an interface block without an
             *    instance name, it is enumerated using just the variable name.
             *
             *  * If a variable is a member of an interface block with an
             *    instance name, it is enumerated as "BlockName.Member", where
             *    "BlockName" is the name of the interface block (not the
             *    instance name) and "Member" is the name of the variable."
             *
             * In particular, it indicates that it should be "BlockName",
             * not "BlockName[array length]".  The conformance suite and
             * dEQP both require this behavior.
             *
             * Here, we unwrap the extra array level added by named interface
             * block array lowering so we have the correct variable type.  We
             * also unwrap the interface type when constructing the name.
             *
             * We leave interface_type the same so that ES 3.x SSO pipeline
             * validation can enforce the rules requiring array length to
             * match on interface blocks.
             */
            type = glsl_get_array_element(type);

            interface_name =
               glsl_get_type_name(glsl_get_array_element(interface_type));
         }

         name = ralloc_asprintf(shProg, "%s.%s", interface_name, name);
      }
   }

   switch (glsl_get_base_type(type)) {
   case GLSL_TYPE_STRUCT: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as a structure, a separate entry
       *     will be generated for each active structure member.  The name of
       *     each entry is formed by concatenating the name of the structure,
       *     the "."  character, and the name of the structure member.  If a
       *     structure member to enumerate is itself a structure or array,
       *     these enumeration rules are applied recursively."
       */
      if (outermost_struct_type == NULL)
         outermost_struct_type = type;

      unsigned field_location = location;
      for (unsigned i = 0; i < glsl_get_length(type); i++) {
         const struct glsl_type *field_type = glsl_get_struct_field(type, i);
         const struct glsl_struct_field *field =
            glsl_get_struct_field_data(type, i);

         char *field_name = ralloc_asprintf(shProg, "%s.%s", name, field->name);
         if (!add_shader_variable(ctx, shProg, resource_set,
                                  stage_mask, programInterface,
                                  var, field_name, field_type,
                                  use_implicit_location, field_location,
                                  false, outermost_struct_type))
            return false;

         field_location += glsl_count_attribute_slots(field_type, false);
      }
      return true;
   }

   case GLSL_TYPE_ARRAY: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as an array of basic types, a
       *      single entry will be generated, with its name string formed by
       *      concatenating the name of the array and the string "[0]"."
       *
       *     "For an active variable declared as an array of an aggregate data
       *      type (structures or arrays), a separate entry will be generated
       *      for each active array element, unless noted immediately below.
       *      The name of each entry is formed by concatenating the name of
       *      the array, the "[" character, an integer identifying the element
       *      number, and the "]" character.  These enumeration rules are
       *      applied recursively, treating each enumerated array element as a
       *      separate active variable."
       */
      const struct glsl_type *array_type = glsl_get_array_element(type);
      if (glsl_get_base_type(array_type) == GLSL_TYPE_STRUCT ||
          glsl_get_base_type(array_type) == GLSL_TYPE_ARRAY) {
         unsigned elem_location = location;
         unsigned stride = inouts_share_location ? 0 :
                           glsl_count_attribute_slots(array_type, false);
         for (unsigned i = 0; i < glsl_get_length(type); i++) {
            char *elem = ralloc_asprintf(shProg, "%s[%d]", name, i);
            if (!add_shader_variable(ctx, shProg, resource_set,
                                     stage_mask, programInterface,
                                     var, elem, array_type,
                                     use_implicit_location, elem_location,
                                     false, outermost_struct_type))
               return false;
            elem_location += stride;
         }
         return true;
      }
   }
   FALLTHROUGH;

   default: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as a single instance of a basic
       *     type, a single entry will be generated, using the variable name
       *     from the shader source."
       */
      struct gl_shader_variable *sha_v =
         create_shader_variable(shProg, var, name, type, interface_type,
                                use_implicit_location, location,
                                outermost_struct_type);
      if (!sha_v)
         return false;

      return link_util_add_program_resource(shProg, resource_set,
                                            programInterface, sha_v, stage_mask);
   }
   }
}

static bool
add_vars_with_modes(const struct gl_context *ctx,
                    struct gl_shader_program *prog, struct set *resource_set,
                    nir_shader *nir, nir_variable_mode modes,
                    unsigned stage, GLenum programInterface)
{
   nir_foreach_variable_with_modes(var, nir, modes) {
      if (var->data.how_declared == nir_var_hidden)
         continue;

      int loc_bias = 0;
      switch(var->data.mode) {
      case nir_var_system_value:
      case nir_var_shader_in:
         if (programInterface != GL_PROGRAM_INPUT)
            continue;
         loc_bias = (stage == MESA_SHADER_VERTEX) ? VERT_ATTRIB_GENERIC0
                                                  : VARYING_SLOT_VAR0;
         break;
      case nir_var_shader_out:
         if (programInterface != GL_PROGRAM_OUTPUT)
            continue;
         loc_bias = (stage == MESA_SHADER_FRAGMENT) ? FRAG_RESULT_DATA0
                                                    : VARYING_SLOT_VAR0;
         break;
      default:
         continue;
      }

      if (var->data.patch)
         loc_bias = VARYING_SLOT_PATCH0;

      if (prog->data->spirv) {
         struct gl_shader_variable *sh_var =
            rzalloc(prog, struct gl_shader_variable);

         /* In the ARB_gl_spirv spec, names are considered optional debug info, so
          * the linker needs to work without them. Returning them is optional.
          * For simplicity, we ignore names.
          */
         sh_var->name = NULL;
         sh_var->type = var->type;
         sh_var->location = var->data.location - loc_bias;
         sh_var->index = var->data.index;

         if (!link_util_add_program_resource(prog, resource_set,
                                             programInterface,
                                             sh_var, 1 << stage)) {
           return false;
         }
      } else {
         /* Skip packed varyings, packed varyings are handled separately
          * by add_packed_varyings in the GLSL IR
          * build_program_resource_list() call.
          * TODO: handle packed varyings here instead. We likely want a NIR
          * based packing pass first.
          */
         if (strncmp(var->name, "packed:", 7) == 0)
            continue;

         const bool vs_input_or_fs_output =
            (stage == MESA_SHADER_VERTEX &&
             var->data.mode == nir_var_shader_in) ||
            (stage == MESA_SHADER_FRAGMENT &&
             var->data.mode == nir_var_shader_out);

         if (!add_shader_variable(ctx, prog, resource_set,
                                  1 << stage, programInterface,
                                  var, var->name, var->type,
                                  vs_input_or_fs_output,
                                  var->data.location - loc_bias,
                                  inout_has_same_location(var, stage),
                                  NULL))
            return false;
      }
   }

   return true;
}

static bool
add_interface_variables(const struct gl_context *ctx,
                        struct gl_shader_program *prog,
                        struct set *resource_set,
                        unsigned stage, GLenum programInterface)
{
   struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
   if (!sh)
      return true;

   nir_shader *nir = sh->Program->nir;
   assert(nir);

   switch (programInterface) {
   case GL_PROGRAM_INPUT: {
      return add_vars_with_modes(ctx, prog, resource_set,
                                 nir, nir_var_shader_in | nir_var_system_value,
                                 stage, programInterface);
   }
   case GL_PROGRAM_OUTPUT:
      return add_vars_with_modes(ctx, prog, resource_set,
                                 nir, nir_var_shader_out,
                                 stage, programInterface);
   default:
      assert("!Should not get here");
      break;
   }

   return false;
}

/* TODO: as we keep adding features, this method is becoming more and more
 * similar to its GLSL counterpart at linker.cpp. Eventually it would be good
 * to check if they could be refactored, and reduce code duplication somehow
 */
void
nir_build_program_resource_list(struct gl_context *ctx,
                                struct gl_shader_program *prog,
                                bool rebuild_resourse_list)
{
   /* Rebuild resource list. */
   if (prog->data->ProgramResourceList && rebuild_resourse_list) {
      ralloc_free(prog->data->ProgramResourceList);
      prog->data->ProgramResourceList = NULL;
      prog->data->NumProgramResourceList = 0;
   }

   int input_stage = MESA_SHADER_STAGES, output_stage = 0;

   /* Determine first input and final output stage. These are used to
    * detect which variables should be enumerated in the resource list
    * for GL_PROGRAM_INPUT and GL_PROGRAM_OUTPUT.
    */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (!prog->_LinkedShaders[i])
         continue;
      if (input_stage == MESA_SHADER_STAGES)
         input_stage = i;
      output_stage = i;
   }

   /* Empty shader, no resources. */
   if (input_stage == MESA_SHADER_STAGES && output_stage == 0)
      return;

   struct set *resource_set = _mesa_pointer_set_create(NULL);

   /* Add inputs and outputs to the resource list. */
   if (!add_interface_variables(ctx, prog, resource_set, input_stage,
                                GL_PROGRAM_INPUT)) {
      return;
   }

   if (!add_interface_variables(ctx, prog, resource_set, output_stage,
                                GL_PROGRAM_OUTPUT)) {
      return;
   }

   /* Add transform feedback varyings and buffers. */
   if (prog->last_vert_prog) {
      struct gl_transform_feedback_info *linked_xfb =
         prog->last_vert_prog->sh.LinkedTransformFeedback;

      /* Add varyings. */
      if (linked_xfb->NumVarying > 0) {
         for (int i = 0; i < linked_xfb->NumVarying; i++) {
            if (!link_util_add_program_resource(prog, resource_set,
                                                GL_TRANSFORM_FEEDBACK_VARYING,
                                                &linked_xfb->Varyings[i], 0))
            return;
         }
      }

      /* Add buffers. */
      for (unsigned i = 0; i < ctx->Const.MaxTransformFeedbackBuffers; i++) {
         if ((linked_xfb->ActiveBuffers >> i) & 1) {
            linked_xfb->Buffers[i].Binding = i;
            if (!link_util_add_program_resource(prog, resource_set,
                                                GL_TRANSFORM_FEEDBACK_BUFFER,
                                                &linked_xfb->Buffers[i], 0))
            return;
         }
      }
   }

   /* Add uniforms
    *
    * Here, it is expected that nir_link_uniforms() has already been
    * called, so that UniformStorage table is already available.
    */
   int top_level_array_base_offset = -1;
   int top_level_array_size_in_bytes = -1;
   int second_element_offset = -1;
   int block_index = -1;
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (uniform->hidden) {
         for (int j = MESA_SHADER_VERTEX; j < MESA_SHADER_STAGES; j++) {
            if (!uniform->opaque[j].active ||
                glsl_get_base_type(uniform->type) != GLSL_TYPE_SUBROUTINE)
               continue;

            GLenum type =
               _mesa_shader_stage_to_subroutine_uniform((gl_shader_stage)j);
            /* add shader subroutines */
            if (!link_util_add_program_resource(prog, resource_set,
                                                type, uniform, 0))
               return;
         }

         continue;
      }

      if (!link_util_should_add_buffer_variable(prog, uniform,
                                                top_level_array_base_offset,
                                                top_level_array_size_in_bytes,
                                                second_element_offset, block_index))
         continue;


      if (prog->data->UniformStorage[i].offset >= second_element_offset) {
         top_level_array_base_offset =
            prog->data->UniformStorage[i].offset;

         top_level_array_size_in_bytes =
            prog->data->UniformStorage[i].top_level_array_size *
            prog->data->UniformStorage[i].top_level_array_stride;

         /* Set or reset the second element offset. For non arrays this
          * will be set to -1.
          */
         second_element_offset = top_level_array_size_in_bytes ?
            top_level_array_base_offset +
            prog->data->UniformStorage[i].top_level_array_stride : -1;
      }
      block_index = uniform->block_index;


      GLenum interface = uniform->is_shader_storage ? GL_BUFFER_VARIABLE : GL_UNIFORM;
      if (!link_util_add_program_resource(prog, resource_set, interface, uniform,
                                          uniform->active_shader_mask)) {
         return;
      }
   }


   for (unsigned i = 0; i < prog->data->NumUniformBlocks; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_UNIFORM_BLOCK,
                                          &prog->data->UniformBlocks[i],
                                          prog->data->UniformBlocks[i].stageref))
         return;
   }

   for (unsigned i = 0; i < prog->data->NumShaderStorageBlocks; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_SHADER_STORAGE_BLOCK,
                                          &prog->data->ShaderStorageBlocks[i],
                                          prog->data->ShaderStorageBlocks[i].stageref))
         return;
   }

   /* Add atomic counter buffers. */
   for (unsigned i = 0; i < prog->data->NumAtomicBuffers; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_ATOMIC_COUNTER_BUFFER,
                                          &prog->data->AtomicBuffers[i], 0))
         return;
   }

   unsigned mask = prog->data->linked_stages;
   while (mask) {
      const int i = u_bit_scan(&mask);
      struct gl_program *p = prog->_LinkedShaders[i]->Program;

      GLuint type = _mesa_shader_stage_to_subroutine((gl_shader_stage)i);
      for (unsigned j = 0; j < p->sh.NumSubroutineFunctions; j++) {
         if (!link_util_add_program_resource(prog, resource_set,
                                             type,
                                             &p->sh.SubroutineFunctions[j],
                                             0))
            return;
      }
   }

   _mesa_set_destroy(resource_set, NULL);
}

bool
gl_nir_link_spirv(struct gl_context *ctx, struct gl_shader_program *prog,
                  const struct gl_nir_linker_options *options)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *shader = prog->_LinkedShaders[i];
      if (shader) {
         const nir_remove_dead_variables_options opts = {
            .can_remove_var = can_remove_uniform,
         };
         nir_remove_dead_variables(shader->Program->nir, nir_var_uniform,
                                   &opts);
      }
   }

   if (!gl_nir_link_uniform_blocks(ctx, prog))
      return false;

   if (!gl_nir_link_uniforms(ctx, prog, options->fill_parameters))
      return false;

   gl_nir_link_assign_atomic_counter_resources(ctx, prog);
   gl_nir_link_assign_xfb_resources(ctx, prog);

   return true;
}

/**
 * Validate shader image resources.
 */
static void
check_image_resources(struct gl_context *ctx, struct gl_shader_program *prog)
{
   unsigned total_image_units = 0;
   unsigned fragment_outputs = 0;
   unsigned total_shader_storage_blocks = 0;

   if (!ctx->Extensions.ARB_shader_image_load_store)
      return;

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[i];
      if (!sh)
         continue;

      total_image_units += sh->Program->info.num_images;
      total_shader_storage_blocks += sh->Program->info.num_ssbos;
   }

   if (total_image_units > ctx->Const.MaxCombinedImageUniforms)
      linker_error(prog, "Too many combined image uniforms\n");

   struct gl_linked_shader *frag_sh =
      prog->_LinkedShaders[MESA_SHADER_FRAGMENT];
   if (frag_sh) {
      uint64_t frag_outputs_written = frag_sh->Program->info.outputs_written;
      fragment_outputs = util_bitcount64(frag_outputs_written);
   }

   if (total_image_units + fragment_outputs + total_shader_storage_blocks >
       ctx->Const.MaxCombinedShaderOutputResources)
      linker_error(prog, "Too many combined image uniforms, shader storage "
                         " buffers and fragment outputs\n");
}

bool
gl_nir_link_glsl(struct gl_context *ctx, struct gl_shader_program *prog)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *shader = prog->_LinkedShaders[i];
      if (shader) {
         const nir_remove_dead_variables_options opts = {
            .can_remove_var = can_remove_uniform,
         };
         nir_remove_dead_variables(shader->Program->nir, nir_var_uniform,
                                   &opts);
      }
   }

   if (!gl_nir_link_uniforms(ctx, prog, true))
      return false;

   link_util_calculate_subroutine_compat(prog);
   link_util_check_uniform_resources(ctx, prog);
   link_util_check_subroutine_resources(prog);
   check_image_resources(ctx, prog);
   gl_nir_link_assign_atomic_counter_resources(ctx, prog);
   gl_nir_link_check_atomic_counter_resources(ctx, prog);

   if (prog->data->LinkStatus == LINKING_FAILURE)
      return false;

   return true;
}
