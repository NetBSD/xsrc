/*
 * Copyright © 2017 Google
 * Copyright © 2019 Red Hat
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

/* Rules for device selection.
 * Is there an X or wayland connection open (or DISPLAY set).
 * If no - try and find which device was the boot_vga device.
 * If yes - try and work out which device is the connection primary,
 * DRI_PRIME tagged overrides only work if bus info, =1 will just pick an alternate.
 */

#include <vulkan/vk_layer.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "device_select.h"
#include "c99_compat.h"
#include "hash_table.h"
#include "vk_util.h"
#include "c11/threads.h"

struct instance_info {
   PFN_vkDestroyInstance DestroyInstance;
   PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
   PFN_vkEnumeratePhysicalDeviceGroups EnumeratePhysicalDeviceGroups;
   PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
   PFN_GetPhysicalDeviceProcAddr  GetPhysicalDeviceProcAddr;
   PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
   PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
   PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2;
   bool has_pci_bus, has_vulkan11;
   bool has_wayland, has_xcb;
};

static struct hash_table *device_select_instance_ht = NULL;
static mtx_t device_select_mutex;

static once_flag device_select_is_init = ONCE_FLAG_INIT;

static void device_select_once_init(void) {
   mtx_init(&device_select_mutex, mtx_plain);
}

static void
device_select_init_instances(void)
{
   call_once(&device_select_is_init, device_select_once_init);

   mtx_lock(&device_select_mutex);
   if (!device_select_instance_ht)
      device_select_instance_ht = _mesa_hash_table_create(NULL, _mesa_hash_pointer,
							  _mesa_key_pointer_equal);
   mtx_unlock(&device_select_mutex);
}

static void
device_select_try_free_ht(void)
{
   mtx_lock(&device_select_mutex);
   if (device_select_instance_ht) {
      if (_mesa_hash_table_num_entries(device_select_instance_ht) == 0) {
	 _mesa_hash_table_destroy(device_select_instance_ht, NULL);
	 device_select_instance_ht = NULL;
      }
   }
   mtx_unlock(&device_select_mutex);
}

static void
device_select_layer_add_instance(VkInstance instance, struct instance_info *info)
{
   device_select_init_instances();
   mtx_lock(&device_select_mutex);
   _mesa_hash_table_insert(device_select_instance_ht, instance, info);
   mtx_unlock(&device_select_mutex);
}

static struct instance_info *
device_select_layer_get_instance(VkInstance instance)
{
   struct hash_entry *entry;
   struct instance_info *info = NULL;
   mtx_lock(&device_select_mutex);
   entry = _mesa_hash_table_search(device_select_instance_ht, (void *)instance);
   if (entry)
      info = (struct instance_info *)entry->data;
   mtx_unlock(&device_select_mutex);
   return info;
}

static void
device_select_layer_remove_instance(VkInstance instance)
{
   mtx_lock(&device_select_mutex);
   _mesa_hash_table_remove_key(device_select_instance_ht, instance);
   mtx_unlock(&device_select_mutex);
   device_select_try_free_ht();
}

static VkResult device_select_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
					     const VkAllocationCallbacks *pAllocator,
					     VkInstance *pInstance)
{
   VkLayerInstanceCreateInfo *chain_info;
   for(chain_info = (VkLayerInstanceCreateInfo*)pCreateInfo->pNext; chain_info; chain_info = (VkLayerInstanceCreateInfo*)chain_info->pNext)
      if(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == VK_LAYER_LINK_INFO)
         break;

   assert(chain_info->u.pLayerInfo);
   struct instance_info *info = (struct instance_info *)calloc(1, sizeof(struct instance_info));

   info->GetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
   PFN_vkCreateInstance fpCreateInstance =
      (PFN_vkCreateInstance)info->GetInstanceProcAddr(NULL, "vkCreateInstance");
   if (fpCreateInstance == NULL) {
      free(info);
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

   VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
   if (result != VK_SUCCESS) {
      free(info);
      return result;
   }

   for (unsigned i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
      if (!strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME))
         info->has_wayland = true;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
      if (!strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_XCB_SURFACE_EXTENSION_NAME))
         info->has_xcb = true;
#endif
   }

   /*
    * The loader is currently not able to handle GetPhysicalDeviceProperties2KHR calls in
    * EnumeratePhysicalDevices when there are other layers present. To avoid mysterious crashes
    * for users just use only the vulkan version for now.
    */
   info->has_vulkan11 = pCreateInfo->pApplicationInfo &&
                        pCreateInfo->pApplicationInfo->apiVersion >= VK_MAKE_VERSION(1, 1, 0);

   info->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr)info->GetInstanceProcAddr(*pInstance, "vk_layerGetPhysicalDeviceProcAddr");
#define DEVSEL_GET_CB(func) info->func = (PFN_vk##func)info->GetInstanceProcAddr(*pInstance, "vk" #func)
   DEVSEL_GET_CB(DestroyInstance);
   DEVSEL_GET_CB(EnumeratePhysicalDevices);
   DEVSEL_GET_CB(EnumeratePhysicalDeviceGroups);
   DEVSEL_GET_CB(GetPhysicalDeviceProperties);
   DEVSEL_GET_CB(EnumerateDeviceExtensionProperties);
   if (info->has_vulkan11)
      DEVSEL_GET_CB(GetPhysicalDeviceProperties2);
#undef DEVSEL_GET_CB

   device_select_layer_add_instance(*pInstance, info);

   return VK_SUCCESS;
}

static void device_select_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
   struct instance_info *info = device_select_layer_get_instance(instance);

   device_select_layer_remove_instance(instance);
   info->DestroyInstance(instance, pAllocator);
   free(info);
}

static void get_device_properties(const struct instance_info *info, VkPhysicalDevice device, VkPhysicalDeviceProperties2 *properties)
{
    info->GetPhysicalDeviceProperties(device, &properties->properties);

    if (info->GetPhysicalDeviceProperties2 && properties->properties.apiVersion >= VK_API_VERSION_1_1)
        info->GetPhysicalDeviceProperties2(device, properties);
}

static void print_gpu(const struct instance_info *info, unsigned index, VkPhysicalDevice device)
{
   const char *type = "";
   VkPhysicalDevicePCIBusInfoPropertiesEXT ext_pci_properties = (VkPhysicalDevicePCIBusInfoPropertiesEXT) {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT
   };
   VkPhysicalDeviceProperties2KHR properties = (VkPhysicalDeviceProperties2KHR){
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR
   };
   if (info->has_vulkan11 && info->has_pci_bus)
      properties.pNext = &ext_pci_properties;
   get_device_properties(info, device, &properties);

   switch(properties.properties.deviceType) {
   case VK_PHYSICAL_DEVICE_TYPE_OTHER:
   default:
      type = "other";
      break;
   case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      type = "integrated GPU";
      break;
   case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      type = "discrete GPU";
      break;
   case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      type = "virtual GPU";
      break;
   case VK_PHYSICAL_DEVICE_TYPE_CPU:
      type = "CPU";
      break;
   }
   fprintf(stderr, "  GPU %d: %x:%x \"%s\" %s", index, properties.properties.vendorID,
           properties.properties.deviceID, properties.properties.deviceName, type);
   if (info->has_pci_bus)
      fprintf(stderr, " %04x:%02x:%02x.%x", ext_pci_properties.pciDomain,
              ext_pci_properties.pciBus, ext_pci_properties.pciDevice,
              ext_pci_properties.pciFunction);
   fprintf(stderr, "\n");
}

static bool fill_drm_device_info(const struct instance_info *info,
                                 struct device_pci_info *drm_device,
                                 VkPhysicalDevice device)
{
   VkPhysicalDevicePCIBusInfoPropertiesEXT ext_pci_properties = (VkPhysicalDevicePCIBusInfoPropertiesEXT) {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT
   };

   VkPhysicalDeviceProperties2KHR properties = (VkPhysicalDeviceProperties2KHR){
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR
   };

   if (info->has_vulkan11 && info->has_pci_bus)
      properties.pNext = &ext_pci_properties;
   get_device_properties(info, device, &properties);

   drm_device->cpu_device = properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
   drm_device->dev_info.vendor_id = properties.properties.vendorID;
   drm_device->dev_info.device_id = properties.properties.deviceID;
   if (info->has_vulkan11 && info->has_pci_bus) {
     drm_device->has_bus_info = true;
     drm_device->bus_info.domain = ext_pci_properties.pciDomain;
     drm_device->bus_info.bus = ext_pci_properties.pciBus;
     drm_device->bus_info.dev = ext_pci_properties.pciDevice;
     drm_device->bus_info.func = ext_pci_properties.pciFunction;
   }
   return drm_device->cpu_device;
}

static int device_select_find_explicit_default(struct device_pci_info *pci_infos,
                                               uint32_t device_count,
                                               const char *selection)
{
   int default_idx = -1;
   unsigned vendor_id, device_id;
   int matched = sscanf(selection, "%x:%x", &vendor_id, &device_id);
   if (matched != 2)
      return default_idx;

   for (unsigned i = 0; i < device_count; ++i) {
      if (pci_infos[i].dev_info.vendor_id == vendor_id &&
          pci_infos[i].dev_info.device_id == device_id)
         default_idx = i;
   }
   return default_idx;
}

static int device_select_find_dri_prime_tag_default(struct device_pci_info *pci_infos,
                                                    uint32_t device_count,
                                                    const char *dri_prime)
{
   int default_idx = -1;
   for (unsigned i = 0; i < device_count; ++i) {
      char *tag = NULL;
      if (asprintf(&tag, "pci-%04x_%02x_%02x_%1u",
                   pci_infos[i].bus_info.domain,
                   pci_infos[i].bus_info.bus,
                   pci_infos[i].bus_info.dev,
                   pci_infos[i].bus_info.func) >= 0) {
         if (strcmp(dri_prime, tag))
            default_idx = i;
      }
      free(tag);
   }
   return default_idx;
}

static int device_select_find_boot_vga_default(struct device_pci_info *pci_infos,
                                               uint32_t device_count)
{
   char boot_vga_path[1024];
   int default_idx = -1;
   for (unsigned i = 0; i < device_count; ++i) {
      /* fallback to probing the pci bus boot_vga device. */
      snprintf(boot_vga_path, 1023, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/boot_vga", pci_infos[i].bus_info.domain,
               pci_infos[i].bus_info.bus, pci_infos[i].bus_info.dev, pci_infos[i].bus_info.func);
      int fd = open(boot_vga_path, O_RDONLY);
      if (fd != -1) {
         uint8_t val;
         if (read(fd, &val, 1) == 1) {
            if (val == '1')
               default_idx = i;
         }
         close(fd);
      }
      if (default_idx != -1)
         break;
   }
   return default_idx;
}

static int device_select_find_non_cpu(struct device_pci_info *pci_infos,
                                      uint32_t device_count)
{
   int default_idx = -1;

   /* pick first GPU device */
   for (unsigned i = 0; i < device_count; ++i) {
      if (!pci_infos[i].cpu_device){
         default_idx = i;
         break;
      }
   }
   return default_idx;
}

static int find_non_cpu_skip(struct device_pci_info *pci_infos,
                        uint32_t device_count,
                        int skip_idx)
{
   for (unsigned i = 0; i < device_count; ++i) {
      if (i == skip_idx)
         continue;
      if (pci_infos[i].cpu_device)
         continue;
      return i;
   }
   return -1;
}

static uint32_t get_default_device(const struct instance_info *info,
                                   const char *selection,
                                   uint32_t physical_device_count,
                                   VkPhysicalDevice *pPhysicalDevices)
{
   int default_idx = -1;
   const char *dri_prime = getenv("DRI_PRIME");
   bool dri_prime_is_one = false;
   int cpu_count = 0;
   if (dri_prime && !strcmp(dri_prime, "1"))
      dri_prime_is_one = true;

   if (dri_prime && !dri_prime_is_one && !info->has_pci_bus) {
      fprintf(stderr, "device-select: cannot correctly use DRI_PRIME tag\n");
   }

   struct device_pci_info *pci_infos = (struct device_pci_info *)calloc(physical_device_count, sizeof(struct device_pci_info));
   if (!pci_infos)
     return 0;

   for (unsigned i = 0; i < physical_device_count; ++i) {
      cpu_count += fill_drm_device_info(info, &pci_infos[i], pPhysicalDevices[i]) ? 1 : 0;
   }

   if (selection)
      default_idx = device_select_find_explicit_default(pci_infos, physical_device_count, selection);
   if (default_idx == -1 && info->has_pci_bus && dri_prime && !dri_prime_is_one)
      default_idx = device_select_find_dri_prime_tag_default(pci_infos, physical_device_count, dri_prime);
   if (default_idx == -1 && info->has_wayland)
      default_idx = device_select_find_wayland_pci_default(pci_infos, physical_device_count);
   if (default_idx == -1 && info->has_xcb)
      default_idx = device_select_find_xcb_pci_default(pci_infos, physical_device_count);
   if (default_idx == -1 && info->has_pci_bus)
      default_idx = device_select_find_boot_vga_default(pci_infos, physical_device_count);
   if (default_idx == -1 && cpu_count)
      default_idx = device_select_find_non_cpu(pci_infos, physical_device_count);

   /* DRI_PRIME=1 handling - pick any other device than default. */
   if (default_idx != -1 && dri_prime_is_one && physical_device_count > (cpu_count + 1)) {
      if (default_idx == 0 || default_idx == 1)
         default_idx = find_non_cpu_skip(pci_infos, physical_device_count, default_idx);
   }
   free(pci_infos);
   return default_idx == -1 ? 0 : default_idx;
}

static VkResult device_select_EnumeratePhysicalDevices(VkInstance instance,
						       uint32_t* pPhysicalDeviceCount,
						       VkPhysicalDevice *pPhysicalDevices)
{
   struct instance_info *info = device_select_layer_get_instance(instance);
   uint32_t physical_device_count = 0;
   uint32_t selected_physical_device_count = 0;
   const char* selection = getenv("MESA_VK_DEVICE_SELECT");
   VkResult result = info->EnumeratePhysicalDevices(instance, &physical_device_count, NULL);
   VK_OUTARRAY_MAKE(out, pPhysicalDevices, pPhysicalDeviceCount);
   if (result != VK_SUCCESS)
      return result;

   VkPhysicalDevice *physical_devices = (VkPhysicalDevice*)calloc(sizeof(VkPhysicalDevice),  physical_device_count);
   VkPhysicalDevice *selected_physical_devices = (VkPhysicalDevice*)calloc(sizeof(VkPhysicalDevice),
                                                                           physical_device_count);

   if (!physical_devices || !selected_physical_devices) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto out;
   }

   result = info->EnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);
   if (result != VK_SUCCESS)
      goto out;

   for (unsigned i = 0; i < physical_device_count; i++) {
      uint32_t count;
      info->EnumerateDeviceExtensionProperties(physical_devices[i], NULL, &count, NULL);
      if (count > 0) {
	 VkExtensionProperties *extensions = calloc(count, sizeof(VkExtensionProperties));
         if (info->EnumerateDeviceExtensionProperties(physical_devices[i], NULL, &count, extensions) == VK_SUCCESS) {
	    for (unsigned j = 0; j < count; j++) {
               if (!strcmp(extensions[j].extensionName, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME))
                  info->has_pci_bus = true;
            }
         }
	 free(extensions);
      }
   }
   if (selection && strcmp(selection, "list") == 0) {
      fprintf(stderr, "selectable devices:\n");
      for (unsigned i = 0; i < physical_device_count; ++i)
         print_gpu(info, i, physical_devices[i]);
      exit(0);
   } else {
      unsigned selected_index = get_default_device(info, selection, physical_device_count, physical_devices);
      selected_physical_device_count = physical_device_count;
      selected_physical_devices[0] = physical_devices[selected_index];
      for (unsigned i = 0; i < physical_device_count - 1; ++i) {
         unsigned  this_idx = i < selected_index ? i : i + 1;
         selected_physical_devices[i + 1] = physical_devices[this_idx];
      }
   }

   if (selected_physical_device_count == 0) {
      fprintf(stderr, "WARNING: selected no devices with MESA_VK_DEVICE_SELECT\n");
   }

   assert(result == VK_SUCCESS);

   for (unsigned i = 0; i < selected_physical_device_count; i++) {
      vk_outarray_append(&out, ent) {
         *ent = selected_physical_devices[i];
      }
   }
   result = vk_outarray_status(&out);
 out:
   free(physical_devices);
   free(selected_physical_devices);
   return result;
}

static VkResult device_select_EnumeratePhysicalDeviceGroups(VkInstance instance,
                                                            uint32_t* pPhysicalDeviceGroupCount,
                                                            VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroups)
{
   struct instance_info *info = device_select_layer_get_instance(instance);
   uint32_t physical_device_group_count = 0;
   uint32_t selected_physical_device_group_count = 0;
   VkResult result = info->EnumeratePhysicalDeviceGroups(instance, &physical_device_group_count, NULL);
   VK_OUTARRAY_MAKE(out, pPhysicalDeviceGroups, pPhysicalDeviceGroupCount);

   if (result != VK_SUCCESS)
      return result;

   VkPhysicalDeviceGroupProperties *physical_device_groups = (VkPhysicalDeviceGroupProperties*)calloc(sizeof(VkPhysicalDeviceGroupProperties), physical_device_group_count);
   VkPhysicalDeviceGroupProperties *selected_physical_device_groups = (VkPhysicalDeviceGroupProperties*)calloc(sizeof(VkPhysicalDeviceGroupProperties), physical_device_group_count);

   if (!physical_device_groups || !selected_physical_device_groups) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto out;
   }

   result = info->EnumeratePhysicalDeviceGroups(instance, &physical_device_group_count, physical_device_groups);
   if (result != VK_SUCCESS)
      goto out;

   /* just sort groups with CPU devices to the end? - assume nobody will mix these */
   int num_gpu_groups = 0;
   int num_cpu_groups = 0;
   selected_physical_device_group_count = physical_device_group_count;
   for (unsigned i = 0; i < physical_device_group_count; i++) {
      bool group_has_cpu_device = false;
      for (unsigned j = 0; j < physical_device_groups[i].physicalDeviceCount; j++) {
         VkPhysicalDevice physical_device = physical_device_groups[i].physicalDevices[j];
         VkPhysicalDeviceProperties2KHR properties = (VkPhysicalDeviceProperties2KHR){
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR
         };
         info->GetPhysicalDeviceProperties(physical_device, &properties.properties);
         group_has_cpu_device = properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
      }

      if (group_has_cpu_device) {
         selected_physical_device_groups[physical_device_group_count - num_cpu_groups - 1] = physical_device_groups[i];
         num_cpu_groups++;
      } else {
         selected_physical_device_groups[num_gpu_groups] = physical_device_groups[i];
         num_gpu_groups++;
      }
   }

   assert(result == VK_SUCCESS);

   for (unsigned i = 0; i < selected_physical_device_group_count; i++) {
      vk_outarray_append(&out, ent) {
         *ent = selected_physical_device_groups[i];
      }
   }
   result = vk_outarray_status(&out);
out:
   free(physical_device_groups);
   free(selected_physical_device_groups);
   return result;
}

static void  (*get_pdevice_proc_addr(VkInstance instance, const char* name))()
{
   struct instance_info *info = device_select_layer_get_instance(instance);
   return info->GetPhysicalDeviceProcAddr(instance, name);
}

static void  (*get_instance_proc_addr(VkInstance instance, const char* name))()
{
   if (strcmp(name, "vkGetInstanceProcAddr") == 0)
      return (void(*)())get_instance_proc_addr;
   if (strcmp(name, "vkCreateInstance") == 0)
      return (void(*)())device_select_CreateInstance;
   if (strcmp(name, "vkDestroyInstance") == 0)
      return (void(*)())device_select_DestroyInstance;
   if (strcmp(name, "vkEnumeratePhysicalDevices") == 0)
      return (void(*)())device_select_EnumeratePhysicalDevices;
   if (strcmp(name, "vkEnumeratePhysicalDeviceGroups") == 0)
      return (void(*)())device_select_EnumeratePhysicalDeviceGroups;

   struct instance_info *info = device_select_layer_get_instance(instance);
   return info->GetInstanceProcAddr(instance, name);
}

VK_LAYER_EXPORT VkResult vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct)
{
   if (pVersionStruct->loaderLayerInterfaceVersion < 2)
      return VK_ERROR_INITIALIZATION_FAILED;
   pVersionStruct->loaderLayerInterfaceVersion = 2;

   pVersionStruct->pfnGetInstanceProcAddr = get_instance_proc_addr;
   pVersionStruct->pfnGetPhysicalDeviceProcAddr = get_pdevice_proc_addr;

   return VK_SUCCESS;
}
