/*	$NetBSD: decKeyMap.c,v 1.1 2004/01/18 05:21:41 rtr Exp $	*/

/* XConsortium: sunKeyMap.c,v 4.22 94/05/18 11:16:07 kaleb Exp */
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

#include	"dec.h"
#include	"keysym.h"

#ifdef __alpha__
#include	"atKeynames.h"
#include	"xf86Keymap.h"
#endif
#include	"lk201Keymap.h"

KeySymsRec decKeySyms[] = {
    /*	map	   minKeyCode	maxKC	width */
    NULL,               0,      0,                      0,   /* None */
    lk201map,		0,	LK201_NUM_KEYCODES,	4,   /* lk201 */
    lk201map,		0,	LK201_NUM_KEYCODES,	4,   /* lk401 */
#ifdef __alpha__
    map,                0,      MAX_STD_KEYCODE,        4,   /* XT */
    map,		0,	MAX_STD_KEYCODE,	4,   /* AT */
    map,                0,      MAX_STD_KEYCODE,        4,   /* USB/XT */
#else
    NULL,               0,      0,                      0,   /* XT */
    NULL,               0,      0,                      0,   /* AT */
    NULL,               0,      0,                      0,   /* USB/XT */
#endif
};

#ifdef __alpha__
static DecModmapRec xf86modmap[] = {
	KEY_ShiftL,	ShiftMask,
	KEY_ShiftR,	ShiftMask,
	KEY_CapsLock,	LockMask,
	KEY_LCtrl,	ControlMask,
	KEY_RCtrl,	ControlMask,
	KEY_Alt,	Mod1Mask,
	KEY_AltLang,	Mod1Mask,
	0,		0
};
#endif

DecModmapRec *decModMaps[] = {
    NULL,
    lk201modmap,
    lk201modmap,
#ifdef __alpha__
    xf86modmap,
    xf86modmap,
    xf86modmap,
#else
    NULL,
    NULL,
    NULL,
#endif
};
