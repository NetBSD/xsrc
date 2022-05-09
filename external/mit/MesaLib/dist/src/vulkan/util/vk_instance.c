/*
 * Copyright © 2021 Intel Corporation
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

#include "vk_instance.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_log.h"
#include "vk_util.h"
#include "vk_debug_utils.h"

#include "compiler/glsl_types.h"

VkResult
vk_instance_init(struct vk_instance *instance,
                 const struct vk_instance_extension_table *supported_extensions,
                 const struct vk_instance_dispatch_table *dispatch_table,
                 const VkInstanceCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *alloc)
{
   memset(instance, 0, sizeof(*instance));
   vk_object_base_init(NULL, &instance->base, VK_OBJECT_TYPE_INSTANCE);
   instance->alloc = *alloc;

   /* VK_EXT_debug_utils */
   /* These messengers will only be used during vkCreateInstance or
    * vkDestroyInstance calls.  We do this first so that it's safe to use
    * vk_errorf and friends.
    */
   list_inithead(&instance->debug_utils.instance_callbacks);
   vk_foreach_struct_const(ext, pCreateInfo->pNext) {
      if (ext->sType ==
          VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
         const VkDebugUtilsMessengerCreateInfoEXT *debugMessengerCreateInfo =
            (const VkDebugUtilsMessengerCreateInfoEXT *)ext;
         struct vk_debug_utils_messenger *messenger =
            vk_alloc2(alloc, alloc, sizeof(struct vk_debug_utils_messenger), 8,
                      VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

         if (!messenger)
            return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

         vk_object_base_init(NULL, &messenger->base,
                             VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT);

         messenger->alloc = *alloc;
         messenger->severity = debugMessengerCreateInfo->messageSeverity;
         messenger->type = debugMessengerCreateInfo->messageType;
         messenger->callback = debugMessengerCreateInfo->pfnUserCallback;
         messenger->data = debugMessengerCreateInfo->pUserData;

         list_addtail(&messenger->link,
                      &instance->debug_utils.instance_callbacks);
      }
   }

   instance->app_info = (struct vk_app_info) { .api_version = 0 };
   if (pCreateInfo->pApplicationInfo) {
      const VkApplicationInfo *app = pCreateInfo->pApplicationInfo;

      instance->app_info.app_name =
         vk_strdup(&instance->alloc, app->pApplicationName,
                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      instance->app_info.app_version = app->applicationVersion;

      instance->app_info.engine_name =
         vk_strdup(&instance->alloc, app->pEngineName,
                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      instance->app_info.engine_version = app->engineVersion;

      instance->app_info.api_version = app->apiVersion;
   }

   if (instance->app_info.api_version == 0)
      instance->app_info.api_version = VK_API_VERSION_1_0;

   for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
      int idx;
      for (idx = 0; idx < VK_INSTANCE_EXTENSION_COUNT; idx++) {
         if (strcmp(pCreateInfo->ppEnabledExtensionNames[i],
                    vk_instance_extensions[idx].extensionName) == 0)
            break;
      }

      if (idx >= VK_INSTANCE_EXTENSION_COUNT)
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);

      if (!supported_extensions->extensions[idx])
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);

#ifdef ANDROID
      if (!vk_android_allowed_instance_extensions.extensions[idx])
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);
#endif

      instance->enabled_extensions.extensions[idx] = true;
   }

   instance->dispatch_table = *dispatch_table;

   /* Add common entrypoints without overwriting driver-provided ones. */
   vk_instance_dispatch_table_from_entrypoints(
      &instance->dispatch_table, &vk_common_instance_entrypoints, false);

   if (mtx_init(&instance->debug_report.callbacks_mutex, mtx_plain) != 0)
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);

   list_inithead(&instance->debug_report.callbacks);

   if (mtx_init(&instance->debug_utils.callbacks_mutex, mtx_plain) != 0) {
      mtx_destroy(&instance->debug_report.callbacks_mutex);
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   list_inithead(&instance->debug_utils.callbacks);

   glsl_type_singleton_init_or_ref();

   return VK_SUCCESS;
}

void
vk_instance_finish(struct vk_instance *instance)
{
   glsl_type_singleton_decref();
   if (unlikely(!list_is_empty(&instance->debug_utils.callbacks))) {
      list_for_each_entry_safe(struct vk_debug_utils_messenger, messenger,
                               &instance->debug_utils.callbacks, link) {
         list_del(&messenger->link);
         vk_object_base_finish(&messenger->base);
         vk_free2(&instance->alloc, &messenger->alloc, messenger);
      }
   }
   if (unlikely(!list_is_empty(&instance->debug_utils.instance_callbacks))) {
      list_for_each_entry_safe(struct vk_debug_utils_messenger, messenger,
                               &instance->debug_utils.instance_callbacks,
                               link) {
         list_del(&messenger->link);
         vk_object_base_finish(&messenger->base);
         vk_free2(&instance->alloc, &messenger->alloc, messenger);
      }
   }
   mtx_destroy(&instance->debug_report.callbacks_mutex);
   mtx_destroy(&instance->debug_utils.callbacks_mutex);
   vk_free(&instance->alloc, (char *)instance->app_info.app_name);
   vk_free(&instance->alloc, (char *)instance->app_info.engine_name);
   vk_object_base_finish(&instance->base);
}

VkResult
vk_enumerate_instance_extension_properties(
    const struct vk_instance_extension_table *supported_extensions,
    uint32_t *pPropertyCount,
    VkExtensionProperties *pProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkExtensionProperties, out, pProperties, pPropertyCount);

   for (int i = 0; i < VK_INSTANCE_EXTENSION_COUNT; i++) {
      if (!supported_extensions->extensions[i])
         continue;

#ifdef ANDROID
      if (!vk_android_allowed_instance_extensions.extensions[i])
         continue;
#endif

      vk_outarray_append_typed(VkExtensionProperties, &out, prop) {
         *prop = vk_instance_extensions[i];
      }
   }

   return vk_outarray_status(&out);
}

PFN_vkVoidFunction
vk_instance_get_proc_addr(const struct vk_instance *instance,
                          const struct vk_instance_entrypoint_table *entrypoints,
                          const char *name)
{
   PFN_vkVoidFunction func;

   /* The Vulkan 1.0 spec for vkGetInstanceProcAddr has a table of exactly
    * when we have to return valid function pointers, NULL, or it's left
    * undefined.  See the table for exact details.
    */
   if (name == NULL)
      return NULL;

#define LOOKUP_VK_ENTRYPOINT(entrypoint) \
   if (strcmp(name, "vk" #entrypoint) == 0) \
      return (PFN_vkVoidFunction)entrypoints->entrypoint

   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceExtensionProperties);
   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceLayerProperties);
   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceVersion);
   LOOKUP_VK_ENTRYPOINT(CreateInstance);

   /* GetInstanceProcAddr() can also be called with a NULL instance.
    * See https://gitlab.khronos.org/vulkan/vulkan/issues/2057
    */
   LOOKUP_VK_ENTRYPOINT(GetInstanceProcAddr);

#undef LOOKUP_VK_ENTRYPOINT

   if (instance == NULL)
      return NULL;

   func = vk_instance_dispatch_table_get_if_supported(&instance->dispatch_table,
                                                      name,
                                                      instance->app_info.api_version,
                                                      &instance->enabled_extensions);
   if (func != NULL)
      return func;

   func = vk_physical_device_dispatch_table_get_if_supported(&vk_physical_device_trampolines,
                                                             name,
                                                             instance->app_info.api_version,
                                                             &instance->enabled_extensions);
   if (func != NULL)
      return func;

   func = vk_device_dispatch_table_get_if_supported(&vk_device_trampolines,
                                                    name,
                                                    instance->app_info.api_version,
                                                    &instance->enabled_extensions,
                                                    NULL);
   if (func != NULL)
      return func;

   return NULL;
}

PFN_vkVoidFunction
vk_instance_get_proc_addr_unchecked(const struct vk_instance *instance,
                                    const char *name)
{
   PFN_vkVoidFunction func;

   if (instance == NULL || name == NULL)
      return NULL;

   func = vk_instance_dispatch_table_get(&instance->dispatch_table, name);
   if (func != NULL)
      return func;

   func = vk_physical_device_dispatch_table_get(
      &vk_physical_device_trampolines, name);
   if (func != NULL)
      return func;

   func = vk_device_dispatch_table_get(&vk_device_trampolines, name);
   if (func != NULL)
      return func;

   return NULL;
}

PFN_vkVoidFunction
vk_instance_get_physical_device_proc_addr(const struct vk_instance *instance,
                                          const char *name)
{
   if (instance == NULL || name == NULL)
      return NULL;

   return vk_physical_device_dispatch_table_get_if_supported(&vk_physical_device_trampolines,
                                                             name,
                                                             instance->app_info.api_version,
                                                             &instance->enabled_extensions);
}
