/*
 * Copyright © 2015 Intel Corporation
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
#ifndef WSI_COMMON_H
#define WSI_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "vk_alloc.h"
#include "vk_dispatch_table.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>

#ifndef WSI_ENTRYPOINTS_H
extern const struct vk_instance_entrypoint_table wsi_instance_entrypoints;
extern const struct vk_physical_device_entrypoint_table wsi_physical_device_entrypoints;
extern const struct vk_device_entrypoint_table wsi_device_entrypoints;
#endif

/* This is guaranteed to not collide with anything because it's in the
 * VK_KHR_swapchain namespace but not actually used by the extension.
 */
#define VK_STRUCTURE_TYPE_WSI_IMAGE_CREATE_INFO_MESA (VkStructureType)1000001002
#define VK_STRUCTURE_TYPE_WSI_MEMORY_ALLOCATE_INFO_MESA (VkStructureType)1000001003
#define VK_STRUCTURE_TYPE_WSI_SURFACE_SUPPORTED_COUNTERS_MESA (VkStructureType)1000001005
#define VK_STRUCTURE_TYPE_WSI_MEMORY_SIGNAL_SUBMIT_INFO_MESA (VkStructureType)1000001006

/* This is always chained to VkImageCreateInfo when a wsi image is created.
 * It indicates that the image can be transitioned to/from
 * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
 */
struct wsi_image_create_info {
    VkStructureType sType;
    const void *pNext;
    bool scanout;

    /* if true, the image is a prime blit source */
    bool prime_blit_src;
};

struct wsi_memory_allocate_info {
    VkStructureType sType;
    const void *pNext;
    bool implicit_sync;
};

/* To be chained into VkSurfaceCapabilities2KHR */
struct wsi_surface_supported_counters {
   VkStructureType sType;
   const void *pNext;

   VkSurfaceCounterFlagsEXT supported_surface_counters;

};

/* To be chained into VkSubmitInfo */
struct wsi_memory_signal_submit_info {
    VkStructureType sType;
    const void *pNext;
    VkDeviceMemory memory;
};

struct wsi_fence {
   VkDevice                     device;
   const struct wsi_device      *wsi_device;
   VkDisplayKHR                 display;
   const VkAllocationCallbacks  *alloc;
   VkResult                     (*wait)(struct wsi_fence *fence, uint64_t abs_timeout);
   void                         (*destroy)(struct wsi_fence *fence);
};

struct wsi_interface;

struct driOptionCache;

#define VK_ICD_WSI_PLATFORM_MAX (VK_ICD_WSI_PLATFORM_DISPLAY + 1)

struct wsi_device {
   /* Allocator for the instance */
   VkAllocationCallbacks instance_alloc;

   VkPhysicalDevice pdevice;
   VkPhysicalDeviceMemoryProperties memory_props;
   uint32_t queue_family_count;

   VkPhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info;

   bool supports_modifiers;
   uint32_t maxImageDimension2D;
   VkPresentModeKHR override_present_mode;
   bool force_bgra8_unorm_first;

   /* Whether to enable adaptive sync for a swapchain if implemented and
    * available. Not all window systems might support this. */
   bool enable_adaptive_sync;

   struct {
      /* Override the minimum number of images on the swapchain.
       * 0 = no override */
      uint32_t override_minImageCount;

      /* Forces strict number of image on the swapchain using application
       * provided VkSwapchainCreateInfoKH::RminImageCount.
       */
      bool strict_imageCount;

      /* Ensures to create at least the number of image specified by the
       * driver in VkSurfaceCapabilitiesKHR::minImageCount.
       */
      bool ensure_minImageCount;

      /* Wait for fences before submitting buffers to Xwayland. Defaults to
       * true.
       */
      bool xwaylandWaitReady;
   } x11;

   bool sw;

   /* Signals the semaphore such that any wait on the semaphore will wait on
    * any reads or writes on the give memory object.  This is used to
    * implement the semaphore signal operation in vkAcquireNextImage.
    */
   void (*signal_semaphore_for_memory)(VkDevice device,
                                       VkSemaphore semaphore,
                                       VkDeviceMemory memory);

   /* Signals the fence such that any wait on the fence will wait on any reads
    * or writes on the give memory object.  This is used to implement the
    * semaphore signal operation in vkAcquireNextImage.
    */
   void (*signal_fence_for_memory)(VkDevice device,
                                   VkFence fence,
                                   VkDeviceMemory memory);

   /*
    * This sets the ownership for a WSI memory object:
    *
    * The ownership is true if and only if the application is allowed to submit
    * command buffers that reference the buffer.
    *
    * This can be used to prune BO lists without too many adverse affects on
    * implicit sync.
    *
    * Side note: care needs to be taken for internally delayed submissions wrt
    * timeline semaphores.
    */
   void (*set_memory_ownership)(VkDevice device,
                                VkDeviceMemory memory,
                                VkBool32 ownership);

   /*
    * If this is set, the WSI device will call it to let the driver backend
    * decide if it can present images directly on the given device fd.
    */
   bool (*can_present_on_device)(VkPhysicalDevice pdevice, int fd);

#define WSI_CB(cb) PFN_vk##cb cb
   WSI_CB(AllocateMemory);
   WSI_CB(AllocateCommandBuffers);
   WSI_CB(BindBufferMemory);
   WSI_CB(BindImageMemory);
   WSI_CB(BeginCommandBuffer);
   WSI_CB(CmdCopyImageToBuffer);
   WSI_CB(CreateBuffer);
   WSI_CB(CreateCommandPool);
   WSI_CB(CreateFence);
   WSI_CB(CreateImage);
   WSI_CB(DestroyBuffer);
   WSI_CB(DestroyCommandPool);
   WSI_CB(DestroyFence);
   WSI_CB(DestroyImage);
   WSI_CB(EndCommandBuffer);
   WSI_CB(FreeMemory);
   WSI_CB(FreeCommandBuffers);
   WSI_CB(GetBufferMemoryRequirements);
   WSI_CB(GetImageDrmFormatModifierPropertiesEXT);
   WSI_CB(GetImageMemoryRequirements);
   WSI_CB(GetImageSubresourceLayout);
   WSI_CB(GetMemoryFdKHR);
   WSI_CB(GetPhysicalDeviceFormatProperties);
   WSI_CB(GetPhysicalDeviceFormatProperties2KHR);
   WSI_CB(GetPhysicalDeviceImageFormatProperties2);
   WSI_CB(ResetFences);
   WSI_CB(QueueSubmit);
   WSI_CB(WaitForFences);
   WSI_CB(MapMemory);
   WSI_CB(UnmapMemory);
#undef WSI_CB

    struct wsi_interface *                  wsi[VK_ICD_WSI_PLATFORM_MAX];
};

typedef PFN_vkVoidFunction (VKAPI_PTR *WSI_FN_GetPhysicalDeviceProcAddr)(VkPhysicalDevice physicalDevice, const char* pName);

VkResult
wsi_device_init(struct wsi_device *wsi,
                VkPhysicalDevice pdevice,
                WSI_FN_GetPhysicalDeviceProcAddr proc_addr,
                const VkAllocationCallbacks *alloc,
                int display_fd,
                const struct driOptionCache *dri_options,
                bool sw_device);

void
wsi_device_finish(struct wsi_device *wsi,
                  const VkAllocationCallbacks *alloc);

#define ICD_DEFINE_NONDISP_HANDLE_CASTS(__VkIcdType, __VkType)             \
                                                                           \
   static inline __VkIcdType *                                             \
   __VkIcdType ## _from_handle(__VkType _handle)                           \
   {                                                                       \
      return (__VkIcdType *)(uintptr_t) _handle;                           \
   }                                                                       \
                                                                           \
   static inline __VkType                                                  \
   __VkIcdType ## _to_handle(__VkIcdType *_obj)                            \
   {                                                                       \
      return (__VkType)(uintptr_t) _obj;                                   \
   }

#define ICD_FROM_HANDLE(__VkIcdType, __name, __handle) \
   __VkIcdType *__name = __VkIcdType ## _from_handle(__handle)

ICD_DEFINE_NONDISP_HANDLE_CASTS(VkIcdSurfaceBase, VkSurfaceKHR)

VkResult
wsi_common_get_images(VkSwapchainKHR _swapchain,
                      uint32_t *pSwapchainImageCount,
                      VkImage *pSwapchainImages);

VkResult
wsi_common_acquire_next_image2(const struct wsi_device *wsi,
                               VkDevice device,
                               const VkAcquireNextImageInfoKHR *pAcquireInfo,
                               uint32_t *pImageIndex);

VkResult
wsi_common_queue_present(const struct wsi_device *wsi,
                         VkDevice device_h,
                         VkQueue queue_h,
                         int queue_family_index,
                         const VkPresentInfoKHR *pPresentInfo);

uint64_t
wsi_common_get_current_time(void);

#endif
