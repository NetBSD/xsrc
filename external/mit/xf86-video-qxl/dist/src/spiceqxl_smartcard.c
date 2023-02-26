/*
 * Copyright 2014 Jeremy White for CodeWeavers Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


#include "spiceqxl_smartcard.h"

typedef struct XSpiceSmartcardCharDeviceInstance {
    SpiceCharDeviceInstance base;
    qxl_screen_t *qxl;
    int listen_fd;
    int fd;
    SpiceWatch *listen_watch;
    SpiceWatch *watch;
} XSpiceSmartcardCharDeviceInstance;

static XSpiceSmartcardCharDeviceInstance smartcard_sin = {
    .base = {
        .subtype = "smartcard"
    }
};

static int smartcard_write(SpiceCharDeviceInstance *sin, const uint8_t *buf, int len)
{
    int written;

    if (smartcard_sin.fd == -1)
        return 0;

    written = write(smartcard_sin.fd, buf, len);
    if (written != len)
        ErrorF("%s: ERROR: short write to smartcard socket - TODO buffering\n", __FUNCTION__);

    return written;
}

static int smartcard_read(SpiceCharDeviceInstance *sin, uint8_t *buf, int len)
{
    int rc;

    if (smartcard_sin.fd == -1)
        return 0;

    rc = read(smartcard_sin.fd, buf, len);
    if (rc <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return 0;
        }
        ErrorF("smartcard socket died: %s\n", strerror(errno));

        smartcard_sin.qxl->core->watch_remove(smartcard_sin.watch);
        close(smartcard_sin.fd);
        smartcard_sin.fd = -1;
        smartcard_sin.watch = NULL;
    }

    return rc;
}

static void on_read_available(int fd, int event, void *opaque)
{
    spice_server_char_device_wakeup(&smartcard_sin.base);
}

static void on_accept_available(int fd, int event, void *opaque)
{
    qxl_screen_t *qxl = (qxl_screen_t *) opaque;
    int flags;
    int client_fd;

    client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
        return;

    if (smartcard_sin.fd != -1) {
        ErrorF("smartcard error: a new connection came in while an old one was active.\n");
        close(client_fd);
        return;
    }

    flags = fcntl(client_fd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    flags |= O_NONBLOCK;
    fcntl(client_fd, F_SETFL, flags);

    smartcard_sin.fd = client_fd;
    smartcard_sin.watch = qxl->core->watch_add(smartcard_sin.fd, SPICE_WATCH_EVENT_READ, on_read_available, qxl);

}


#if SPICE_SERVER_VERSION >= 0x000c02
static void smartcard_event(SpiceCharDeviceInstance *sin, uint8_t event)
{
    ErrorF("%s: unimplemented; event is %d\n", __FUNCTION__, event);
}
#endif

static void smartcard_state(SpiceCharDeviceInstance *sin, int connected)
{
    ErrorF("%s: unimplemented; connected is %d\n", __FUNCTION__, connected);
}

static SpiceCharDeviceInterface smartcard_interface = {
    .base.type          = SPICE_INTERFACE_CHAR_DEVICE,
    .base.description   = "Xspice virtual channel char device",
    .base.major_version = SPICE_INTERFACE_CHAR_DEVICE_MAJOR,
    .base.minor_version = SPICE_INTERFACE_CHAR_DEVICE_MINOR,
    .state              = smartcard_state,
    .write              = smartcard_write,
    .read               = smartcard_read,
#if SPICE_SERVER_VERSION >= 0x000c02
    .event              = smartcard_event,
#endif
};

int
qxl_add_spice_smartcard_interface (qxl_screen_t *qxl)
{
    int rc;
    struct sockaddr_un addr;

    if (qxl->smartcard_file[0] == 0) {
        xf86DrvMsg(qxl->pScrn->scrnIndex, X_INFO, "smartcard: no file given, smartcard is disabled\n");
        return 0;
    }

    smartcard_sin.fd = -1;
    smartcard_sin.listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (smartcard_sin.listen_fd < 0) {
        ErrorF("smartcard: unable to open socket: %s\n", strerror(errno));
        return errno;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, qxl->smartcard_file, sizeof(addr.sun_path) - 1);
    unlink(qxl->smartcard_file);

    if (bind(smartcard_sin.listen_fd, (struct sockaddr *) &addr, sizeof(addr))) {
        ErrorF("smartcard: unable to bind to unix domain %s: %s\n", qxl->smartcard_file, strerror(errno));
        close(smartcard_sin.listen_fd);
        return errno;
    }

    if (listen(smartcard_sin.listen_fd, 1)) {
        ErrorF("smartcard: unable to listen to unix domain %s: %s\n", qxl->smartcard_file, strerror(errno));
        close(smartcard_sin.listen_fd);
        return errno;
    }

    smartcard_sin.listen_watch = qxl->core->watch_add(smartcard_sin.listen_fd, SPICE_WATCH_EVENT_READ, on_accept_available, qxl);

    smartcard_sin.base.base.sif = &smartcard_interface.base;
    smartcard_sin.qxl = qxl;

    rc = spice_server_add_interface(qxl->spice_server, &smartcard_sin.base.base);
    if (rc < 0)
        return errno;

    return 0;
}
