
/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com>
 */

/* Helper for drivers which find themselves rendering a range of
 * indices starting somewhere above zero.  Typically the application
 * is issuing multiple DrawArrays() or DrawElements() to draw
 * successive primitives layed out linearly in the vertex arrays.
 * Unless the vertex arrays are all in a VBO, the OpenGL semantics
 * imply that we need to re-upload the vertex data on each draw call.
 * In that case, we want to avoid starting the upload at zero, as it
 * will mean every draw call uploads an increasing amount of not-used
 * vertex data.  Worse - in the software tnl module, all those
 * vertices will be transformed and lit.
 *
 * If we just upload the new data, however, the indices will be
 * incorrect as we tend to upload each set of vertex data to a new
 * region.
 *
 * This file provides a helper to adjust the arrays, primitives and
 * indices of a draw call so that it can be re-issued with a min_index
 * of zero.
 */

#include <stdio.h>
#include "main/bufferobj.h"
#include "main/errors.h"
#include "main/glheader.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "vbo/vbo.h"

#include "t_rebase.h"


#define REBASE(TYPE) 						\
static void *rebase_##TYPE(const void *ptr,			\
                           unsigned start, 			\
                           unsigned count, 			\
                           TYPE min_index)			\
{								\
   const TYPE *in = (TYPE *)ptr;				\
   TYPE *tmp_indices = malloc((start + count) * sizeof(TYPE));	\
                                                                \
   if (tmp_indices == NULL) {                                   \
      _mesa_error_no_memory(__func__);                          \
      return NULL;                                              \
   }                                                            \
                                                                \
   for (unsigned i = 0; i < count; i++)                         \
      tmp_indices[start + i] = in[start + i] - min_index;	\
                                                                \
   return (void *)tmp_indices;					\
}

REBASE(GLuint)
REBASE(GLushort)
REBASE(GLubyte)


/* Adjust primitives, indices and vertex definitions so that min_index
 * becomes zero. There are lots of reasons for wanting to do this, eg:
 *
 * Software tnl:
 *    - any time min_index != 0, otherwise unused vertices lower than
 *      min_index will be transformed.
 *
 * Hardware tnl:
 *    - if ib != NULL and min_index != 0, otherwise vertices lower than
 *      min_index will be uploaded.  Requires adjusting index values.
 *
 *    - if ib == NULL and min_index != 0, just for convenience so this doesn't
 *      have to be handled within the driver.
 *
 * Hardware tnl with VBO support:
 *    - as above, but only when vertices are not (all?) in VBO's.
 *    - can't save time by trying to upload half a vbo - typically it is
 *      all or nothing.
 */
void t_rebase_prims(struct gl_context *ctx,
                    const struct tnl_vertex_array *arrays,
                    const struct _mesa_prim *prim,
                    GLuint nr_prims,
                    const struct _mesa_index_buffer *ib,
                    GLuint min_index,
                    GLuint max_index,
                    GLuint num_instances,
                    GLuint base_instance,
                    tnl_draw_func draw)
{
   struct gl_array_attributes tmp_attribs[VERT_ATTRIB_MAX];
   struct tnl_vertex_array tmp_arrays[VERT_ATTRIB_MAX];

   struct _mesa_index_buffer tmp_ib;
   struct _mesa_prim *tmp_prims = NULL;
   void *tmp_indices = NULL;
   GLuint i;

   assert(min_index != 0);

   if (0)
      printf("%s %d..%d\n", __func__, min_index, max_index);

   /* XXX this path is disabled for now.
    * There's rendering corruption in some apps when it's enabled.
    */
   if (0 && ib && ctx->Extensions.ARB_draw_elements_base_vertex) {
      /* If we can just tell the hardware or the TNL to interpret our indices
       * with a different base, do so.
       */
      tmp_prims = malloc(sizeof(*prim) * nr_prims);

      if (tmp_prims == NULL) {
         _mesa_error_no_memory(__func__);
         return;
      }

      for (i = 0; i < nr_prims; i++) {
         tmp_prims[i] = prim[i];
         tmp_prims[i].basevertex -= min_index;
      }

      prim = tmp_prims;
   } else if (ib) {
      unsigned start = prim[0].start;
      for (i = 1; i < nr_prims; i++) {
         if (prim[i].start != start) {
            if (0) {
               printf("%s recursing due to mismatched start "
                      "(prim[0].start = %u vs. prim[%u].start = %u)\n",
                      __func__, start, i, prim[i].start);
            }

            t_rebase_prims(ctx, arrays, &prim[0], i, ib, min_index,
                           max_index, num_instances, base_instance, draw);
            t_rebase_prims(ctx, arrays, &prim[i], nr_prims - i, ib, min_index,
                           max_index, num_instances, base_instance, draw);
            return;
         }
      }

      /* Unfortunately need to adjust each index individually.
       */
      bool map_ib = false;
      const void *ptr;

      if (ib->obj) {
         if (!ib->obj->Mappings[MAP_INTERNAL].Pointer) {
            ctx->Driver.MapBufferRange(ctx, 0, ib->obj->Size, GL_MAP_READ_BIT,
                                       ib->obj, MAP_INTERNAL);
            map_ib = true;
         }

         ptr = ADD_POINTERS(ib->obj->Mappings[MAP_INTERNAL].Pointer, ib->ptr);
      } else
         ptr = ib->ptr;

      /* Some users might prefer it if we translated elements to GLuints here.
       * Others wouldn't...
       */
      switch (ib->index_size_shift) {
      case 2:
         tmp_indices = rebase_GLuint(ptr, start, ib->count, min_index);
         break;
      case 1:
         tmp_indices = rebase_GLushort(ptr, start, ib->count, min_index);
         break;
      case 0:
         tmp_indices = rebase_GLubyte(ptr, start, ib->count, min_index);
         break;
      }

      if (map_ib)
         ctx->Driver.UnmapBuffer(ctx, ib->obj, MAP_INTERNAL);

      if (tmp_indices == NULL)
         return;

      tmp_ib.obj = NULL;
      tmp_ib.ptr = tmp_indices;
      tmp_ib.count = ib->count;
      tmp_ib.index_size_shift = ib->index_size_shift;

      ib = &tmp_ib;
   }
   else {
      /* Otherwise the primitives need adjustment. */
      tmp_prims = malloc(sizeof(*prim) * nr_prims);

      if (tmp_prims == NULL) {
         _mesa_error_no_memory(__func__);
         return;
      }

      for (i = 0; i < nr_prims; i++) {
         /* If this fails, it could indicate an application error: */
         assert(prim[i].start >= min_index);

         tmp_prims[i] = prim[i];
         tmp_prims[i].start -= min_index;
      }

      prim = tmp_prims;
   }

   /* Just need to adjust the pointer values on each incoming array.
    * This works for VBO and non-vbo rendering and shouldn't pesimize
    * VBO-based upload schemes.  However this may still not be a fast
    * path for hardware tnl for VBO based rendering as most machines
    * will be happier if you just specify a starting vertex value in
    * each primitive.
    *
    * For drivers with hardware tnl, you only want to do this if you
    * are forced to, eg non-VBO indexed rendering with start != 0.
    */
   for (i = 0; i < VERT_ATTRIB_MAX; i++) {
      tmp_attribs[i] = *(arrays[i].VertexAttrib);
      tmp_arrays[i].BufferBinding = arrays[i].BufferBinding;
      tmp_arrays[i].VertexAttrib = &tmp_attribs[i];
      if (arrays[i].BufferBinding->BufferObj)
         tmp_attribs[i].RelativeOffset +=
            min_index * arrays[i].BufferBinding->Stride;
      else
         tmp_attribs[i].Ptr += min_index * arrays[i].BufferBinding->Stride;
   }

   /* Re-issue the draw call. */
   draw(ctx,
        tmp_arrays,
        prim,
        nr_prims,
        ib,
        GL_TRUE,
        0,
        max_index - min_index,
        num_instances, base_instance);

   free(tmp_indices);
   free(tmp_prims);
}



