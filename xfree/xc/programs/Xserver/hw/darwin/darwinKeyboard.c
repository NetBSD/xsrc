//=============================================================================
//
// Keyboard support for the Darwin X Server
//
// By Torrey T. Lyons
//
// The code to parse the Darwin keymap is derived from dumpkeymap.c
// by Eric Sunshine, which includes the following license:
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. The name of the author may not be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
// NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=============================================================================

/* $XFree86: xc/programs/Xserver/hw/darwin/darwinKeyboard.c,v 1.1 2000/11/15 01:36:14 dawes Exp $ */

/*
===========================================================================

 An X keyCode must be in the range XkbMinLegalKeyCode (8) to
 XkbMaxLegalKeyCode(255).

 The keyCodes we get from the kernel range from 0 to 127, so we need to
 offset the range before passing the keyCode to X.

 An X KeySym is an extended ascii code that is device independent.

 The modifier map is accessed by the keyCode, but the normal map is
 accessed by keyCode - MIN_KEYCODE.  Sigh.
 
===========================================================================
*/

// Define this to get a diagnostic output to stderr which is helpful
// in determining how the X server is interpreting the Darwin keymap.
#undef DUMP_DARWIN_KEYMAP

#include <drivers/event_status_driver.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include "darwin.h"
extern DarwinFramebufferRec dfb;
extern unsigned char darwinKeyCommandL, darwinKeyOptionL;

#define XK_TECHNICAL		// needed to get XK_Escape
#include "keysym.h"

#define GLYPHS_PER_KEY  4
#define NUM_KEYCODES    248		// NX_NUMKEYCODES might be better
#define MAX_KEYCODE     NUM_KEYCODES + MIN_KEYCODE - 1

#define AltMask	        Mod1Mask
#define NumLockMask     Mod2Mask
#define MetaMask        Mod3Mask
#define ScrollLockMask  Mod4Mask

static KeySym const ascii_to_x[256] = {
	NoSymbol,	NoSymbol,	NoSymbol,	XK_KP_Enter,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_Delete,	XK_Tab,		XK_Linefeed,	NoSymbol,
	NoSymbol,	XK_Return,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	XK_Escape,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_space,	XK_exclam,	XK_quotedbl,	XK_numbersign,
	XK_dollar,	XK_percent,	XK_ampersand,	XK_apostrophe,
	XK_parenleft,	XK_parenright,	XK_asterisk,	XK_plus,
	XK_comma,	XK_minus,	XK_period,	XK_slash,
	XK_0,		XK_1,		XK_2,		XK_3,
	XK_4,		XK_5,		XK_6,		XK_7,
	XK_8,		XK_9,		XK_colon,	XK_semicolon,
	XK_less,	XK_equal,	XK_greater,	XK_question,
	XK_at,		XK_A,		XK_B,		XK_C,
	XK_D,		XK_E,		XK_F,		XK_G,
	XK_H,		XK_I,		XK_J,		XK_K,
	XK_L,		XK_M,		XK_N,		XK_O,
	XK_P,		XK_Q,		XK_R,		XK_S,
	XK_T,		XK_U,		XK_V,		XK_W,
	XK_X,		XK_Y,		XK_Z,		XK_bracketleft,
	XK_backslash,	XK_bracketright,XK_asciicircum,	XK_underscore,
	XK_grave,	XK_a,		XK_b,		XK_c,
	XK_d,		XK_e,		XK_f,		XK_g,
	XK_h,		XK_i,		XK_j,		XK_k,
	XK_l,		XK_m,		XK_n,		XK_o,
	XK_p,		XK_q,		XK_r,		XK_s,
	XK_t,		XK_u,		XK_v,		XK_w,
	XK_x,		XK_y,		XK_z,		XK_braceleft,
	XK_bar,		XK_braceright,	XK_asciitilde,	XK_BackSpace,
// 128
	XK_Ccedilla,	XK_udiaeresis,	XK_eacute,	XK_acircumflex,
	XK_adiaeresis,	XK_agrave,	XK_aring,	XK_ccedilla,
	XK_ecircumflex,	XK_ediaeresis,	XK_egrave,	XK_idiaeresis,
	XK_icircumflex,	XK_igrave,	XK_Adiaeresis,	XK_Aring,
	XK_Eacute,	XK_ae,		XK_AE,		XK_ocircumflex,
	XK_odiaeresis,	XK_ograve,	XK_ntilde,	XK_ugrave,
	XK_ydiaeresis,	XK_Odiaeresis,	XK_Udiaeresis,	XK_cent,
	XK_sterling,	XK_yen,		XK_paragraph,	XK_section,
// 160
	XK_aacute,	XK_degree,	XK_cent,	XK_sterling,
	XK_ntilde,	XK_Ntilde,	XK_paragraph,	XK_Greek_BETA,
	XK_questiondown,XK_hyphen,	XK_notsign,	XK_onehalf,
	XK_onequarter,	XK_exclamdown,	XK_guillemotleft,XK_guillemotright,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
// 192
	XK_questiondown,XK_exclamdown,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	XK_AE,		XK_ae,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
// 224
	XK_Greek_alpha,	XK_ssharp,	XK_Greek_GAMMA,	XK_Greek_pi,
	XK_Greek_SIGMA,	XK_Greek_sigma,	XK_mu,	        XK_Greek_tau,
	XK_Greek_PHI,	XK_Greek_THETA,	XK_Greek_OMEGA,	XK_Greek_delta,
	XK_infinity,	XK_Ooblique,	XK_Greek_epsilon, XK_intersection,
	XK_identical,	XK_plusminus,	XK_greaterthanequal, XK_lessthanequal,
	XK_topintegral,	XK_botintegral,	XK_division,	XK_similarequal,
	XK_degree,	NoSymbol,	NoSymbol,	XK_radical,
	XK_Greek_eta,	XK_twosuperior,	XK_periodcentered, NoSymbol,
  };

#define MIN_SYMBOL		0xAC
static KeySym const symbol_to_x[] = {
    XK_Left,        XK_Up,          XK_Right,      XK_Down
  };
int const NUM_SYMBOL = sizeof(symbol_to_x) / sizeof(symbol_to_x[0]);

#define MIN_FUNCKEY		0x20
static KeySym const funckey_to_x[] = {
    XK_F1,          XK_F2,          XK_F3,          XK_F4,
    XK_F5,          XK_F6,          XK_F7,          XK_F8,
    XK_F9,          XK_F10,         XK_F11,         XK_F12,
    XK_Insert,      XK_Delete,      XK_Home,        XK_End,
    XK_Page_Up,     XK_Page_Down,   XK_F13,         XK_F14,
    XK_F15
  };
int const NUM_FUNCKEY = sizeof(funckey_to_x) / sizeof(funckey_to_x[0]);

typedef struct {
    KeySym		normalSym;
    KeySym		keypadSym;
} darwinKeyPad_t;

static darwinKeyPad_t const normal_to_keypad[] = {
    { XK_0,         XK_KP_0 },
    { XK_1,         XK_KP_1 },
    { XK_2,         XK_KP_2 },
    { XK_3,         XK_KP_3 },
    { XK_4,         XK_KP_4 },
    { XK_5,         XK_KP_5 },
    { XK_6,         XK_KP_6 },
    { XK_7,         XK_KP_7 },
    { XK_8,         XK_KP_8 },
    { XK_9,         XK_KP_9 },
    { XK_equal,     XK_KP_Equal },
    { XK_asterisk,  XK_KP_Multiply },
    { XK_plus,      XK_KP_Add },
    { XK_comma,     XK_KP_Separator },
    { XK_minus,     XK_KP_Subtract },
    { XK_period,    XK_KP_Decimal },
    { XK_slash,     XK_KP_Divide }
};
int const NUM_KEYPAD = sizeof(normal_to_keypad) / sizeof(normal_to_keypad[0]);

static void DarwinBell( int loud, DeviceIntPtr pDevice, pointer ctrl, int fbclass) {
    // FIXME
}

static void DarwinChangeKeyboardControl( DeviceIntPtr device, KeybdCtrl *ctrl ) {
    // keyclick, bell volume / pitch, autorepead, LED's
}

static	CARD8 modMap[MAP_LENGTH];
static	KeySym map[256 * GLYPHS_PER_KEY];

//-----------------------------------------------------------------------------
// Data Stream Object
//	Can be configured to treat embedded "numbers" as being composed of
//	either 1, 2, or 4 bytes, apiece.
//-----------------------------------------------------------------------------
typedef struct _DataStream
{
    unsigned char const *data;
    unsigned char const *data_end;
    short number_size;  // Size in bytes of a "number" in the stream.
} DataStream;

static DataStream* new_data_stream( unsigned char const* data, int size )
{
    DataStream* s = (DataStream*)xalloc( sizeof(DataStream) );
    s->data = data;
    s->data_end = data + size;
    s->number_size = 1; // Default to byte-sized numbers.
    return s;
}

static void destroy_data_stream( DataStream* s )
{
    xfree(s);
}

static unsigned char get_byte( DataStream* s )
{
    assert(s->data + 1 <= s->data_end);
    return *s->data++;
}

static short get_word( DataStream* s )
{
    short hi, lo;
    assert(s->data + 2 <= s->data_end);
    hi = *s->data++;
    lo = *s->data++;
    return ((hi << 8) | lo);
}

static int get_dword( DataStream* s )
{
    int b1, b2, b3, b4;
    assert(s->data + 4 <= s->data_end);
    b4 = *s->data++;
    b3 = *s->data++;
    b2 = *s->data++;
    b1 = *s->data++;
    return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}

static int get_number( DataStream* s )
{
    switch (s->number_size) {
	case 4:  return get_dword(s);
	case 2:  return get_word(s);
	default: return get_byte(s);
    }
}

//-----------------------------------------------------------------------------
// Utility functions to help parse Darwin keymap
//-----------------------------------------------------------------------------

/*
 * bits_set
 * Calculate number of bits set in the modifier mask.
 */
static short bits_set( short mask )
{
    short n = 0;

    for ( ; mask != 0; mask >>= 1)
	if ((mask & 0x01) != 0)
	    n++;
    return n;
}

/*
 * parse_next_char_code
 * Read the next character code from the Darwin keymapping
 * and write it to the X keymap.
 */
static void parse_next_char_code(
    DataStream  *s,
    KeySym      *k )
{
    const short charSet = get_number(s);
    const short charCode = get_number(s);

    if (charSet == 0) {                 // ascii character
        if (charCode >= 0 && charCode < 256)
            *k = ascii_to_x[charCode];
    } else if (charSet == 0x01) {       // symbol character
        if (charCode >= MIN_SYMBOL &&
            charCode <= MIN_SYMBOL + NUM_SYMBOL)
            *k = symbol_to_x[charCode - MIN_SYMBOL];
    } else if (charSet == 0xFE) {       // function key
        if (charCode >= MIN_FUNCKEY &&
            charCode <= MIN_FUNCKEY + NUM_FUNCKEY)
            *k = funckey_to_x[charCode - MIN_FUNCKEY];
    }
}

/*
 * DarwinKeyboardInit
 *      Get the Darwin keyboard map and compute an equivalent
 *      X keyboard map and modifier map. Set the new keyboard
 *      device structure.
 */
void DarwinKeyboardInit(
    DeviceIntPtr        pDev )
{
    KeySym              *k;
    int                 i;
    short		numMods, numKeys, numPadKeys = 0;
    KeySymsRec          keySyms;
    NXKeyMapping        keyMap;
    DataStream		*keyMapStream;
    unsigned char const *numPadStart = 0;

    memset( modMap, NoSymbol, sizeof( modMap ) );
    memset( map, 0, sizeof( map ) );

    // Open a shared connection to the HID System.
    // Note that the Event Status Driver is really just a wrapper
    // for a kIOHIDParamConnectType connection.
    assert( dfb.hidParam = NXOpenEventStatus() );

    // get the Darwin keyboard map
    keyMap.size = NXKeyMappingLength( dfb.hidParam );
    keyMap.mapping = (char*) xalloc( keyMap.size );
    assert( NXGetKeyMapping( dfb.hidParam, &keyMap ));
    keyMapStream = new_data_stream( (unsigned char const*)keyMap.mapping,
                                    keyMap.size );

    // check the type of map
    if (get_word(keyMapStream)) {
        keyMapStream->number_size = 2;
        ErrorF("Current 16-bit keymapping may not be interpreted correctly.\n");
    }

    // Compute the modifier map and
    // insert X modifier KeySyms into keyboard map.
    numMods = get_number(keyMapStream);
    while (numMods-- > 0) {
        int	    	left = 1;                   // first keycode is left
        short const     charCode = get_number(keyMapStream);
        short           numKeyCodes = get_number(keyMapStream);
        if (charCode == NX_MODIFIERKEY_NUMERICPAD) {
            numPadStart = keyMapStream->data;
            numPadKeys = numKeyCodes;
        }
        while (numKeyCodes-- > 0) {
            const short keyCode = get_number(keyMapStream);
            if (charCode == NX_MODIFIERKEY_ALPHALOCK) {
                modMap[keyCode + MIN_KEYCODE] = LockMask;
                map[keyCode * GLYPHS_PER_KEY] = XK_Caps_Lock;
            } else if (charCode == NX_MODIFIERKEY_SHIFT) {
                modMap[keyCode + MIN_KEYCODE] = ShiftMask;
                map[keyCode * GLYPHS_PER_KEY] =
                        (left ? XK_Shift_L : XK_Shift_R);
            } else if (charCode == NX_MODIFIERKEY_CONTROL) {
                modMap[keyCode + MIN_KEYCODE] = ControlMask;
                map[keyCode * GLYPHS_PER_KEY] =
                        (left ? XK_Control_L : XK_Control_R);
            } else if (charCode == NX_MODIFIERKEY_ALTERNATE) {
                modMap[keyCode + MIN_KEYCODE] = AltMask;
                if (left) {
                    map[keyCode * GLYPHS_PER_KEY] = XK_Alt_L;
                    darwinKeyOptionL = keyCode + MIN_KEYCODE;
                } else
                    map[keyCode * GLYPHS_PER_KEY] = XK_Alt_R;
            } else if (charCode == NX_MODIFIERKEY_COMMAND) {
                modMap[keyCode + MIN_KEYCODE] = MetaMask;
                if (left) {
                    map[keyCode * GLYPHS_PER_KEY] = XK_Meta_L;
                    darwinKeyCommandL = keyCode + MIN_KEYCODE;
                } else
                    map[keyCode * GLYPHS_PER_KEY] = XK_Meta_R;
            } else if (charCode == NX_MODIFIERKEY_NUMERICPAD) {
                continue;
            } else if (charCode == NX_MODIFIERKEY_HELP) {
                map[keyCode * GLYPHS_PER_KEY] = XK_Help;
            } else {
                break;
            }
            left = 0;
        }
    }

    // Convert the Darwin keyboard map to an X keyboard map.
    // A key can have shifted and unshifted character codes.
    // Other modifiers are ignored although they are
    // present in the Darwin keyboard map.
    numKeys = get_number(keyMapStream);
    for (i = 0, k = map; i < numKeys; i++, k += GLYPHS_PER_KEY) {
        short const     charGenMask = get_number(keyMapStream);
        if (charGenMask != 0xFF) {              // is key bound?
            short       numKeyCodes = 1 << bits_set(charGenMask);

            // If alphalock and shift modifiers produce different codes,
            // we only need the shift case since X handles alphalock.
            if (charGenMask & 0x01 && charGenMask & 0x02) {
                // record unshifted case
                parse_next_char_code( keyMapStream, k );
                // skip alphalock case
                get_number(keyMapStream); get_number(keyMapStream);
                // record shifted case
                parse_next_char_code( keyMapStream, k+1 );
                if (k[1] == k[0]) k[1] = NoSymbol;
                numKeyCodes -= 3;
                // skip the rest
                while (numKeyCodes-- > 0) {
                    get_number(keyMapStream); get_number(keyMapStream);
                }

            // If alphalock and shift modifiers produce same code, use it.
            } else if (charGenMask & 0x03) {
                // record unshifted case
                parse_next_char_code( keyMapStream, k );
                // record shifted case
                parse_next_char_code( keyMapStream, k+1 );
                if (k[1] == k[0]) k[1] = NoSymbol;
                numKeyCodes -= 2;
                // skip the rest
                while (numKeyCodes-- > 0) {
                    get_number(keyMapStream); get_number(keyMapStream);
                }

            // If neither alphalock or shift produce characters,
            // use only one character code for this key,
            // but it can be a special character.
            } else {
                parse_next_char_code( keyMapStream, k );
                numKeyCodes--;
                while (numKeyCodes-- > 0) {     // skip the rest
                    get_number(keyMapStream); get_number(keyMapStream);
                
                }
            }
        }
    }

    // Now we have to go back through the list of keycodes that are on the
    // numeric keypad and update the X keymap.
    keyMapStream->data = numPadStart;
    while(numPadKeys-- > 0) {
        const short keyCode = get_number(keyMapStream);
        k = &map[keyCode * GLYPHS_PER_KEY];
        for (i = 0; i < NUM_KEYPAD; i++) {
            if (*k == normal_to_keypad[i].normalSym) {
                k[0] = normal_to_keypad[i].keypadSym;
                break;
            }
        }
    }

    // free Darwin keyboard map
    destroy_data_stream( keyMapStream );
    xfree( keyMap.mapping );

#ifdef DUMP_DARWIN_KEYMAP
    ErrorF("Darwin -> X converted keyboard map\n");
    for (i = 0, k = map; i < NX_NUMKEYCODES; i++, k += GLYPHS_PER_KEY) {
        int j;
        ErrorF("0x%02x:", i);
        for (j = 0; j < GLYPHS_PER_KEY; j++) {
            if (k[j] == NoSymbol) {
                ErrorF("\tNoSym");
            } else {
                ErrorF("\t0x%x", k[j]);
            }
        }
        ErrorF("\n");
    }
#endif

    keySyms.map        = map;
    keySyms.mapWidth   = GLYPHS_PER_KEY;
    keySyms.minKeyCode = MIN_KEYCODE;
    keySyms.maxKeyCode = MAX_KEYCODE;

    assert( InitKeyboardDeviceStruct( (DevicePtr)pDev, &keySyms, modMap,
                                      DarwinBell,
                                      DarwinChangeKeyboardControl ));
}

/*
 * LegalModifier
 * This allows the driver level to prevent some keys from being remapped
 * as modifier keys.
 * I have no idea why this is useful.
 */
Bool LegalModifier(unsigned int key, DevicePtr pDev)
{
    return 1;
}
