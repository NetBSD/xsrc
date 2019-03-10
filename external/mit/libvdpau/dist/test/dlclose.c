#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>
#include <X11/Xlib.h>

#define PASS 0
#define FAIL 1
#define SKIP 77

static int countOpenFDs(void)
{
    DIR *dir = opendir("/proc/self/fd");
    int count = 0;

    if (!dir) {
        fprintf(stderr, "Couldn't open /proc/self/fd; skipping file descriptor "
                "leak test\n");
        return 0;
    }

    while (readdir(dir) != NULL) {
        count++;
    }

    closedir(dir);
    return count;
}

int main(void)
{
    // Work around a bug in libXext: dlclosing it after it has registered the
    // Generic Event Extension causes an identical bug to the one this program
    // is trying to test for.
    int nOpenFDs = countOpenFDs();
    void *libXext = dlopen("libXext.so.6", RTLD_LAZY);
    void *libvdpau = dlopen("../src/.libs/libvdpau.so", RTLD_LAZY);
    Display *dpy = XOpenDisplay(NULL);
    VdpDeviceCreateX11 *pvdp_device_create_x11;
    VdpDevice device;
    VdpGetProcAddress *get_proc_address;
    VdpStatus status;

    if (!libXext) {
        fprintf(stderr, "Failed to open libXext.so.6: %s", dlerror());
        return SKIP;
    }

    if (!libvdpau) {
        fprintf(stderr, "Failed to open libvdpau.so: %s", dlerror());
        return FAIL;
    }

    if (!dpy) {
        fprintf(stderr, "Failed to connect to X display %s\n", XDisplayName(NULL));
        return SKIP;
    }

    pvdp_device_create_x11 = dlsym(libvdpau, "vdp_device_create_x11");
    if (!pvdp_device_create_x11) {
        fprintf(stderr, "Failed to find the symbol vdp_device_create_x11\n");
        return FAIL;
    }

    status = pvdp_device_create_x11(dpy, 0, &device, &get_proc_address);
    if (status == VDP_STATUS_OK) {
        // It's okay if creating the device fails.  This will still install the
        // DRI2 extension in libX11 and trigger the bug.
        VdpDeviceDestroy *pvdp_device_destroy;

        status = get_proc_address(device, VDP_FUNC_ID_DEVICE_DESTROY, (void**)&pvdp_device_destroy);
        if (status != VDP_STATUS_OK) {
            fprintf(stderr, "Failed to find the VdpDeviceDestroy function: %d\n", status);
            return FAIL;
        }

        pvdp_device_destroy(device);
    }

    dlclose(libvdpau);
    XCloseDisplay(dpy);

    // Make sure no file descriptors were leaked.
    if (countOpenFDs() != nOpenFDs) {
        fprintf(stderr, "Mismatch in the number of open file descriptors!\n");
        return FAIL;
    }

    return PASS;
}
