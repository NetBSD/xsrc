/*
 * Copyright (c) 2008-2015 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vdpau/vdpau_x11.h>
#if DRI2
#include "mesa_dri2.h"
#include <X11/Xlib.h>
#endif
#include "util.h"

typedef void SetDllHandle(
    void * driver_dll_handle
);

static void * _vdp_backend_dll;
static void * _vdp_trace_dll;
static void * _vdp_driver_dll;
static VdpDeviceCreateX11 * _vdp_imp_device_create_x11_proc;

#if defined(__GNUC__)

static void _vdp_close_driver(void) __attribute__((destructor));

#endif

#if DEBUG

static void _vdp_wrapper_error_breakpoint(char const * file, int line, char const * function)
{
    fprintf(stderr, "VDPAU wrapper: Error detected at %s:%d %s()\n", file, line, function);
}

#define _VDP_ERROR_BREAKPOINT() _vdp_wrapper_error_breakpoint(__FILE__, __LINE__, __FUNCTION__)

#else

#define _VDP_ERROR_BREAKPOINT()

#endif

#define DRIVER_FALLBACK_LIB_FORMAT "libvdpau_%s.so"
#define DRIVER_LIB_FORMAT "%s/libvdpau_%s.so.1"

static char * _vdp_get_driver_name_from_dri2(
    Display *             display,
    int                   screen
)
{
    char * driver_name = NULL;
#if DRI2
    Window root = RootWindow(display, screen);
    int event_base, error_base;
    int major, minor;
    char * device_name;

    if (!_vdp_DRI2QueryExtension(display, &event_base, &error_base)) {
        return NULL;
    }

    if (!_vdp_DRI2QueryVersion(display, &major, &minor) ||
            (major < 1 || (major == 1 && minor < 2))) {
        _vdp_DRI2RemoveExtension(display);
        return NULL;
    }

    if (!_vdp_DRI2Connect(display, root, &driver_name, &device_name)) {
        _vdp_DRI2RemoveExtension(display);
        return NULL;
    }

    XFree(device_name);
    _vdp_DRI2RemoveExtension(display);
#else
    (void) display; (void) screen;
#endif /* DRI2 */
    return driver_name;
}

static VdpStatus _vdp_open_driver(
    Display *             display,
    int                   screen)
{
    char const * vdpau_driver;
    char * vdpau_driver_dri2 = NULL;
    const char * vdpau_driver_path = NULL;
    char         vdpau_driver_lib[PATH_MAX];
    char const * vdpau_trace;
    char const * func_name;

    vdpau_driver = secure_getenv("VDPAU_DRIVER");
    if (vdpau_driver) {
        if (strchr(vdpau_driver, '/')) {
            vdpau_driver = NULL;
        }
    }
    if (!vdpau_driver) {
        vdpau_driver = vdpau_driver_dri2 =
            _vdp_get_driver_name_from_dri2(display, screen);
    }
    if (!vdpau_driver) {
        vdpau_driver = "nvidia";
    }

    /* Don't allow setuid apps to use VDPAU_DRIVER_PATH */
    vdpau_driver_path = secure_getenv("VDPAU_DRIVER_PATH");
    if (vdpau_driver_path &&
        snprintf(vdpau_driver_lib, sizeof(vdpau_driver_lib),
                 DRIVER_LIB_FORMAT, vdpau_driver_path, vdpau_driver) <
            sizeof(vdpau_driver_lib)) {
        _vdp_driver_dll = dlopen(vdpau_driver_lib, RTLD_NOW | RTLD_GLOBAL);
    }

    /* Fallback to VDPAU_MODULEDIR when VDPAU_DRIVER_PATH is not set,
     * or if we fail to create the driver path/dlopen the library. */
    if (!_vdp_driver_dll) {
        if (snprintf(vdpau_driver_lib, sizeof(vdpau_driver_lib),
                     DRIVER_LIB_FORMAT, VDPAU_MODULEDIR, vdpau_driver) >=
                sizeof(vdpau_driver_lib)) {
            fprintf(stderr, "Failed to construct driver path: path too long\n");
            if (vdpau_driver_dri2) {
                XFree(vdpau_driver_dri2);
                vdpau_driver_dri2 = NULL;
            }
            _VDP_ERROR_BREAKPOINT();
            return VDP_STATUS_NO_IMPLEMENTATION;
        }
        else {
            _vdp_driver_dll = dlopen(vdpau_driver_lib, RTLD_NOW | RTLD_GLOBAL);
        }
    }

    if (!_vdp_driver_dll) {
        /* Try again using the old path, which is guaranteed to fit in PATH_MAX
         * if the complete path fit above. */
        snprintf(vdpau_driver_lib, sizeof(vdpau_driver_lib),
                 DRIVER_FALLBACK_LIB_FORMAT, vdpau_driver);
        _vdp_driver_dll = dlopen(vdpau_driver_lib, RTLD_NOW | RTLD_GLOBAL);
    }

    if (vdpau_driver_dri2) {
        XFree(vdpau_driver_dri2);
        vdpau_driver_dri2 = NULL;
    }

    if (!_vdp_driver_dll) {
        fprintf(stderr, "Failed to open VDPAU backend %s\n", dlerror());
        _VDP_ERROR_BREAKPOINT();
        return VDP_STATUS_NO_IMPLEMENTATION;
    }

    _vdp_backend_dll = _vdp_driver_dll;

    vdpau_trace = secure_getenv("VDPAU_TRACE");
    if (vdpau_trace && atoi(vdpau_trace)) {
        SetDllHandle * set_dll_handle;

        _vdp_trace_dll = dlopen(VDPAU_MODULEDIR "/libvdpau_trace.so.1",
                                RTLD_NOW | RTLD_GLOBAL);
        if (!_vdp_trace_dll) {
            fprintf(stderr, "Failed to open VDPAU trace library %s\n", dlerror());
            _VDP_ERROR_BREAKPOINT();
            return VDP_STATUS_NO_IMPLEMENTATION;
        }

        set_dll_handle = (SetDllHandle*)dlsym(
            _vdp_trace_dll,
            "vdp_trace_set_backend_handle"
        );
        if (!set_dll_handle) {
            fprintf(stderr, "%s\n", dlerror());
            _VDP_ERROR_BREAKPOINT();
            return VDP_STATUS_NO_IMPLEMENTATION;
        }

        set_dll_handle(_vdp_backend_dll);

        _vdp_backend_dll = _vdp_trace_dll;

        func_name = "vdp_trace_device_create_x11";
    }
    else {
        func_name = "vdp_imp_device_create_x11";
    }

    _vdp_imp_device_create_x11_proc = (VdpDeviceCreateX11*)dlsym(
        _vdp_backend_dll,
        func_name
    );
    if (!_vdp_imp_device_create_x11_proc) {
        fprintf(stderr, "%s\n", dlerror());
        _VDP_ERROR_BREAKPOINT();
        return VDP_STATUS_NO_IMPLEMENTATION;
    }

    return VDP_STATUS_OK;
}

static void _vdp_close_driver(void)
{
    if (_vdp_driver_dll) {
        dlclose(_vdp_driver_dll);
        _vdp_driver_dll = NULL;
    }
    if (_vdp_trace_dll) {
        dlclose(_vdp_trace_dll);
        _vdp_trace_dll = NULL;
    }
    _vdp_backend_dll = NULL;
    _vdp_imp_device_create_x11_proc = NULL;
}

static VdpGetProcAddress * _imp_get_proc_address;
static VdpVideoSurfacePutBitsYCbCr * _imp_vid_put_bits_y_cb_cr;
static VdpPresentationQueueSetBackgroundColor * _imp_pq_set_bg_color;
static int _running_under_flash;
static int _enable_flash_uv_swap = 1;
static int _disable_flash_pq_bg_color = 1;

static VdpStatus vid_put_bits_y_cb_cr_swapped(
    VdpVideoSurface      surface,
    VdpYCbCrFormat       source_ycbcr_format,
    void const * const * source_data,
    uint32_t const *     source_pitches
)
{
    void const * data_reordered[3];
    void const * const * data;

    if (source_ycbcr_format == VDP_YCBCR_FORMAT_YV12) {
        data_reordered[0] = source_data[0];
        data_reordered[1] = source_data[2];
        data_reordered[2] = source_data[1];
        /*
         * source_pitches[1] and source_pitches[2] should be equal,
         * so no need to re-order.
         */
        data = data_reordered;
    }
    else {
        data = source_data;
    }

    return _imp_vid_put_bits_y_cb_cr(
        surface,
        source_ycbcr_format,
        data,
        source_pitches
    );
}

static VdpStatus pq_set_bg_color_noop(
    VdpPresentationQueue presentation_queue,
    VdpColor * const     background_color
)
{
    (void) presentation_queue; (void) background_color;
    return VDP_STATUS_OK;
}

static VdpStatus vdp_wrapper_get_proc_address(
    VdpDevice device,
    VdpFuncId function_id,
    /* output parameters follow */
    void * *  function_pointer
)
{
    VdpStatus status;

    status = _imp_get_proc_address(device, function_id, function_pointer);
    if (status != VDP_STATUS_OK) {
        return status;
    }

    if (_running_under_flash) {
        switch (function_id) {
        case VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR:
            if (_enable_flash_uv_swap) {
                _imp_vid_put_bits_y_cb_cr = *function_pointer;
                *function_pointer = vid_put_bits_y_cb_cr_swapped;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR:
            if (_disable_flash_pq_bg_color) {
                _imp_pq_set_bg_color = *function_pointer;
                *function_pointer = pq_set_bg_color_noop;
            }
            break;
        default:
            break;
        }
    }

    return VDP_STATUS_OK;
}

static void init_running_under_flash(void)
{
    FILE *fp;
    char buffer[1024];
    int ret, i;

    fp = fopen("/proc/self/cmdline", "r");
    if (!fp) {
        return;
    }
    ret = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);
    if (ret < 0) {
        return;
    }
    /*
     * Sometimes the file contains null between arguments. Wipe these out so
     * strstr doesn't stop early.
     */
    for (i = 0; i < ret; i++) {
        if (buffer[i] == '\0') {
            buffer[i] = 'x';
        }
    }
    buffer[ret] = '\0';

    if (strstr(buffer, "libflashplayer") != NULL) {
        _running_under_flash = 1;
    }
}

static void init_config(void)
{
    FILE *fp;
    char buffer[1024];

    fp = fopen(VDPAU_SYSCONFDIR "/vdpau_wrapper.cfg", "r");
    if (!fp) {
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char * equals = strchr(buffer, '=');
        char * param;

        if (equals == NULL) {
            continue;
        }

        *equals = '\0';
        param = equals + 1;

        if (!strcmp(buffer, "enable_flash_uv_swap")) {
            _enable_flash_uv_swap = atoi(param);
        }
        else if (!strcmp(buffer, "disable_flash_pq_bg_color")) {
            _disable_flash_pq_bg_color = atoi(param);
        }
    }

    fclose(fp);
}

static void init_fixes(void)
{
    init_running_under_flash();
    init_config();
}

VdpStatus vdp_device_create_x11(
    Display *             display,
    int                   screen,
    /* output parameters follow */
    VdpDevice *           device,
    VdpGetProcAddress * * get_proc_address
)
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    VdpGetProcAddress *gpa;
    VdpStatus status = VDP_STATUS_OK;

    pthread_once(&once, init_fixes);

    pthread_mutex_lock(&lock);
    if (!_vdp_imp_device_create_x11_proc) {
        status = _vdp_open_driver(display, screen);
        if (status != VDP_STATUS_OK)
            _vdp_close_driver();
    }
    pthread_mutex_unlock(&lock);

    if (status != VDP_STATUS_OK)
        return status;

    status = _vdp_imp_device_create_x11_proc(display, screen, device, &gpa);
    if (status != VDP_STATUS_OK) {
        return status;
    }

    *get_proc_address = vdp_wrapper_get_proc_address;

    pthread_mutex_lock(&lock);
    if (_imp_get_proc_address != gpa) {
        if (_imp_get_proc_address == NULL)
            _imp_get_proc_address = gpa;
        else
        /* Currently the wrapper can only deal with one back-end.
         * This should never happen, but better safe than sorry. */
            status = VDP_STATUS_NO_IMPLEMENTATION;
    }
    pthread_mutex_unlock(&lock);

    if (status != VDP_STATUS_OK) {
        void *pv;

        if (gpa(*device, VDP_FUNC_ID_DEVICE_DESTROY, &pv) == VDP_STATUS_OK) {
            VdpDeviceDestroy *device_destroy = pv;

            device_destroy(*device);
        }
    }

    return status;
}
