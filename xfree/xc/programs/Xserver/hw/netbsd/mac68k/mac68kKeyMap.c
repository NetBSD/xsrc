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

#include	"mac68k.h"
#include	"keysym.h"

/* twm and Motif have hard-coded dependencies on Meta being Mod1 :-( */
#if 0
/* This set has optimal characteristics for use in the Toolkit... */
#define Meta_Mask Mod1Mask
#define Mode_switch_Mask Mod2Mask
#define Num_Lock_Mask Mod3Mask
#define Alt_Mask Mod4Mask
#else
/* but this set is compatible with what we shipped in R6. */
#define Meta_Mask Mod1Mask
#define Mode_switch_Mask Mod2Mask
#define Alt_Mask Mod3Mask
#define Num_Lock_Mask Mod4Mask
#endif

static KeySym map[] = {
	XK_A,		NoSymbol,		/* 0x0 */
	XK_S,		NoSymbol,		/* 0x1 */
	XK_D,		NoSymbol,		/* 0x2 */
	XK_F,		NoSymbol,		/* 0x3 */
	XK_H,		NoSymbol,		/* 0x4 */
	XK_G,		NoSymbol,		/* 0x5 */
	XK_Z,		NoSymbol,		/* 0x6 */
	XK_X,		NoSymbol,		/* 0x7 */
	XK_C,		NoSymbol,		/* 0x8 */
	XK_V,		NoSymbol,		/* 0x9 */
	NoSymbol,	NoSymbol,		/* 0xa */
	XK_B,		NoSymbol,		/* 0xb */
	XK_Q,		NoSymbol,		/* 0xc */
	XK_W,		NoSymbol,		/* 0xd */
	XK_E,		NoSymbol,		/* 0xe */
	XK_R,		NoSymbol,		/* 0xf */
	XK_Y,		NoSymbol,		/* 0x10 */
	XK_T,		NoSymbol,		/* 0x11 */
	XK_1,		XK_exclam,		/* 0x12 */
	XK_2,		XK_at,			/* 0x13 */
	XK_3,		XK_numbersign,		/* 0x14 */
	XK_4,		XK_dollar,		/* 0x15 */
	XK_6,		XK_asciicircum,		/* 0x16 */
	XK_5,		XK_percent,		/* 0x17 */
	XK_equal,	XK_plus,		/* 0x18 */
	XK_9,		XK_parenleft,		/* 0x19 */
	XK_7,		XK_ampersand,		/* 0x1a */
	XK_minus,	XK_underscore,		/* 0x1b */
	XK_8,		XK_asterisk,		/* 0x1c */
	XK_0,		XK_parenright,		/* 0x1d */
	XK_bracketright,XK_braceright,		/* 0x1e */
	XK_O,		NoSymbol,		/* 0x1f */
	XK_U,		NoSymbol,		/* 0x20 */
	XK_bracketleft,	XK_braceleft,		/* 0x21 */
	XK_I,		NoSymbol,		/* 0x22 */
	XK_P,		NoSymbol,		/* 0x23 */
	XK_Return,	NoSymbol,		/* 0x24 */
	XK_L,		NoSymbol,		/* 0x25 */
	XK_J,		NoSymbol,		/* 0x26 */
	XK_quoteright,	XK_quotedbl,		/* 0x27 */
	XK_K,		NoSymbol,		/* 0x28 */
	XK_semicolon,	XK_colon,		/* 0x29 */
	XK_backslash,	XK_bar,			/* 0x2a */
	XK_comma,	XK_less,		/* 0x2b */
	XK_slash,	XK_question,		/* 0x2c */
	XK_N,		NoSymbol,		/* 0x2d */
	XK_M,		NoSymbol,		/* 0x2e */
	XK_period,	XK_greater,		/* 0x2f */
	XK_Tab,		NoSymbol,		/* 0x30 */
	XK_space,	NoSymbol,		/* 0x31 */
	XK_quoteleft,	XK_asciitilde,		/* 0x32 */
	XK_BackSpace,	NoSymbol,		/* 0x33 */
	NoSymbol,	NoSymbol,		/* 0x34 */
	XK_Escape,	NoSymbol,		/* 0x35 */
	XK_Control_L,	NoSymbol,		/* 0x36 */
	XK_Meta_L,	NoSymbol,		/* 0x37 */ /* apple/clover */
	XK_Shift_L,	NoSymbol,		/* 0x38 */
	XK_Caps_Lock,	NoSymbol,		/* 0x39 */
	XK_Alt_L,	NoSymbol,		/* 0x3a */ /* option/alt */
	XK_Left,	NoSymbol,		/* 0x3b */
	XK_Right,	NoSymbol,		/* 0x3c */
	XK_Down,	NoSymbol,		/* 0x3d */
	XK_Up,		NoSymbol,		/* 0x3e */
	NoSymbol,	NoSymbol,		/* 0x3f */
	NoSymbol,	NoSymbol,		/* 0x40 */
	XK_KP_Decimal,	NoSymbol,		/* 0x41 */
	NoSymbol,	NoSymbol,		/* 0x42 */
	XK_KP_Multiply,	NoSymbol,		/* 0x43 */
	NoSymbol,	NoSymbol,		/* 0x44 */
	XK_KP_Add,	NoSymbol,		/* 0x45 */
	NoSymbol,	NoSymbol,		/* 0x46 */
	XK_Clear,	NoSymbol,		/* 0x47 */
	NoSymbol,	NoSymbol,		/* 0x48 */
	NoSymbol,	NoSymbol,		/* 0x49 */
	NoSymbol,	NoSymbol,		/* 0x4a */
	XK_KP_Divide,	NoSymbol,		/* 0x4b */
	XK_KP_Enter,	NoSymbol,		/* 0x4c */
	NoSymbol,	NoSymbol,		/* 0x4d */
	XK_KP_Subtract,	NoSymbol,		/* 0x4e */
	NoSymbol,	NoSymbol,		/* 0x4f */
	NoSymbol,	NoSymbol,		/* 0x50 */
	XK_KP_Equal,	NoSymbol,		/* 0x51 */
	XK_KP_0,	NoSymbol,		/* 0x52 */
	XK_KP_1,	NoSymbol,		/* 0x53 */
	XK_KP_2,	NoSymbol,		/* 0x54 */
	XK_KP_3,	NoSymbol,		/* 0x55 */
	XK_KP_4,	NoSymbol,		/* 0x56 */
	XK_KP_5,	NoSymbol,		/* 0x57 */
	XK_KP_6,	NoSymbol,		/* 0x58 */
	XK_KP_7,	NoSymbol,		/* 0x59 */
	NoSymbol,	NoSymbol,		/* 0x5a */
	XK_KP_8,	NoSymbol,		/* 0x5b */
	XK_KP_9,	NoSymbol,		/* 0x5c */
	NoSymbol,	NoSymbol,		/* 0x5d */
	NoSymbol,	NoSymbol,		/* 0x5e */
	NoSymbol,	NoSymbol,		/* 0x5f */
	XK_F5,		NoSymbol,		/* 0x60 */
	XK_F6,		NoSymbol,		/* 0x61 */
	XK_F7,		NoSymbol,		/* 0x62 */
	XK_F3,		NoSymbol,		/* 0x63 */
	XK_F8,		NoSymbol,		/* 0x64 */
	XK_F9,		NoSymbol,		/* 0x65 */
	NoSymbol,	NoSymbol,		/* 0x66 */
	XK_F11,		NoSymbol,		/* 0x67 */
	NoSymbol,	NoSymbol,		/* 0x68 */
	XK_F13,		XK_Print,		/* 0x69 */
	NoSymbol,	NoSymbol,		/* 0x6a */
	XK_F14,		XK_Pause,		/* 0x6b */
	NoSymbol,	NoSymbol,		/* 0x6c */
	XK_F10,		NoSymbol,		/* 0x6d */
	NoSymbol,	NoSymbol,		/* 0x6e */
	XK_F12,		NoSymbol,		/* 0x6f */
	NoSymbol,	NoSymbol,		/* 0x70 */ 
	XK_F15,		XK_Pause,		/* 0x71 */
	XK_Help,	XK_Insert,		/* 0x72 */
	XK_Home,	NoSymbol,		/* 0x73 */
	XK_Prior,	NoSymbol,		/* 0x74 */ /* Page Up */
	XK_Delete,	NoSymbol,		/* 0x75 */
	XK_F4,		NoSymbol,		/* 0x76 */
	XK_End,		NoSymbol,		/* 0x77 */
	XK_F2,		NoSymbol,		/* 0x78 */
	XK_Next,	NoSymbol,		/* 0x79 */ /* Page Down */
	XK_F1,		NoSymbol,		/* 0x7a */
	XK_Shift_R,	NoSymbol,		/* 0x7b */
	XK_Alt_R,	NoSymbol,		/* 0x7c */
	XK_Control_R,	NoSymbol,		/* 0x7d */
	NoSymbol,	NoSymbol,		/* 0x7e */
	NoSymbol,	NoSymbol,		/* 0x7f */ /* Soft Power */
};

KeySymsRec macKeySyms[] = {
    /*	map	minKC	maxKC	width */
	map,	0,	0x7e,	2,
};

static MacModmapRec macModMap[] = {
	0x36,	ControlMask,
	0x38,	ShiftMask,
	0x39,	LockMask,
	0x3a,	Meta_Mask,
	0,	0
};

static MacModmapRec altModMap[] = {
	0x36,	ControlMask,
	0x37,	Meta_Mask,
	0x38,	ShiftMask,
	0x39,	LockMask,
	0x7b,	ShiftMask,
	0x7d,	ControlMask,
	0,	0
};

MacModmapRec *macModMaps[] = {
	macModMap,
	altModMap,
};
