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

#include "newsmips.h"
#include "keysym.h"

#define	GLYPHS_PER_KEY  2

static KeySym map_jpkbd[] = {
	/* 0  */ NoSymbol,	NoSymbol,
	/* 1  */ XK_F1,		NoSymbol,
	/* 2  */ XK_F2,		NoSymbol,
	/* 3  */ XK_F3,		NoSymbol,
	/* 4  */ XK_F4,		NoSymbol,
	/* 5  */ XK_F5,		NoSymbol,
	/* 6  */ XK_F6,		NoSymbol,
	/* 7  */ XK_F7,		NoSymbol,
	/* 8  */ XK_F8,		NoSymbol,
	/* 9  */ XK_F9,		NoSymbol,
	/* 10 */ XK_F10,	NoSymbol,
	/* 11 */ XK_Escape,	NoSymbol,
	/* 12 */ XK_1,		XK_exclam,
	/* 13 */ XK_2,		XK_at,
	/* 14 */ XK_3,		XK_numbersign,
	/* 15 */ XK_4,		XK_dollar,
	/* 16 */ XK_5,		XK_percent,
	/* 17 */ XK_6,		XK_asciicircum,
	/* 18 */ XK_7,		XK_ampersand,
	/* 19 */ XK_8,		XK_asterisk,
	/* 20 */ XK_9,		XK_parenleft,
	/* 21 */ XK_0,		XK_parenright,
	/* 22 */ XK_minus,	XK_underscore,
	/* 23 */ XK_equal,	XK_plus,
	/* 24 */ XK_backslash,	XK_bar,
	/* 25 */ XK_BackSpace,	NoSymbol,
	/* 26 */ XK_Tab,	NoSymbol,
	/* 27 */ XK_q,		NoSymbol,
	/* 28 */ XK_w,		NoSymbol,
	/* 29 */ XK_e,		NoSymbol,
	/* 30 */ XK_r,		NoSymbol,
	/* 31 */ XK_t,		NoSymbol,
	/* 32 */ XK_y,		NoSymbol,
	/* 33 */ XK_u,		NoSymbol,
	/* 34 */ XK_i,		NoSymbol,
	/* 35 */ XK_o,		NoSymbol,
	/* 36 */ XK_p,		NoSymbol,
	/* 37 */ XK_bracketleft,XK_braceleft,
	/* 38 */ XK_bracketright, XK_braceright,
	/* 39 */ XK_Delete,	NoSymbol,
	/* 40 */ XK_Control_L,	NoSymbol,
	/* 41 */ XK_a,		NoSymbol,
	/* 42 */ XK_s,		NoSymbol,
	/* 43 */ XK_d,		NoSymbol,
	/* 44 */ XK_f,		NoSymbol,
	/* 45 */ XK_g,		NoSymbol,
	/* 46 */ XK_h,		NoSymbol,
	/* 47 */ XK_j,		NoSymbol,
	/* 48 */ XK_k,		NoSymbol,
	/* 49 */ XK_l,		NoSymbol,
	/* 50 */ XK_semicolon,	XK_colon,
	/* 51 */ XK_apostrophe,	XK_quotedbl,
	/* 52 */ XK_grave,	XK_asciitilde,
	/* 53 */ XK_Return,	NoSymbol,
	/* 54 */ XK_Shift_L,	NoSymbol,
	/* 55 */ XK_z,		NoSymbol,
	/* 56 */ XK_x,		NoSymbol,
	/* 57 */ XK_c,		NoSymbol,
	/* 58 */ XK_v,		NoSymbol,
	/* 59 */ XK_b,		NoSymbol,
	/* 60 */ XK_n,		NoSymbol,
	/* 61 */ XK_m,		NoSymbol,
	/* 62 */ XK_comma,	XK_less,
	/* 63 */ XK_period,	XK_greater,
	/* 64 */ XK_slash,	XK_question,
	/* 65 */ NoSymbol,	NoSymbol,
	/* 66 */ XK_Shift_R,	NoSymbol,
	/* 67 */ XK_Alt_L,	NoSymbol,
	/* 68 */ XK_Caps_Lock,	NoSymbol,
	/* 69 */ XK_Muhenkan,	NoSymbol,
	/* 70 */ XK_space,	NoSymbol,
	/* 71 */ XK_Henkan,	NoSymbol,
	/* 72 */ XK_Eisu_toggle,NoSymbol,
	/* 73 */ XK_Hiragana,	NoSymbol,
	/* 74 */ XK_Execute,	NoSymbol,
	/* 75 */ XK_KP_7,	NoSymbol,
	/* 76 */ XK_KP_8,	NoSymbol,
	/* 77 */ XK_KP_9,	NoSymbol,
	/* 78 */ XK_KP_Subtract,NoSymbol,
	/* 79 */ XK_KP_4,	NoSymbol,
	/* 80 */ XK_KP_5,	NoSymbol,
	/* 81 */ XK_KP_6,	NoSymbol,
	/* 82 */ XK_KP_Add,	NoSymbol,
	/* 83 */ XK_KP_1,	NoSymbol,
	/* 84 */ XK_KP_2,	NoSymbol,
	/* 85 */ XK_KP_3,	NoSymbol,
	/* 86 */ XK_KP_Separator,NoSymbol,
	/* 87 */ XK_KP_0,	NoSymbol,
	/* 88 */ XK_KP_Up,	NoSymbol,
	/* 89 */ XK_KP_Decimal,	NoSymbol,
	/* 90 */ XK_KP_Enter,	NoSymbol,
	/* 91 */ XK_KP_Left,	NoSymbol,
	/* 92 */ XK_KP_Down,	NoSymbol,
	/* 93 */ XK_KP_Right,	NoSymbol,
	/* 94 */ NoSymbol,	NoSymbol,
	/* 95 */ NoSymbol,	NoSymbol,
	/* 96 */ NoSymbol,	NoSymbol,
	/* 97 */ NoSymbol,	NoSymbol,
	/* 98 */ NoSymbol,	NoSymbol,
	/* 99 */ NoSymbol,	NoSymbol,
	/* 100 */ XK_KP_Multiply,NoSymbol,
	/* 101 */ XK_KP_Divide,	NoSymbol,
	/* 102 */ XK_KP_Tab,	NoSymbol,
	/* 104 */ XK_F11,	NoSymbol,
	/* 105 */ XK_F12,	NoSymbol,
	/* 106 */ XK_Help,	NoSymbol,
	/* 107 */ XK_Insert,	NoSymbol,
	/* 108 */ XK_Clear,	NoSymbol,
	/* 109 */ XK_Prior,	NoSymbol,
	/* 110 */ XK_Next,	NoSymbol,
};

static newsmipsModmapRec modmap_jpkbd[] = {
	54,	ShiftMask,	/* Shift_L */
	66,	ShiftMask,	/* Shift_R */
	68,	LockMask,	/* CapsLock */
	40,	ControlMask,	/* Control_L */
	67,	Mod1Mask,	/* Alt_L */
	73,	Mod4Mask,	/* Hiragana */
	0,	0
};

KeySymsRec newsmipsKeySyms[] = {
	/* map		minKC	maxKC	width */
	map_jpkbd,	0,	110,	GLYPHS_PER_KEY,
};

newsmipsModmapRec *newsmipsModMaps[] = {
	modmap_jpkbd,
};
