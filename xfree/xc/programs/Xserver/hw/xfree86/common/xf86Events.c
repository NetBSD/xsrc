/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Events.c,v 3.104 2000/12/07 20:32:54 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: xf86Events.c /main/46 1996/10/25 11:36:30 kaleb $ */

/* [JCH-96/01/21] Extended std reverse map to four buttons. */

#include "X.h"
#include "Xproto.h"
#include "misc.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSlib.h"
#include "atKeynames.h"
#include "Xpoll.h"


#ifdef XFreeXDGA
#include "dgaproc.h"
#endif

#ifdef XINPUT
#include "XI.h"
#include "XIproto.h"
#include "xf86Xinput.h"
#else
#include "inputstr.h"
#endif

#include "mipointer.h"

#ifdef XKB
extern Bool noXkbExtension;
#endif

#define XE_POINTER  1
#define XE_KEYBOARD 2

#ifdef XINPUT
#define __EqEnqueue(ev) xf86eqEnqueue(ev)
#else
#define __EqEnqueue(ev) mieqEnqueue(ev)
#endif

#define EqEnqueue(ev) { \
    int __sigstate = xf86BlockSIGIO (); \
    __EqEnqueue (ev); \
    xf86UnblockSIGIO(__sigstate); \
}

#ifdef XTESTEXT1

#define	XTestSERVER_SIDE
#include "xtestext1.h"
extern short xtest_mousex;
extern short xtest_mousey;
extern int   on_steal_input;          
extern Bool  XTestStealKeyData();
extern void  XTestStealMotionData();
#define ENQUEUE(ev, code, direction, dev_type) \
  (ev)->u.u.detail = (code); \
  (ev)->u.u.type   = (direction); \
  if (!on_steal_input ||  \
      XTestStealKeyData((ev)->u.u.detail, (ev)->u.u.type, dev_type, \
			xtest_mousex, xtest_mousey)) \
  EqEnqueue((ev))
#else /* ! XTESTEXT1 */

#define ENQUEUE(ev, code, direction, dev_type) \
  (ev)->u.u.detail = (code); \
  (ev)->u.u.type   = (direction); \
  EqEnqueue((ev))

#endif

/*
 * The first of many hack's to get VT switching to work under
 * Solaris 2.1 for x86. The basic problem is that Solaris is supposed
 * to be SVR4. It is for the most part, except where the video interface
 * is concerned.  These hacks work around those problems.
 * See the comments for Linux, and SCO. 
 *
 * This is a toggleling variable:
 *  FALSE = No VT switching keys have been pressed last time around
 *  TRUE  = Possible VT switch Pending
 * (DWH - 12/2/93)
 *
 * This has been generalised to work with Linux and *BSD+syscons (DHD)
 */

#ifdef USE_VT_SYSREQ
static Bool VTSysreqToggle = FALSE;
#endif /* !USE_VT_SYSREQ */
static Bool VTSwitchEnabled = TRUE;   /* Allows run-time disabling for
                                         *BSD and for avoiding VT
                                         switches when using the DRI
                                         automatic full screen mode.*/

extern fd_set EnabledDevices;

#if defined(XQUEUE)
extern void xf86XqueRequest(void);
#endif
extern void (*xf86OSPMClose)(void);

static void xf86VTSwitch(void);

/*
 * Allow arbitrary drivers or other XFree86 code to register with our main
 * Wakeup handler.
 */
typedef struct x_IHRec {
    int			fd;
    InputHandlerProc	ihproc;
    pointer		data;
    Bool		enabled;
    struct x_IHRec *	next;
} IHRec, *IHPtr;

static IHPtr InputHandlers = NULL;


/*
 * TimeSinceLastInputEvent --
 *      Function used for screensaver purposes by the os module. Retruns the
 *      time in milliseconds since there last was any input.
 */

int
TimeSinceLastInputEvent()
{
  if (xf86Info.lastEventTime == 0) {
    xf86Info.lastEventTime = GetTimeInMillis();
  }
  return GetTimeInMillis() - xf86Info.lastEventTime;
}



/*
 * SetTimeSinceLastInputEvent --
 *      Set the lastEventTime to now.
 */

void
SetTimeSinceLastInputEvent()
{
  xf86Info.lastEventTime = GetTimeInMillis();
}



/*
 * ProcessInputEvents --
 *      Retrieve all waiting input events and pass them to DIX in their
 *      correct chronological order. Only reads from the system pointer
 *      and keyboard.
 */

void
ProcessInputEvents ()
{
  int x, y;
#ifdef INHERIT_LOCK_STATE
  static int generation = 0;
#endif

#ifdef AMOEBA
#define MAXEVENTS	    32
#define BUTTON_PRESS	    0x1000
#define MAP_BUTTON(ev,but)  (((ev) == EV_ButtonPress) ? \
			     ((but) | BUTTON_PRESS) : ((but) & ~BUTTON_PRESS))
#define KEY_RELEASE	    0x80
#define MAP_KEY(ev, key)    (((ev) == EV_KeyReleaseEvent) ? \
			     ((key) | KEY_RELEASE) : ((key) & ~KEY_RELEASE))

    register IOPEvent  *e, *elast;
    IOPEvent		events[MAXEVENTS];
    int			dx, dy, nevents;
#endif

    /*
     * With INHERIT_LOCK_STATE defined, the initial state of CapsLock, NumLock
     * and ScrollLock will be set to match that of the VT the server is
     * running on.
     */
#ifdef INHERIT_LOCK_STATE
    if (generation != serverGeneration) {
      xEvent kevent;
      DevicePtr pKeyboard = xf86Info.pKeyboard;
      extern unsigned int xf86InitialCaps, xf86InitialNum, xf86InitialScroll;

      generation = serverGeneration;
      kevent.u.keyButtonPointer.time = GetTimeInMillis();
      kevent.u.keyButtonPointer.rootX = 0;
      kevent.u.keyButtonPointer.rootY = 0;
      kevent.u.u.type = KeyPress;


      if (xf86InitialCaps) {
        kevent.u.u.detail = xf86InitialCaps;
        (* pKeyboard->processInputProc)(&kevent, (DeviceIntPtr)pKeyboard, 1);
        xf86InitialCaps = 0;
      }
      if (xf86InitialNum) {
        kevent.u.u.detail = xf86InitialNum;
        (* pKeyboard->processInputProc)(&kevent, (DeviceIntPtr)pKeyboard, 1);
        xf86InitialNum = 0;
      }
      if (xf86InitialScroll) {
        kevent.u.u.detail = xf86InitialScroll;
        (* pKeyboard->processInputProc)(&kevent, (DeviceIntPtr)pKeyboard, 1);
        xf86InitialScroll = 0;
      }
    }
#endif

#ifdef AMOEBA
    /*
     * Get all events from the IOP server
     */
    while ((nevents = AmoebaGetEvents(events, MAXEVENTS)) > 0) {
      for (e = &events[0], elast = &events[nevents]; e < elast; e++) {
          xf86Info.lastEventTime = e->time;
          switch (e->type) {
          case EV_PointerDelta:
	      if (e->x != 0 || e->y != 0) {
                  xf86PostMseEvent(&xf86Info.pMouse, 0, e->x, e->y);
	      }
              break;
          case EV_ButtonPress:
          case EV_ButtonRelease:
              xf86PostMseEvent(&xf86Info.pMouse, MAP_BUTTON(e->type, e->keyorbut), 0, 0);
              break;
          case EV_KeyPressEvent:
          case EV_KeyReleaseEvent:
              xf86PostKbdEvent(MAP_KEY(e->type, e->keyorbut));
              break;
          default:
              /* this shouldn't happen */
              ErrorF("stray event %d (%d,%d) %x\n",
                      e->type, e->x, e->y, e->keyorbut);
              break;
          }
      }
    }
#endif

  xf86Info.inputPending = FALSE;

#ifdef XINPUT
  xf86eqProcessInputEvents();
#else
  mieqProcessInputEvents();
#endif
  miPointerUpdate();

  miPointerPosition(&x, &y);
  xf86SetViewport(xf86Info.currentScreen, x, y);
}



/*
 * xf86PostKbdEvent --
 *	Translate the raw hardware KbdEvent into an XEvent, and tell DIX
 *	about it. Scancode preprocessing and so on is done ...
 *
 *  OS/2 specific xf86PostKbdEvent(key) has been moved to os-support/os2/os2_kbd.c
 *  as some things differ, and I did not want to scatter this routine with
 *  ifdefs further (hv).
 */

#ifdef ASSUME_CUSTOM_KEYCODES
extern u_char SpecialServerMap[];
#endif /* ASSUME_CUSTOM_KEYCODES */

#if !defined(__EMX__) && !defined(__SOL8__) && !defined(__CYGWIN__)
void
xf86PostKbdEvent(unsigned key)
{
  int         scanCode = (key & 0x7f);
  int         specialkey = 0;
  Bool        down = (key & 0x80 ? FALSE : TRUE);
  KeyClassRec *keyc = ((DeviceIntPtr)xf86Info.pKeyboard)->key;
  Bool        updateLeds = FALSE;
  Bool        UsePrefix = FALSE;
  Bool        Direction = FALSE;
  xEvent      kevent;
  KeySym      *keysym;
  int         keycode;
  static int  lockkeys = 0;
#if defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
  static Bool first_time = TRUE;
#endif
#if defined(__sparc__)
  static int  kbdSun = -1;
#endif

#if defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
  if (first_time)
  {
    first_time = FALSE;
    VTSwitchEnabled = (xf86Info.consType == SYSCONS)
	    || (xf86Info.consType == PCVT);
  }
#endif

#if defined (__sparc__)
  if (kbdSun == -1) {
    if ((xf86Info.xkbmodel && !strcmp(xf86Info.xkbmodel, "sun"))
	|| (xf86Info.xkbrules && !strcmp(xf86Info.xkbrules, "sun")))
      kbdSun = 1;
    else
      kbdSun = 0;
  }
  if (kbdSun)
    goto special;
#endif /* __sparc__ */

#if defined (i386) && defined (SVR4)
    /* 
     * PANIX returns DICOP standards based keycodes in using 106jp 
     * keyboard. We need to remap some keys. 
     */
#define KEY_P_UP	0x5A
#define KEY_P_PGUP	0x5B
#define KEY_P_LEFT	0x5C
#define KEY_P_BKSL	0x73
#define KEY_P_YEN	0x7D
#define KEY_P_NFER	0x7B
#define KEY_P_XFER	0x79

  if(xf86Info.panix106 == TRUE){
    switch (scanCode) {
    /* case 0x78:        scanCode = KEY_P_UP;     break;   not needed*/
    case 0x56:        scanCode = KEY_P_BKSL;   break;  /* Backslash */
    case 0x5A:        scanCode = KEY_P_NFER;   break;  /* No Kanji Transfer*/
    case 0x5B:        scanCode = KEY_P_XFER;   break;  /* Kanji Tranfer */
    case 0x5C:        scanCode = KEY_P_YEN;    break;  /* Yen curs pgup */
    case 0x6B:        scanCode = KEY_P_LEFT;   break;  /* Cur Left */
    case 0x6F:        scanCode = KEY_P_PGUP;   break;  /* Cur PageUp */
    case 0x72:        scanCode = KEY_AltLang;  break;  /* AltLang(right) */
    case 0x73:        scanCode = KEY_RCtrl;    break;  /* not needed */
    }
  }
#endif  /* i386 && SVR4 */

#ifndef ASSUME_CUSTOM_KEYCODES
  /*
   * First do some special scancode remapping ...
   */
  if (xf86Info.scanPrefix == 0) {

    switch (scanCode) {
    case KEY_Prefix0:
    case KEY_Prefix1:
#if defined(PCCONS_SUPPORT) || defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
      if (xf86Info.consType == PCCONS || xf86Info.consType == SYSCONS
	  || xf86Info.consType == PCVT) {
#endif
        xf86Info.scanPrefix = scanCode;  /* special prefixes */
        return;
#if defined(PCCONS_SUPPORT) || defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
      }
      break;
#endif
    }
  }

  else if (
#ifdef CSRG_BASED
           (xf86Info.consType == PCCONS || xf86Info.consType == SYSCONS
	    || xf86Info.consType == PCVT) &&
#endif
           (xf86Info.scanPrefix == KEY_Prefix0)) {
    xf86Info.scanPrefix = 0;
	  
    switch (scanCode) {
    case KEY_KP_7:        scanCode = KEY_Home;      break;  /* curs home */
    case KEY_KP_8:        scanCode = KEY_Up;        break;  /* curs up */
    case KEY_KP_9:        scanCode = KEY_PgUp;      break;  /* curs pgup */
    case KEY_KP_4:        scanCode = KEY_Left;      break;  /* curs left */
    case KEY_KP_5:        scanCode = KEY_Begin;     break;  /* curs begin */
    case KEY_KP_6:        scanCode = KEY_Right;     break;  /* curs right */
    case KEY_KP_1:        scanCode = KEY_End;       break;  /* curs end */
    case KEY_KP_2:        scanCode = KEY_Down;      break;  /* curs down */
    case KEY_KP_3:        scanCode = KEY_PgDown;    break;  /* curs pgdown */
    case KEY_KP_0:        scanCode = KEY_Insert;    break;  /* curs insert */
    case KEY_KP_Decimal:  scanCode = KEY_Delete;    break;  /* curs delete */
    case KEY_Enter:       scanCode = KEY_KP_Enter;  break;  /* keypad enter */
    case KEY_LCtrl:       scanCode = KEY_RCtrl;     break;  /* right ctrl */
    case KEY_KP_Multiply: scanCode = KEY_Print;     break;  /* print */
    case KEY_Slash:       scanCode = KEY_KP_Divide; break;  /* keyp divide */
    case KEY_Alt:         scanCode = KEY_AltLang;   break;  /* right alt */
    case KEY_ScrollLock:  scanCode = KEY_Break;     break;  /* curs break */
    case 0x5b:            scanCode = KEY_LMeta;     break;
    case 0x5c:            scanCode = KEY_RMeta;     break;
    case 0x5d:            scanCode = KEY_Menu;      break;
    case KEY_F3:          scanCode = KEY_F13;       break;
    case KEY_F4:          scanCode = KEY_F14;       break;
    case KEY_F5:          scanCode = KEY_F15;       break;
    case KEY_F6:          scanCode = KEY_F16;       break;
    case KEY_F7:          scanCode = KEY_F17;       break;
    case KEY_KP_Plus:     scanCode = KEY_KP_DEC;    break;
      /*
       * Ignore virtual shifts (E0 2A, E0 AA, E0 36, E0 B6)
       */
    case 0x2A:
    case 0x36:
	return;
    default:
      xf86MsgVerb(X_INFO, 2, "Unreported Prefix0 scancode: 0x%02x\n",
		  scanCode);
      /*
       * "Internet" keyboards are generating lots of new codes.  Let them
       * pass.  There is little consistency between them, so don't bother
       * with symbolic names at this level.
       */
      scanCode += 0x78;
    }
  }
  
  else if (xf86Info.scanPrefix == KEY_Prefix1)
    {
      xf86Info.scanPrefix = (scanCode == KEY_LCtrl) ? KEY_LCtrl : 0;
      return;
    }
  
  else if (xf86Info.scanPrefix == KEY_LCtrl)
    {
      xf86Info.scanPrefix = 0;
      if (scanCode != KEY_NumLock) return;
      scanCode = KEY_Pause;       /* pause */
    }
#endif /* !ASSUME_CUSTOM_KEYCODES */

  /*
   * and now get some special keysequences
   */

#ifdef ASSUME_CUSTOM_KEYCODES
  specialkey = SpecialServerMap[scanCode];
#else /* ASSUME_CUSTOM_KEYCODES */
  specialkey = scanCode;
#endif /* ASSUME_CUSTOM_KEYCODES */

  if (xf86IsPc98()) {
    switch (scanCode) {
      case 0x0e: specialkey = 0x0e; break; /* KEY_BackSpace */
      case 0x40: specialkey = 0x4a; break; /* KEY_KP_Minus  */
      case 0x49: specialkey = 0x4e; break; /* KEY_KP_Plus   */
      case 0x62: specialkey = 0x3b; break; /* KEY_F1        */
      case 0x63: specialkey = 0x3c; break; /* KEY_F2        */
      case 0x64: specialkey = 0x3d; break; /* KEY_F3        */
      case 0x65: specialkey = 0x3e; break; /* KEY_F4        */
      case 0x66: specialkey = 0x3f; break; /* KEY_F5        */
      case 0x67: specialkey = 0x40; break; /* KEY_F6        */
      case 0x68: specialkey = 0x41; break; /* KEY_F7        */
      case 0x69: specialkey = 0x42; break; /* KEY_F8        */
      case 0x6a: specialkey = 0x43; break; /* KEY_F9        */
      case 0x6b: specialkey = 0x44; break; /* KEY_F10       */
      /* case 0x73: specialkey = 0x38; break; KEY_Alt       */
      /* case 0x74: specialkey = 0x1d; break; KEY_LCtrl     */
      default:   specialkey = 0x00; break;
    }
  }

#if defined (__sparc__)
special:
  if (kbdSun) {
    switch (scanCode) {
      case 0x2b: specialkey = KEY_BackSpace; break;
      case 0x47: specialkey = KEY_KP_Minus; break;
      case 0x7d: specialkey = KEY_KP_Plus; break;
      case 0x05: specialkey = KEY_F1; break;
      case 0x06: specialkey = KEY_F2; break;
      case 0x08: specialkey = KEY_F3; break;
      case 0x0a: specialkey = KEY_F4; break;
      case 0x0c: specialkey = KEY_F5; break;
      case 0x0e: specialkey = KEY_F6; break;
      case 0x10: specialkey = KEY_F7; break;
      case 0x11: specialkey = KEY_F8; break;
      case 0x12: specialkey = KEY_F9; break;
      case 0x07: specialkey = KEY_F10; break;
      case 0x09: specialkey = KEY_F11; break;
      case 0x0b: specialkey = KEY_F12; break;
      default: specialkey = 0; break;
    }
    /*
     * XXX XXX XXX:
     *
     * I really don't know what's wrong here, but passing the real
     * scanCode offsets by one from XKB's point of view.
     *
     * (ecd@skynet.be, 980405)
     */
    scanCode--;
  }
#endif /* defined (__sparc__) */

  if ((ModifierDown(ControlMask | AltMask)) ||
      (ModifierDown(ControlMask | AltLangMask)))
    {
      
      switch (specialkey) {
	
      case KEY_BackSpace:
	if (!xf86Info.dontZap) {
#ifdef XFreeXDGA
	 DGAShutdown();
#endif
	 GiveUp(0);
        }
	break;
	
	/*
	 * The idea here is to pass the scancode down to a list of
	 * registered routines. There should be some standard conventions
	 * for processing certain keys.
	 */
      case KEY_KP_Minus:   /* Keypad - */
	if (!xf86Info.dontZoom) {
	  if (down) xf86ZoomViewport(xf86Info.currentScreen, -1);
	  return;
	}
	break;
	
      case KEY_KP_Plus:   /* Keypad + */
	if (!xf86Info.dontZoom) {
	  if (down) xf86ZoomViewport(xf86Info.currentScreen,  1);
	  return;
	}
	break;

	/* Under QNX4, we set the vtPending flag for VT switching and 
	 * let the VT switch function do the rest...
	 * This is a little different from the other OS'es.
	 */
#if defined(QNX4)
      case KEY_1:
      case KEY_2:
      case KEY_3:
      case KEY_4:
      case KEY_5:
      case KEY_6:
      case KEY_7:
      case KEY_8:
      case KEY_9:
		if (down){
			xf86Info.vtRequestsPending = 
				specialkey - KEY_1 + 1;
			return;
			}
		break;
#endif

#if defined(linux) || (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT))) || defined(SCO)
	/*
	 * Under Linux, the raw keycodes are consumed before the kernel
	 * does any processing on them, so we must emulate the vt switching
	 * we want ourselves.
	 */
      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
      case KEY_F8:
      case KEY_F9:
      case KEY_F10:
        if (VTSwitchEnabled && !xf86Info.vtSysreq
#if (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)))
	    && (xf86Info.consType == SYSCONS || xf86Info.consType == PCVT)
#endif
	    )
        {
	  if (down)
#ifdef SCO325
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey - KEY_F1);
#else
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey - KEY_F1 + 1);
#endif
          return;
        }
	break;
      case KEY_F11:
      case KEY_F12:
        if (VTSwitchEnabled && !xf86Info.vtSysreq
#if (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)))
	    && (xf86Info.consType == SYSCONS || xf86Info.consType == PCVT)
#endif
	    )
        {
	  if (down)
#ifdef SCO325
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey - KEY_F11 + 10);
#else
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey - KEY_F11 + 11);
#endif
          return;
        }
	break;
#endif /* linux || BSD with VTs */

      /* just worth mentioning here: any 386bsd keyboard driver
       * (pccons.c or co_kbd.c) catches CTRL-ALT-DEL and CTRL-ALT-ESC
       * before any application (e.g. XF86) will see it
       * OBS: syscons does not, nor does pcvt !
       */
      } 
    }

    /*
     * Start of actual Solaris VT switching code.  
     * This should pretty much emulate standard SVR4 switching keys.
     * 
     * DWH 12/2/93
     */

#ifdef USE_VT_SYSREQ
    if (VTSwitchEnabled && xf86Info.vtSysreq)
    {
      switch (specialkey)
      {
      /*
       * syscons on *BSD doesn't have a VT #0  -- don't think Linux does
       * either
       */
#if defined (sun) && defined (i386) && defined (SVR4)
      case KEY_H: 
	if (VTSysreqToggle && down)
        {
          ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0);
          VTSysreqToggle = 0;
          return; 
        }
	break;

      /*
       * Yah, I know the N, and P keys seem backwards, however that's
       * how they work under Solaris
       * XXXX N means go to next active VT not necessarily vtno+1 (or vtno-1)
       */

      case KEY_N:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno - 1 ) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      case KEY_P:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno + 1 ) < 0)
            if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0) < 0)
              ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;
#endif

      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
      case KEY_F8:
      case KEY_F9:
      case KEY_F10:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey-KEY_F1 + 1) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      case KEY_F11:
      case KEY_F12:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, specialkey-KEY_F11 + 11) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      /* Ignore these keys -- ie don't let them cancel an alt-sysreq */
      case KEY_Alt:
      case KEY_AltLang:
	break;

      case KEY_SysReqest:
        if (down && (ModifierDown(AltMask) || ModifierDown(AltLangMask)))
          VTSysreqToggle = TRUE;
	break;

      default:
        if (VTSysreqToggle)
	{
	  /*
	   * We only land here when Alt-SysReq is followed by a
	   * non-switching key.
	   */
          VTSysreqToggle = FALSE;

        }
      }
    }

#endif /* USE_VT_SYSREQ */

#ifdef SCO
    /*
     *	With the console in raw mode, SCO will not switch consoles,
     *	you get around this by activating the next console along, if
     *	this fails then go back to console 0, if there is only one
     *	then it doesn't matter, switching to yourself is a nop as far
     *	as the console driver is concerned.
     *	We could do something similar to linux here but SCO ODT uses
     *	Ctrl-PrintScrn, so why change?
     */
    if (specialkey == KEY_Print && ModifierDown(ControlMask)) {
      if (down)
        if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno + 1) < 0)
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
      return;
    }
#endif /* SCO */

  /*
   * Now map the scancodes to real X-keycodes ...
   */
  keycode = scanCode + MIN_KEYCODE;
  keysym = (keyc->curKeySyms.map +
	    keyc->curKeySyms.mapWidth * 
	    (keycode - keyc->curKeySyms.minKeyCode));
#ifdef XKB
  if (noXkbExtension) {
#endif
  /*
   * Filter autorepeated caps/num/scroll lock keycodes.
   */
#define CAPSFLAG 0x01
#define NUMFLAG 0x02
#define SCROLLFLAG 0x04
#define MODEFLAG 0x08
  if( down ) {
    switch( keysym[0] ) {
        case XK_Caps_Lock :
          if (lockkeys & CAPSFLAG)
              return;
	  else
	      lockkeys |= CAPSFLAG;
          break;

        case XK_Num_Lock :
          if (lockkeys & NUMFLAG)
              return;
	  else
	      lockkeys |= NUMFLAG;
          break;

        case XK_Scroll_Lock :
          if (lockkeys & SCROLLFLAG)
              return;
	  else
	      lockkeys |= SCROLLFLAG;
          break;
    }
    if (keysym[1] == XF86XK_ModeLock)
    {
      if (lockkeys & MODEFLAG)
          return;
      else
          lockkeys |= MODEFLAG;
    }
      
  }
  else {
    switch( keysym[0] ) {
        case XK_Caps_Lock :
            lockkeys &= ~CAPSFLAG;
            break;

        case XK_Num_Lock :
            lockkeys &= ~NUMFLAG;
            break;

        case XK_Scroll_Lock :
            lockkeys &= ~SCROLLFLAG;
            break;
    }
    if (keysym[1] == XF86XK_ModeLock)
      lockkeys &= ~MODEFLAG;
  }

  /*
   * LockKey special handling:
   * ignore releases, toggle on & off on presses.
   * Don't deal with the Caps_Lock keysym directly, but check the lock modifier
   */
  if (keyc->modifierMap[keycode] & LockMask ||
      keysym[0] == XK_Scroll_Lock ||
      keysym[1] == XF86XK_ModeLock ||
      keysym[0] == XK_Num_Lock)
    {
      Bool flag;

      if (!down) return;
      if (KeyPressed(keycode)) {
	down = !down;
	flag = FALSE;
      }
      else
	flag = TRUE;

      if (keyc->modifierMap[keycode] & LockMask)   xf86Info.capsLock   = flag;
      if (keysym[0] == XK_Num_Lock)    xf86Info.numLock    = flag;
      if (keysym[0] == XK_Scroll_Lock) xf86Info.scrollLock = flag;
      if (keysym[1] == XF86XK_ModeLock)   xf86Info.modeSwitchLock = flag;
      updateLeds = TRUE;
    }

#ifndef ASSUME_CUSTOM_KEYCODES
  /*
   * normal, non-keypad keys
   */
  if (scanCode < KEY_KP_7 || scanCode > KEY_KP_Decimal) {
#if !defined(CSRG_BASED) && !defined(MACH386) && !defined(MINIX) && !defined(__OSF__) && !defined(__GNU__) && !defined(__CYGWIN__)
    /*
     * magic ALT_L key on AT84 keyboards for multilingual support
     */
    if (xf86Info.kbdType == KB_84 &&
	ModifierDown(AltMask) &&
	keysym[2] != NoSymbol)
      {
	UsePrefix = TRUE;
	Direction = TRUE;
      }
#endif /* !CSRG_BASED && !MACH386 && !MINIX && !__OSF__ */
  }
#endif /* !ASSUME_CUSTOM_KEYCODES */
  if (updateLeds) xf86KbdLeds();
#ifdef XKB
  }
#endif

  /*
   * check for an autorepeat-event
   */
  if (down && KeyPressed(keycode)) {
      KbdFeedbackClassRec *kbdfeed = ((DeviceIntPtr)xf86Info.pKeyboard)->kbdfeed;
      if ((xf86Info.autoRepeat != AutoRepeatModeOn) ||
          keyc->modifierMap[keycode] ||
          (kbdfeed && !(kbdfeed->ctrl.autoRepeats[keycode>>3] & ( 1<<(keycode&7) ))))
          return;
  }


  xf86Info.lastEventTime = kevent.u.keyButtonPointer.time = GetTimeInMillis();
  /*
   * And now send these prefixes ...
   * NOTE: There cannot be multiple Mode_Switch keys !!!!
   */
  if (UsePrefix)
    {
      ENQUEUE(&kevent,
	      keyc->modifierKeyMap[keyc->maxKeysPerModifier*7],
	      (Direction ? KeyPress : KeyRelease),
	      XE_KEYBOARD);
      ENQUEUE(&kevent, keycode, (down ? KeyPress : KeyRelease), XE_KEYBOARD);
      ENQUEUE(&kevent,
	      keyc->modifierKeyMap[keyc->maxKeysPerModifier*7],
	      (Direction ? KeyRelease : KeyPress),
	      XE_KEYBOARD);
    }
  else 
    {
      ENQUEUE(&kevent, keycode, (down ? KeyPress : KeyRelease), XE_KEYBOARD);
    }
}
#endif /* !__EMX__ */


#ifndef AMOEBA

/*
 * xf86Wakeup --
 *      Os wakeup handler.
 */

/* ARGSUSED */
void
xf86Wakeup(pointer blockData, int err, pointer pReadmask)
{
#if !defined(__EMX__) && !defined(__QNX__) && !defined(__CYGWIN__)
#ifdef	__OSF__
    fd_set kbdDevices;
    fd_set mseDevices;
#endif	/* __OSF__ */
    fd_set* LastSelectMask = (fd_set*)pReadmask;
    fd_set devicesWithInput;
    InputInfoPtr pInfo;

    if (err >= 0) {

	XFD_ANDSET(&devicesWithInput, LastSelectMask, &EnabledDevices);
#ifndef __OSF__
	if (XFD_ANYSET(&devicesWithInput)) {
	    (xf86Info.kbdEvents)();
	    pInfo = xf86InputDevs;
	    while (pInfo) {
		if (pInfo->read_input && pInfo->fd >= 0 &&
		    (FD_ISSET(pInfo->fd, ((fd_set *)pReadmask)) != 0)) {
		    int sigstate = xf86BlockSIGIO();
		    
		    pInfo->read_input(pInfo);
		    xf86UnblockSIGIO(sigstate);		    
		    /*
		     * Must break here because more than one device may share
		     * the same file descriptor.
		     */
		    break;
		}
		pInfo = pInfo->next;
	    }
	}
#else /* __OSF__ */
	/*
	 * Until the two devices are made nonblock on read, we have to do this.
	 */
	MASKANDSETBITS(devicesWithInput, pReadmask, EnabledDevices);

	CLEARBITS(kbdDevices);
	BITSET(kbdDevices, xf86Info.consoleFd);
	MASKANDSETBITS(kbdDevices, kbdDevices, devicesWithInput);

	CLEARBITS(mseDevices);
	BITSET(mseDevices, xf86Info.mouseDev->mseFd);
	MASKANDSETBITS(mseDevices, mseDevices, devicesWithInput);

	if (ANYSET(kbdDevices) || xf86Info.kbdRate)
            (xf86Info.kbdEvents)(ANYSET(kbdDevices));
	if (ANYSET(mseDevices))
        (xf86Info.mouseDev->mseEvents)(1);
#endif	/* __OSF__ */
    }
#else   /* __EMX__ and __QNX__ */

    InputInfoPtr pInfo;

    (xf86Info.kbdEvents)();  /* Under OS/2 and QNX, always call */

    pInfo = xf86InputDevs;
    while (pInfo) {
		if (pInfo->read_input && pInfo->fd >= 0) {
		    int sigstate = xf86BlockSIGIO();
		    
		    pInfo->read_input(pInfo);
		    xf86UnblockSIGIO(sigstate);		    
		    /*
		     * Must break here because more than one device may share
		     * the same file descriptor.
		     */
		    break;
		}
		pInfo = pInfo->next;
    }

#endif  /* __EMX__ and __QNX__ */

    if (err >= 0) { /* we don't want the handlers called if select() */
	IHPtr ih;   /* returned with an error condition, do we?      */
	
	for (ih = InputHandlers; ih; ih = ih->next) {
	    if (ih->enabled && ih->fd >= 0 && ih->ihproc &&
		(FD_ISSET(ih->fd, ((fd_set *)pReadmask)) != 0)) {
		ih->ihproc(ih->fd, ih->data);
	    }
	}
    }
    
    if (xf86VTSwitchPending()) xf86VTSwitch();

    if (xf86Info.inputPending) ProcessInputEvents();
}

#endif /* AMOEBA */

/*
 * xf86SigioReadInput --
 *    signal handler for the SIGIO signal.
 */
static void
xf86SigioReadInput(int fd,
		   void *closure)
{
    int sigstate = xf86BlockSIGIO();
    InputInfoPtr pInfo = (InputInfoPtr) closure;

    pInfo->read_input(pInfo);

    xf86UnblockSIGIO(sigstate);
}

/*
 * xf86AddEnabledDevice --
 *    
 */
void
xf86AddEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86InstallSIGIOHandler (pInfo->fd, xf86SigioReadInput, pInfo)) {
	AddEnabledDevice(pInfo->fd);
    }
}

/*
 * xf86RemoveEnabledDevice --
 *    
 */
void
xf86RemoveEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86RemoveSIGIOHandler (pInfo->fd)) {
	RemoveEnabledDevice(pInfo->fd);
    }
}

static int *xf86SignalIntercept = NULL;

void
xf86InterceptSignals(int *signo)
{
    if ((xf86SignalIntercept = signo))
	*signo = -1;
}

/*
 * xf86SigHandler --
 *    Catch unexpected signals and exit or continue cleanly.
 */
void
xf86SigHandler(int signo)
{
  if (xf86SignalIntercept && (*xf86SignalIntercept < 0)) {
    /* Re-arm handler just in case */
    (void) signal(signo, xf86SigHandler);
    *xf86SignalIntercept = signo;
    return;
  }

  signal(signo,SIG_IGN);
  xf86Info.caughtSignal = TRUE;
#ifdef XF86BIGFONT
  XF86BigfontCleanup();
#endif
#if defined(XFree86LOADER)
  if (xf86Initialising)
      LoaderCheckUnresolved(LD_RESOLV_IFDONE);
#endif
  FatalError("Caught signal %d.  Server aborting\n", signo);
}

/*
 * xf86VTSwitch --
 *      Handle requests for switching the vt.
 */
static void
xf86VTSwitch()
{
  int i;
  InputInfoPtr pInfo;
  IHPtr ih;

#ifdef DEBUG
  ErrorF("xf86VTSwitch()\n");
#endif

#ifdef XFreeXDGA
  if(!DGAVTSwitch())
	return;
#endif

  /*
   * Since all screens are currently all in the same state it is sufficient
   * check the first.  This might change in future.
   */
  if (xf86Screens[0]->vtSema) {

#ifdef DEBUG
    ErrorF("xf86VTSwitch: Leaving, xf86Exiting is %s\n",
	   BOOLTOSTRING((dispatchException & DE_TERMINATE) ? TRUE : FALSE));
#endif
    for (i = 0; i < xf86NumScreens; i++) {
      if (!(dispatchException & DE_TERMINATE))
	if (xf86Screens[i]->EnableDisableFBAccess)
	  (*xf86Screens[i]->EnableDisableFBAccess) (i, FALSE);
    }
    xf86EnterServerState(SETUP);
    for (i = 0; i < xf86NumScreens; i++) {
      xf86Screens[i]->LeaveVT(i, 0);
    }
#if !defined(__EMX__) && !defined(__CYGWIN__)
    DisableDevice((DeviceIntPtr)xf86Info.pKeyboard);
    pInfo = xf86InputDevs;
    while (pInfo) {
      DisableDevice(pInfo->dev);
      pInfo = pInfo->next;
    }
#endif /* !__EMX__ */
    for (ih = InputHandlers; ih; ih = ih->next)
      xf86DisableInputHandler(ih);
    xf86AccessLeave();      /* We need this here, otherwise */
    xf86AccessLeaveState(); /* console won't be restored    */

    if (!xf86VTSwitchAway()) {
      /*
       * switch failed 
       */

#ifdef DEBUG
      ErrorF("xf86VTSwitch: Leave failed\n");
#endif
      xf86AccessEnter();
      xf86EnterServerState(SETUP);
      for (i = 0; i < xf86NumScreens; i++) {
	if (!xf86Screens[i]->EnterVT(i, 0))
	  FatalError("EnterVT failed for screen %d\n", i);
      }
      xf86EnterServerState(OPERATING);
      if (!(dispatchException & DE_TERMINATE)) {
	for (i = 0; i < xf86NumScreens; i++) {
	  if (xf86Screens[i]->EnableDisableFBAccess)
	    (*xf86Screens[i]->EnableDisableFBAccess) (i, TRUE);
	}
      }
      SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);

#if !defined(__EMX__) && !defined(__CYGWIN__)
      EnableDevice((DeviceIntPtr)xf86Info.pKeyboard);
      pInfo = xf86InputDevs;
      while (pInfo) {
	EnableDevice(pInfo->dev);
	pInfo = pInfo->next;
      }
#endif /* !__EMX__ */
    for (ih = InputHandlers; ih; ih = ih->next)
      xf86EnableInputHandler(ih);

    } else {
	  if (xf86OSPMClose)
	      xf86OSPMClose();
	  xf86OSPMClose = NULL;

	for (i = 0; i < xf86NumScreens; i++) {
 	    /*
 	     * zero all access functions to
 	     * trap calls when switched away.
 	     */
	    xf86Screens[i]->vtSema = FALSE;
	    xf86Screens[i]->access = NULL;
	    xf86Screens[i]->busAccess = NULL;
	}
      xf86DisableIO();
    }
  } else {
#ifdef DEBUG
    ErrorF("xf86VTSwitch: Entering\n");
#endif
    if (!xf86VTSwitchTo()) return;
    xf86OSPMClose = xf86OSPMOpen();

    xf86EnableIO();
    xf86AccessEnter();
    xf86EnterServerState(SETUP);
    for (i = 0; i < xf86NumScreens; i++) {
      xf86Screens[i]->vtSema = TRUE;
      if (!xf86Screens[i]->EnterVT(i, 0))
	  FatalError("EnterVT failed for screen %d\n", i);
    }
    xf86EnterServerState(OPERATING);
    for (i = 0; i < xf86NumScreens; i++) {
      if (xf86Screens[i]->EnableDisableFBAccess)
	(*xf86Screens[i]->EnableDisableFBAccess)(i, TRUE);
    }

    /* Turn screen saver off when switching back */
    SaveScreens(SCREEN_SAVER_FORCER,ScreenSaverReset);

#if !defined(__EMX__) && !defined(__CYGWIN__)
    EnableDevice((DeviceIntPtr)xf86Info.pKeyboard);
    pInfo = xf86InputDevs;
    while (pInfo) {
      EnableDevice(pInfo->dev);
      pInfo = pInfo->next;
    }
#endif /* !__EMX__ */
    for (ih = InputHandlers; ih; ih = ih->next)
      xf86EnableInputHandler(ih);
  }
}


/* Input handler registration */

pointer
xf86AddInputHandler(int fd, InputHandlerProc proc, pointer data)
{
    IHPtr ih;

    if (fd < 0 || !proc)
	return NULL;

    ih = xcalloc(sizeof(*ih), 1);
    if (!ih)
	return NULL;

    ih->fd = fd;
    ih->ihproc = proc;
    ih->data = data;
    ih->enabled = TRUE;

    ih->next = InputHandlers;
    InputHandlers = ih;

    AddEnabledDevice(fd);

    return ih;
}

int
xf86RemoveInputHandler(pointer handler)
{
    IHPtr ih, p;
    int fd;
    
    if (!handler)
	return -1;

    ih = handler;
    fd = ih->fd;
    
    if (ih->fd >= 0)
	RemoveEnabledDevice(ih->fd);

    if (ih == InputHandlers)
	InputHandlers = ih->next;
    else {
	p = InputHandlers;
	while (p && p->next != ih)
	    p = p->next;
	if (ih)
	    p->next = ih->next;
    }
    xfree(ih);
    return fd;
}

void
xf86DisableInputHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = FALSE;
    if (ih->fd >= 0)
	RemoveEnabledDevice(ih->fd);
}

void
xf86EnableInputHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = TRUE;
    if (ih->fd >= 0)
	AddEnabledDevice(ih->fd);
}

/*
 * As used currently by the DRI, the return value is ignored.
 */
Bool
xf86EnableVTSwitch(Bool new)
{
    static Bool def = TRUE;
    Bool old;

    old = VTSwitchEnabled;
    if (!new) {
	/* Disable VT switching */
	def = VTSwitchEnabled;
	VTSwitchEnabled = FALSE;
    } else {
	/* Restore VT switching to default */
	VTSwitchEnabled = def;
    }
    return old;
}

#ifdef XTESTEXT1

void
XTestGetPointerPos(short *fmousex, short *fmousey)
{
  int x,y;

  miPointerPosition(&x, &y);
  *fmousex = x;
  *fmousey = y;
}



void
XTestJumpPointer(int jx, int jy, int dev_type)
{
  miPointerAbsoluteCursor(jx, jy, GetTimeInMillis() );
}

void
XTestGenerateEvent(int dev_type, int keycode, int keystate, int mousex,
		   int mousey)
{
  xEvent tevent;
  
  tevent.u.u.type = (dev_type == XE_POINTER) ?
    (keystate == XTestKEY_UP) ? ButtonRelease : ButtonPress :
      (keystate == XTestKEY_UP) ? KeyRelease : KeyPress;
  tevent.u.u.detail = keycode;
  tevent.u.keyButtonPointer.rootX = mousex;
  tevent.u.keyButtonPointer.rootY = mousey;
  tevent.u.keyButtonPointer.time = xf86Info.lastEventTime = GetTimeInMillis();
#ifdef XINPUT
  xf86eqEnqueue(&tevent);
#else
  mieqEnqueue(&tevent);
#endif
  xf86Info.inputPending = TRUE;               /* virtual event */
}

#endif /* XTESTEXT1 */

#ifdef WSCONS_SUPPORT

/* XXX Currently XKB is mandatory. */

void
xf86PostWSKbdEvent(struct wscons_event *event)
{
  int         type = event->type;
  int         value = event->value;
  Bool        down = (type == WSCONS_EVENT_KEY_DOWN ? TRUE : FALSE);
  KeyClassRec *keyc = ((DeviceIntPtr)xf86Info.pKeyboard)->key;
  xEvent      kevent;
  KeySym      *keysym;
  int         keycode;

  /*
   * Now map the scancodes to real X-keycodes ...
   */
  keycode = value + MIN_KEYCODE;
  keysym = keyc->curKeySyms.map +
	keyc->curKeySyms.mapWidth * (keycode - keyc->curKeySyms.minKeyCode);
	    
  /*
   * check for an autorepeat-event
   */
  if ((down && KeyPressed(keycode)) &&
      (xf86Info.autoRepeat != AutoRepeatModeOn || keyc->modifierMap[keycode]))
    return;

  xf86Info.lastEventTime = kevent.u.keyButtonPointer.time
	= event->time.tv_sec * 1000 + event->time.tv_nsec / 1000000;

  ENQUEUE(&kevent, keycode, (down ? KeyPress : KeyRelease), XE_KEYBOARD);
}
#endif /* WSCONS_SUPPORT */
