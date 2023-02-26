#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "qxl_option_helpers.h"
#include "spiceqxl_util.h"
#include "spiceqxl_inputs.h"

#include "spiceqxl_uinput.h"

static const char *uinput_filename;
static int uinput_fd;
static SpiceWatch *uinput_watch;
static struct input_event inp_event;
static int offset;

static void spiceqxl_uinput_read_cb(int fd, int event, void *opaque)
{
    int n;
    static int x = -1;
    static int y = -1;
    static int buttons_state = 0;
    int button = -1;

    n = read(uinput_fd, (char *)&inp_event + offset, sizeof(inp_event) - offset);
    if (n == -1) {
        if (errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK) {
            fprintf(stderr, "spice: uinput read failed: %s\n", strerror(errno));
        }
        return;
    }
    offset += n;
    if (offset < sizeof(inp_event)) {
        return;
    }
    offset = 0;
    switch (inp_event.type) {
    case EV_KEY:
        /*  XXX Here we hardcode vdagent-uinput.c mapping since we don't support ioctls.
         *  We could replace the ioctls with additional non uinput messages
         *  used in vdagentd fake uinput mode. */
        switch (inp_event.code) {
        case BTN_LEFT:
            button = 1 << 0;
            break;
        case BTN_MIDDLE:
            button = 1 << 1;
            break;
        case BTN_RIGHT:
            button = 1 << 2;
            break;
        }
        if (inp_event.value > 0) {
            buttons_state |= button;
        } else {
            buttons_state &= ~button;
        }
        spiceqxl_tablet_buttons(buttons_state);
        break;
    case EV_REL:
        button = 1;
        if (inp_event.value == 1) {
            button = 1 << 3;
        } else {
            button = 1 << 4;
        }
        buttons_state |= button;
        spiceqxl_tablet_buttons(buttons_state);
        buttons_state &= ~button;
        spiceqxl_tablet_buttons(buttons_state);
        break;
    case EV_ABS:
        switch (inp_event.code) {
        case ABS_X:
            x = inp_event.value;
            break;
        case ABS_Y:
            y = inp_event.value;
            break;
        default:
            fprintf(stderr, "%s: unknown axis %d, ignoring\n", __func__, inp_event.code);
            return;
            break;
        }
        spiceqxl_tablet_position(x, y, buttons_state);
        break;
    }
}

void spiceqxl_uinput_init(qxl_screen_t *qxl)
{
    int ret;
    int enabled;

    uinput_filename = get_str_option(qxl->options, OPTION_SPICE_VDAGENT_UINPUT_PATH,
               "XSPICE_VDAGENT_UINPUT_PATH");
    enabled = get_bool_option(qxl->options, OPTION_SPICE_VDAGENT_ENABLED, "XSPICE_VDAGENT_ENABLED");

    if (!enabled || uinput_filename == NULL) {
        return;
    }
    ret = mkfifo(uinput_filename, 0666);
    if (ret != 0) {
        fprintf(stderr, "spice: failed to create uinput fifo %s: %s\n",
                uinput_filename, strerror(errno));
        return;
    }
    spiceqxl_chown_agent_file(qxl, uinput_filename);
    uinput_fd = open(uinput_filename, O_RDONLY | O_NONBLOCK, 0666);
    if (uinput_fd == -1) {
        fprintf(stderr, "spice: failed creating uinput file %s: %s\n",
               uinput_filename, strerror(errno));
        return;
    }
}

void spiceqxl_uinput_watch(qxl_screen_t *qxl, Bool on)
{
    if (uinput_watch) {
        qxl->core->watch_remove(uinput_watch);
        uinput_watch = NULL;
    }

    if (on)
        uinput_watch = qxl->core->watch_add(uinput_fd, SPICE_WATCH_EVENT_READ,
                            spiceqxl_uinput_read_cb, qxl);
}
