/*	$NetBSD: kbd.h,v 1.1 2004/01/18 04:15:18 rtr Exp $	*/

/*
 * Copyright (c) 1999 Mark Brinicombe & Neil A. Carson 
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * X11 driver code for VIDC20
 */

/*
 * This is in a severely hacked up state and is a rush job to implement
 * a X server for the RiscPC.
 *
 * A lot of cleanup is already being worked on.
 */

/* Our private translation table to work around the missing 8042 */

int kbdmap[] = {
	-1,		/* 0x00 */
	KEY_F9,		/* 0x01 */
	-1,		/* 0x02 */
	KEY_F5,		/* 0x03 */
	KEY_F3,		/* 0x04 */
	KEY_F1,		/* 0x05 */
	KEY_F2,		/* 0x06 */
	KEY_F12,	/* 0x07 */
	-1,		/* 0x08 */
	KEY_F10,	/* 0x09 */
	KEY_F8,		/* 0x0a */
	KEY_F6,		/* 0x0b */
	KEY_F4,		/* 0x0c */
	KEY_Tab,	/* 0x0d */
	KEY_Tilde,	/* 0x0e */
	-1,		/* 0x0f */
	-1,		/* 0x10 */
	KEY_Alt,	/* 0x11 */
	KEY_ShiftL,	/* 0x12 */
	-1,		/* 0x13 */
	KEY_LCtrl,	/* 0x14 */
	KEY_Q,		/* 0x15 */
	KEY_1,		/* 0x16 */
	-1,		/* 0x17 */
	-1,		/* 0x18 */
	-1,		/* 0x19 */
	KEY_Z,		/* 0x1a */
	KEY_S,		/* 0x1b */
	KEY_A,		/* 0x1c */
	KEY_W,		/* 0x1d */
	KEY_2,		/* 0x1e */
	-1,		/* 0x1f */
	-1,		/* 0x20 */
	KEY_C,		/* 0x21 */
	KEY_X,		/* 0x22 */
	KEY_D,		/* 0x23 */
	KEY_E,		/* 0x24 */
	KEY_4,		/* 0x25 */
	KEY_3,		/* 0x26 */
	-1,		/* 0x27 */
	-1,		/* 0x28 */
	KEY_Space,	/* 0x29 */
	KEY_V,		/* 0x2a */
	KEY_F,		/* 0x2b */
	KEY_T,		/* 0x2c */
	KEY_R,		/* 0x2d */
	KEY_5,		/* 0x2e */
	-1,		/* 0x2f */
	-1,		/* 0x30 */
	KEY_N,		/* 0x31 */
	KEY_B,		/* 0x32 */
	KEY_H,		/* 0x33 */
	KEY_G,		/* 0x34 */
	KEY_Y,		/* 0x35 */
	KEY_6,		/* 0x36 */
	-1,		/* 0x37 */
	-1,		/* 0x38 */
	-1,		/* 0x39 */
	KEY_M,		/* 0x3a */
	KEY_J,		/* 0x3b */
	KEY_U,		/* 0x3c */
	KEY_7,		/* 0x3d */
	KEY_8,		/* 0x3e */
	-1,		/* 0x3f */
	-1,		/* 0x40 */
	KEY_Comma,	/* 0x41 */
	KEY_K,		/* 0x42 */
	KEY_I,		/* 0x43 */
	KEY_O,		/* 0x44 */
	KEY_0,		/* 0x45 */
	KEY_9,		/* 0x46 */
	-1,		/* 0x47 */
	-1,		/* 0x48 */
	KEY_Period,	/* 0x49 */
	KEY_Slash,	/* 0x4a */
	KEY_L,		/* 0x4b */
	KEY_SemiColon,	/* 0x4c */
	KEY_P,		/* 0x4d */
	KEY_Minus,	/* 0x4e */
	-1,		/* 0x4f */
	-1,		/* 0x50 */
	-1,		/* 0x51 */
	KEY_Quote,	/* 0x52 */
	-1,		/* 0x53 */
	KEY_LBrace,	/* 0x54 */
	KEY_Equal,	/* 0x55 */
	-1,		/* 0x56 */
	-1,		/* 0x57 */
	KEY_CapsLock,	/* 0x58 */
	KEY_ShiftR,	/* 0x59 */
	KEY_Enter,	/* 0x5a */
	KEY_RBrace,	/* 0x5b */
	-1,		/* 0x5c */
	KEY_BSlash,	/* 0x5d */
	-1,		/* 0x5e */
	-1,		/* 0x5f */
	-1,		/* 0x60 */
	KEY_Less,	/* 0x61 */
	-1,		/* 0x62 */
	-1,		/* 0x63 */
	-1,		/* 0x64 */
	-1,		/* 0x65 */
	KEY_BackSpace,	/* 0x66 */
	-1,		/* 0x67 */
	-1,		/* 0x68 */
	KEY_KP_1,	/* 0x69 */
	-1,		/* 0x6a */
	KEY_KP_4,	/* 0x6b */
	KEY_KP_7,	/* 0x6c */
	-1,		/* 0x6d */
	-1,		/* 0x6e */
	-1,		/* 0x6f */
	KEY_KP_0,	/* 0x70 */
	KEY_KP_Decimal,	/* 0x71 */
	KEY_KP_2,	/* 0x72 */
	KEY_KP_5,	/* 0x73 */
	KEY_KP_6,	/* 0x74 */
	KEY_KP_8,	/* 0x75 */
	KEY_Escape,	/* 0x76 */
	KEY_NumLock,	/* 0x77 */
	KEY_F11,	/* 0x78 */
	KEY_KP_Plus,	/* 0x79 */
	KEY_KP_3,	/* 0x7a */
	KEY_KP_Minus,	/* 0x7b */
	KEY_KP_Multiply,/* 0x7c */
	KEY_KP_9,	/* 0x7d */
	KEY_ScrollLock,	/* 0x7e */
	KEY_Break,	/* 0x7f */
	-1,		/* 0x80 */
	-1,		/* 0x81 */
	-1,		/* 0x82 */
	KEY_F7,		/* 0x83 */
	-1,		/* 0x84 */
	-1,		/* 0x85 */
	-1,		/* 0x86 */
	-1,		/* 0x87 */
	-1,		/* 0x88 */
	-1,		/* 0x89 */
	-1,		/* 0x8a */
	-1,		/* 0x8b */
	-1,		/* 0x8c */
	-1,		/* 0x8d */
	-1,		/* 0x8e */
	-1,		/* 0x8f */
};

int kbdmap1[] = {
	-1,		/* 0x210*/
	KEY_AltLang,	/* 0x211 */
	-1,		/* 0x212 */
	-1,		/* 0x213 */
	KEY_RCtrl,	/* 0x214 */
};

int kbdmap2[] = {
	-1,		/* 0x240 */
	-1,		/* 0x241 */
	-1,		/* 0x242 */
	-1,		/* 0x243 */
	-1,		/* 0x244 */
	-1,		/* 0x245 */
	-1,		/* 0x246 */
	-1,		/* 0x247 */
	-1,		/* 0x248 */
	-1,		/* 0x249 */
	KEY_KP_Divide,	/* 0x24a */
	-1,		/* 0x24b */
	-1,		/* 0x24c */
	-1,		/* 0x24d */
	-1,		/* 0x24e */
	-1,		/* 0x24f */
	-1,		/* 0x250 */
	-1,		/* 0x251 */
	-1,		/* 0x252 */
	-1,		/* 0x253 */
	-1,		/* 0x254 */
	-1,		/* 0x255 */
	-1,		/* 0x256 */
	-1,		/* 0x257 */
	-1,		/* 0x258 */
	-1,		/* 0x259 */
	KEY_KP_Enter,	/* 0x25a */
	-1,		/* 0x25b */
	-1,		/* 0x25c */
	-1,		/* 0x25d */
	-1,		/* 0x25e */
	-1,		/* 0x25f */
	-1,		/* 0x260 */
	-1,		/* 0x261 */
	-1,		/* 0x262 */
	-1,		/* 0x263 */
	-1,		/* 0x264 */
	-1,		/* 0x265 */
	-1,		/* 0x266 */
	-1,		/* 0x267 */
	-1,		/* 0x268 */
	KEY_End,	/* 0x269 */
	-1,		/* 0x26a */
	KEY_Left,	/* 0x26b */
	KEY_Home,	/* 0x26c */
	-1,		/* 0x26d */
	-1,		/* 0x26e */
	-1,		/* 0x26f */
	KEY_Insert,	/* 0x270 */
	KEY_Delete,	/* 0x271 */
	KEY_Down,	/* 0x272 */
	-1,		/* 0x273 */
	KEY_Right,	/* 0x274 */
	KEY_Up,		/* 0x275 */
	-1,		/* 0x276 */
	KEY_Pause,	/* 0x277 */
	-1,		/* 0x278 */
	-1,		/* 0x279 */
	KEY_PgDown,	/* 0x27a */
	-1,		/* 0x27b */
	KEY_SysReqest,	/* 0x27c */
	KEY_PgUp,	/* 0x27d */
	-1,		/* 0x27e */
	-1,		/* 0x27f */
};
