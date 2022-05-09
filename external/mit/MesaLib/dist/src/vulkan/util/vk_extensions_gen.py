COPYRIGHT = """\
/*
 * Copyright 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
"""

import xml.etree.ElementTree as et

from mako.template import Template

# Mesa-local imports must be declared in meson variable
# '{file_without_suffix}_depend_files'.
from vk_extensions import *

_TEMPLATE_H = Template(COPYRIGHT + """

#ifndef ${driver.upper()}_EXTENSIONS_H
#define ${driver.upper()}_EXTENSIONS_H

#include <stdbool.h>

%for include in includes:
#include "${include}"
%endfor

%if driver == 'vk':
#define VK_INSTANCE_EXTENSION_COUNT ${len(instance_extensions)}

extern const VkExtensionProperties vk_instance_extensions[];

struct vk_instance_extension_table {
   union {
      bool extensions[VK_INSTANCE_EXTENSION_COUNT];
      struct {
%for ext in instance_extensions:
         bool ${ext.name[3:]};
%endfor
      };
   };
};


#define VK_DEVICE_EXTENSION_COUNT ${len(device_extensions)}

extern const VkExtensionProperties vk_device_extensions[];

struct vk_device_extension_table {
   union {
      bool extensions[VK_DEVICE_EXTENSION_COUNT];
      struct {
%for ext in device_extensions:
        bool ${ext.name[3:]};
%endfor
      };
   };
};
%else:
#include "vk_extensions.h"
%endif

struct ${driver}_physical_device;

%if driver == 'vk':
#ifdef ANDROID
extern const struct vk_instance_extension_table vk_android_allowed_instance_extensions;
extern const struct vk_device_extension_table vk_android_allowed_device_extensions;
#endif
%else:
extern const struct vk_instance_extension_table ${driver}_instance_extensions_supported;

void
${driver}_physical_device_get_supported_extensions(const struct ${driver}_physical_device *device,
                                             struct vk_device_extension_table *extensions);
%endif

#endif /* ${driver.upper()}_EXTENSIONS_H */
""")

_TEMPLATE_C = Template(COPYRIGHT + """
%if driver == 'vk':
#include "vk_object.h"
%else:
#include "${driver}_private.h"
%endif

#include "${driver}_extensions.h"

%if driver == 'vk':
const VkExtensionProperties ${driver}_instance_extensions[${driver.upper()}_INSTANCE_EXTENSION_COUNT] = {
%for ext in instance_extensions:
   {"${ext.name}", ${ext.ext_version}},
%endfor
};

const VkExtensionProperties ${driver}_device_extensions[${driver.upper()}_DEVICE_EXTENSION_COUNT] = {
%for ext in device_extensions:
   {"${ext.name}", ${ext.ext_version}},
%endfor
};

#ifdef ANDROID
const struct vk_instance_extension_table vk_android_allowed_instance_extensions = {
%for ext in instance_extensions:
   .${ext.name[3:]} = ${ext.c_android_condition()},
%endfor
};

extern const struct vk_device_extension_table vk_android_allowed_device_extensions = {
%for ext in device_extensions:
   .${ext.name[3:]} = ${ext.c_android_condition()},
%endfor
};
#endif
%endif

%if driver != 'vk':
#include "vk_util.h"

/* Convert the VK_USE_PLATFORM_* defines to booleans */
%for platform_define in platform_defines:
#ifdef ${platform_define}
#   undef ${platform_define}
#   define ${platform_define} true
#else
#   define ${platform_define} false
#endif
%endfor

/* And ANDROID too */
#ifdef ANDROID
#   undef ANDROID
#   define ANDROID true
#else
#   define ANDROID false
#   define ANDROID_API_LEVEL 0
#endif

#define ${driver.upper()}_HAS_SURFACE (VK_USE_PLATFORM_WIN32_KHR || \\
                                       VK_USE_PLATFORM_WAYLAND_KHR || \\
                                       VK_USE_PLATFORM_XCB_KHR || \\
                                       VK_USE_PLATFORM_XLIB_KHR || \\
                                       VK_USE_PLATFORM_DISPLAY_KHR)

static const uint32_t MAX_API_VERSION = ${MAX_API_VERSION.c_vk_version()};

VKAPI_ATTR VkResult VKAPI_CALL ${driver}_EnumerateInstanceVersion(
    uint32_t*                                   pApiVersion)
{
    *pApiVersion = MAX_API_VERSION;
    return VK_SUCCESS;
}

const struct vk_instance_extension_table ${driver}_instance_extensions_supported = {
%for ext in instance_extensions:
   .${ext.name[3:]} = ${ext.enable},
%endfor
};

uint32_t
${driver}_physical_device_api_version(struct ${driver}_physical_device *device)
{
    uint32_t version = 0;

    uint32_t override = vk_get_version_override();
    if (override)
        return MIN2(override, MAX_API_VERSION);

%for version in API_VERSIONS:
    if (!(${version.enable}))
        return version;
    version = ${version.version.c_vk_version()};

%endfor
    return version;
}

void
${driver}_physical_device_get_supported_extensions(const struct ${driver}_physical_device *device,
                                                   struct vk_device_extension_table *extensions)
{
   *extensions = (struct vk_device_extension_table) {
%for ext in device_extensions:
      .${ext.name[3:]} = ${ext.enable},
%endfor
   };
}
%endif
""")

def gen_extensions(driver, xml_files, api_versions, max_api_version,
                   extensions, out_c, out_h, includes = []):
    platform_defines = []
    for filename in xml_files:
        init_exts_from_xml(filename, extensions, platform_defines)

    for ext in extensions:
        assert ext.type == 'instance' or ext.type == 'device'

    template_env = {
        'driver': driver,
        'API_VERSIONS': api_versions,
        'MAX_API_VERSION': max_api_version,
        'instance_extensions': [e for e in extensions if e.type == 'instance'],
        'device_extensions': [e for e in extensions if e.type == 'device'],
        'platform_defines': platform_defines,
        'includes': includes,
    }

    if out_h:
        with open(out_h, 'w') as f:
            f.write(_TEMPLATE_H.render(**template_env))

    if out_c:
        with open(out_c, 'w') as f:
            f.write(_TEMPLATE_C.render(**template_env))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--out-c', help='Output C file.')
    parser.add_argument('--out-h', help='Output H file.')
    parser.add_argument('--xml',
                        help='Vulkan API XML file.',
                        required=True,
                        action='append',
                        dest='xml_files')
    args = parser.parse_args()

    extensions = []
    for filename in args.xml_files:
        extensions += get_all_exts_from_xml(filename)

    gen_extensions('vk', args.xml_files, None, None,
                   extensions, args.out_c, args.out_h, [])
