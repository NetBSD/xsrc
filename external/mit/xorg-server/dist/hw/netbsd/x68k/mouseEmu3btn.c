/*
 *
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 * Copyright 1993 by David Dawes <dawes@xfree86.org>
 * Copyright 2002 by SuSE Linux AG, Author: Egbert Eich
 * Copyright 1994-2002 by The XFree86 Project, Inc.
 * Copyright 2002 by Paul Elliott
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of copyright holders not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  The copyright holders
 * make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * 3 button emulation stuff
 * based on the emulation method in xf86-input-mouse/dist/src/mouse.c
 */

#include "inpututils.h"
#include "mouseEmu3btn.h"

static CARD32 buttonTimer(MouseEmu3btnPtr pEmu3btn);
static void Emulate3ButtonsSetEnabled(MouseEmu3btnPtr pEmu3btn, Bool enable);
static Bool Emulate3ButtonsSoft(MouseEmu3btnPtr pEmu3btn);

static void MouseBlockHandler(void *data, void *waitTime);
static void MouseWakeupHandler(void *data, int i);

/**********************************************************************
 *
 *  Emulate3Button support code
 *
 **********************************************************************/


/*
 * Lets create a simple finite-state machine for 3 button emulation:
 *
 * We track buttons 1 and 3 (left and right).  There are 11 states:
 *   0 ground           - initial state
 *   1 delayed left     - left pressed, waiting for right
 *   2 delayed right    - right pressed, waiting for left
 *   3 pressed middle   - right and left pressed, emulated middle sent
 *   4 pressed left     - left pressed and sent
 *   5 pressed right    - right pressed and sent
 *   6 released left    - left released after emulated middle
 *   7 released right   - right released after emulated middle
 *   8 repressed left   - left pressed after released left
 *   9 repressed right  - right pressed after released right
 *  10 pressed both     - both pressed, not emulating middle
 */
#define ST_INVALID		-1
#define ST_GROUND		0	/* initial state */
#define ST_DELAYED_LEFT		1	/* left pressed and waiting timeout */
#define ST_DELAYED_RIGHT	2	/* right pressed and waiting timeout */
#define ST_PRESSED_MIDDLE	3	/* middle pressed deteremined */
#define ST_PRESSED_LEFT		4	/* left pressed determined */
#define ST_PRESSED_RIGHT	5	/* right pressed determined */
#define ST_RELEASED_LEFT	6	/* left released after pressed both */
#define ST_RELEASED_RIGHT	7	/* right released after pressed both */
#define ST_REPRESSED_LEFT	8	/* left repressed after release */
#define ST_REPRESSED_RIGHT	9	/* right repressed after release  */
#define ST_PRESSED_BOTH		10	/* both pressed (not as middle) */
#define NSTATES			11

/*
 * At each state, we need handlers for the following events
 *   0: no buttons down
 *   1: left button down
 *   2: right button down
 *   3: both buttons down
 *   4: emulate3Timeout passed without a button change
 * Note that button events are not deltas, they are the set of buttons being
 * pressed now.  It's possible (ie, mouse hardware does it) to go from (eg)
 * left down to right down without anything in between, so all cases must be
 * handled.
 *
 * a handler consists of three values:
 *   0: action1
 *   1: action2
 *   2: new emulation state
 */
struct button_event {
	int type;	/* ButtonNone / ButtonPress / ButtonRelease */
#define ButtonNone	0
	int button;
#define ButtonLeft	Button1
#define ButtonMiddle	Button2
#define ButtonRight	Button3
};

struct button_action {
	struct button_event event1;
	struct button_event event2;
	int new_state;
};

/* The set of buttons being pressed passed from DDX mouse events */
#define BMASK_LEFT	0x01
#define BMASK_MIDDLE	0x02
#define BMASK_RIGHT	0x04

/* Event index values per buttons being pressed */
#define EMU_BUTTONS_NONE	0
#define EMU_BUTTONS_LEFT	1
#define EMU_BUTTONS_RIGHT	2
#define EMU_BUTTONS_BOTH	3
#define NEMU_BUTTONSTATE	4

#define BMASKTOINDEX(bmask)						\
	((((bmask) & BMASK_RIGHT) >> 1) | ((bmask) & BMASK_LEFT))

struct button_state {
	struct button_action buttons[NEMU_BUTTONSTATE];
	struct button_action timeout;
};

/*
 * The comment preceeding each section is the current emulation state.
 * The comments to the right are of the form
 *      <button state> (<events>) -> <new emulation state>
 * which should be read as
 *      If the buttons are in <button state>, generate <events> then go to
 *      <new emulation state>.
 */
static const struct button_state stateTab[NSTATES] = {

  /*   0 ground           - initial state */
  [ST_GROUND] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing -> ground (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left -> delayed left */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right -> delayed right */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (middle press) -> pressed middle */
      .event1 = { ButtonPress,   ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_MIDDLE,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   1 delayed left     - left pressed, waiting for right */
  [ST_DELAYED_LEFT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (left event) -> ground */
      .event1 = { ButtonPress,   ButtonLeft   },
      .event2 = { ButtonRelease, ButtonLeft   },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left -> delayed left (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (left event) -> delayed right */
      .event1 = { ButtonPress,   ButtonLeft   },
      .event2 = { ButtonRelease, ButtonLeft   },
      .new_state = ST_DELAYED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (middle press) -> pressed middle */
      .event1 = { ButtonPress,   ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_MIDDLE,
    },

    .timeout = {
     /* timeout (left press) -> pressed left */
      .event1 = { ButtonPress,   ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_LEFT,
    },
  },

  /*   2 delayed right    - right pressed, waiting for left */
  [ST_DELAYED_RIGHT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (right event) -> ground */
      .event1 = { ButtonPress,   ButtonRight  },
      .event2 = { ButtonRelease, ButtonRight  },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (right event) -> delayed left */
      .event1 = { ButtonPress,   ButtonRight  },
      .event2 = { ButtonRelease, ButtonRight  },
      .new_state = ST_DELAYED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right -> delayed right (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (middle press) -> pressed middle */
      .event1 = { ButtonPress,   ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_MIDDLE,
    },

    .timeout = {
     /* timeout (right press) -> pressed right */
      .event1 = { ButtonPress,   ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_RIGHT,
    },
  },

  /*   3 pressed middle   - right and left pressed, emulated middle sent */
  [ST_PRESSED_MIDDLE] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (middle release) -> ground */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left -> released right */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_RIGHT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right -> released left */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_LEFT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right -> pressed middle (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_MIDDLE,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   4 pressed left     - left pressed and sent */
  [ST_PRESSED_LEFT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (left release) -> ground */
      .event1 = { ButtonRelease, ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left -> pressed left (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (left release) -> delayed right */
      .event1 = { ButtonRelease, ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (right press) -> pressed both */
      .event1 = { ButtonPress,   ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_BOTH,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   5 pressed right    - right pressed and sent */
  [ST_PRESSED_RIGHT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (right release) -> ground */
      .event1 = { ButtonRelease, ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (right release) -> delayed left */
      .event1 = { ButtonRelease, ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right -> pressed right (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (left press) -> pressed both */
      .event1 = { ButtonPress,   ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_BOTH,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   6 released left    - left released after emulated middle */
  [ST_RELEASED_LEFT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (middle release) -> ground */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (middle release) -> delayed left */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right -> released left (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_LEFT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (left press) -> repressed left */
      .event1 = { ButtonPress,   ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_REPRESSED_LEFT,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   7 released right   - right released after emulated middle */
  [ST_RELEASED_RIGHT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (middle release) -> ground */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left -> released right (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_RIGHT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (middle release) -> delayed right */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_DELAYED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right (right press) -> repressed right */
      .event1 = { ButtonPress,   ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_REPRESSED_RIGHT,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   8 repressed left   - left pressed after released left */
  [ST_REPRESSED_LEFT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (middle release, left release) -> ground */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonRelease, ButtonLeft   },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (middle release) -> pressed left */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (left release) -> released left */
      .event1 = { ButtonRelease, ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_LEFT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right -> repressed left (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_REPRESSED_LEFT,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*   9 repressed right  - right pressed after released right */
  [ST_REPRESSED_RIGHT] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (middle release, right release) -> ground */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonRelease, ButtonRight  },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (right release) -> released right */
      .event1 = { ButtonRelease, ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_RELEASED_RIGHT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (middle release) -> pressed right */
      .event1 = { ButtonRelease, ButtonMiddle },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right -> repressed right (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_REPRESSED_RIGHT,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },

  /*  10 pressed both     - both pressed, not emulating middle */
  [ST_PRESSED_BOTH] = {

    .buttons[EMU_BUTTONS_NONE] = {
      /* nothing (left release, right release) -> ground */
      .event1 = { ButtonRelease, ButtonLeft   },
      .event2 = { ButtonRelease, ButtonRight  },
      .new_state = ST_GROUND,
    },

    .buttons[EMU_BUTTONS_LEFT] = {
      /* left (right release) -> pressed left */
      .event1 = { ButtonRelease, ButtonRight  },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_LEFT,
    },

    .buttons[EMU_BUTTONS_RIGHT] = {
      /* right (left release) -> pressed right */
      .event1 = { ButtonRelease, ButtonLeft   },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_RIGHT,
    },

    .buttons[EMU_BUTTONS_BOTH] = {
      /* left & right -> pressed both (no change) */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_PRESSED_BOTH,
    },

    .timeout = {
      /* timeout N/A */
      .event1 = { ButtonNone,    0            },
      .event2 = { ButtonNone,    0            },
      .new_state = ST_INVALID,
    },
  },
};

static CARD32
buttonTimer(MouseEmu3btnPtr pEmu3btn)
{
    sigset_t sigmask;
    int type, button, flag;
    ValuatorMask mask;
    const struct button_action *timeout_action;

    (void)sigemptyset(&sigmask);
    (void)sigaddset(&sigmask, SIGIO);
    (void)sigprocmask(SIG_BLOCK, &sigmask, NULL);

    pEmu3btn->emulate3Pending = FALSE;
    timeout_action = &stateTab[pEmu3btn->emulateState].timeout;
    if ((type = timeout_action->event1.type) != ButtonNone) {
        button = timeout_action->event1.button;
        flag = POINTER_RELATIVE;
        valuator_mask_zero(&mask);
        QueuePointerEvents(pEmu3btn->device, type, button, flag, &mask);
        pEmu3btn->emulateState = timeout_action->new_state;
    } else {
        LogMessageVerbSigSafe(X_WARNING, -1,
            "Got unexpected buttonTimer in state %d\n", pEmu3btn->emulateState);
    }

    (void)sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
    return 0;
}

static void
Emulate3ButtonsSetEnabled(MouseEmu3btnPtr pEmu3btn, Bool enable)
{

    if (pEmu3btn->emulate3Buttons == enable)
        return;

    pEmu3btn->emulate3Buttons = enable;

    if (enable) {
        pEmu3btn->emulateState = ST_GROUND;
        pEmu3btn->emulate3Pending = FALSE;
        pEmu3btn->emulate3ButtonsSoft = FALSE; /* specifically requested now */

        RegisterBlockAndWakeupHandlers(MouseBlockHandler, MouseWakeupHandler,
                                       (void *)pEmu3btn);
    } else {
        if (pEmu3btn->emulate3Pending)
            buttonTimer(pEmu3btn);

        RemoveBlockAndWakeupHandlers(MouseBlockHandler, MouseWakeupHandler,
                                     (void *)pEmu3btn);
    }
}

static Bool
Emulate3ButtonsSoft(MouseEmu3btnPtr pEmu3btn)
{

    if (!pEmu3btn->emulate3ButtonsSoft)
        return TRUE;

#if defined(__NetBSD__) && defined(WSCONS_SUPPORT)
    /*
     * On NetBSD a wsmouse is a multiplexed device. Imagine a notebook
     * with two-button mousepad, and an external USB mouse plugged in
     * temporarily. After using button 3 on the external mouse and
     * unplugging it again, the mousepad will still need to emulate
     * 3 buttons.
     */
    return TRUE;
#else
    LogMessageVerbSigSafe(X_INFO, 4,
        "mouse: 3rd Button detected: disabling emulate3Button\n");

    Emulate3ButtonsSetEnabled(pEmu3btn, FALSE);

    return FALSE;
#endif
}

static void
MouseBlockHandler(void *data, void *waitTime)
{
    MouseEmu3btnPtr pEmu3btn = data;
    int ms;

    if (pEmu3btn->emulate3Pending) {
        ms = pEmu3btn->emulate3Expires - GetTimeInMillis();
        if (ms <= 0)
            ms = 0;
        AdjustWaitForDelay(waitTime, ms);
    }
}

static void
MouseWakeupHandler(void *data, int i)
{
    MouseEmu3btnPtr pEmu3btn = data;
    int ms;

    if (pEmu3btn->emulate3Pending) {
        ms = pEmu3btn->emulate3Expires - GetTimeInMillis();
        if (ms <= 0)
            buttonTimer(pEmu3btn);
    }
}

/*******************************************************************
 * function "Emulate3ButtonsEnable"
 *
 *  purpose:
 *   Enable and initialize Emulate3Buttons structures.
 *  argument:
 *    (MouseEmu3btnPtr)pEmu3btn : Emu3btn private record
 *    (DeviceIntPtr)device      : pointer device private record
 *    (int)timeout              : timeout to wait another button [ms]
 *
 *******************************************************************/
void
Emulate3ButtonsEnable(MouseEmu3btnPtr pEmu3btn, DeviceIntPtr device, int timeout)
{

    BUG_RETURN_MSG(device == NULL, "Invalid DeviceIntPtr.\n");

    if (timeout <= 0) {
        timeout = EMU3B_DEF_TIMEOUT;
    }
    pEmu3btn->device = device;
    pEmu3btn->emulate3Timeout = timeout;

    Emulate3ButtonsSetEnabled(pEmu3btn, TRUE);
}

/*******************************************************************
 * function "Emulate3ButtonsQueueEvent"
 *
 *  purpose:
 *   Emulate middle button per left/right button events and post events.
 *  argument:
 *    (MouseEmu3btnPtr)pEmu3btn : Emu3btn private record
 *    (int)type                 : event  (ButtonPress / ButtonRelease)
 *    (int)buttons              : button (Button1 / Button2 / Button3)
 *    (int)bmask                : buttons being pressed (0x1:left / 0x4:right)
 *
 *******************************************************************/

void
Emulate3ButtonsQueueEvent(MouseEmu3btnPtr pEmu3btn, int type, int buttons, int bmask)
{
    DeviceIntPtr device = pEmu3btn->device;
    int emulateButtons;
    int button, flag;
    ValuatorMask mask;

    BUG_RETURN_MSG(buttons != ButtonLeft && buttons != ButtonRight,
      "not left or right button event\n");

    if (pEmu3btn->emulate3ButtonsSoft && pEmu3btn->emulate3Pending)
        buttonTimer(pEmu3btn);

    if (pEmu3btn->emulate3Buttons
        && ((bmask & BMASK_MIDDLE) == 0 || Emulate3ButtonsSoft(pEmu3btn))) {
        const struct button_action *button_action, *timeout_action;

        /* emulate the third button by the other two */

        emulateButtons = BMASKTOINDEX(bmask);
        button_action =
          &stateTab[pEmu3btn->emulateState].buttons[emulateButtons];

        if ((type = button_action->event1.type) != ButtonNone) {
            button = button_action->event1.button;
            flag = POINTER_RELATIVE;
            valuator_mask_zero(&mask);
            QueuePointerEvents(device, type, button, flag, &mask);
        }
        if ((type = button_action->event2.type) != ButtonNone) {
            button = button_action->event2.button;
            flag = POINTER_RELATIVE;
            valuator_mask_zero(&mask);
            QueuePointerEvents(device, type, button, flag, &mask);
        }

        pEmu3btn->emulateState = button_action->new_state;

        timeout_action = &stateTab[pEmu3btn->emulateState].timeout;
        if (timeout_action->event1.type != ButtonNone) {
            pEmu3btn->emulate3Expires =
                GetTimeInMillis() + pEmu3btn->emulate3Timeout;
            pEmu3btn->emulate3Pending = TRUE;
        } else {
            pEmu3btn->emulate3Pending = FALSE;
        }
    } else {
        /* no emulation; post left or right button event as is */
        flag = POINTER_RELATIVE;
        valuator_mask_zero(&mask);
        QueuePointerEvents(device, type, buttons, flag, &mask);
    }
}
