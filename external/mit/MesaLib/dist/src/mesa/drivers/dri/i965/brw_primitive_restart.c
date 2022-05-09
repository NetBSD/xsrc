/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Authors:
 *    Jordan Justen <jordan.l.justen@intel.com>
 *
 */

#include "main/bufferobj.h"
#include "main/varray.h"
#include "vbo/vbo.h"

#include "brw_context.h"
#include "brw_defines.h"
#include "brw_draw.h"

#include "brw_batch.h"


#define UPDATE_MIN2(a, b) (a) = MIN2((a), (b))
#define UPDATE_MAX2(a, b) (a) = MAX2((a), (b))

/*
 * Notes on primitive restart:
 * The code below is used when the driver does not fully support primitive
 * restart (for example, if it only does restart index of ~0).
 *
 * We map the index buffer, find the restart indexes, unmap
 * the index buffer then draw the sub-primitives delineated by the restarts.
 *
 * A couple possible optimizations:
 * 1. Save the list of sub-primitive (start, count) values in a list attached
 *    to the index buffer for re-use in subsequent draws.  The list would be
 *    invalidated when the contents of the buffer changed.
 * 2. If drawing triangle strips or quad strips, create a new index buffer
 *    that uses duplicated vertices to render the disjoint strips as one
 *    long strip.  We'd have to be careful to avoid using too much memory
 *    for this.
 *
 * Finally, some apps might perform better if they don't use primitive restart
 * at all rather than this fallback path.  Set MESA_EXTENSION_OVERRIDE to
 * "-GL_NV_primitive_restart" to test that.
 */


struct sub_primitive
{
   GLuint start;
   GLuint count;
   GLuint min_index;
   GLuint max_index;
};


/**
 * Scan the elements array to find restart indexes.  Return an array
 * of struct sub_primitive to indicate how to draw the sub-primitives
 * are delineated by the restart index.
 */
static struct sub_primitive *
find_sub_primitives(const void *elements, unsigned element_size,
                    unsigned start, unsigned end, unsigned restart_index,
                    unsigned *num_sub_prims)
{
   const unsigned max_prims = end - start;
   struct sub_primitive *sub_prims;
   unsigned i, cur_start, cur_count;
   GLuint scan_index;
   unsigned scan_num;

   sub_prims =
      malloc(max_prims * sizeof(struct sub_primitive));

   if (!sub_prims) {
      *num_sub_prims = 0;
      return NULL;
   }

   cur_start = start;
   cur_count = 0;
   scan_num = 0;

#define IB_INDEX_READ(TYPE, INDEX) (((const GL##TYPE *) elements)[INDEX])

#define SCAN_ELEMENTS(TYPE) \
   sub_prims[scan_num].min_index = (GL##TYPE) 0xffffffff; \
   sub_prims[scan_num].max_index = 0; \
   for (i = start; i < end; i++) { \
      scan_index = IB_INDEX_READ(TYPE, i); \
      if (scan_index == restart_index) { \
         if (cur_count > 0) { \
            assert(scan_num < max_prims); \
            sub_prims[scan_num].start = cur_start; \
            sub_prims[scan_num].count = cur_count; \
            scan_num++; \
            sub_prims[scan_num].min_index = (GL##TYPE) 0xffffffff; \
            sub_prims[scan_num].max_index = 0; \
         } \
         cur_start = i + 1; \
         cur_count = 0; \
      } \
      else { \
         UPDATE_MIN2(sub_prims[scan_num].min_index, scan_index); \
         UPDATE_MAX2(sub_prims[scan_num].max_index, scan_index); \
         cur_count++; \
      } \
   } \
   if (cur_count > 0) { \
      assert(scan_num < max_prims); \
      sub_prims[scan_num].start = cur_start; \
      sub_prims[scan_num].count = cur_count; \
      scan_num++; \
   }

   switch (element_size) {
   case 1:
      SCAN_ELEMENTS(ubyte);
      break;
   case 2:
      SCAN_ELEMENTS(ushort);
      break;
   case 4:
      SCAN_ELEMENTS(uint);
      break;
   default:
      assert(0 && "bad index_size in find_sub_primitives()");
   }

#undef SCAN_ELEMENTS

   *num_sub_prims = scan_num;

   return sub_prims;
}


/**
 * Handle primitive restart in software.
 *
 * This function breaks up calls into the driver so primitive restart
 * support is not required in the driver.
 */
static void
vbo_sw_primitive_restart_common_start(struct gl_context *ctx,
                                      const struct _mesa_prim *prims,
                                      GLuint nr_prims,
                                      const struct _mesa_index_buffer *ib,
                                      GLuint num_instances,
                                      GLuint base_instance,
                                      struct gl_buffer_object *indirect,
                                      GLsizeiptr indirect_offset,
                                      bool primitive_restart,
                                      unsigned restart_index)
{
   GLuint prim_num;
   struct _mesa_prim new_prim;
   struct _mesa_index_buffer new_ib;
   struct sub_primitive *sub_prims;
   struct sub_primitive *sub_prim;
   GLuint num_sub_prims;
   GLuint sub_prim_num;
   GLuint end_index;
   GLuint sub_end_index;
   struct _mesa_prim temp_prim;
   GLboolean map_ib = ib->obj && !ib->obj->Mappings[MAP_INTERNAL].Pointer;
   const void *ptr;

   /* If there is an indirect buffer, map it and extract the draw params */
   if (indirect) {
      const uint32_t *indirect_params;
      if (!ctx->Driver.MapBufferRange(ctx, 0, indirect->Size, GL_MAP_READ_BIT,
                                      indirect, MAP_INTERNAL)) {

         /* something went wrong with mapping, give up */
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
                     "failed to map indirect buffer for sw primitive restart");
         return;
      }

      assert(nr_prims == 1);
      new_prim = prims[0];
      indirect_params = (const uint32_t *)
                        ADD_POINTERS(indirect->Mappings[MAP_INTERNAL].Pointer,
                                     indirect_offset);

      new_prim.count = indirect_params[0];
      new_prim.start = indirect_params[2];
      new_prim.basevertex = indirect_params[3];

      num_instances = indirect_params[1];
      base_instance = indirect_params[4];

      new_ib = *ib;
      new_ib.count = new_prim.count;

      prims = &new_prim;
      ib = &new_ib;

      ctx->Driver.UnmapBuffer(ctx, indirect, MAP_INTERNAL);
   }

   /* Find the sub-primitives. These are regions in the index buffer which
    * are split based on the primitive restart index value.
    */
   if (map_ib) {
      ctx->Driver.MapBufferRange(ctx, 0, ib->obj->Size, GL_MAP_READ_BIT,
                                 ib->obj, MAP_INTERNAL);
   }

   if (ib->obj)
      ptr = ADD_POINTERS(ib->obj->Mappings[MAP_INTERNAL].Pointer, ib->ptr);
   else
      ptr = ib->ptr;

   sub_prims = find_sub_primitives(ptr, 1 << ib->index_size_shift,
                                   prims[0].start, prims[0].start + ib->count,
                                   restart_index, &num_sub_prims);

   if (map_ib) {
      ctx->Driver.UnmapBuffer(ctx, ib->obj, MAP_INTERNAL);
   }

   /* Loop over the primitives, and use the located sub-primitives to draw
    * each primitive with a break to implement each primitive restart.
    */
   for (prim_num = 0; prim_num < nr_prims; prim_num++) {
      end_index = prims[prim_num].start + prims[prim_num].count;
      memcpy(&temp_prim, &prims[prim_num], sizeof (temp_prim));
      /* Loop over the sub-primitives drawing sub-ranges of the primitive. */
      for (sub_prim_num = 0; sub_prim_num < num_sub_prims; sub_prim_num++) {
         sub_prim = &sub_prims[sub_prim_num];
         sub_end_index = sub_prim->start + sub_prim->count;
         if (prims[prim_num].start <= sub_prim->start) {
            temp_prim.start = MAX2(prims[prim_num].start, sub_prim->start);
            temp_prim.count = MIN2(sub_end_index, end_index) - temp_prim.start;
            if ((temp_prim.start == sub_prim->start) &&
                (temp_prim.count == sub_prim->count)) {
               ctx->Driver.Draw(ctx, &temp_prim, 1, ib, true, false, 0,
                                sub_prim->min_index, sub_prim->max_index,
                                num_instances, base_instance);
            } else {
               ctx->Driver.Draw(ctx, &temp_prim, 1, ib,
                                false, false, 0, -1, -1,
                                num_instances, base_instance);
            }
         }
         if (sub_end_index >= end_index) {
            break;
         }
      }
   }

   free(sub_prims);
}

static void
vbo_sw_primitive_restart(struct gl_context *ctx,
                         const struct _mesa_prim *prims,
                         GLuint nr_prims,
                         const struct _mesa_index_buffer *ib,
                         GLuint num_instances,
                         GLuint base_instance,
                         struct gl_buffer_object *indirect,
                         GLsizeiptr indirect_offset,
                         bool primitive_restart,
                         unsigned restart_index)
{
   unsigned i;
   for (i = 1; i < nr_prims; i++) {
      if (prims[i].start != prims[0].start)
         break;
   }

   vbo_sw_primitive_restart_common_start(ctx, &prims[0], i, ib,
                                         num_instances, base_instance,
                                         indirect, indirect_offset,
                                         primitive_restart,
                                         restart_index);
   if (i != nr_prims) {
      vbo_sw_primitive_restart(ctx, &prims[i], nr_prims - i, ib,
                               num_instances, base_instance,
                               indirect, indirect_offset,
                               primitive_restart,
                               restart_index);
   }
}

/**
 * Check if the hardware's cut index support can handle the primitive
 * restart index value (pre-Haswell only).
 */
static bool
can_cut_index_handle_restart_index(struct gl_context *ctx,
                                   const struct _mesa_index_buffer *ib,
                                   unsigned restart_index)
{
   /* The FixedIndex variant means 0xFF, 0xFFFF, or 0xFFFFFFFF based on
    * the index buffer type, which corresponds exactly to the hardware.
    */
   if (ctx->Array.PrimitiveRestartFixedIndex)
      return true;

   bool cut_index_will_work;

   switch (ib->index_size_shift) {
   case 0:
      cut_index_will_work = restart_index == 0xff;
      break;
   case 1:
      cut_index_will_work = restart_index == 0xffff;
      break;
   case 2:
      cut_index_will_work = restart_index == 0xffffffff;
      break;
   default:
      unreachable("not reached");
   }

   return cut_index_will_work;
}

/**
 * Check if the hardware's cut index support can handle the primitive
 * restart case.
 */
static bool
can_cut_index_handle_prims(struct gl_context *ctx,
                           const struct _mesa_prim *prim,
                           GLuint nr_prims,
                           const struct _mesa_index_buffer *ib,
                           unsigned restart_index)
{
   struct brw_context *brw = brw_context(ctx);
   const struct intel_device_info *devinfo = &brw->screen->devinfo;

   /* Otherwise Haswell can do it all. */
   if (devinfo->verx10 >= 75)
      return true;

   if (!can_cut_index_handle_restart_index(ctx, ib, restart_index)) {
      /* The primitive restart index can't be handled, so take
       * the software path
       */
      return false;
   }

   for (unsigned i = 0; i < nr_prims; i++) {
      switch (prim[i].mode) {
      case GL_POINTS:
      case GL_LINES:
      case GL_LINE_STRIP:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_LINES_ADJACENCY:
      case GL_LINE_STRIP_ADJACENCY:
      case GL_TRIANGLES_ADJACENCY:
      case GL_TRIANGLE_STRIP_ADJACENCY:
         /* Cut index supports these primitive types */
         break;
      default:
         /* Cut index does not support these primitive types */
      //case GL_LINE_LOOP:
      //case GL_TRIANGLE_FAN:
      //case GL_QUADS:
      //case GL_QUAD_STRIP:
      //case GL_POLYGON:
         return false;
      }
   }

   return true;
}

/**
 * Check if primitive restart is enabled, and if so, handle it properly.
 *
 * In some cases the support will be handled in software. When available
 * hardware will handle primitive restart.
 */
GLboolean
brw_handle_primitive_restart(struct gl_context *ctx,
                             const struct _mesa_prim *prims,
                             GLuint nr_prims,
                             const struct _mesa_index_buffer *ib,
                             GLuint num_instances, GLuint base_instance,
                             bool primitive_restart,
                             unsigned restart_index)
{
   struct brw_context *brw = brw_context(ctx);

   /* We only need to handle cases where there is an index buffer. */
   if (ib == NULL) {
      return GL_FALSE;
   }

   /* If we have set the in_progress flag, then we are in the middle
    * of handling the primitive restart draw.
    */
   if (brw->prim_restart.in_progress) {
      return GL_FALSE;
   }

   /* If PrimitiveRestart is not enabled, then we aren't concerned about
    * handling this draw.
    */
   if (!primitive_restart) {
      return GL_FALSE;
   }

   /* Signal that we are in the process of handling the
    * primitive restart draw
    */
   brw->prim_restart.in_progress = true;

   if (can_cut_index_handle_prims(ctx, prims, nr_prims, ib, restart_index)) {
      /* Cut index should work for primitive restart, so use it
       */
      brw->prim_restart.enable_cut_index = true;
      brw->prim_restart.restart_index = restart_index;
      brw_draw_prims(ctx, prims, nr_prims, ib, false, primitive_restart,
                     restart_index, -1, -1,
                     num_instances, base_instance);
      brw->prim_restart.enable_cut_index = false;
   } else {
      /* Not all the primitive draw modes are supported by the cut index,
       * so take the software path
       */
      struct gl_buffer_object *indirect_data = brw->draw.draw_indirect_data;

      /* Clear this to make the draw direct. */
      brw->draw.draw_indirect_data = NULL;

      vbo_sw_primitive_restart(ctx, prims, nr_prims, ib, num_instances,
                               base_instance, indirect_data,
                               brw->draw.draw_indirect_offset,
                               primitive_restart, restart_index);
   }

   brw->prim_restart.in_progress = false;

   /* The primitive restart draw was completed, so return true. */
   return GL_TRUE;
}
