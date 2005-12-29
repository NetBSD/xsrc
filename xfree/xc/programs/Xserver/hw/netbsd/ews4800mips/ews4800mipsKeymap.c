/* $XConsortium: sunKeyMap.c,v 4.22 94/05/18 11:16:07 kaleb Exp $ */
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

#include "ews4800mips.h"
#include "keysym.h"

#define	GLYPHS_PER_KEY  2

static KeySym map_jpkbd[] = {
	/* 0x0 */ XK_0,			NoSymbol,
	/* 0x1 */ XK_1,			XK_exclam,
	/* 0x2 */ XK_2,			XK_quotedbl,
	/* 0x3 */ XK_3,			XK_numbersign,
	/* 0x4 */ XK_4,			XK_dollar,
	/* 0x5 */ XK_5,			XK_percent,
	/* 0x6 */ XK_6,			XK_ampersand,
	/* 0x7 */ XK_7,			XK_apostrophe,
	/* 0x8 */ XK_8,			XK_parenleft,
	/* 0x9 */ XK_9,			XK_parenright,
	/* 0xa */ XK_minus,		XK_equal,
	/* 0xb */ XK_asciicircum,	XK_grave,
	/* 0xc */ XK_backslash,		XK_bar,
	/* 0xd */ XK_colon,		XK_asterisk,
	/* 0xe */ XK_period,		XK_greater,
	/* 0xf */ XK_slash,		XK_question,
	/* 0x10 */ XK_at,		XK_asciitilde,
	/* 0x11 */ XK_a,		NoSymbol,
	/* 0x12 */ XK_b,		NoSymbol,
	/* 0x13 */ XK_c,		NoSymbol,
	/* 0x14 */ XK_d,		NoSymbol,
	/* 0x15 */ XK_e,		NoSymbol,
	/* 0x16 */ XK_f,		NoSymbol,
	/* 0x17 */ XK_g,		NoSymbol,
	/* 0x18 */ XK_h,		NoSymbol,
	/* 0x19 */ XK_i,		NoSymbol,
	/* 0x1a */ XK_j,		NoSymbol,
	/* 0x1b */ XK_k,		NoSymbol,
	/* 0x1c */ XK_l,		NoSymbol,
	/* 0x1d */ XK_m,		NoSymbol,
	/* 0x1e */ XK_n,		NoSymbol,
	/* 0x1f */ XK_o,		NoSymbol,
	/* 0x20 */ XK_p,		NoSymbol,
	/* 0x21 */ XK_q,		NoSymbol,
	/* 0x22 */ XK_r,		NoSymbol,
	/* 0x23 */ XK_s,		NoSymbol,
	/* 0x24 */ XK_t,		NoSymbol,
	/* 0x25 */ XK_u,		NoSymbol,
	/* 0x26 */ XK_v,		NoSymbol,
	/* 0x27 */ XK_w,		NoSymbol,
	/* 0x28 */ XK_x,		NoSymbol,
	/* 0x29 */ XK_y,		NoSymbol,
	/* 0x2a */ XK_z,		NoSymbol,
	/* 0x2b */ XK_bracketleft,	XK_braceleft,
	/* 0x2c */ XK_comma,		XK_less,
	/* 0x2d */ XK_bracketright,	XK_braceright,
	/* 0x2e */ XK_semicolon,	XK_plus,
	/* 0x2f */ XK_underscore,	XK_underscore,
	/* 0x30 */ XK_KP_0,		NoSymbol,
	/* 0x31 */ XK_KP_1,		NoSymbol,
	/* 0x32 */ XK_KP_2,		NoSymbol,
	/* 0x33 */ XK_KP_3,		NoSymbol,
	/* 0x34 */ XK_KP_4,		NoSymbol,
	/* 0x35 */ XK_KP_5,		NoSymbol,
	/* 0x36 */ XK_KP_6,		NoSymbol,
	/* 0x37 */ XK_KP_7,		NoSymbol,
	/* 0x38 */ XK_KP_8,		NoSymbol,
	/* 0x39 */ XK_KP_9,		NoSymbol,
	/* 0x3a */ XK_space,		NoSymbol,
	/* 0x3b */ XK_comma,		NoSymbol,
	/* 0x3c */ NoSymbol,		NoSymbol,
	/* 0x3d */ NoSymbol,		NoSymbol,
	/* 0x3e */ XK_Hiragana_Katakana,NoSymbol, /* KANA */
	/* 0x3f */ NoSymbol,		NoSymbol,
	/* 0x40 */ XK_Return,		NoSymbol,
	/* 0x41 */ XK_Return,		NoSymbol,
	/* 0x42 */ XK_Prior,		NoSymbol,
	/* 0x43 */ XK_Next,		NoSymbol,
	/* 0x44 */ XK_KP_Enter,		NoSymbol,
	/* 0x46 */ XK_minus,		NoSymbol,
	/* 0x47 */ XK_period,		NoSymbol,
	/* 0x48 */ XK_Left,		NoSymbol,
	/* 0x48 */ XK_Return,		NoSymbol,
	/* 0x49 */ XK_Right,		NoSymbol,
	/* 0x4a */ NoSymbol,		NoSymbol,
	/* 0x4b */ NoSymbol,		NoSymbol,
	/* 0x4c */ NoSymbol,		NoSymbol,
	/* 0x4d */ NoSymbol,		NoSymbol,
	/* 0x4e */ NoSymbol,		NoSymbol,
	/* 0x4f */ NoSymbol,		NoSymbol,
	/* 0x50 */ XK_BackSpace,	NoSymbol,
	/* 0x51 */ NoSymbol,		NoSymbol,
	/* 0x52 */ XK_Clear,		XK_Home,
	/* 0x53 */ XK_Delete,		NoSymbol,
	/* 0x54 */ XK_Insert,		NoSymbol,
	/* 0x55 */ NoSymbol,		NoSymbol,
	/* 0x56 */ XK_F1,		NoSymbol,
	/* 0x57 */ XK_F2,		NoSymbol,
	/* 0x58 */ XK_F3,		NoSymbol,
	/* 0x59 */ XK_F4,		NoSymbol,
	/* 0x5a */ XK_F5,		NoSymbol,
	/* 0x5b */ XK_F6,		NoSymbol,
	/* 0x5c */ XK_F7,		NoSymbol,
	/* 0x5d */ XK_F8,		NoSymbol,
	/* 0x5e */ XK_F9,		NoSymbol,
	/* 0x5f */ XK_F10,		NoSymbol,
	/* 0x60 */ XK_F11,		NoSymbol, /* No key top print */
	/* 0x61 */ XK_F12,		NoSymbol, /* No key top print */
	/* 0x62 */ XK_F13,		NoSymbol, /* No key top print */
	/* 0x63 */ XK_F14,		NoSymbol, /* No key top print */
	/* 0x64 */ XK_F15,		NoSymbol, /* No key top print */
	/* 0x65 */ NoSymbol,		NoSymbol,
	/* 0x66 */ NoSymbol,		NoSymbol,
	/* 0x67 */ XK_Muhenkan,		NoSymbol, /* NFER */
	/* 0x68 */ XK_Henkan,		NoSymbol, /* XFER */
	/* 0x69 */ NoSymbol,		NoSymbol,
	/* 0x6a */ XK_Up,		NoSymbol,
	/* 0x6b */ XK_Down,		NoSymbol,
	/* 0x6c */ NoSymbol,		NoSymbol,
	/* 0x6d */ XK_Meta_L,		NoSymbol, /* GRAPH */
	/* 0x6e */ NoSymbol,		NoSymbol,
	/* 0x6f */ NoSymbol,		NoSymbol,
	/* 0x70 */ XK_KP_Add,		NoSymbol,
	/* 0x71 */ XK_KP_Multiply,	NoSymbol,
	/* 0x72 */ XK_KP_Divide,	NoSymbol,
	/* 0x73 */ XK_Escape,		NoSymbol,
	/* 0x74 */ XK_Help,		NoSymbol,
	/* 0x75 */ XK_KP_Equal,		NoSymbol,
	/* 0x76 */ XK_Tab,		NoSymbol,
	/* 0x78 */ XK_Control_L,	NoSymbol,
	/* 0x79 */ XK_Caps_Lock,	NoSymbol,
	/* 0x7a */ NoSymbol,		NoSymbol,
	/* 0x7b */ XK_Shift_L,		NoSymbol,
	/* 0x7c */ XK_Shift_R,		NoSymbol,
	/* 0x7e */ XK_Alt_L,		NoSymbol,
	/* 0x7f */ NoSymbol,		NoSymbol, /* FNC */
};

static ews4800mipsModmapRec modmap_jpkbd[] = {
	0x7b,	ShiftMask,	/* Shift_L */
	0x7c,	ShiftMask,	/* Shift_R */
	0x79,	LockMask,	/* CapsLock */
	0x78,	ControlMask,	/* Control_L */
	0x7e,	Mod1Mask,	/* Alt_L */
	0x3e,	Mod4Mask,	/* Hiragana_Katakana */
	0,	0
};

KeySymsRec ews4800mipsKeySyms[] = {
	/* map		minKC	maxKC	width */
	map_jpkbd,	0x00,	0x7f,	GLYPHS_PER_KEY,
};

ews4800mipsModmapRec *ews4800mipsModMaps[] = {
	modmap_jpkbd,
};
