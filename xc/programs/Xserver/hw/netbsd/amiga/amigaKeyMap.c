/* $XConsortium: amigaKeyMap.c,v 4.19 94/04/17 20:29:43 kaleb Exp $ */
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include	"amiga.h"
#include	"keysym.h"


static KeySym AmigaUSA1Keymap[] = {
  	XK_quoteleft,	XK_asciitilde,	/* 0x00 */
	XK_1,		XK_exclam,	/* 0x01 */
	XK_2,		XK_at,		/* 0x02 */
	XK_3,		XK_numbersign,	/* 0x03 */
	XK_4,		XK_dollar,	/* 0x04 */
	XK_5,		XK_percent,	/* 0x05 */
	XK_6,		XK_asciicircum,	/* 0x06 */
	XK_7,		XK_ampersand,	/* 0x07 */
	XK_8,		XK_asterisk,	/* 0x08 */
	XK_9,		XK_parenleft,	/* 0x09 */
	XK_0,		XK_parenright,	/* 0x0a */
	XK_minus,	XK_underscore,	/* 0x0b */
	XK_equal,	XK_plus,	/* 0x0c */
	XK_backslash,	XK_bar,		/* 0x0d */
	NoSymbol,	NoSymbol,	/* 0x0e */
	XK_KP_0,	XK_Insert,	/* 0x0f */

	XK_q,		XK_Q,		/* 0x10 */
	XK_w,		XK_W,		/* 0x11 */
	XK_e,		XK_E,		/* 0x12 */
	XK_r,		XK_R,		/* 0x13 */
	XK_t,		XK_T,		/* 0x14 */
	XK_y,		XK_Y,		/* 0x15 */
	XK_u,		XK_U,		/* 0x16 */
	XK_i,		XK_I,		/* 0x17 */
	XK_o,		XK_O,		/* 0x18 */
	XK_p,		XK_P,		/* 0x19 */
	XK_bracketleft,	XK_braceleft,	/* 0x1a */
	XK_bracketright,XK_braceright,	/* 0x1b */
	NoSymbol,	NoSymbol,	/* 0x1c */
	XK_KP_1,	XK_KP_End,		/* 0x1d */
	XK_KP_2,	XK_KP_Down,	/* 0x1e */
	XK_KP_3,	XK_KP_Next,	/* 0x1f */

	XK_a,		XK_A,		/* 0x20 */
	XK_s,		XK_S,		/* 0x21 */
	XK_d,		XK_D,		/* 0x22 */
	XK_f,		XK_F,		/* 0x23 */
	XK_g,		XK_G,		/* 0x24 */
	XK_h,		XK_H,		/* 0x25 */
	XK_j,		XK_J,		/* 0x26 */
	XK_k,		XK_K,		/* 0x27 */
	XK_l,		XK_L,		/* 0x28 */
	XK_semicolon,	XK_colon,	/* 0x29 */
	XK_quoteright,	XK_quotedbl,	/* 0x2a */
	NoSymbol,	NoSymbol,	/* 0x2b */
	NoSymbol,	NoSymbol,	/* 0x2c */
	XK_KP_4,	XK_KP_Left,	/* 0x2d */
	XK_KP_5,	NoSymbol,	/* 0x2e */
	XK_KP_6,	XK_KP_Right,	/* 0x2f */

	NoSymbol,	NoSymbol,	/* 0x30 */
	XK_z,		XK_Z,		/* 0x31 */
	XK_x,		XK_X,		/* 0x32 */
	XK_c,		XK_C,		/* 0x33 */
	XK_v,		XK_V,		/* 0x34 */
	XK_b,		XK_B,		/* 0x35 */
	XK_n,		XK_N,		/* 0x36 */
	XK_m,		XK_M,		/* 0x37 */
	XK_comma,	XK_less,	/* 0x38 */
	XK_period,	XK_greater,	/* 0x39 */
	XK_slash,	XK_question,	/* 0x3a */
	NoSymbol,	NoSymbol,	/* 0x3b */
	XK_KP_Decimal,	XK_KP_Delete,	/* 0x3c */
	XK_KP_7,	XK_KP_Home,	/* 0x3d */
	XK_KP_8,	XK_KP_Up,	/* 0x3e */
	XK_KP_9,	XK_KP_Prior,	/* 0x3f */

	XK_space,	NoSymbol,	/* 0x40 */
	XK_BackSpace,	NoSymbol,	/* 0x41 */
	XK_Tab,		XK_ISO_Left_Tab,	/* 0x42 */
	XK_KP_Enter,	NoSymbol,	/* 0x43 */
	XK_Return,	NoSymbol,	/* 0x44 */
	XK_Escape,	NoSymbol,	/* 0x45 */
	XK_Delete,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,	/* 0x47 */
	NoSymbol,	NoSymbol,	/* 0x48 */
	NoSymbol,	NoSymbol,	/* 0x49 */
	XK_KP_Subtract,	NoSymbol,	/* 0x4a */
	NoSymbol,	NoSymbol,	/* 0x4b */
	XK_Up,		NoSymbol,	/* 0x4c */
	XK_Down,	NoSymbol,	/* 0x4d */
	XK_Right,	NoSymbol,	/* 0x4e */
	XK_Left,	NoSymbol,	/* 0x4f */

	XK_F1,		NoSymbol,	/* 0x50 */
	XK_F2,		NoSymbol,	/* 0x51 */
	XK_F3,		NoSymbol,	/* 0x52 */
	XK_F4,		NoSymbol,	/* 0x53 */
	XK_F5,		NoSymbol,	/* 0x54 */
	XK_F6,		NoSymbol,	/* 0x55 */
	XK_F7,		NoSymbol,	/* 0x56 */
	XK_F8,		NoSymbol,	/* 0x57 */
	XK_F9,		NoSymbol,	/* 0x58 */
	XK_F10,		NoSymbol,	/* 0x59 */
	XK_parenleft,	XK_Num_Lock,	/* 0x5a */
	XK_parenright,	XK_Scroll_Lock,	/* 0x5b */
	XK_KP_Divide,	NoSymbol,	/* 0x5c */
	XK_KP_Multiply,	XK_Print,	/* 0x5d */
	XK_KP_Add,	NoSymbol,	/* 0x5e */
	XK_Help,	NoSymbol,	/* 0x5f */

	XK_Shift_L,	NoSymbol,	/* 0x60 */
	XK_Shift_R,	NoSymbol,	/* 0x61 */
	XK_Caps_Lock,	NoSymbol,	/* 0x62 */
	XK_Control_L,	NoSymbol,	/* 0x63 */
	XK_Alt_L,	NoSymbol,	/* 0x64 */
	XK_Alt_R,	NoSymbol,	/* 0x65 */
	XK_Meta_L,	NoSymbol,	/* 0x66 */
	XK_Meta_R,	NoSymbol,	/* 0x67 */
};

/* twm and Motif have hard-coded dependencies on Meta being Mod1 :-( */
#if 0
/* This set has optimal characteristics for use in the Toolkit... */
#define Meta_Mask Mod1Mask
#define Mode_switch_Mask Mod2Mask
#define Num_Lock_Mask Mod3Mask
#define Alt_Mask Mod4Mask
#else
/* but this set is compatible with what we shipped in R6. */
#define Alt_Mask Mod1Mask
#define Mode_switch_Mask Mod2Mask
#define Meta_Mask Mod3Mask
#endif

static AmigaModmapRec AmigaModmap[] = {
	0x60,	ShiftMask,
	0x61,	ShiftMask,
	0x62,	LockMask,
	0x63,	ControlMask,
	0x64,	Alt_Mask,
	0x65,	Alt_Mask,
	0x66,	Meta_Mask,
	0x67,	Meta_Mask,
	0,	0
};

KeySym *amigaKeyMaps[] = {
	AmigaUSA1Keymap,	/* 0 */
	NULL			/* 1 */
};

KeySymsRec amigaKeySyms[] = {
	/*  map		minKeyCode	maxKC	width */
	{ AmigaUSA1Keymap,		0,	0x67,	2},
	{ (KeySym *)NULL,	0,	0,	0}
};

int amigaMaxLayout = sizeof amigaKeyMaps / sizeof amigaKeyMaps[0];

AmigaModmapRec *amigaModMaps[] = {
	AmigaModmap,		/* 0 */
	NULL
};
