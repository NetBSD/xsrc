/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * InputAssembly.cpp --
 *    Functions that manipulate the input assembly stage.
 */


#include <stdio.h>
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf _snprintf
#endif

#include "InputAssembly.h"
#include "State.h"

#include "Debug.h"
#include "Format.h"


/*
 * ----------------------------------------------------------------------
 *
 * IaSetTopology --
 *
 *    The IaSetTopology function sets the primitive topology to
 *    enable drawing for the input assember.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
IaSetTopology(D3D10DDI_HDEVICE hDevice,                        // IN
              D3D10_DDI_PRIMITIVE_TOPOLOGY PrimitiveTopology)  // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);

   enum pipe_prim_type primitive;
   switch (PrimitiveTopology) {
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_UNDEFINED:
      /* Apps might set topology to UNDEFINED when cleaning up on exit. */
      primitive = PIPE_PRIM_MAX;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_POINTLIST:
      primitive = PIPE_PRIM_POINTS;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINELIST:
      primitive = PIPE_PRIM_LINES;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINESTRIP:
      primitive = PIPE_PRIM_LINE_STRIP;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
      primitive = PIPE_PRIM_TRIANGLES;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
      primitive = PIPE_PRIM_TRIANGLE_STRIP;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
      primitive = PIPE_PRIM_LINES_ADJACENCY;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
      primitive = PIPE_PRIM_LINE_STRIP_ADJACENCY;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
      primitive = PIPE_PRIM_TRIANGLES_ADJACENCY;
      break;
   case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
      primitive = PIPE_PRIM_TRIANGLE_STRIP_ADJACENCY;
      break;
   default:
      assert(0);
      primitive = PIPE_PRIM_MAX;
      break;
   }

   pDevice->primitive = primitive;
}


/*
 * ----------------------------------------------------------------------
 *
 * IaSetVertexBuffers --
 *
 *    The IaSetVertexBuffers function sets vertex buffers
 *    for an input assembler.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
IaSetVertexBuffers(D3D10DDI_HDEVICE hDevice,                                     // IN
                   UINT StartBuffer,                                             // IN
                   UINT NumBuffers,                                              // IN
                   __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers, // IN
                   __in_ecount (NumBuffers) const UINT *pStrides,                // IN
                   __in_ecount (NumBuffers) const UINT *pOffsets)                // IN
{
   static const float dummy[4] = {0.0f, 0.0f, 0.0f, 0.0f};

   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   unsigned i;

   for (i = 0; i < NumBuffers; i++) {
      struct pipe_vertex_buffer *vb = &pDevice->vertex_buffers[StartBuffer + i];
      struct pipe_resource *resource = CastPipeResource(phBuffers[i]);
      Resource *res = CastResource(phBuffers[i]);
      struct pipe_stream_output_target *so_target =
         res ? res->so_target : NULL;

      if (so_target && pDevice->draw_so_target != so_target) {
         if (pDevice->draw_so_target) {
            pipe_so_target_reference(&pDevice->draw_so_target, NULL);
         }
         pipe_so_target_reference(&pDevice->draw_so_target,
                                  so_target);
      }

      if (resource) {
         vb->stride = pStrides[i];
         vb->buffer_offset = pOffsets[i];
         if (vb->is_user_buffer) {
            vb->buffer.resource = NULL;
            vb->is_user_buffer = FALSE;
         }
         pipe_resource_reference(&vb->buffer.resource, resource);
      }
      else {
         vb->stride = 0;
         vb->buffer_offset = 0;
         if (!vb->is_user_buffer) {
            pipe_resource_reference(&vb->buffer.resource, NULL);
            vb->is_user_buffer = TRUE;
         }
         vb->buffer.user = dummy;
      }
   }

   for (i = 0; i < PIPE_MAX_ATTRIBS; ++i) {
      struct pipe_vertex_buffer *vb = &pDevice->vertex_buffers[i];

      /* XXX this is odd... */
      if (!vb->is_user_buffer && !vb->buffer.resource) {
         vb->stride = 0;
         vb->buffer_offset = 0;
         vb->is_user_buffer = TRUE;
         vb->buffer.user = dummy;
      }
   }

   /* Resubmit old and new vertex buffers.
    */
   pipe->set_vertex_buffers(pipe, 0, PIPE_MAX_ATTRIBS, 0, FALSE, pDevice->vertex_buffers);
}


/*
 * ----------------------------------------------------------------------
 *
 * IaSetIndexBuffer --
 *
 *    The IaSetIndexBuffer function sets an index buffer for
 *    an input assembler.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
IaSetIndexBuffer(D3D10DDI_HDEVICE hDevice,   // IN
                 D3D10DDI_HRESOURCE hBuffer, // IN
                 DXGI_FORMAT Format,         // IN
                 UINT Offset)                // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_resource *resource = CastPipeResource(hBuffer);

   if (resource) {
      pDevice->ib_offset = Offset;

      switch (Format) {
      case DXGI_FORMAT_R16_UINT:
         pDevice->index_size = 2;
         pDevice->restart_index = 0xffff;
         break;
      case DXGI_FORMAT_R32_UINT:
         pDevice->restart_index = 0xffffffff;
         pDevice->index_size = 4;
         break;
      default:
         assert(0);             /* should not happen */
         pDevice->index_size = 2;
         break;
      }
      pipe_resource_reference(&pDevice->index_buffer, resource);
   } else {
      pipe_resource_reference(&pDevice->index_buffer, NULL);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateElementLayoutSize --
 *
 *    The CalcPrivateElementLayoutSize function determines the size
 *    of the user-mode display driver's private region of memory
 *    (that is, the size of internal driver structures, not the size
 *    of the resource video memory) for an element layout.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateElementLayoutSize(
   D3D10DDI_HDEVICE hDevice,                                         // IN
   __in const D3D10DDIARG_CREATEELEMENTLAYOUT *pCreateElementLayout) // IN
{
   return sizeof(ElementLayout);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateElementLayout --
 *
 *    The CreateElementLayout function creates an element layout.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateElementLayout(
   D3D10DDI_HDEVICE hDevice,                                         // IN
   __in const D3D10DDIARG_CREATEELEMENTLAYOUT *pCreateElementLayout, // IN
   D3D10DDI_HELEMENTLAYOUT hElementLayout,                           // IN
   D3D10DDI_HRTELEMENTLAYOUT hRTElementLayout)                       // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   ElementLayout *pElementLayout = CastElementLayout(hElementLayout);

   struct pipe_vertex_element elements[PIPE_MAX_ATTRIBS];
   memset(elements, 0, sizeof elements);

   unsigned num_elements = pCreateElementLayout->NumElements;
   unsigned max_elements = 0;
   for (unsigned i = 0; i < num_elements; i++) {
      const D3D10DDIARG_INPUT_ELEMENT_DESC* pVertexElement =
            &pCreateElementLayout->pVertexElements[i];
      struct pipe_vertex_element *ve =
            &elements[pVertexElement->InputRegister];

      ve->src_offset          = pVertexElement->AlignedByteOffset;
      ve->vertex_buffer_index = pVertexElement->InputSlot;
      ve->src_format          = FormatTranslate(pVertexElement->Format, FALSE);

      switch (pVertexElement->InputSlotClass) {
      case D3D10_DDI_INPUT_PER_VERTEX_DATA:
         ve->instance_divisor = 0;
         break;
      case D3D10_DDI_INPUT_PER_INSTANCE_DATA:
         if (!pVertexElement->InstanceDataStepRate) {
            LOG_UNSUPPORTED(!pVertexElement->InstanceDataStepRate);
            ve->instance_divisor = ~0;
         } else {
            ve->instance_divisor = pVertexElement->InstanceDataStepRate;
         }
         break;
      default:
         assert(0);
         break;
      }

      max_elements = MAX2(max_elements, pVertexElement->InputRegister + 1);
   }

   /* XXX: What do we do when there's a gap? */
   if (max_elements != num_elements) {
      DebugPrintf("%s: gap\n", __FUNCTION__);
   }

   pElementLayout->handle =
         pipe->create_vertex_elements_state(pipe, max_elements, elements);
}


/*
 * ----------------------------------------------------------------------
 *
 * DestroyElementLayout --
 *
 *    The DestroyElementLayout function destroys the specified
 *    element layout object. The element layout object can be
 *    destoyed only if it is not currently bound to a display device.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
DestroyElementLayout(D3D10DDI_HDEVICE hDevice,                 // IN
                     D3D10DDI_HELEMENTLAYOUT hElementLayout)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   ElementLayout *pElementLayout = CastElementLayout(hElementLayout);

   pipe->delete_vertex_elements_state(pipe, pElementLayout->handle);}


/*
 * ----------------------------------------------------------------------
 *
 * IaSetInputLayout --
 *
 *    The IaSetInputLayout function sets an input layout for
 *    the input assembler.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
IaSetInputLayout(D3D10DDI_HDEVICE hDevice,               // IN
                 D3D10DDI_HELEMENTLAYOUT hInputLayout)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   void *state = CastPipeInputLayout(hInputLayout);

   pipe->bind_vertex_elements_state(pipe, state);
}


