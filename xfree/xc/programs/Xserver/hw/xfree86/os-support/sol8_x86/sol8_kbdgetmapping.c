/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_kbdgetmapping.c,v 1.1 1999/09/25 14:38:09 dawes Exp $ */
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
/* $XConsortium: sol8Kbd.c /main/10 1996/02/21 17:38:32 kaleb $ */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "sol8_keynames.h"
#include "xf86Config.h"

#include "sol8_keymap.h"


/*
 * LegalModifier --
 *      determine whether a key is a legal modifier key, i.e send a
 *      press/release sequence.
 */

/*ARGSUSED*/
Bool
LegalModifier(key, pDev)
     unsigned int  key;
     DevicePtr	pDev;
{
  return (TRUE);
}


      
/*
 * xf86KbdGetMapping --
 *	Get the national keyboard mapping. The keyboard type is set, a new map
 *      and the modifiermap is computed.
 */

void
xf86KbdGetMapping (pKeySyms, pModMap)
     KeySymsPtr pKeySyms;
     CARD8      *pModMap;
{
  KeySym        *k;
  char          type;
  int           i;
  KeySym        *pMap;

  xf86Info.kbdType = 0;
  pMap = sol8defaultKeymap;

  /*
   * compute the modifier map
   */
  for (i = 0; i < MAP_LENGTH; i++)
    pModMap[i] = NoSymbol;  /* make sure it is restored */
  
  for (k = pMap, i = MIN_KEYCODE;
       i < (NUM_KEYCODES + MIN_KEYCODE);
       i++, k += 4)
    
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
  
  xf86Info.kbdType = 0;

  pKeySyms->map        = pMap;
  pKeySyms->mapWidth   = GLYPHS_PER_KEY;
  pKeySyms->minKeyCode = MIN_KEYCODE;
  pKeySyms->maxKeyCode = MAX_KEYCODE;
}
