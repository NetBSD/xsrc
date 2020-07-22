/* $Xorg: sunKbd.c,v 1.3 2000/08/17 19:48:30 cpqbld Exp $ */
/*-
 * Copyright 1987 by the Regents of the University of California
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
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and The Open Group make no
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
/* $XFree86: xc/programs/Xserver/hw/sun/sunKbd.c,v 1.9 2003/11/17 22:20:36 dawes Exp $ */

#define NEED_EVENTS
#include "sun.h"
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
#include "mi.h"

#include <X11/extensions/XKB.h>
#include "xkbsrv.h"
#include "xkbstr.h"

#define SUN_LED_MASK	0x0f
#define MIN_KEYCODE	7	/* necessary to avoid the mouse buttons */
#define MAX_KEYCODE	255	/* limited by the protocol */
#ifndef KB_SUN4
#define KB_SUN4		4
#endif

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

static void SwapLKeys(KeySymsRec *);
static void SetLights(KeybdCtrl *, int);
static KeyCode LookupKeyCode(KeySym, XkbDescPtr, KeySymsPtr);
static void pseudoKey(DeviceIntPtr, Bool, KeyCode);
static void DoLEDs(DeviceIntPtr, KeybdCtrl *, sunKbdPrivPtr);

DeviceIntPtr	sunKeyboardDevice = NULL;

void
sunKbdWait(void)
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

static
void SwapLKeys(KeySymsRec* keysyms)
{
    unsigned int i;
    KeySym k;

    for (i = 2; i < keysyms->maxKeyCode * keysyms->mapWidth; i++)
	if (keysyms->map[i] == XK_L1 ||
	    keysyms->map[i] == XK_L2 ||
	    keysyms->map[i] == XK_L3 ||
	    keysyms->map[i] == XK_L4 ||
	    keysyms->map[i] == XK_L5 ||
	    keysyms->map[i] == XK_L6 ||
	    keysyms->map[i] == XK_L7 ||
	    keysyms->map[i] == XK_L8 ||
	    keysyms->map[i] == XK_L9 ||
	    keysyms->map[i] == XK_L10) {
	    /* yes, I could have done a clever two line swap! */
	    k = keysyms->map[i - 2];
	    keysyms->map[i - 2] = keysyms->map[i];
	    keysyms->map[i] = k;
	}
}

static void
SetLights(KeybdCtrl* ctrl, int fd)
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


/*-
 *-----------------------------------------------------------------------
 * sunBell --
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

static void
bell(int fd, int duration)
{
    int		    kbdCmd;   	    /* Command to give keyboard */

    kbdCmd = KBD_CMD_BELL;
    if (ioctl (fd, KIOCCMD, &kbdCmd) == -1) {
 	Error("Failed to activate bell");
	return;
    }
    if (duration) usleep (duration);
    kbdCmd = KBD_CMD_NOBELL;
    if (ioctl (fd, KIOCCMD, &kbdCmd) == -1)
	Error ("Failed to deactivate bell");
}

static void
sunBell(int percent, DeviceIntPtr device, pointer ctrl, int unused)
{
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
    sunKbdPrivPtr   pPriv = (sunKbdPrivPtr) device->public.devicePrivate;

    if (percent == 0 || kctrl->bell == 0)
 	return;

    bell (pPriv->fd, kctrl->bell_duration * 1000);
}

void
DDXRingBell(int volume, int pitch, int duration)
{
    DeviceIntPtr pKeyboard;
    sunKbdPrivPtr pPriv;

    pKeyboard = sunKeyboardDevice;
    if (pKeyboard != NULL) {
	pPriv = (sunKbdPrivPtr)pKeyboard->public.devicePrivate;
	bell(pPriv->fd, duration * 1000);
    }
}


#define XLED_NUM_LOCK    0x1
#define XLED_COMPOSE     0x4
#define XLED_SCROLL_LOCK 0x2
#define XLED_CAPS_LOCK   0x8

static KeyCode
LookupKeyCode(KeySym keysym, XkbDescPtr xkb, KeySymsPtr syms)
{
    KeyCode i;
    int ii, index = 0;

    for (i = xkb->min_key_code; i < xkb->max_key_code; i++)
	for (ii = 0; ii < syms->mapWidth; ii++)
	    if (syms->map[index++] == keysym)
		return i;
    return 0;
}

static void
pseudoKey(DeviceIntPtr device, Bool down, KeyCode keycode)
{
    int bit;
    CARD8 modifiers;
    CARD16 mask;
    BYTE* kptr;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    modifiers = device->key->xkbInfo->desc->map->modmap[keycode];
    if (down) {
	/* fool dix into thinking this key is now "down" */
	int i;
	*kptr |= bit;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    if (mask & modifiers) {
		device->key->modifierKeyCount[i]++;
		modifiers &= ~mask;
	    }
    } else {
	/* fool dix into thinking this key is now "up" */
	if (*kptr & bit) {
	    int i;
	    *kptr &= ~bit;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
		if (mask & modifiers) {
		    if (--device->key->modifierKeyCount[i] <= 0) {
			device->key->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	}
    }
}

static void
DoLEDs(
    DeviceIntPtr    device,	    /* Keyboard to alter */
    KeybdCtrl* ctrl,
    sunKbdPrivPtr pPriv
)
{
    XkbDescPtr xkb;
    KeySymsPtr syms;

    xkb = device->key->xkbInfo->desc;
    syms = XkbGetCoreMap(device);
    if (!syms)
	return;	/* XXX */

    if ((ctrl->leds & XLED_CAPS_LOCK) && !(pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Caps_Lock, xkb, syms));

    if (!(ctrl->leds & XLED_CAPS_LOCK) && (pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Caps_Lock, xkb, syms));

    if ((ctrl->leds & XLED_NUM_LOCK) && !(pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Num_Lock, xkb, syms));

    if (!(ctrl->leds & XLED_NUM_LOCK) && (pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Num_Lock, xkb, syms));

    if ((ctrl->leds & XLED_SCROLL_LOCK) && !(pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Scroll_Lock, xkb, syms));

    if (!(ctrl->leds & XLED_SCROLL_LOCK) && (pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Scroll_Lock, xkb, syms));

    if ((ctrl->leds & XLED_COMPOSE) && !(pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, TRUE,
		LookupKeyCode(SunXK_Compose, xkb, syms));

    if (!(ctrl->leds & XLED_COMPOSE) && (pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, FALSE,
		LookupKeyCode(SunXK_Compose, xkb, syms));

    pPriv->leds = ctrl->leds & 0x0f;
    SetLights (ctrl, pPriv->fd);
    xfree(syms->map);
    xfree(syms);
}

/*-
 *-----------------------------------------------------------------------
 * sunKbdCtrl --
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

static void
sunKbdCtrl(DeviceIntPtr device, KeybdCtrl* ctrl)
{
    sunKbdPrivPtr pPriv = (sunKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

    if (ctrl->click != pPriv->click) {
    	int kbdClickCmd;

	pPriv->click = ctrl->click;
	kbdClickCmd = pPriv->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (pPriv->fd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
    if ((pPriv->type == KB_SUN4) && (pPriv->leds != (ctrl->leds & 0x0f)))
	DoLEDs(device, ctrl, pPriv);
}

/*-
 *-----------------------------------------------------------------------
 * sunInitKbdNames --
 *	Handle the XKB initialization
 *
 * Results:
 *	None.
 *
 * Comments:
 *     This function needs considerable work, in conjunctions with
 *     the need to add geometry descriptions of Sun Keyboards.
 *     It would also be nice to have #defines for all the keyboard
 *     layouts so that we don't have to have these hard-coded
 *     numbers.
 *
 *-----------------------------------------------------------------------
 */
static void
sunInitKbdNames(XkbRMLVOSet *rmlvo, sunKbdPrivPtr pKbd)
{
#if 0 /* XXX to be revisited later */
#ifndef XKBBUFSIZE
#define XKBBUFSIZE 64
#endif
    static char keycodesbuf[XKBBUFSIZE];
    static char geometrybuf[XKBBUFSIZE];
    static char  symbolsbuf[XKBBUFSIZE];

    names->keymap = NULL;
    names->compat = "compat/complete";
    names->types  = "types/complete";
    names->keycodes = keycodesbuf;
    names->geometry = geometrybuf;
    names->symbols = symbolsbuf;
    (void) strcpy (keycodesbuf, "keycodes/");
    (void) strcpy (geometrybuf, "geometry/");
    (void) strcpy (symbolsbuf, "symbols/");

    /* keycodes & geometry */
    switch (pKbd->type) {
    case KB_SUN2:
	(void) strcat (names->keycodes, "sun(type2)");
	(void) strcat (names->geometry, "sun(type2)");
	(void) strcat (names->symbols, "us(sun2)");
	break;
    case KB_SUN3:
	(void) strcat (names->keycodes, "sun(type3)");
	(void) strcat (names->geometry, "sun(type3)");
	(void) strcat (names->symbols, "us(sun3)");
	break;
    case KB_SUN4:
	/* First, catch "fully known" models */
	switch (pKbd->layout) {
	case 11:		/* type4, Sweden */
	    (void) strcat (names->geometry, "sun(type4_se)");
	    (void) strcat (names->keycodes,
			   "sun(type4_se_swapctl)");
	    (void) strcat (names->symbols,
			   "sun/se(sun4)+se(fixdollar)");
	    return;
	    break;
	case 43:		/* type5/5c, Sweden */
	    (void) strcat (names->geometry, "sun(type5c_se)");
	    (void) strcat (names->keycodes, "sun(type5_se)");
	    (void) strcat (names->symbols,
			   "sun/se(sun5)+se(fixdollar)");
	    return;
	    break;
	case 90:		/* "Compact 1", Sweden (???) */
	    break;		/* No specific mapping, yet */
	default:
	    break;
	}

	if (pKbd->layout == 19) {
	    (void) strcat (names->keycodes, "sun(US101A)");
	    (void) strcat (names->geometry, "pc101-NG"); /* XXX */
	    (void) strcat (names->symbols, "us(pc101)");
	} else if (pKbd->layout < 33) {
	    (void) strcat (names->keycodes, "sun(type4)");
	    (void) strcat (names->geometry, "sun(type4)");
	    if (sunSwapLkeys)
		(void) strcat (names->symbols, "sun/us(sun4ol)");
	    else
		(void) strcat (names->symbols, "sun/us(sun4)");
	} else {
	    switch (pKbd->layout) {
	    case 33: case 80: /* U.S. */
	    case 47: case 94: /* Korea */
	    case 48: case 95: /* Taiwan */
	    case 49: case 96: /* Japan */
		(void) strcat (names->keycodes, "sun(type5)");
		(void) strcat (names->geometry, "sun(type5)");
		break;
	    case 34: case 81: /* U.S. Unix */
		(void) strcat (names->keycodes, "sun(type5)");
		(void) strcat (names->geometry, "sun(type5unix)");
		break;
	    default:
		(void) strcat (names->keycodes, "sun(type5_euro)");
		(void) strcat (names->geometry, "sun(type5euro)");
	    }

	    if (sunSwapLkeys)
		(void) strcat (names->symbols, "sun/us(sun5ol)");
	    else
		(void) strcat (names->symbols, "sun/us(sun5)");
	}
	break;
    default:
	names->keycodes = names->geometry = NULL;
	break;
    }

    /* extra symbols */

    if (pKbd->type == KB_SUN4) {
	switch (pKbd->layout) {
	case  4: case 36: case 83:
	case  5: case 37: case 84:
	case  6: case 38: case 85:
	case  8: case 40: case 87:
	case  9: case 41: case 88:
	case 10: case 42: case 89:
/*	case 11: case 43: case 90: */ /* handled earlier */
	case 12: case 44: case 91:
	case 13: case 45: case 92:
	case 14: case 46: case 93:
	    (void) strcat (names->symbols, "+iso9995-3(basic)"); break;
	}
    }

    if (pKbd->type == KB_SUN4) {
	switch (pKbd->layout) {
	case  0: case  1: case 33: case 34: case 80: case 81:
	    break;
	case  3:
	    (void) strcat (names->symbols, "+ca"); break;
	case  4: case 36: case 83:
	    (void) strcat (names->symbols, "+dk"); break;
	case  5: case 37: case 84:
	    (void) strcat (names->symbols, "+de"); break;
	case  6: case 38: case 85:
	    (void) strcat (names->symbols, "+it"); break;
	case  8: case 40: case 87:
	    (void) strcat (names->symbols, "+no"); break;
	case  9: case 41: case 88:
	    (void) strcat (names->symbols, "+pt"); break;
	case 10: case 42: case 89:
	    (void) strcat (names->symbols, "+es"); break;
	    /* case 11: case 43: */ /* handled earlier */
	case 90:
	    (void) strcat (names->symbols, "+se"); break;
	case 12: case 44: case 91:
	    (void) strcat (names->symbols, "+fr_CH"); break;
	case 13: case 45: case 92:
	    (void) strcat (names->symbols, "+de_CH"); break;
	case 14: case 46: case 93:
	    (void) strcat (names->symbols, "+gb"); break; /* s/b en_UK */
	case 52:
	    (void) strcat (names->symbols, "+pl"); break;
	case 53:
	    (void) strcat (names->symbols, "+cs"); break;
	case 54:
	    (void) strcat (names->symbols, "+ru"); break;
#if 0
	/* don't have symbols defined for these yet, let them default */
	case  2:
	    (void) strcat (names->symbols, "+fr_BE"); break;
	case  7: case 39: case 86:
	    (void) strcat (names->symbols, "+nl"); break;
	case 50: case 97:
	    (void) strcat (names->symbols, "+fr_CA"); break;
	case 16: case 47: case 94:
	    (void) strcat (names->symbols, "+ko"); break;
	case 17: case 48: case 95:
	    (void) strcat (names->symbols, "+tw"); break;
	case 32: case 49: case 96:
	    (void) strcat (names->symbols, "+jp"); break;
	case 51:
	    (void) strcat (names->symbols, "+hu"); break;
#endif
	/*
	 * by setting the symbols to NULL XKB will use the symbols in
	 * the "default" keymap.
	 */
	default:
	    names->symbols = NULL; return; break;
	}
    }
#else
    rmlvo->rules = "base";
    rmlvo->model = NULL;
    rmlvo->layout = NULL;
    rmlvo->variant = NULL;
    rmlvo->options = NULL;
#endif
}

/*-
 *-----------------------------------------------------------------------
 * sunKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *-----------------------------------------------------------------------
 */

int
sunKbdProc(DeviceIntPtr device, int what)
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    sunKbdPrivPtr pPriv;
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    XkbRMLVOSet rmlvo;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

    switch (what) {
    case DEVICE_INIT:
	if (pKeyboard != (DevicePtr)sunKeyboardDevice) {
	    ErrorF ("Cannot open non-system keyboard\n");
	    return (!Success);
	}

	if (!workingKeySyms) {
	    workingKeySyms = &sunKeySyms[sunKbdPriv.type];

	    if (sunKbdPriv.type == KB_SUN4 && sunSwapLkeys)
		SwapLKeys(workingKeySyms);

	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}

	if (!workingModMap) {
	    workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) memset(workingModMap, 0, MAP_LENGTH);
	    for(i=0; sunModMaps[sunKbdPriv.type][i].key != 0; i++)
		workingModMap[sunModMaps[sunKbdPriv.type][i].key + MIN_KEYCODE] =
		sunModMaps[sunKbdPriv.type][i].modifiers;
	}

	pKeyboard->devicePrivate = (pointer)&sunKbdPriv;
	pKeyboard->on = FALSE;

	sunInitKbdNames(&rmlvo, pKeyboard->devicePrivate);
#if 0 /* XXX needs more work for Xorg xkb */
	InitKeyboardDeviceStruct(device, rmlvo,
				 sunBell, sunKbdCtrl);
#else
	InitKeyboardDeviceStruct(device, NULL,
				 sunBell, sunKbdCtrl);
	XkbApplyMappingChange(device, workingKeySyms,
			      workingKeySyms->minKeyCode,
			      workingKeySyms->maxKeyCode -
			      workingKeySyms->minKeyCode + 1,
			      workingModMap, serverClient);
#endif
	break;

    case DEVICE_ON:
	pPriv = (sunKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
	if (sunChangeKbdTranslation(pPriv->fd,TRUE) == -1)
	    FatalError("Can't set keyboard translation\n");
	AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (sunKbdPrivPtr)pKeyboard->devicePrivate;
	if (pPriv->type == KB_SUN4) {
	    /* dumb bug in Sun's keyboard! Turn off LEDS before resetting */
	    pPriv->leds = 0;
	    ctrl->leds = 0;
	    SetLights(ctrl, pPriv->fd);
	}
	/*
	 * Restore original keyboard directness and translation.
	 */
	if (sunChangeKbdTranslation(pPriv->fd,FALSE) == -1)
	    FatalError("Can't reset keyboard translation\n");
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
 * sunKbdGetEvents --
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

Firm_event *
sunKbdGetEvents(int fd, Bool on, int *pNumEvents, Bool *pAgain)
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
	if (on) {
	    *pNumEvents = nBytes / sizeof (Firm_event);
	    *pAgain = (nBytes == sizeof (evBuf));
	} else {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	}
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * sunKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */

void
sunKbdEnqueueEvent(DeviceIntPtr device, Firm_event *fe)
{
    BYTE		keycode;
    int			type;
    int			i, nevents;

    GetEventList(&sunEvents);
    keycode = (fe->id & 0x7f) + MIN_KEYCODE;

    type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    nevents = GetKeyboardEvents(sunEvents, device, type, keycode);
    for (i = 0; i < nevents; i++)
	mieqEnqueue(device, (InternalEvent *)(sunEvents + i)->event);
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
int
sunChangeKbdTranslation(int fd, Bool makeTranslated)
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

/*ARGSUSED*/
Bool
LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
    return TRUE;
}

/*ARGSUSED*/
void
sunBlockHandler(int nscreen, pointer pbdata, pointer pTimeout, pointer pReadmask)
{
}

/*ARGSUSED*/
void
sunWakeupHandler(int nscreen, pointer pbdata, unsigned long err, pointer pReadmask)
{
}
