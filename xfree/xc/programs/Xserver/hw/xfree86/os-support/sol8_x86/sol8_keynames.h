/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_keynames.h,v 1.8 2000/12/18 02:25:58 dawes Exp $ */

#ifndef _SOL8KEYNAMES_H
#define _SOL8KEYNAMES_H

#include "atKeynames.h"

#ifdef DEFINE_SOL8_MAP
/* Map the Solaris 8 keycodes to the "XFree86" keycodes. */
unsigned char sol8map[256] = {
	KEY_NOTUSED,		/*   0 */
	KEY_Tilde,		/*   1 */
	KEY_1,			/*   2 */
	KEY_2,			/*   3 */
	KEY_3,			/*   4 */
	KEY_4,			/*   5 */
	KEY_5,			/*   6 */
	KEY_6,			/*   7 */
	KEY_7,			/*   8 */
	KEY_8,			/*   9 */
	KEY_9,			/*  10 */
	KEY_0,			/*  11 */
	KEY_Minus,		/*  12 */
	KEY_Equal,		/*  13 */
	0x7D, /*KEY_P_YEN*/	/*  14 */
	KEY_BackSpace,		/*  15 */
	KEY_Tab,		/*  16 */
	KEY_Q,			/*  17 */
	KEY_W,			/*  18 */
	KEY_E,			/*  19 */
	KEY_R,			/*  20 */
	KEY_T,			/*  21 */
	KEY_Y,			/*  22 */
	KEY_U,			/*  23 */
	KEY_I,			/*  24 */
	KEY_O,			/*  25 */
	KEY_P,			/*  26 */
	KEY_LBrace,		/*  27 */
	KEY_RBrace,		/*  28 */
	KEY_BSlash,		/*  29 */
	KEY_CapsLock,		/*  30 */
	KEY_A,			/*  31 */
	KEY_S,			/*  32 */
	KEY_D,			/*  33 */
	KEY_F,			/*  34 */
	KEY_G,			/*  35 */
	KEY_H,			/*  36 */
	KEY_J,			/*  37 */
	KEY_K,			/*  38 */
	KEY_L,			/*  39 */
	KEY_SemiColon,		/*  40 */
	KEY_Quote,		/*  41 */
	KEY_UNKNOWN,		/*  42 */
	KEY_Enter,		/*  43 */
	KEY_ShiftL,		/*  44 */
	KEY_Less,		/*  45 */
	KEY_Z,			/*  46 */
	KEY_X,			/*  47 */
	KEY_C,			/*  48 */
	KEY_V,			/*  49 */
	KEY_B,			/*  50 */
	KEY_N,			/*  51 */
	KEY_M,			/*  52 */
	KEY_Comma,		/*  53 */
	KEY_Period,		/*  54 */
	KEY_Slash,		/*  55 */
	0x73, /*KEY_P_BKSL*/	/*  56 */
	KEY_ShiftR,		/*  57 */
	KEY_LCtrl,		/*  58 */
	KEY_LMeta,		/*  59 */
	KEY_Alt,		/*  60 */
	KEY_Space,		/*  61 */
	KEY_AltLang,		/*  62 */
	KEY_RMeta,		/*  63 */
	KEY_RCtrl,		/*  64 */
	KEY_Menu,		/*  65 */
	KEY_UNKNOWN,		/*  66 */
	KEY_UNKNOWN,		/*  67 */
	KEY_UNKNOWN,		/*  68 */
	KEY_UNKNOWN,		/*  69 */
	KEY_UNKNOWN,		/*  70 */
	KEY_UNKNOWN,		/*  71 */
	KEY_UNKNOWN,		/*  72 */
	KEY_UNKNOWN,		/*  73 */
	KEY_UNKNOWN,		/*  74 */
	KEY_Insert,		/*  75 */
	KEY_Delete,		/*  76 */
	KEY_UNKNOWN,		/*  77 */
	KEY_UNKNOWN,		/*  78 */
	KEY_Left,		/*  79 */
	KEY_Home,		/*  80 */
	KEY_End,		/*  81 */
	KEY_UNKNOWN,		/*  82 */
	KEY_Up,			/*  83 */
	KEY_Down,		/*  84 */
	KEY_PgUp,		/*  85 */
	KEY_PgDown,		/*  86 */
	KEY_UNKNOWN,		/*  87 */
	KEY_UNKNOWN,		/*  88 */
	KEY_Right,		/*  89 */
	KEY_NumLock,		/*  90 */
	KEY_KP_7,		/*  91 */
	KEY_KP_4,		/*  92 */
	KEY_KP_1,		/*  93 */
	KEY_UNKNOWN,		/*  94 */
	KEY_KP_Divide,		/*  95 */
	KEY_KP_8,		/*  96 */
	KEY_KP_5,		/*  97 */
	KEY_KP_2,		/*  98 */
	KEY_KP_0,		/*  99 */
	KEY_KP_Multiply,	/* 100 */
	KEY_KP_9,		/* 101 */
	KEY_KP_6,		/* 102 */
	KEY_KP_3,		/* 103 */
	KEY_KP_Decimal,		/* 104 */
	KEY_KP_Minus,		/* 105 */
	KEY_KP_Plus,		/* 106 */
	KEY_UNKNOWN,		/* 107 */
	KEY_KP_Enter,		/* 108 */
	KEY_UNKNOWN,		/* 109 */
	KEY_Escape,		/* 110 */
	KEY_UNKNOWN,		/* 111 */
	KEY_F1,			/* 112 */
	KEY_F2,			/* 113 */
	KEY_F3,			/* 114 */
	KEY_F4,			/* 115 */
	KEY_F5,			/* 116 */
	KEY_F6,			/* 117 */
	KEY_F7,			/* 118 */
	KEY_F8,			/* 119 */
	KEY_F9,			/* 120 */
	KEY_F10,		/* 121 */
	KEY_F11,		/* 122 */
	KEY_F12,		/* 123 */
	KEY_Print,		/* 124 */
	KEY_ScrollLock,		/* 125 */
	KEY_Pause,		/* 126 */
	KEY_UNKNOWN,		/* 127 */
	KEY_UNKNOWN,		/* 128 */
	KEY_UNKNOWN,		/* 129 */
	KEY_UNKNOWN,		/* 130 */
	0x7B, /*KEY_P_NFER*/	/* 131 */
	0x79, /*KEY_P_XFER*/	/* 132 */
	0x70, /*KEY_HKTG*/	/* 133 */
	KEY_UNKNOWN,		/* 134 */
	/* The rest default to KEY_UNKNOWN */
};
#endif

#endif /* _SOL8KEYNAMES_H */
