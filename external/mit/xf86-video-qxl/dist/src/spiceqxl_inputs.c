/*
 * Copyright 2011 Red Hat, Inc.
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

/* Handle inputs channel for spice, and register the X parts,
 * a mouse and a keyboard device pair.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xf86Xinput.h>
#include <exevents.h>
#include <xserver-properties.h>
#include <list.h>
#include <input.h>
#include <xkbsrv.h>
#include <spice.h>
#include "qxl.h"
#include "spiceqxl_inputs.h"

static
int XSpicePointerPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags);
static
int XSpiceKeyboardPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags);
static
void XSpicePointerUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags);
static
void XSpiceKeyboardUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags);

static char xspice_pointer_name[] = "xspice pointer";
static InputDriverRec XSPICE_POINTER = {
    1,
    xspice_pointer_name,
    NULL,
    XSpicePointerPreInit,
    XSpicePointerUnInit,
    NULL,
    NULL /* defaults */
};

static char xspice_keyboard_name[] = "xspice keyboard";
static InputDriverRec XSPICE_KEYBOARD = {
    1,
    xspice_keyboard_name,
    NULL,
    XSpiceKeyboardPreInit,
    XSpiceKeyboardUnInit,
    NULL,
    NULL
};

#define BUTTONS 5

typedef struct XSpiceKbd {
    SpiceKbdInstance sin;
    uint8_t          ledstate;
    InputInfoPtr     pInfo; /* xf86 device handle to post events */
    /* Note: spice sends some of the keys escaped by this.
     * This is supposed to be AT key codes, but I can't figure out where that
     * thing is defined after looking at xf86-input-keyboard. Ended up reverse
     * engineering a escaped table using xev.
     */
    int              escape;
} XSpiceKbd;

static int xspice_pointer_proc(DeviceIntPtr pDevice, int onoff)
{
    DevicePtr pDev = (DevicePtr)pDevice;
    BYTE map[BUTTONS + 1];
    Atom btn_labels[BUTTONS];
    Atom axes_labels[2];
    int i;

    switch (onoff) {
        case DEVICE_INIT:
            for (i = 0; i < BUTTONS + 1; i++) {
                map[i] = i;
            }
            btn_labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
            btn_labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
            btn_labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
            btn_labels[3] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_UP);
            btn_labels[4] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_DOWN);
            axes_labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
            axes_labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);
            InitPointerDeviceStruct(pDev, map, BUTTONS,btn_labels,(PtrCtrlProcPtr)NoopDDA,
                GetMotionHistorySize(), 2, axes_labels);
            break;
        case DEVICE_ON:
            pDev->on = TRUE;
            break;
        case DEVICE_OFF:
            pDev->on = FALSE;
            break;
    }
    return Success;
}

static void xspice_keyboard_bell(int percent, DeviceIntPtr device, pointer ctrl, int class_)
{
}

#define CAPSFLAG        1
#define NUMFLAG         2
#define SCROLLFLAG      4
/* MODEFLAG and COMPOSEFLAG currently unused (reminder for future) */
#define MODEFLAG        8
#define COMPOSEFLAG    16

#define ArrayLength(a) (sizeof(a) / (sizeof((a)[0])))

static void xspice_keyboard_control(DeviceIntPtr device, KeybdCtrl *ctrl)
{
    static struct { int xbit, code; } bits[] = {
        { CAPSFLAG, SPICE_KEYBOARD_MODIFIER_FLAGS_CAPS_LOCK },
        { NUMFLAG,  SPICE_KEYBOARD_MODIFIER_FLAGS_NUM_LOCK },
        { SCROLLFLAG,   SPICE_KEYBOARD_MODIFIER_FLAGS_SCROLL_LOCK },
        /* TODO: there is no MODEFLAG nor COMPOSEFLAG in SPICE. */
    };

    XSpiceKbd *kbd;
    InputInfoPtr pInfo;
    int i;

    pInfo = device->public.devicePrivate;
    kbd = pInfo->private;
    kbd->ledstate = 0;
    for (i = 0; i < ArrayLength(bits); i++) {
        if (ctrl->leds & bits[i].xbit) {
            kbd->ledstate |= bits[i].code;
        } else {
            kbd->ledstate &= ~bits[i].code;
        }
    }
}

static char xspice_keyboard_rules[] = "evdev";
static char xspice_keyboard_model[] = "pc105";
static char xspice_keyboard_layout[] = "us";
static char xspice_keyboard_variant[] = "";
static char xspice_keyboard_options[] = "";
static int xspice_keyboard_proc(DeviceIntPtr pDevice, int onoff)
{
    DevicePtr pDev = (DevicePtr)pDevice;
    XkbRMLVOSet rmlvo = {
        .rules = xspice_keyboard_rules,
        .model = xspice_keyboard_model,
        .layout = xspice_keyboard_layout,
        .variant = xspice_keyboard_variant,
        .options = xspice_keyboard_options,
    };

    switch (onoff) {
        case DEVICE_INIT:
            InitKeyboardDeviceStruct(
                pDevice, &rmlvo, xspice_keyboard_bell, xspice_keyboard_control
            );
            break;
        case DEVICE_ON:
            pDev->on = TRUE;
            break;
        case DEVICE_OFF:
            pDev->on = FALSE;
            break;
    }
    return Success;
}

/* from spice-input.c */
/* keyboard bits */

static void kbd_push_key(SpiceKbdInstance *sin, uint8_t frag);
static uint8_t kbd_get_leds(SpiceKbdInstance *sin);

static const SpiceKbdInterface kbd_interface = {
    .base.type          = SPICE_INTERFACE_KEYBOARD,
    .base.description   = "xspice keyboard",
    .base.major_version = SPICE_INTERFACE_KEYBOARD_MAJOR,
    .base.minor_version = SPICE_INTERFACE_KEYBOARD_MINOR,
    .push_scan_freg     = kbd_push_key,
    .get_leds           = kbd_get_leds,
};

/* spice sends AT scancodes (with a strange escape).
 * But xf86PostKeyboardEvent expects scancodes. Apparently most of the time
 * you just need to add MIN_KEYCODE, see xf86-input-keyboard/src/atKeynames
 * and xf86-input-keyboard/src/kbd.c:PostKbdEvent:
 *   xf86PostKeyboardEvent(device, scanCode + MIN_KEYCODE, down); */
#define MIN_KEYCODE     8

static uint8_t escaped_map[256] = {
    [0x1c] = 104, //KEY_KP_Enter,
    [0x1d] = 105, //KEY_RCtrl,
    [0x2a] = 0,//KEY_LMeta, // REDKEY_FAKE_L_SHIFT
    [0x35] = 106,//KEY_KP_Divide,
    [0x36] = 0,//KEY_RMeta, // REDKEY_FAKE_R_SHIFT
    [0x37] = 107,//KEY_Print,
    [0x38] = 108,//KEY_AltLang,
    [0x46] = 127,//KEY_Break,
    [0x47] = 110,//KEY_Home,
    [0x48] = 111,//KEY_Up,
    [0x49] = 112,//KEY_PgUp,
    [0x4b] = 113,//KEY_Left,
    [0x4d] = 114,//KEY_Right,
    [0x4f] = 115,//KEY_End,
    [0x50] = 116,//KEY_Down,
    [0x51] = 117,//KEY_PgDown,
    [0x52] = 118,//KEY_Insert,
    [0x53] = 119,//KEY_Delete,
    [0x5b] = 133,//0, // REDKEY_LEFT_CMD,
    [0x5c] = 134,//0, // REDKEY_RIGHT_CMD,
    [0x5d] = 135,//KEY_Menu,
};

static void kbd_push_key(SpiceKbdInstance *sin, uint8_t frag)
{
    XSpiceKbd *kbd = container_of(sin, XSpiceKbd, sin);
    int is_down;

    if (frag == 224) {
        kbd->escape = frag;
        return;
    }
    is_down = frag & 0x80 ? FALSE : TRUE;
    frag = frag & 0x7f;
    if (kbd->escape == 224) {
        kbd->escape = 0;
        if (escaped_map[frag] == 0) {
            fprintf(stderr, "spiceqxl_inputs.c: kbd_push_key: escaped_map[%d] == 0\n", frag);
        }
        frag = escaped_map[frag];
    } else {
        frag += MIN_KEYCODE;
    }

    xf86PostKeyboardEvent(kbd->pInfo->dev, frag, is_down);
}

static uint8_t kbd_get_leds(SpiceKbdInstance *sin)
{
    XSpiceKbd *kbd = container_of(sin, XSpiceKbd, sin);

    return kbd->ledstate;
}

/* mouse bits */

typedef struct XSpicePointer {
    SpiceMouseInstance  mouse;
    SpiceTabletInstance tablet;
    int width, height, x, y;
    Bool absolute;
    InputInfoPtr     pInfo; /* xf86 device handle to post events */
} XSpicePointer;

static XSpicePointer *g_xspice_pointer;

static void mouse_motion(SpiceMouseInstance *sin, int dx, int dy, int dz,
                         uint32_t buttons_state)
{
    // TODO
}

static void mouse_buttons(SpiceMouseInstance *sin, uint32_t buttons_state)
{
    // TODO
}

static const SpiceMouseInterface mouse_interface = {
    .base.type          = SPICE_INTERFACE_MOUSE,
    .base.description   = "xspice mouse",
    .base.major_version = SPICE_INTERFACE_MOUSE_MAJOR,
    .base.minor_version = SPICE_INTERFACE_MOUSE_MINOR,
    .motion             = mouse_motion,
    .buttons            = mouse_buttons,
};

static void tablet_set_logical_size(SpiceTabletInstance* sin, int width, int height)
{
    XSpicePointer *spice_pointer = container_of(sin, XSpicePointer, tablet);

    if (height < 16) {
        height = 16;
    }
    if (width < 16) {
        width = 16;
    }
    spice_pointer->width  = width;
    spice_pointer->height = height;
}

void spiceqxl_tablet_position(int x, int y, uint32_t buttons_state)
{
    // TODO: don't ignore buttons_state
    xf86PostMotionEvent(g_xspice_pointer->pInfo->dev, 1, 0, 2, x, y);
}

static void tablet_position(SpiceTabletInstance* sin, int x, int y,
                            uint32_t buttons_state)
{
    spiceqxl_tablet_position(x, y, buttons_state);
}

void spiceqxl_tablet_buttons(uint32_t buttons_state)
{
    static uint32_t old_buttons_state = 0;
    int i;

    for (i = 0; i < BUTTONS; i++) {
        if ((buttons_state ^ old_buttons_state) & (1 << i)) {
            int action = (buttons_state & (1 << i));
            xf86PostButtonEvent(g_xspice_pointer->pInfo->dev, 0, i + 1, action, 0, 0);
        }
    }
    old_buttons_state = buttons_state;
}

static void tablet_buttons(SpiceTabletInstance *sin,
                           uint32_t buttons_state)
{
    // For some reason spice switches the second and third button, undo that.
    // basically undo RED_MOUSE_STATE_TO_LOCAL
    buttons_state = (buttons_state & SPICE_MOUSE_BUTTON_MASK_LEFT) |
        ((buttons_state & SPICE_MOUSE_BUTTON_MASK_MIDDLE) << 1) |
        ((buttons_state & SPICE_MOUSE_BUTTON_MASK_RIGHT) >> 1) |
        (buttons_state & ~(SPICE_MOUSE_BUTTON_MASK_LEFT | SPICE_MOUSE_BUTTON_MASK_MIDDLE
                          |SPICE_MOUSE_BUTTON_MASK_RIGHT));
    spiceqxl_tablet_buttons(buttons_state);
}

static void tablet_wheel(SpiceTabletInstance* sin, int wheel,
                         uint32_t buttons_state)
{
    // convert wheel into fourth and fifth buttons
    tablet_buttons(sin, buttons_state
                        | (wheel > 0 ? (1<<4) : 0)
                        | (wheel < 0 ? (1<<3) : 0));
}

static const SpiceTabletInterface tablet_interface = {
    .base.type          = SPICE_INTERFACE_TABLET,
    .base.description   = "xspice tablet",
    .base.major_version = SPICE_INTERFACE_TABLET_MAJOR,
    .base.minor_version = SPICE_INTERFACE_TABLET_MINOR,
    .set_logical_size   = tablet_set_logical_size,
    .position           = tablet_position,
    .wheel              = tablet_wheel,
    .buttons            = tablet_buttons,
};

static char unknown_type_string[] = "UNKNOWN";

static int
XSpiceKeyboardPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
    XSpiceKbd *kbd;

    kbd = calloc(sizeof(*kbd), 1);
    kbd->sin.base.sif = &kbd_interface.base;
    kbd->pInfo = pInfo;

    pInfo->private = kbd;
    pInfo->type_name = unknown_type_string;
    pInfo->device_control = xspice_keyboard_proc;
    pInfo->read_input = NULL;
    pInfo->switch_mode = NULL;

    spice_server_add_interface(xspice_get_spice_server(), &kbd->sin.base);
    return Success;
}

static int
XSpicePointerPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
    XSpicePointer *spice_pointer;

    g_xspice_pointer = spice_pointer = calloc(sizeof(*spice_pointer), 1);
    spice_pointer->mouse.base.sif  = &mouse_interface.base;
    spice_pointer->tablet.base.sif = &tablet_interface.base;
    spice_pointer->absolute = TRUE;
    spice_pointer->pInfo = pInfo;

    pInfo->private = NULL;
    pInfo->type_name = unknown_type_string;
    pInfo->device_control = xspice_pointer_proc;
    pInfo->read_input = NULL;
    pInfo->switch_mode = NULL;

    spice_server_add_interface(xspice_get_spice_server(), &spice_pointer->tablet.base);
    return Success;
}

static void
XSpicePointerUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
}

static void
XSpiceKeyboardUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
}

void xspice_add_input_drivers(pointer module)
{
    xf86AddInputDriver(&XSPICE_POINTER, module, 0);
    xf86AddInputDriver(&XSPICE_KEYBOARD, module, 0);
}
