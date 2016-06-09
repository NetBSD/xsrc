/* $NetBSD: x68kKeyMap.c,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

#include	"x68k.h"

/*-------------------------------------------------------------------------
 * x68k standard keyboard map
 *-----------------------------------------------------------------------*/
static KeySym jisKeymap[] = {
    /* normal       shifted         kana            kana, shifted        */
    XK_Escape,      NoSymbol,       NoSymbol,       NoSymbol,    /* 0x01 */
    XK_1,           XK_exclam,	    XK_kana_NU,     NoSymbol,    /* 0x02 */
    XK_2,           XK_quotedbl,    XK_kana_FU,     NoSymbol,    /* 0x03 */
    XK_3,           XK_numbersign,  XK_kana_A,      XK_kana_a,   /* 0x04 */
    XK_4,           XK_dollar,	    XK_kana_U,      XK_kana_u,   /* 0x05 */
    XK_5,           XK_percent,	    XK_kana_E,      XK_kana_e,   /* 0x06 */
    XK_6,           XK_ampersand,   XK_kana_O,      XK_kana_o,   /* 0x07 */
    XK_7,           XK_quoteright,  XK_kana_YA,     XK_kana_ya,  /* 0x08 */
    XK_8,           XK_parenleft,   XK_kana_YU,     XK_kana_yu,  /* 0x09 */
    XK_9,           XK_parenright,  XK_kana_YO,     XK_kana_yo,  /* 0x0a */
    XK_0,           NoSymbol,       XK_kana_WA,     XK_kana_WO,  /* 0x0b */
    XK_minus,       XK_equal,       XK_kana_HO,     NoSymbol,    /* 0x0c */
    XK_asciicircum, XK_asciitilde,  XK_kana_HE,     NoSymbol,    /* 0x0d */
    XK_backslash,   XK_bar,         XK_prolongedsound, NoSymbol, /* 0x0e */
    XK_BackSpace,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x0f */
    XK_Tab,         NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x10 */
    XK_Q,           NoSymbol,	    XK_kana_TA,     NoSymbol,    /* 0x11 */
    XK_W,           NoSymbol,	    XK_kana_TE,     NoSymbol,    /* 0x12 */
    XK_E,           NoSymbol,	    XK_kana_I,      XK_kana_i,   /* 0x13 */
    XK_R,           NoSymbol,	    XK_kana_SU,     NoSymbol,    /* 0x14 */
    XK_T,           NoSymbol,	    XK_kana_KA,     NoSymbol,    /* 0x15 */
    XK_Y,           NoSymbol,	    XK_kana_N,      NoSymbol,    /* 0x16 */
    XK_U,           NoSymbol,	    XK_kana_NA,     NoSymbol,    /* 0x17 */
    XK_I,           NoSymbol,	    XK_kana_NI,     NoSymbol,    /* 0x18 */
    XK_O,           NoSymbol,	    XK_kana_RA,     NoSymbol,    /* 0x19 */
    XK_P,           NoSymbol,       XK_kana_SE,     NoSymbol,    /* 0x1a */
    XK_at,          XK_quoteleft,   XK_voicedsound, NoSymbol,    /* 0x1b */
    XK_bracketleft, XK_braceleft,   XK_semivoicedsound, NoSymbol,/* 0x1c */
    XK_Return,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x1d */
    XK_A,           NoSymbol,	    XK_kana_CHI,    NoSymbol,    /* 0x1e */
    XK_S,           NoSymbol,	    XK_kana_TO,     NoSymbol,    /* 0x1f */
    XK_D,           NoSymbol,	    XK_kana_SHI,    NoSymbol,    /* 0x20 */
    XK_F,           NoSymbol,	    XK_kana_HA,     NoSymbol,    /* 0x21 */
    XK_G,           NoSymbol,	    XK_kana_KI,     NoSymbol,    /* 0x22 */
    XK_H,           NoSymbol,	    XK_kana_KU,     NoSymbol,    /* 0x23 */
    XK_J,           NoSymbol,	    XK_kana_MA,     NoSymbol,    /* 0x24 */
    XK_K,           NoSymbol,	    XK_kana_NO,     NoSymbol,    /* 0x25 */
    XK_L,           NoSymbol,	    XK_kana_RI,     NoSymbol,    /* 0x26 */
    XK_semicolon,   XK_plus,	    XK_kana_RE,     NoSymbol,    /* 0x27 */
    XK_colon,       XK_asterisk,    XK_kana_KE,     NoSymbol,    /* 0x28 */
    XK_bracketright,XK_braceright,  XK_kana_MU,     XK_kana_closingbracket, /* 0x29 */
    XK_Z,           NoSymbol,	    XK_kana_TSU,    NoSymbol,    /* 0x2a */
    XK_X,           NoSymbol,	    XK_kana_SA,     NoSymbol,    /* 0x2b */
    XK_C,           NoSymbol,	    XK_kana_SO,     NoSymbol,    /* 0x2c */
    XK_V,           NoSymbol,	    XK_kana_HI,     NoSymbol,    /* 0x2d */
    XK_B,           NoSymbol,	    XK_kana_KO,     NoSymbol,    /* 0x2e */
    XK_N,           NoSymbol,	    XK_kana_MI,     NoSymbol,    /* 0x2f */
    XK_M,           NoSymbol,	    XK_kana_MO,     NoSymbol,    /* 0x30 */
    XK_comma,       XK_less,	    XK_kana_NE,     XK_kana_comma,    /* 0x31 */
    XK_period,      XK_greater,	    XK_kana_RU,     XK_kana_fullstop, /* 0x32 */
    XK_slash,       XK_question,    XK_kana_ME,     XK_kana_middledot,/* 0x33 */
    NoSymbol,       XK_underscore,  XK_kana_RO,     NoSymbol,    /* 0x34 */
    XK_space,       NoSymbol,       NoSymbol,       NoSymbol,    /* 0x35 */
    XK_Home,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x36 */
    XK_Delete,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x37 */
    XK_Page_Up,     NoSymbol,       NoSymbol,       NoSymbol,    /* 0x38 */
    XK_Page_Down,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x39 */
    XK_Undo,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3a */
    XK_Left,        NoSymbol,       NoSymbol,       NoSymbol,    /* 0x3b */
    XK_Up,          NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3c */
    XK_Right,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3d */
    XK_Down,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3e */
    XK_Clear,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3f */
    XK_KP_Divide,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x40 */
    XK_KP_Multiply, NoSymbol,       NoSymbol,       NoSymbol,    /* 0x41 */
    XK_KP_Subtract, NoSymbol,       NoSymbol,       NoSymbol,    /* 0x42 */
    XK_KP_7,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x43 */
    XK_KP_8,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x44 */
    XK_KP_9,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x45 */
    XK_KP_Add,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x46 */
    XK_KP_4,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x47 */
    XK_KP_5,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x48 */
    XK_KP_6,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x49 */
    XK_KP_Equal,    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x4a */
    XK_KP_1,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4b */
    XK_KP_2,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4c */
    XK_KP_3,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4d */
    XK_KP_Enter,    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x4e */
    XK_KP_0,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4f */
    XK_comma,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x50 */
    XK_period,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x51 */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x52 */
    XK_Touroku,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x53 */
    XK_Help,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x54 */
    XK_Alt_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x55 */
    XK_Meta_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x56 */
    XK_Meta_R,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x57 */
    XK_Alt_R,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x58 */
    XK_Control_R,   NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x59 */
    XK_Kana_Lock,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5a */
    XK_Romaji,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x5b */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x5c */
    XK_Caps_Lock,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5d */
    XK_Insert,	    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5e */
    XK_Hiragana_Katakana, NoSymbol, NoSymbol,       NoSymbol,    /* 0x5f */
    XK_Zenkaku_Hankaku,   NoSymbol, NoSymbol,       NoSymbol,    /* 0x60 */
    XK_Break,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x61 */
    XK_Print,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x62 */
    XK_F1,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x63 */
    XK_F2,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x64 */
    XK_F3,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x65 */
    XK_F4,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x66 */
    XK_F5,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x67 */
    XK_F6,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x68 */
    XK_F7,  	    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x69 */
    XK_F8,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6a */
    XK_F9,  	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6b */
    XK_F10, 	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6c */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6d */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6e */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6f */
    XK_Shift_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x70 */
    XK_Control_L,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x71 */
    XK_Super_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x72 */
    XK_Super_R,     NoSymbol,       NoSymbol,       NoSymbol,    /* 0x73 */
};

KeySymsRec jisKeySyms = {
    /*	map	minKeyCode maxKeyCode mapWidth */
    jisKeymap,	   0x01,      0x73,      4
};

/*-------------------------------------------------------------------------
 * ascii keyboard map
 *-----------------------------------------------------------------------*/
static KeySym asciiKeymap[] = {
    /* normal       shifted         kana            kana, shifted        */
    XK_Escape,      NoSymbol,       NoSymbol,       NoSymbol,    /* 0x01 */
    XK_1,           XK_exclam,	    XK_kana_NU,     NoSymbol,    /* 0x02 */
    XK_2,           XK_at,          XK_kana_FU,     NoSymbol,    /* 0x03 */
    XK_3,           XK_numbersign,  XK_kana_A,      XK_kana_a,   /* 0x04 */
    XK_4,           XK_dollar,	    XK_kana_U,      XK_kana_u,   /* 0x05 */
    XK_5,           XK_percent,	    XK_kana_E,      XK_kana_e,   /* 0x06 */
    XK_6,           XK_asciicircum, XK_kana_O,      XK_kana_o,   /* 0x07 */
    XK_7,           XK_ampersand,   XK_kana_YA,     XK_kana_ya,  /* 0x08 */
    XK_8,           XK_asterisk,    XK_kana_YU,     XK_kana_yu,  /* 0x09 */
    XK_9,           XK_parenleft,   XK_kana_YO,     XK_kana_yo,  /* 0x0a */
    XK_0,           XK_parenright,  XK_kana_WA,     XK_kana_WO,  /* 0x0b */
    XK_minus,       XK_underscore,  XK_kana_HO,     NoSymbol,    /* 0x0c */
    XK_equal,       XK_plus,        XK_kana_HE,     NoSymbol,    /* 0x0d */
    XK_backslash,   XK_bar,         XK_prolongedsound, NoSymbol, /* 0x0e */
    XK_BackSpace,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x0f */
    XK_Tab,         NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x10 */
    XK_Q,           NoSymbol,	    XK_kana_TA,     NoSymbol,    /* 0x11 */
    XK_W,           NoSymbol,	    XK_kana_TE,     NoSymbol,    /* 0x12 */
    XK_E,           NoSymbol,	    XK_kana_I,      XK_kana_i,   /* 0x13 */
    XK_R,           NoSymbol,	    XK_kana_SU,     NoSymbol,    /* 0x14 */
    XK_T,           NoSymbol,	    XK_kana_KA,     NoSymbol,    /* 0x15 */
    XK_Y,           NoSymbol,	    XK_kana_N,      NoSymbol,    /* 0x16 */
    XK_U,           NoSymbol,	    XK_kana_NA,     NoSymbol,    /* 0x17 */
    XK_I,           NoSymbol,	    XK_kana_NI,     NoSymbol,    /* 0x18 */
    XK_O,           NoSymbol,	    XK_kana_RA,     NoSymbol,    /* 0x19 */
    XK_P,           NoSymbol,       XK_kana_SE,     NoSymbol,    /* 0x1a */
    XK_bracketleft, XK_braceleft,   XK_voicedsound, NoSymbol,    /* 0x1b */
    XK_bracketright,XK_braceright,  XK_semivoicedsound, NoSymbol,/* 0x1c */
    XK_Return,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x1d */
    XK_A,           NoSymbol,	    XK_kana_CHI,    NoSymbol,    /* 0x1e */
    XK_S,           NoSymbol,	    XK_kana_TO,     NoSymbol,    /* 0x1f */
    XK_D,           NoSymbol,	    XK_kana_SHI,    NoSymbol,    /* 0x20 */
    XK_F,           NoSymbol,	    XK_kana_HA,     NoSymbol,    /* 0x21 */
    XK_G,           NoSymbol,	    XK_kana_KI,     NoSymbol,    /* 0x22 */
    XK_H,           NoSymbol,	    XK_kana_KU,     NoSymbol,    /* 0x23 */
    XK_J,           NoSymbol,	    XK_kana_MA,     NoSymbol,    /* 0x24 */
    XK_K,           NoSymbol,	    XK_kana_NO,     NoSymbol,    /* 0x25 */
    XK_L,           NoSymbol,	    XK_kana_RI,     NoSymbol,    /* 0x26 */
    XK_semicolon,   XK_colon,	    XK_kana_RE,     NoSymbol,    /* 0x27 */
    XK_quoteright,  XK_quotedbl,    XK_kana_KE,     NoSymbol,    /* 0x28 */
    XK_quoteleft,   XK_asciitilde,  XK_kana_MU,     XK_kana_closingbracket, /* 0x29 */
    XK_Z,           NoSymbol,	    XK_kana_TSU,    NoSymbol,    /* 0x2a */
    XK_X,           NoSymbol,	    XK_kana_SA,     NoSymbol,    /* 0x2b */
    XK_C,           NoSymbol,	    XK_kana_SO,     NoSymbol,    /* 0x2c */
    XK_V,           NoSymbol,	    XK_kana_HI,     NoSymbol,    /* 0x2d */
    XK_B,           NoSymbol,	    XK_kana_KO,     NoSymbol,    /* 0x2e */
    XK_N,           NoSymbol,	    XK_kana_MI,     NoSymbol,    /* 0x2f */
    XK_M,           NoSymbol,	    XK_kana_MO,     NoSymbol,    /* 0x30 */
    XK_comma,       XK_less,	    XK_kana_NE,     XK_kana_comma,    /* 0x31 */
    XK_period,      XK_greater,	    XK_kana_RU,     XK_kana_fullstop, /* 0x32 */
    XK_slash,       XK_question,    XK_kana_ME,     XK_kana_middledot,/* 0x33 */
    NoSymbol,       NoSymbol,       XK_kana_RO,     NoSymbol,    /* 0x34 */
    XK_space,       NoSymbol,       NoSymbol,       NoSymbol,    /* 0x35 */
    XK_Home,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x36 */
    XK_Delete,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x37 */
    XK_Page_Up,     NoSymbol,       NoSymbol,       NoSymbol,    /* 0x38 */
    XK_Page_Down,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x39 */
    XK_Undo,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3a */
    XK_Left,        NoSymbol,       NoSymbol,       NoSymbol,    /* 0x3b */
    XK_Up,          NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3c */
    XK_Right,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3d */
    XK_Down,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3e */
    XK_Clear,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x3f */
    XK_KP_Divide,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x40 */
    XK_KP_Multiply, NoSymbol,       NoSymbol,       NoSymbol,    /* 0x41 */
    XK_KP_Subtract, NoSymbol,       NoSymbol,       NoSymbol,    /* 0x42 */
    XK_KP_7,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x43 */
    XK_KP_8,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x44 */
    XK_KP_9,        NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x45 */
    XK_KP_Add,      NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x46 */
    XK_KP_4,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x47 */
    XK_KP_5,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x48 */
    XK_KP_6,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x49 */
    XK_KP_Equal,    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x4a */
    XK_KP_1,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4b */
    XK_KP_2,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4c */
    XK_KP_3,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4d */
    XK_KP_Enter,    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x4e */
    XK_KP_0,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x4f */
    XK_comma,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x50 */
    XK_period,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x51 */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x52 */
    XK_Touroku,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x53 */
    XK_Help,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x54 */
    XK_Alt_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x55 */
    XK_Meta_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x56 */
    XK_Meta_R,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x57 */
    XK_Alt_R,       NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x58 */
    XK_Control_R,   NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x59 */
    XK_Kana_Lock,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5a */
    XK_Romaji,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x5b */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x5c */
    XK_Caps_Lock,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5d */
    XK_Insert,	    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x5e */
    XK_Hiragana_Katakana, NoSymbol, NoSymbol,       NoSymbol,    /* 0x5f */
    XK_Zenkaku_Hankaku,   NoSymbol, NoSymbol,       NoSymbol,    /* 0x60 */
    XK_Break,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x61 */
    XK_Print,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x62 */
    XK_F1,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x63 */
    XK_F2,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x64 */
    XK_F3,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x65 */
    XK_F4,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x66 */
    XK_F5,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x67 */
    XK_F6,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x68 */
    XK_F7,  	    NoSymbol,       NoSymbol,       NoSymbol,    /* 0x69 */
    XK_F8,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6a */
    XK_F9,  	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6b */
    XK_F10, 	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6c */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6d */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6e */
    NoSymbol,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x6f */
    XK_Shift_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x70 */
    XK_Control_L,   NoSymbol,       NoSymbol,       NoSymbol,    /* 0x71 */
    XK_Super_L,	    NoSymbol,	    NoSymbol,       NoSymbol,    /* 0x72 */
    XK_Super_R,     NoSymbol,       NoSymbol,       NoSymbol,    /* 0x73 */
};

KeySymsRec asciiKeySyms = {
    /*	map	minKeyCode maxKeyCode mapWidth */
    asciiKeymap,   0x01,      0x73,      4
};

KeySymsRec *x68kKeySyms = &jisKeySyms;

/* EOF x68kKeyMap.c */
