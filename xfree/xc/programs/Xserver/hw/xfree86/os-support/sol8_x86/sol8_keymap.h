/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_keymap.h,v 1.2 1999/09/27 14:59:28 dawes Exp $ */
/* 
 * replacement for xf86Keymap.h - for Solaris8_x86
 */ 

/* Assumes a US English keyboard as default - sorry 'bout that 
 * 
 * Hopefully it'll be enough someone can have a sorta working 
 * keyboard, if they're not using XKB
 * 
 * DWH 9/12/99
 */

static KeySym sol8defaultKeymap[NUM_KEYCODES * GLYPHS_PER_KEY] = {

    /* 000 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 001 */  XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* ` */
    /* 002 */  XK_1,		XK_exclam,	NoSymbol,	NoSymbol,	/* 1 */
    /* 003 */  XK_2,		XK_at,		NoSymbol,	NoSymbol,	/* 2 */
    /* 004 */  XK_3,		XK_numbersign,	NoSymbol,	NoSymbol,	/* 3 */
    /* 005 */  XK_4,		XK_dollar,	NoSymbol,	NoSymbol,	/* 4 */
    /* 006 */  XK_5,		XK_percent,	NoSymbol,	NoSymbol,	/* 5 */
    /* 007 */  XK_6,		XK_asciicircum,	NoSymbol,	NoSymbol,	/* 6 */
    /* 008 */  XK_7,		XK_ampersand,	NoSymbol,	NoSymbol,	/* 7 */
    /* 009 */  XK_8,		XK_asterisk,	NoSymbol,	NoSymbol,	/* 8 */
    /* 010 */  XK_9,		XK_parenleft,	NoSymbol,	NoSymbol,	/* 9 */
    /* 011 */  XK_0,		XK_parenright,	NoSymbol,	NoSymbol,	/* 10 */
    /* 012 */  XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* - */
    /* 013 */  XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* = */
    /* 014 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 015 */  XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* ^H */
    /* 016 */  XK_Tab,		NoSymbol,	NoSymbol,	NoSymbol,	/* ^I */
    /* 017 */  XK_Q,		NoSymbol,	NoSymbol,	NoSymbol,	/* q */
    /* 018 */  XK_W,		NoSymbol,	NoSymbol,	NoSymbol,	/* w */
    /* 019 */  XK_E,		NoSymbol,	NoSymbol,	NoSymbol,	/* e */
    /* 020 */  XK_R,		NoSymbol,	NoSymbol,	NoSymbol,	/* r */
    /* 021 */  XK_T,		NoSymbol,	NoSymbol,	NoSymbol,	/* t */
    /* 022 */  XK_Y,		NoSymbol,	NoSymbol,	NoSymbol,	/* y */
    /* 023 */  XK_U,		NoSymbol,	NoSymbol,	NoSymbol,	/* u */
    /* 024 */  XK_I,		NoSymbol,	NoSymbol,	NoSymbol,	/* i */
    /* 025 */  XK_O,		NoSymbol,	NoSymbol,	NoSymbol,	/* o */
    /* 026 */  XK_P,		NoSymbol,	NoSymbol,	NoSymbol,	/* p */
    /* 027 */  XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* [ */
    /* 028 */  XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* { */
    /* 029 */  XK_backslash,	XK_bar,		NoSymbol,	NoSymbol,	/* | */
    /* 030 */  XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 031 */  XK_A,		NoSymbol,	NoSymbol,	NoSymbol,	/* a */
    /* 032 */  XK_S,		NoSymbol,	NoSymbol,	NoSymbol,	/* s */
    /* 033 */  XK_D,		NoSymbol,	NoSymbol,	NoSymbol,	/* d */
    /* 034 */  XK_F,		NoSymbol,	NoSymbol,	NoSymbol,	/* f */
    /* 035 */  XK_G,		NoSymbol,	NoSymbol,	NoSymbol,	/* g */
    /* 036 */  XK_H,		NoSymbol,	NoSymbol,	NoSymbol,	/* h */
    /* 037 */  XK_J,		NoSymbol,	NoSymbol,	NoSymbol,	/* j */
    /* 038 */  XK_K,		NoSymbol,	NoSymbol,	NoSymbol,	/* k */
    /* 039 */  XK_L,		NoSymbol,	NoSymbol,	NoSymbol,	/* l */
    /* 040 */  XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* ; */
    /* 041 */  XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* ' */
    /* 042 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 043 */  XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 044 */  XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 045 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 046 */  XK_Z,		NoSymbol,	NoSymbol,	NoSymbol,	/* z */
    /* 047 */  XK_X,		NoSymbol,	NoSymbol,	NoSymbol,	/* x */
    /* 048 */  XK_C,		NoSymbol,	NoSymbol,	NoSymbol,	/* c */
    /* 049 */  XK_V,		NoSymbol,	NoSymbol,	NoSymbol,	/* v */
    /* 050 */  XK_B,		NoSymbol,	NoSymbol,	NoSymbol,	/* b */
    /* 051 */  XK_N,		NoSymbol,	NoSymbol,	NoSymbol,	/* n */
    /* 052 */  XK_M,		NoSymbol,	NoSymbol,	NoSymbol,	/* m */
    /* 053 */  XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/* , */
    /* 054 */  XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/* . */
    /* 055 */  XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/* / */
    /* 056 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 057 */  XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 058 */  XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 059 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 060 */  XK_Alt_L,	XK_Meta_L,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 061 */  XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*    */
    /* 062 */  XK_Alt_R,	XK_Meta_R,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 063 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 064 */  XK_Control_R,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmmm */
    /* 065 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 066 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 067 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 068 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 069 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 070 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 071 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 072 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 073 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 074 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 075 */  XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 076 */  XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 077 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 078 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 079 */  XK_Left,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 080 */  XK_Home,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 081 */  XK_End,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 082 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 083 */  XK_Up,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 084 */  XK_Down,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 085 */  XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 086 */  XK_Next,		NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 087 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 088 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 089 */  XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 090 */  XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* hmm */
    /* 091 */  XK_KP_Home,	XK_KP_7,	NoSymbol,	NoSymbol,	/* hmm */
    /* 092 */  XK_KP_Left,	XK_KP_4,	NoSymbol,	NoSymbol, 	/* hmm */
    /* 093 */  XK_KP_End,	XK_KP_1,	NoSymbol,	NoSymbol,	/* hmm */
    /* 094 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 095 */  XK_KP_Divide,	NoSymbol,	NoSymbol,	NoSymbol,	/* / */
    /* 096 */  XK_KP_Up,	XK_KP_8,	NoSymbol,	NoSymbol,	/* hmm */
    /* 097 */  NoSymbol,	XK_KP_5,	NoSymbol,	NoSymbol,	/* 5 */
    /* 098 */  XK_KP_Down,	XK_KP_2,	NoSymbol,	NoSymbol,	/* hmm */
    /* 099 */  XK_KP_Insert,	XK_KP_0,	NoSymbol,	NoSymbol,	/* hmm */
    /* 100 */  XK_KP_Multiply,	NoSymbol,	NoSymbol,	NoSymbol,	/* * */
    /* 101 */  XK_KP_Prior,	XK_KP_9,	NoSymbol,	NoSymbol,
    /* 102 */  XK_KP_Right,	XK_KP_6,	NoSymbol,	NoSymbol,
    /* 103 */  XK_KP_Next,	XK_KP_3,	NoSymbol,	NoSymbol,
    /* 104 */  XK_KP_Delete,	XK_KP_Decimal,	NoSymbol,	NoSymbol,
    /* 105 */  XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* - */
    /* 106 */  XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/* + */
    /* 107 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 108 */  XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 109 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 110 */  XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* Esc */
    /* 111 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 112 */  XK_F1,		NoSymbol,	NoSymbol,	NoSymbol,	/* F1 */
    /* 113 */  XK_F2,		NoSymbol,	NoSymbol,	NoSymbol,	/* F2 */
    /* 114 */  XK_F3,		NoSymbol,	NoSymbol,	NoSymbol,	/* F3 */
    /* 115 */  XK_F4,		NoSymbol,	NoSymbol,	NoSymbol,	/* F4 */
    /* 116 */  XK_F5,		NoSymbol,	NoSymbol,	NoSymbol,	/* F5 */
    /* 117 */  XK_F6,		NoSymbol,	NoSymbol,	NoSymbol,	/* F6 */
    /* 118 */  XK_F7,		NoSymbol,	NoSymbol,	NoSymbol,	/* F7 */
    /* 119 */  XK_F8,		NoSymbol,	NoSymbol,	NoSymbol,	/* F8 */
    /* 120 */  XK_F9,		NoSymbol,	NoSymbol,	NoSymbol,	/* F9 */
    /* 121 */  XK_F10,		NoSymbol,	NoSymbol,	NoSymbol,	/* F10 */
    /* 122 */  XK_F11,		NoSymbol,	NoSymbol,	NoSymbol,	/* F11 */
    /* 123 */  XK_F12,		NoSymbol,	NoSymbol,	NoSymbol,	/* F12 */
    /* 124 */  XK_Print,	NoSymbol,	NoSymbol,	NoSymbol,	/* Print Screen */
    /* 125 */  XK_Scroll_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* Scroll Lock */
    /* 126 */  XK_Pause,	NoSymbol,	NoSymbol,	NoSymbol,	/* Pause */
};

