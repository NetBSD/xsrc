/* $NetBSD: hpcKeymap.c,v 1.2 2004/07/22 18:08:59 uch Exp $	*/
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

#include	"hpc.h"
#include	"keysym.h"

#include	"atKeynames.h"
#include	"xf86Keymap.h"
#include <dev/wscons/wsksymdef.h>

static void hpcKeymapSetSymbol(KeySym *, u_int16_t, int);

KeySymsRec hpcKeySyms[] = {
    /*	map	   minKeyCode	maxKC	width */
    map,		0,	NUM_KEYCODES,	4,
};

static hpcModmapRec modmap[] = {
	KEY_ShiftL,	ShiftMask,
	KEY_ShiftR,	ShiftMask,
	KEY_CapsLock,	LockMask,
	KEY_LCtrl,	ControlMask,
	KEY_RCtrl,	ControlMask,
	KEY_Alt,	Mod1Mask,
	KEY_AltLang,	Mod1Mask,
	0,		0
};

hpcModmapRec *hpcModMaps[] = {
    modmap,
};

KeySym wssym_to_xkeysym[] = {
	[KS_BackSpace] = XK_BackSpace,
	[KS_Tab] = XK_Tab,
	[KS_Linefeed] = XK_Linefeed,
	[KS_Clear] = XK_Clear,
	[KS_Return] = XK_Return,
	[KS_Escape] = XK_Escape,
	[KS_space] = XK_space,
	[KS_exclam] = XK_exclam,
	[KS_quotedbl] = XK_quotedbl,
	[KS_numbersign] = XK_numbersign,
	[KS_dollar] = XK_dollar,
	[KS_percent] = XK_percent,
	[KS_ampersand] = XK_ampersand,
	[KS_apostrophe] = XK_apostrophe,
	[KS_parenleft] = XK_parenleft,
	[KS_parenright] = XK_parenright,
	[KS_asterisk] = XK_asterisk,
	[KS_plus] = XK_plus,
	[KS_comma] = XK_comma,
	[KS_minus] = XK_minus,
	[KS_period] = XK_period,
	[KS_slash] = XK_slash,
	[KS_0] = XK_0,
	[KS_1] = XK_1,
	[KS_2] = XK_2,
	[KS_3] = XK_3,
	[KS_4] = XK_4,
	[KS_5] = XK_5,
	[KS_6] = XK_6,
	[KS_7] = XK_7,
	[KS_8] = XK_8,
	[KS_9] = XK_9,
	[KS_colon] = XK_colon,
	[KS_semicolon] = XK_semicolon,
	[KS_less] = XK_less,
	[KS_equal] = XK_equal,
	[KS_greater] = XK_greater,
	[KS_question] = XK_question,
	[KS_at] = XK_at,
	[KS_A] = XK_A,
	[KS_B] = XK_B,
	[KS_C] = XK_C,
	[KS_D] = XK_D,
	[KS_E] = XK_E,
	[KS_F] = XK_F,
	[KS_G] = XK_G,
	[KS_H] = XK_H,
	[KS_I] = XK_I,
	[KS_J] = XK_J,
	[KS_K] = XK_K,
	[KS_L] = XK_L,
	[KS_M] = XK_M,
	[KS_N] = XK_N,
	[KS_O] = XK_O,
	[KS_P] = XK_P,
	[KS_Q] = XK_Q,
	[KS_R] = XK_R,
	[KS_S] = XK_S,
	[KS_T] = XK_T,
	[KS_U] = XK_U,
	[KS_V] = XK_V,
	[KS_W] = XK_W,
	[KS_X] = XK_X,
	[KS_Y] = XK_Y,
	[KS_Z] = XK_Z,
	[KS_bracketleft] = XK_bracketleft,
	[KS_backslash] = XK_backslash,
	[KS_bracketright] = XK_bracketright,
	[KS_asciicircum] = XK_asciicircum,
	[KS_underscore] = XK_underscore,
	[KS_grave] = XK_grave,
	[KS_a] = XK_a,
	[KS_b] = XK_b,
	[KS_c] = XK_c,
	[KS_d] = XK_d,
	[KS_e] = XK_e,
	[KS_f] = XK_f,
	[KS_g] = XK_g,
	[KS_h] = XK_h,
	[KS_i] = XK_i,
	[KS_j] = XK_j,
	[KS_k] = XK_k,
	[KS_l] = XK_l,
	[KS_m] = XK_m,
	[KS_n] = XK_n,
	[KS_o] = XK_o,
	[KS_p] = XK_p,
	[KS_q] = XK_q,
	[KS_r] = XK_r,
	[KS_s] = XK_s,
	[KS_t] = XK_t,
	[KS_u] = XK_u,
	[KS_v] = XK_v,
	[KS_w] = XK_w,
	[KS_x] = XK_x,
	[KS_y] = XK_y,
	[KS_z] = XK_z,
	[KS_braceleft] = XK_braceleft,
	[KS_bar] = XK_bar,
	[KS_braceright] = XK_braceright,
	[KS_asciitilde] = XK_asciitilde,
	[KS_Delete] = XK_Delete,
	[KS_nobreakspace] = XK_nobreakspace,
	[KS_exclamdown] = XK_exclamdown,
	[KS_cent] = XK_cent,
	[KS_sterling] = XK_sterling,
	[KS_currency] = XK_currency,
	[KS_yen] = XK_yen,
	[KS_brokenbar] = XK_brokenbar,
	[KS_section] = XK_section,
	[KS_diaeresis] = XK_diaeresis,
	[KS_copyright] = XK_copyright,
	[KS_ordfeminine] = XK_ordfeminine,
	[KS_guillemotleft] = XK_guillemotleft,
	[KS_notsign] = XK_notsign,
	[KS_hyphen] = XK_hyphen,
	[KS_registered] = XK_registered,
	[KS_macron] = XK_macron,
	[KS_degree] = XK_degree,
	[KS_plusminus] = XK_plusminus,
	[KS_twosuperior] = XK_twosuperior,
	[KS_threesuperior] = XK_threesuperior,
	[KS_acute] = XK_acute,
	[KS_mu] = XK_mu,
	[KS_paragraph] = XK_paragraph,
	[KS_periodcentered] = XK_periodcentered,
	[KS_cedilla] = XK_cedilla,
	[KS_onesuperior] = XK_onesuperior,
	[KS_masculine] = XK_masculine,
	[KS_guillemotright] = XK_guillemotright,
	[KS_onequarter] = XK_onequarter,
	[KS_onehalf] = XK_onehalf,
	[KS_threequarters] = XK_threequarters,
	[KS_questiondown] = XK_questiondown,
	[KS_Agrave] = XK_Agrave,
	[KS_Aacute] = XK_Aacute,
	[KS_Acircumflex] = XK_Acircumflex,
	[KS_Atilde] = XK_Atilde,
	[KS_Adiaeresis] = XK_Adiaeresis,
	[KS_Aring] = XK_Aring,
	[KS_AE] = XK_AE,
	[KS_Ccedilla] = XK_Ccedilla,
	[KS_Egrave] = XK_Egrave,
	[KS_Eacute] = XK_Eacute,
	[KS_Ecircumflex] = XK_Ecircumflex,
	[KS_Ediaeresis] = XK_Ediaeresis,
	[KS_Igrave] = XK_Igrave,
	[KS_Iacute] = XK_Iacute,
	[KS_Icircumflex] = XK_Icircumflex,
	[KS_Idiaeresis] = XK_Idiaeresis,
	[KS_ETH] = XK_ETH,
	[KS_Ntilde] = XK_Ntilde,
	[KS_Ograve] = XK_Ograve,
	[KS_Oacute] = XK_Oacute,
	[KS_Ocircumflex] = XK_Ocircumflex,
	[KS_Otilde] = XK_Otilde,
	[KS_Odiaeresis] = XK_Odiaeresis,
	[KS_multiply] = XK_multiply,
	[KS_Ooblique] = XK_Ooblique,
	[KS_Ugrave] = XK_Ugrave,
	[KS_Uacute] = XK_Uacute,
	[KS_Ucircumflex] = XK_Ucircumflex,
	[KS_Udiaeresis] = XK_Udiaeresis,
	[KS_Yacute] = XK_Yacute,
	[KS_THORN] = XK_THORN,
	[KS_ssharp] = XK_ssharp,
	[KS_agrave] = XK_agrave,
	[KS_aacute] = XK_aacute,
	[KS_acircumflex] = XK_acircumflex,
	[KS_atilde] = XK_atilde,
	[KS_adiaeresis] = XK_adiaeresis,
	[KS_aring] = XK_aring,
	[KS_ae] = XK_ae,
	[KS_ccedilla] = XK_ccedilla,
	[KS_egrave] = XK_egrave,
	[KS_eacute] = XK_eacute,
	[KS_ecircumflex] = XK_ecircumflex,
	[KS_ediaeresis] = XK_ediaeresis,
	[KS_igrave] = XK_igrave,
	[KS_iacute] = XK_iacute,
	[KS_icircumflex] = XK_icircumflex,
	[KS_idiaeresis] = XK_idiaeresis,
	[KS_eth] = XK_eth,
	[KS_ntilde] = XK_ntilde,
	[KS_ograve] = XK_ograve,
	[KS_oacute] = XK_oacute,
	[KS_ocircumflex] = XK_ocircumflex,
	[KS_otilde] = XK_otilde,
	[KS_odiaeresis] = XK_odiaeresis,
	[KS_division] = XK_division,
	[KS_oslash] = XK_oslash,
	[KS_ugrave] = XK_ugrave,
	[KS_uacute] = XK_uacute,
	[KS_ucircumflex] = XK_ucircumflex,
	[KS_udiaeresis] = XK_udiaeresis,
	[KS_yacute] = XK_yacute,
	[KS_thorn] = XK_thorn,
	[KS_ydiaeresis] = XK_ydiaeresis,
	[KS_Odoubleacute] = XK_Odoubleacute,
	[KS_odoubleacute] = XK_odoubleacute,
	[KS_Udoubleacute] = XK_Udoubleacute,
	[KS_udoubleacute] = XK_udoubleacute,
	[KS_dead_grave] = XK_dead_grave,
	[KS_dead_acute] = XK_dead_acute,
	[KS_dead_circumflex] = XK_dead_circumflex,
	[KS_dead_tilde] = XK_dead_tilde,
	[KS_dead_diaeresis] = XK_dead_diaeresis,
	[KS_dead_abovering] = XK_dead_abovering,
	[KS_dead_cedilla] = XK_dead_cedilla,
	[KS_Shift_L] = XK_Shift_L,
	[KS_Shift_R] = XK_Shift_R,
	[KS_Control_L] = XK_Control_L,
	[KS_Control_R] = XK_Control_R,
	[KS_Caps_Lock] = XK_Caps_Lock,
	[KS_Shift_Lock] = XK_Shift_Lock,
	[KS_Alt_L] = XK_Alt_L,
	[KS_Alt_R] = XK_Alt_R,
	[KS_Multi_key] = XK_Multi_key,
	[KS_Mode_switch] = XK_Mode_switch,
	[KS_Num_Lock] = XK_Num_Lock,
	[KS_Meta_L] = XK_Meta_L,
	[KS_Meta_R] = XK_Meta_R,
	[KS_Zenkaku_Hankaku] = XK_Zenkaku_Hankaku,
	[KS_Hiragana_Katakana] = XK_Hiragana_Katakana,
	[KS_Henkan_Mode] = XK_Henkan_Mode,
	[KS_Henkan] = XK_Henkan,
	[KS_Muhenkan] = XK_Muhenkan,
	[KS_KP_F1] = XK_KP_F1,
	[KS_KP_F2] = XK_KP_F2,
	[KS_KP_F3] = XK_KP_F3,
	[KS_KP_F4] = XK_KP_F4,
	[KS_KP_Home] = XK_KP_Home,
	[KS_KP_Left] = XK_KP_Left,
	[KS_KP_Up] = XK_KP_Up,
	[KS_KP_Right] = XK_KP_Right,
	[KS_KP_Down] = XK_KP_Down,
	[KS_KP_Prior] = XK_KP_Prior,
	[KS_KP_Next] = XK_KP_Next,
	[KS_KP_End] = XK_KP_End,
	[KS_KP_Begin] = XK_KP_Begin,
	[KS_KP_Insert] = XK_KP_Insert,
	[KS_KP_Delete] = XK_KP_Delete,
	[KS_KP_Space] = XK_KP_Space,
	[KS_KP_Tab] = XK_KP_Tab,
	[KS_KP_Enter] = XK_KP_Enter,
	[KS_KP_Equal] = XK_KP_Equal,
	[KS_KP_Numbersign] = XK_numbersign,
	[KS_KP_Multiply] = XK_KP_Multiply,
	[KS_KP_Add] = XK_KP_Add,
	[KS_KP_Separator] = XK_KP_Separator,
	[KS_KP_Subtract] = XK_KP_Subtract,
	[KS_KP_Decimal] = XK_KP_Decimal,
	[KS_KP_Divide] = XK_KP_Divide,
	[KS_KP_0] = XK_KP_0,
	[KS_KP_1] = XK_KP_1,
	[KS_KP_2] = XK_KP_2,
	[KS_KP_3] = XK_KP_3,
	[KS_KP_4] = XK_KP_4,
	[KS_KP_5] = XK_KP_5,
	[KS_KP_6] = XK_KP_6,
	[KS_KP_7] = XK_KP_7,
	[KS_KP_8] = XK_KP_8,
	[KS_KP_9] = XK_KP_9,
	[KS_f1] = XK_F1,
	[KS_f2] = XK_F2,
	[KS_f3] = XK_F3,
	[KS_f4] = XK_F4,
	[KS_f5] = XK_F5,
	[KS_f6] = XK_F6,
	[KS_f7] = XK_F7,
	[KS_f8] = XK_F8,
	[KS_f9] = XK_F9,
	[KS_f10] = XK_F10,
	[KS_f11] = XK_F11,
	[KS_f12] = XK_F12,
	[KS_f13] = XK_F13,
	[KS_f14] = XK_F14,
	[KS_f15] = XK_F15,
	[KS_f16] = XK_F16,
	[KS_f17] = XK_F17,
	[KS_f18] = XK_F18,
	[KS_f19] = XK_F19,
	[KS_f20] = XK_F20,
	[KS_F1] = XK_F1,
	[KS_F2] = XK_F2,
	[KS_F3] = XK_F3,
	[KS_F4] = XK_F4,
	[KS_F5] = XK_F5,
	[KS_F6] = XK_F6,
	[KS_F7] = XK_F7,
	[KS_F8] = XK_F8,
	[KS_F9] = XK_F9,
	[KS_F10] = XK_F10,
	[KS_F11] = XK_F11,
	[KS_F12] = XK_F12,
	[KS_F13] = XK_F13,
	[KS_F14] = XK_F14,
	[KS_F15] = XK_F15,
	[KS_F16] = XK_F16,
	[KS_F17] = XK_F17,
	[KS_F18] = XK_F18,
	[KS_F19] = XK_F19,
	[KS_F20] = XK_F20,
	[KS_Home] = XK_Home,
	[KS_Prior] = XK_Prior,
	[KS_Next] = XK_Next,
	[KS_Up] = XK_Up,
	[KS_Down] = XK_Down,
	[KS_Left] = XK_Left,
	[KS_Right] = XK_Right,
	[KS_End] = XK_End,
	[KS_Insert] = XK_Insert,
	[KS_Help] = XK_Help,
	[KS_Execute] = XK_Execute,
	[KS_Find] = XK_Find,
	[KS_Select] = XK_Select,
	[KS_Undo] = XK_Undo,
	[KS_Menu] = XK_Menu,
	[KS_Pause] = XK_Pause,
	[0xffff] = NoSymbol,
};

int
hpcKeymapConvertWssymToXsym(int fd)
{
	struct wskbd_map_data map_data;
	struct wscons_keymap wscons_keymap[WSKBDIO_MAXMAPLEN], *wscons_key;
	KeySym *x_key;
	int i;

	memset(wscons_keymap, 0, sizeof wscons_keymap);
	map_data.maplen = WSKBDIO_MAXMAPLEN;
	map_data.map = wscons_keymap;

	/* Get console keymap */
	if (ioctl(fd, WSKBDIO_GETMAP, &map_data) == -1) {
		hpcErrorF(("can't WSKBDIO_GETMAP"));
		return 1;
	}

	for (i = 0;
	    i < sizeof wssym_to_xkeysym / sizeof(wssym_to_xkeysym[0]); i++)
		if (wssym_to_xkeysym[i] == 0)
			wssym_to_xkeysym[i] = NoSymbol;

	/* Reconstruct keymap */
	wscons_key = wscons_keymap;
	x_key = map;
	memset(modmap, 0, sizeof modmap);
	for (i = 0; i < map_data.maplen; i++, wscons_key++) {
		/* assume GLYPHS_PER_KEY is 4 */
		if (i == NUM_KEYCODES) {
			hpcErrorF(("Xkeysym overflow.\n"));
			break;
		}
		hpcKeymapSetSymbol(x_key++, wscons_key->group1[0], i);
		hpcKeymapSetSymbol(x_key++, wscons_key->group1[1], i);
		hpcKeymapSetSymbol(x_key++, wscons_key->group2[0], i);
		hpcKeymapSetSymbol(x_key++, wscons_key->group2[1], i);
	}
	hpcKeySyms[0].maxKeyCode = map_data.maplen - 1;
	hpcPrintF(("wssym converted xkeysym.\n"));

	return 0;
}

void
hpcKeymapSetSymbol(KeySym *dst, u_int16_t wssym, int keyscan)
{

	*dst = wssym_to_xkeysym[wssym];

	switch (*dst) {
	case XK_Shift_L:
		modmap[0].key = keyscan;
		modmap[0].modifiers = ShiftMask;
		break;
	case XK_Shift_R:
		modmap[1].key = keyscan;
		modmap[1].modifiers = ShiftMask;
		break;
	case XK_Caps_Lock:
		modmap[2].key = keyscan;
		modmap[2].modifiers = LockMask;
		break;
	case XK_Control_L:
		modmap[3].key = keyscan;
		modmap[3].modifiers = ControlMask;
		break;
	case XK_Control_R:
		modmap[4].key = keyscan;
		modmap[4].modifiers = ControlMask;
		break;
	case XK_Alt_L:
		modmap[5].key = keyscan;
		modmap[5].modifiers = AltMask;
		break;
	case XK_Mode_switch:	// ok? -uch
		modmap[6].key = keyscan;
		modmap[6].modifiers = AltLangMask;
		break;
	}
}
