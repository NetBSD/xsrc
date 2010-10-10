/* $XConsortium: sunKbd.c,v 5.47 94/08/16 13:45:30 dpw Exp $ */
/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and X Consortium make no
representations about the suitability of this software for
any purpose. It is provided "as is" without any express or
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define NEED_EVENTS
#include "macppc.h"
#include "keysym.h"
#include <stdio.h>
#if 0 /* XXX */
#include "Sunkeysym.h"

#define SUN_LED_MASK	0x0f
#endif /* 0 XXX */
#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */ /* XXX */
#define MAX_KEYCODE	255	/* limited by the protocol */ /* XXX */
#if 0 /* XXX */
#ifndef KB_SUN4
#define KB_SUN4		4
#endif

#define AUTOREPEAT_INITIATE	200
#define AUTOREPEAT_DELAY	50

#define tvminus(tv, tv1, tv2)   /* tv = tv1 - tv2 */ \
		if ((tv1).tv_usec < (tv2).tv_usec) { \
		    (tv1).tv_usec += 1000000; \
		    (tv1).tv_sec -= 1; \
		} \
		(tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
		(tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;

#define tvplus(tv, tv1, tv2)    /* tv = tv1 + tv2 */ \
		(tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
		(tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
		if ((tv).tv_usec > 1000000) { \
		    (tv).tv_usec -= 1000000; \
		    (tv).tv_sec += 1; \
		}
#endif /* 0 XXX */

extern KeySymsRec macppcKeySyms[];
extern macppcModmapRec *macppcModMaps[];

#if 0 /* XXX */
long	  	  sunAutoRepeatInitiate = 1000 * AUTOREPEAT_INITIATE;
long	  	  sunAutoRepeatDelay = 1000 * AUTOREPEAT_DELAY;

static int		autoRepeatKeyDown = 0;
static int		autoRepeatReady;
static int		autoRepeatFirst;
static struct timeval	autoRepeatLastKeyDownTv;
static struct timeval	autoRepeatDeltaTv;

void sunKbdWait()
{
    static struct timeval lastChngKbdTransTv;
    struct timeval tv;
    struct timeval lastChngKbdDeltaTv;
    unsigned int lastChngKbdDelta;

    X_GETTIMEOFDAY(&tv);
    if (!lastChngKbdTransTv.tv_sec)
	lastChngKbdTransTv = tv;
    tvminus(lastChngKbdDeltaTv, tv, lastChngKbdTransTv);
    lastChngKbdDelta = TVTOMILLI(lastChngKbdDeltaTv);
    if (lastChngKbdDelta < 750) {
	unsigned wait;
	/*
         * We need to guarantee at least 750 milliseconds between
	 * calls to KIOCTRANS. YUCK!
	 */
	wait = (750L - lastChngKbdDelta) * 1000L;
        usleep (wait);
        X_GETTIMEOFDAY(&tv);
    }
    lastChngKbdTransTv = tv;
}

static void SetLights (ctrl, fd)
    KeybdCtrl*	ctrl;
    int fd;
{
#ifdef KIOCSLED
    static unsigned char led_tab[16] = {
	0,
	LED_NUM_LOCK,
	LED_SCROLL_LOCK,
	LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_COMPOSE,
	LED_COMPOSE | LED_NUM_LOCK,
	LED_COMPOSE | LED_SCROLL_LOCK,
	LED_COMPOSE | LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK,
	LED_CAPS_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_SCROLL_LOCK,
	LED_CAPS_LOCK | LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE,
	LED_CAPS_LOCK | LED_COMPOSE | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE | LED_SCROLL_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE | LED_SCROLL_LOCK | LED_NUM_LOCK
    };
    if (ioctl (fd, KIOCSLED, (caddr_t)&led_tab[ctrl->leds & 0x0f]) == -1)
	Error("Failed to set keyboard lights");
#endif
}


static void ModLight (device, on, led)
    DeviceIntPtr device;
    Bool	on;
    int		led;
{
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    sunKbdPrivPtr pPriv = (sunKbdPrivPtr) device->public.devicePrivate;

    if(on) {
	ctrl->leds |= led;
	pPriv->leds |= led;
    } else {
	ctrl->leds &= ~led;
	pPriv->leds &= ~led;
    }
    SetLights (ctrl, pPriv->fd);
}
#endif /* 0 XXX */

/*-
 *-----------------------------------------------------------------------
 * macppcBell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	Ring the keyboard bell for an amount of time proportional to
 *	"loudness."
 *
 * Side Effects:
 *	None, really...
 *
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
static void macppcBell (
    int		    percent,
    DeviceIntPtr    device,
    pointer	    ctrl,
    int		    unused)
#else
static void macppcBell (percent, device, ctrl, unused)
    int		    percent;	    /* Percentage of full volume */
    DeviceIntPtr    device;	    /* Keyboard to ring */
    pointer	    ctrl;
    int		    unused;
#endif
{
#if 0
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
    macppcKbdPrivPtr   pPriv = (macppcKbdPrivPtr) device->public.devicePrivate;
    struct wsconsio_bell_data wbd;

    if (percent == 0 || kctrl->bell == 0)
 	return;

    wbd.wbd_flags = WSCONSIO_BELLDATA_VOLUME | WSCONSIO_BELLDATA_PITCH |
	WSCONSIO_BELLDATA_PERIOD;
    wbd.wbd_volume = percent;
    wbd.wbd_pitch = kctrl->bell_pitch;
    wbd.wbd_period = kctrl->bell_duration;

    if (ioctl (pPriv->fd, WSCONSIO_COMPLEXBELL, &wbd) == -1) {
 	Error("Failed to activate bell");
	return;
    }
#endif
}

#if 0	/* XXX */
static void localEnqueueEvent (xEp, dip, count)
    xEventPtr xEp;
    DeviceIntPtr dip;
    int count;
{
#ifndef i386
    sigset_t holdmask;

#ifdef SVR4
    (void) sigaddset (&holdmask, SIGPOLL);
#else
    (void) sigaddset (&holdmask, SIGIO);
#endif
    (void) sigprocmask (SIG_BLOCK, &holdmask, (sigset_t*)NULL);
    mieqEnqueue (xEp);
    (void) sigprocmask (SIG_UNBLOCK, &holdmask, (sigset_t*)NULL);
#else
    int oldmask = sigblock (sigmask (SIGIO));

    mieqEnqueue (xEp);
    sigsetmask (oldmask);
#endif
}
#endif

#if 0 /* XXX */
#define XLED_NUM_LOCK    0x1
#define XLED_COMPOSE     0x4
#define XLED_SCROLL_LOCK 0x2
#define XLED_CAPS_LOCK   0x8

static KeyCode LookupKeyCode (keysym, keysymsrec)
    KeySym keysym;
    KeySymsPtr keysymsrec;
{
    KeyCode i;
    int ii, index = 0;

    for (i = keysymsrec->minKeyCode; i < keysymsrec->maxKeyCode; i++)
	for (ii = 0; ii < keysymsrec->mapWidth; ii++)
	    if (keysymsrec->map[index++] == keysym)
		return i;
}

static void pseudoKey(device, down, keycode)
    DeviceIntPtr device;
    Bool down;
    KeyCode keycode;
{
    int bit;
    CARD8 modifiers;
    CARD16 mask;
    BYTE* kptr;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    modifiers = device->key->modifierMap[keycode];
    if (down) {
	/* fool dix into thinking this key is now "down" */
	int i;
	*kptr |= bit;
	device->key->prev_state = device->key->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    if (mask & modifiers) {
		device->key->modifierKeyCount[i]++;
		device->key->state += mask;
		modifiers &= ~mask;
	    }
    } else {
	/* fool dix into thinking this key is now "up" */
	if (*kptr & bit) {
	    int i;
	    *kptr &= ~bit;
	    device->key->prev_state = device->key->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
		if (mask & modifiers) {
		    if (--device->key->modifierKeyCount[i] <= 0) {
			device->key->state &= ~mask;
			device->key->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	}
    }
}

static void DoLEDs(device, ctrl, pPriv)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl* ctrl;
    sunKbdPrivPtr pPriv;
{
#ifdef XKB
    if (noXkbExtension) {
#endif
    if ((ctrl->leds & XLED_CAPS_LOCK) && !(pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_CAPS_LOCK) && (pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_NUM_LOCK) && !(pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_NUM_LOCK) && (pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_SCROLL_LOCK) && !(pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_SCROLL_LOCK) && (pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_COMPOSE) && !(pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, TRUE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_COMPOSE) && (pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, FALSE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));
#ifdef XKB
    }
#endif
    pPriv->leds = ctrl->leds & 0x0f;
    SetLights (ctrl, pPriv->fd);
}
#endif /* 0 XXX */

/*-
 *-----------------------------------------------------------------------
 * macppcKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 *
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
static void macppcKbdCtrl (
    DeviceIntPtr    device,
    KeybdCtrl*	    ctrl)
#else
static void macppcKbdCtrl (device, ctrl)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl*	    ctrl;
#endif
{
    macppcKbdPrivPtr pPriv = (macppcKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

#if 0 /* XXX */
    if (ctrl->click != pPriv->click) {
    	int kbdClickCmd;

	pPriv->click = ctrl->click;
	kbdClickCmd = pPriv->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (pPriv->fd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
    if (pPriv->type == KB_SUN4 && pPriv->leds != ctrl->leds & 0x0f)
	DoLEDs(device, ctrl, pPriv);
#endif /* 0 XXX */

    /* Bell info change needs nothing done here. */
}

/*-
 *-----------------------------------------------------------------------
 * macppcKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
int macppcKbdProc (
    DeviceIntPtr  device,
    int	    	  what)
#else
int macppcKbdProc (device, what)
    DeviceIntPtr  device;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
#endif
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    macppcKbdPrivPtr pPriv;
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    extern int XkbDfltRepeatDelay, XkbDfltRepeatInterval;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

    switch (what) {
    case DEVICE_INIT:
fprintf(stderr, "kbd device init\n");
fflush(stderr);
	if (pKeyboard != LookupKeyboardDevice()) {
	    ErrorF ("Cannot open non-system keyboard\n");
	    return (!Success);
	}

	if (!workingKeySyms) {
	    workingKeySyms = &macppcKeySyms[macppcKbdPriv.type];

#if MIN_KEYCODE > 0
	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
#endif
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}

	if (!workingModMap) {
	    workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) memset(workingModMap, 0, MAP_LENGTH);
	    for (i = 0; macppcModMaps[macppcKbdPriv.type][i].key != 0; i++)
		workingModMap[macppcModMaps[macppcKbdPriv.type][i].key + MIN_KEYCODE] =
		    macppcModMaps[macppcKbdPriv.type][i].modifiers;
	}

	(void) memset ((void *) defaultKeyboardControl.autoRepeats,
			~0, sizeof defaultKeyboardControl.autoRepeats);

#if 0 /* XXX */
#ifdef XKB
	if (noXkbExtension) {
	    macppcAutoRepeatInitiate = XkbDfltRepeatDelay * 1000;
	    macppcAutoRepeatDelay = XkbDfltRepeatInterval * 1000;
#endif
	autoRepeatKeyDown = 0;
#ifdef XKB
	}
#endif
#endif /* 0 XXX */
	pKeyboard->devicePrivate = (pointer)&macppcKbdPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct(pKeyboard,
				 workingKeySyms, workingModMap,
				 macppcBell, macppcKbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (macppcKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
#if 0 /* XXX */
	if (macppcChangeKbdTranslation(pPriv->fd,TRUE) == -1)
	    FatalError("Can't set keyboard translation\n");
#endif /* 0 XXX */
#ifdef WSKBDIO_SETVERSION
	{
	    int version = WSKBDIO_EVENT_VERSION;
	    if (ioctl(pPriv->fd, WSKBDIO_SETVERSION, &version) == -1) {
		Error ("macppcKbdProc ioctl WSKBDIO_SETVERSION");
		return !Success;
	    }
	}
#endif
	(void) AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (macppcKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Restore original keyboard directness and translation.
	 */
#if 0 /* XXX */
	if (macppcChangeKbdTranslation(pPriv->fd,FALSE) == -1)
	    FatalError("Can't reset keyboard translation\n");
#endif /* 0 XXX */
	RemoveEnabledDevice(pPriv->fd);
	pKeyboard->on = FALSE;
	break;
    default:
	FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * macppcKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
Firm_event* macppcKbdGetEvents (
    int		fd,
    int*	pNumEvents,
    Bool*	pAgain)
#else
Firm_event* macppcKbdGetEvents (fd, pNumEvents, pAgain)
    int		fd;
    int*	pNumEvents;
    Bool*	pAgain;
#endif
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read(fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * macppcKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */
#if 0 /* XXX */
static xEvent	autoRepeatEvent;
static int	composeCount;

static Bool DoSpecialKeys(device, xE, fe)
    DeviceIntPtr  device;
    xEvent*       xE;
    Firm_event* fe;
{
    int	shift_index, map_index, bit;
    KeySym ksym;
    BYTE* kptr;
    macppcKbdPrivPtr pPriv = (macppcKbdPrivPtr)device->public.devicePrivate;
    BYTE keycode = xE->u.u.detail;
    CARD8 keyModifiers = device->key->modifierMap[keycode];

    /* look up the present idea of the keysym */
    shift_index = 0;
    if (device->key->state & ShiftMask)
	shift_index ^= 1;
    if (device->key->state & LockMask)
	shift_index ^= 1;
    map_index = (fe->id - 1) * device->key->curKeySyms.mapWidth;
    ksym = device->key->curKeySyms.map[shift_index + map_index];
    if (ksym == NoSymbol)
	ksym = device->key->curKeySyms.map[map_index];

    /*
     * Toggle functionality is hardcoded. This is achieved by always
     * discarding KeyReleases on these keys, and converting every other
     * KeyPress into a KeyRelease.
     */
    if (xE->u.u.type == KeyRelease
	&& (ksym == XK_Num_Lock
	|| ksym == XK_Scroll_Lock
#if 0 /* XXX */
	|| ksym == SunXK_Compose
#endif /* 0 XXX */
	|| (keyModifiers & LockMask)))
	return TRUE;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    if ((*kptr & bit) &&
	(ksym == XK_Num_Lock || ksym == XK_Scroll_Lock ||
#if 0 /* XXX */
	ksym == SunXK_Compose ||
#endif /* 0 XXX */
	(keyModifiers & LockMask)))
	xE->u.u.type = KeyRelease;

#if 0 /* XXX */
    if (pPriv->type == KB_SUN4) {
	if (ksym == XK_Num_Lock) {
	    ModLight (device, xE->u.u.type == KeyPress, XLED_NUM_LOCK);
	} else if (ksym == XK_Scroll_Lock) {
	    ModLight (device, xE->u.u.type == KeyPress, XLED_SCROLL_LOCK);
	} else if (ksym == SunXK_Compose) {
	    ModLight (device, xE->u.u.type == KeyPress, XLED_COMPOSE);
	    if (xE->u.u.type == KeyPress) composeCount = 2;
	    else composeCount = 0;
	} else if (keyModifiers & LockMask) {
	    ModLight (device, xE->u.u.type == KeyPress, XLED_CAPS_LOCK);
	}
	if (xE->u.u.type == KeyRelease) {
	    if (composeCount > 0 && --composeCount == 0) {
		pseudoKey(device, FALSE,
		    LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));
		ModLight (device, FALSE, XLED_COMPOSE);
	    }
	}
    }

    if ((xE->u.u.type == KeyPress) && (keyModifiers == 0)) {
	/* initialize new AutoRepeater event & mark AutoRepeater on */
	autoRepeatEvent = *xE;
	autoRepeatFirst = TRUE;
	autoRepeatKeyDown++;
	autoRepeatLastKeyDownTv = fe->time;
    }
#endif /* XXX */
    return FALSE;
}
#endif /* 0 XXX */

#if NeedFunctionPrototypes
void macppcKbdEnqueueEvent (
    DeviceIntPtr  device,
    Firm_event	  *fe)
#else
void macppcKbdEnqueueEvent (device, fe)
    DeviceIntPtr  device;
    Firm_event	  *fe;
#endif
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;

    keycode = (fe->value & 0xff) + MIN_KEYCODE;

    keyModifiers = device->key->modifierMap[keycode];
#if 0 /* XXX */
#ifdef XKB
    if (noXkbExtension) {
#endif
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (keycode == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }
#ifdef XKB
    }
#endif
#endif /* 0 XXX */
    xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);
    xE.u.u.type = ((fe->type == WSCONS_EVENT_KEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = keycode;
#if 0 /* XXX */
#ifdef XKB
    if (noXkbExtension) {
#endif
    if (DoSpecialKeys(device, &xE, fe))
	return;
#ifdef XKB
    }
#endif /* ! XKB */
#endif /* 0 XXX */
    mieqEnqueue (&xE);
}

#if 0 /* XXX */
void sunEnqueueAutoRepeat ()
{
    int	delta;
    int	i, mask;
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;

    if (ctrl->autoRepeat != AutoRepeatModeOn) {
	autoRepeatKeyDown = 0;
	return;
    }
    i=(autoRepeatEvent.u.u.detail >> 3);
    mask=(1 << (autoRepeatEvent.u.u.detail & 7));
    if (!(ctrl->autoRepeats[i] & mask)) {
	autoRepeatKeyDown = 0;
	return;
    }

    /*
     * Generate auto repeat event.	XXX one for now.
     * Update time & pointer location of saved KeyPress event.
     */

    delta = TVTOMILLI(autoRepeatDeltaTv);
    autoRepeatFirst = FALSE;

    /*
     * Fake a key up event and a key down event
     * for the last key pressed.
     */
    autoRepeatEvent.u.keyButtonPointer.time += delta;
    autoRepeatEvent.u.u.type = KeyRelease;

    /*
     * hold off any more inputs while we get these safely queued up
     * further SIGIO are
     */
    localEnqueueEvent (&autoRepeatEvent);
    autoRepeatEvent.u.u.type = KeyPress;
    localEnqueueEvent (&autoRepeatEvent);

    /* Update time of last key down */
    tvplus(autoRepeatLastKeyDownTv, autoRepeatLastKeyDownTv,
		    autoRepeatDeltaTv);
}

/*-
 *-----------------------------------------------------------------------
 * sunChangeKbdTranslation
 *	Makes operating system calls to set keyboard translation
 *	and direction on or off.
 *
 * Results:
 *	-1 if failure, else 0.
 *
 * Side Effects:
 * 	Changes kernel management of keyboard.
 *
 *-----------------------------------------------------------------------
 */
#if NeedFunctionPrototypes
int sunChangeKbdTranslation(
    int fd,
    Bool makeTranslated)

#else
int sunChangeKbdTranslation(fd, makeTranslated)
    int fd;
    Bool makeTranslated;
#endif
{
    int 	tmp;
#ifndef i386 /* { */
    sigset_t	hold_mask, old_mask;
#else /* }{ */
    int		old_mask;
#endif /* } */
    int		toread;
    char	junk[8192];

#ifndef i386 /* { */
    (void) sigfillset(&hold_mask);
    (void) sigprocmask(SIG_BLOCK, &hold_mask, &old_mask);
#else /* }{ */
    old_mask = sigblock (~0);
#endif /* } */
    sunKbdWait();
    if (makeTranslated) {
        /*
         * Next set the keyboard into "direct" mode and turn on
         * event translation. If either of these fails, we can't go
         * on.
         */
	tmp = 1;
	if (ioctl (fd, KIOCSDIRECT, &tmp) == -1) {
	    Error ("Setting keyboard direct mode");
	    return -1;
	}
	tmp = TR_UNTRANS_EVENT;
	if (ioctl (fd, KIOCTRANS, &tmp) == -1) {
	    Error ("Setting keyboard translation");
	    ErrorF ("sunChangeKbdTranslation: kbdFd=%d\n", fd);
	    return -1;
	}
    } else {
        /*
         * Next set the keyboard into "indirect" mode and turn off
         * event translation.
         */
	tmp = 0;
	(void)ioctl (fd, KIOCSDIRECT, &tmp);
	tmp = TR_ASCII;
	(void)ioctl (fd, KIOCTRANS, &tmp);
    }
    if (ioctl (fd, FIONREAD, &toread) != -1 && toread > 0) {
	while (toread) {
	    tmp = toread;
	    if (toread > sizeof (junk))
		tmp = sizeof (junk);
	    (void) read (fd, junk, tmp);
	    toread -= tmp;
	}
    }
#ifndef i386 /* { */
    (void) sigprocmask(SIG_SETMASK, &old_mask, (sigset_t *)NULL);
#else /* }{ */
    sigsetmask (old_mask);
#endif /* } */
    return 0;
}
#endif /* 0 XXX */

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr	pDev;
{
    return TRUE;
}

#if 0 /* XXX */
/*ARGSUSED*/
void sunBlockHandler(nscreen, pbdata, pptv, pReadmask)
    int nscreen;
    pointer pbdata;
    struct timeval **pptv;
    pointer pReadmask;
{
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    static struct timeval artv = { 0, 0 };	/* autorepeat timeval */

    if (!autoRepeatKeyDown)
	return;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatFirst == TRUE)
	artv.tv_usec = sunAutoRepeatInitiate;
    else
	artv.tv_usec = sunAutoRepeatDelay;
    *pptv = &artv;

}

/*ARGSUSED*/
void sunWakeupHandler(nscreen, pbdata, err, pReadmask)
    int nscreen;
    pointer pbdata;
    unsigned long err;
    pointer pReadmask;
{
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    struct timeval tv;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatKeyDown) {
	X_GETTIMEOFDAY(&tv);
	tvminus(autoRepeatDeltaTv, tv, autoRepeatLastKeyDownTv);
	if (autoRepeatDeltaTv.tv_sec > 0 ||
			(!autoRepeatFirst && autoRepeatDeltaTv.tv_usec >
				sunAutoRepeatDelay) ||
			(autoRepeatDeltaTv.tv_usec >
				sunAutoRepeatInitiate))
		autoRepeatReady++;
    }

    if (autoRepeatReady)
    {
	sunEnqueueAutoRepeat ();
	autoRepeatReady = 0;
    }
}
#endif /* 0 XXX */
