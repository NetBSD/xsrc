/* $TOG: InitInput.c /main/12 1998/02/10 13:23:52 kaleb $ */
/*

Copyright 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/hw/xwin/InitInput.c,v 1.1 2000/08/10 17:40:37 dawes Exp $ */



#include "X11/X.h"
#define NEED_EVENTS
#include "X11/Xproto.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "X11/Xos.h"
#include "mibstore.h"
#include "mipointer.h"
#include "winkeynames.h"
#include "winkeymap.h"
#include "keysym.h"




CARD32 lastEventTime = 0;

int TimeSinceLastInputEvent()
{
    if (lastEventTime == 0)
        lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}

void SetTimeSinceLastInputEvent()
{
  lastEventTime = GetTimeInMillis();
}

Bool
LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr	pDev;
{
    return TRUE;
}

void
ProcessInputEvents()
{
    mieqProcessInputEvents();
	miPointerUpdate();
}

#define WIN_MIN_KEY 8
#define WIN_MAX_KEY 255

void
GetWinMappings (pKeySyms, pModMap)
     KeySymsPtr pKeySyms;
     CARD8      *pModMap;
{
	KeySym        *k;
	char          type;
	int           i, j;
	KeySym        *pMap;

	pMap = map;

	for (i = 0; i < MAP_LENGTH; i++)
		pModMap[i] = NoSymbol;  /* make sure it is restored */

	for (k = pMap, i = MIN_KEYCODE; i < (NUM_KEYCODES + MIN_KEYCODE); i++, k += 4)
		switch(*k) {
			case XK_Shift_L:
			case XK_Shift_R:
			pModMap[i] = ShiftMask;
			break;

			case XK_Control_L:
			case XK_Control_R:
			pModMap[i] = ControlMask;
			break;

			case XK_Caps_Lock:
			pModMap[i] = LockMask;
			break;

			case XK_Alt_L:
			case XK_Alt_R:
			pModMap[i] = AltMask;
			break;

			case XK_Num_Lock:
			pModMap[i] = NumLockMask;
			break;

			case XK_Scroll_Lock:
			pModMap[i] = ScrollLockMask;
			break;

			/* kana support */
			case XK_Kana_Lock:
			case XK_Kana_Shift:
			pModMap[i] = KanaMask;
			break;

			/* alternate toggle for multinational support */
			case XK_Mode_switch:
			pModMap[i] = AltLangMask;
			break;

		}
	pKeySyms->map        = pMap;
	pKeySyms->mapWidth   = GLYPHS_PER_KEY;
	pKeySyms->minKeyCode = MIN_KEYCODE;
	pKeySyms->maxKeyCode = MAX_STD_KEYCODE;
}

static int
winKeybdProc(pDevice, onoff)
    DeviceIntPtr pDevice;
    int onoff;
{
    KeySymsRec		keySyms;
    CARD8 		modMap[MAP_LENGTH];
    int i;
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT: 
	GetWinMappings(&keySyms, modMap);
	InitKeyboardDeviceStruct(pDev, &keySyms, modMap,
			(BellProcPtr)NoopDDA, (KbdCtrlProcPtr)NoopDDA);
	DIInitKeyboard () ;
	break;
    case DEVICE_ON: 
	pDev->on = TRUE;
	break;
    case DEVICE_OFF: 
	pDev->on = FALSE;
	break;
    case DEVICE_CLOSE:
	DITermKeyboard () ;
	break;
    }
    return Success;
}

static int
winMouseProc(pDevice, onoff)
    DeviceIntPtr pDevice;
    int onoff;
{
    BYTE map[4];
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT:
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(pDev, map, 3, miPointerGetMotionEvents,
		(PtrCtrlProcPtr)NoopDDA, miPointerGetMotionBufferSize());
		DIInitMouse () ;
	    break;

    case DEVICE_ON:
	pDev->on = TRUE;
        break;

    case DEVICE_OFF:
	pDev->on = FALSE;
	break;

    case DEVICE_CLOSE:
	DITermMouse () ;
 	break;
    }
    return Success;
}

void
InitInput(argc, argv)
    int argc;
    char *argv[];
{
    DevicePtr p, k;
    p = AddInputDevice(winMouseProc, TRUE);
    k = AddInputDevice(winKeybdProc, TRUE);
    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void)mieqInit (k, p);
}

GenerateInputEvent (type, ix, iy, button)
{
	xEvent		x ;

	/* Event types defined in directx.c */
	switch (type)
	{
    case 0: /* Mouse motion */
	  miPointerDeltaCursor (ix,iy,lastEventTime = GetTimeInMillis ());
      break;

    case 1: /* Mouse button pressed */
      x.u.u.type = ButtonPress;
      x.u.u.detail = Button1 + button ;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case 2: /* Mouse button released */
      x.u.u.type = ButtonRelease;
      x.u.u.detail = button + 1 ;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;

	case 3: /* Keyboard keypress */
      x.u.u.type = KeyPress;
      x.u.u.detail = button + MIN_KEYCODE ;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
	case 4: /* Keyboard keyrelease */
      x.u.u.type = KeyRelease;
      x.u.u.detail = button + MIN_KEYCODE ;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
	  break ;
	}
}

#ifdef XTESTEXT1
void
XTestGenerateEvent(dev_type, keycode, keystate, mousex, mousey)
	int	dev_type;
	int	keycode;
	int	keystate;
	int	mousex;
	int	mousey;
{
}

void
XTestGetPointerPos(fmousex, fmousey)
	short *fmousex, *fmousey;
{
}

void
XTestJumpPointer(jx, jy, dev_type)
	int	jx;
	int	jy;
	int	dev_type;
{
}
#endif

