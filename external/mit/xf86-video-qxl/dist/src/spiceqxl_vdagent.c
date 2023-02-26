#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include "qxl_option_helpers.h"

#include "spiceqxl_util.h"
#include "spiceqxl_uinput.h"
#include "spiceqxl_vdagent.h"

static const char *vdagent_virtio_filename;
static int virtio_fd;
static int virtio_client_fd = -1;
static SpiceWatch *virtio_client_watch;

typedef struct XSpiceVdagentCharDeviceInstance {
    SpiceCharDeviceInstance base;
    qxl_screen_t *qxl;
} XSpiceVdagentCharDeviceInstance;

static XSpiceVdagentCharDeviceInstance vdagent_sin = {
    .base = {
        .subtype = "vdagent"
    }
};

static int vmc_write(SpiceCharDeviceInstance *sin, const uint8_t *buf, int len)
{
    int written;

    if (virtio_client_fd == -1) {
        return 0;
    }
    written = send(virtio_client_fd, buf, len, 0);
    if (written != len) {
        fprintf(stderr, "%s: ERROR: short write to vdagentd - TODO buffering\n", __func__);
    }
    return written;
}

static int vmc_read(SpiceCharDeviceInstance *sin, uint8_t *buf, int len)
{
    int nbytes;

    if (virtio_client_fd == -1) {
        return 0;
    }
    nbytes = recv(virtio_client_fd, buf, len, 0);
    if (nbytes <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return 0;
        }
        fprintf(stderr, "ERROR: vdagent died\n");
        close(virtio_client_fd);
        virtio_client_fd = -1;
        vdagent_sin.qxl->core->watch_remove(virtio_client_watch);
        virtio_client_watch = NULL;
        spice_server_remove_interface(&vdagent_sin.base.base);
        spiceqxl_uinput_watch(vdagent_sin.qxl, FALSE);
    }
    return nbytes;
}

static void on_read_available(int fd, int event, void *opaque)
{
    if (virtio_client_fd == -1) {
        return;
    }
    spice_server_char_device_wakeup(&vdagent_sin.base);
}

#if SPICE_SERVER_VERSION >= 0x000c02
static void vmc_event(SpiceCharDeviceInstance *sin, uint8_t event)
{
}
#endif

static void vmc_state(SpiceCharDeviceInstance *sin, int connected)
{
}

static SpiceCharDeviceInterface vmc_interface = {
    .base.type          = SPICE_INTERFACE_CHAR_DEVICE,
    .base.description   = "Xspice virtual channel char device",
    .base.major_version = SPICE_INTERFACE_CHAR_DEVICE_MAJOR,
    .base.minor_version = SPICE_INTERFACE_CHAR_DEVICE_MINOR,
    .state              = vmc_state,
    .write              = vmc_write,
    .read               = vmc_read,
#if SPICE_SERVER_VERSION >= 0x000c02
    .event              = vmc_event,
#endif
};

static void on_accept(int fd, int event, void *opaque)
{
    qxl_screen_t *qxl = opaque;
    struct sockaddr_un address;
    socklen_t length = sizeof(address);
    int flags;

    virtio_client_fd = accept(virtio_fd, (struct sockaddr *)&address, &length);
    if (virtio_client_fd == -1) {
        fprintf(stderr, "error accepting on unix domain socket: %s\n", strerror(errno));
        return;
    }
    flags = fcntl(virtio_client_fd, F_GETFL);
    if (flags == -1) {
        fprintf(stderr, "error getting flags from uds client fd: %s\n", strerror(errno));
        goto error;
    }
    if (fcntl(virtio_client_fd, F_SETFL, flags | O_NONBLOCK | O_CLOEXEC) == -1) {
        fprintf(stderr, "error setting CLOEXEC & NONBLOCK flags from uds client fd: %s\n",
                strerror(errno));
        goto error;
    }
    virtio_client_watch = qxl->core->watch_add(virtio_client_fd, SPICE_WATCH_EVENT_READ
        /* TODO - SPICE_WATCH_EVENT_WRITE */, on_read_available, qxl);

    spice_server_add_interface(qxl->spice_server, &vdagent_sin.base.base);
    spiceqxl_uinput_watch(qxl, TRUE);

    return;

error:
    if (virtio_client_fd != -1) {
        close(virtio_client_fd);
        virtio_client_fd = -1;
    }
}

void spiceqxl_vdagent_init(qxl_screen_t *qxl)
{
    struct sockaddr_un address;
    int c;
    int enabled;

    vdagent_sin.qxl = qxl;
    vdagent_virtio_filename = get_str_option(qxl->options, OPTION_SPICE_VDAGENT_VIRTIO_PATH,
               "XSPICE_VDAGENT_VIRTIO_PATH");
    enabled = get_bool_option(qxl->options, OPTION_SPICE_VDAGENT_ENABLED, "XSPICE_VDAGENT_ENABLED");

    if (!enabled || !vdagent_virtio_filename) {
        return;
    }
    
    virtio_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (virtio_fd == -1) {
        fprintf(stderr, "error creating unix domain socket\n");
        return;
    }
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s", vdagent_virtio_filename);
    c = bind(virtio_fd, (struct sockaddr *)&address, sizeof(address));
    if (c != 0) {
        fprintf(stderr, "error binding unix domain socket to %s: %s\n",
                vdagent_virtio_filename, strerror(errno));
        return;
    }
    spiceqxl_chown_agent_file(qxl, vdagent_virtio_filename);
    c = listen(virtio_fd, 1);
    if (c != 0) {
        fprintf(stderr, "error listening to unix domain socket: %s\n", strerror(errno));
        return;
    }
    qxl->core->watch_add(virtio_fd, SPICE_WATCH_EVENT_READ
        /* TODO - SPICE_WATCH_EVENT_WRITE */, on_accept, qxl);

    vdagent_sin.base.base.sif = &vmc_interface.base;
    spiceqxl_uinput_init(qxl);
}
