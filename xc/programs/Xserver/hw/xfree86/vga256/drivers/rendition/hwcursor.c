/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/hwcursor.c,v 1.1.2.5 1999/07/30 13:19:21 hohndel Exp $ */
/*
 * includes
 */

#define NEED_EVENTS
#include <X.h>
#include "Xproto.h"
#include <misc.h>
#include <input.h>
#include <cursorstr.h>
#include <regionstr.h>
#include <scrnintstr.h>
#include <servermd.h>
#include <windowstr.h>
#include "xf86.h"
#include "inputstr.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86cursor.h"
#include "mipointer.h"

#include "vga.h"
#include "vgaPCI.h"

#include "vtypes.h"
#include "vramdac.h"



/*
 * defines 
 */

#ifdef DEBUG
#undef DEBUG
#endif

/* use a 64x64 cursor, 32x32 otherwise */
#define BIGCURSOR 1

/*
 * extern functions
 */

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();



/*
 * extern data
 */

extern vgaHWCursorRec vgaHWCursor;
extern struct v_board_t BOARD;



/*
 * local function prototypes
 */

void RENDITIONSetCursorColors(int bg, int fg);
void RENDITIONSetCursorPosition(int x, int y, int xorigin, int yorigin);
void RENDITIONHideCursor(void);
void RENDITIONShowCursor(void);
void RENDITIONLoadCursorImage(register unsigned char *src, int xorigin, int yorigin);



/*
 * This is top-level initialization funtion
 */
void RENDITIONHwCursorInit(void)
{
#ifdef DEBUG
    ErrorF( "RENDITION: RENDITIONHwCursorInit called\n");
#endif

#ifdef BIGCURSOR
    XAACursorInfoRec.MaxWidth=64;
    XAACursorInfoRec.MaxHeight=64;
#else
    XAACursorInfoRec.MaxWidth=32;
    XAACursorInfoRec.MaxHeight=32;
#endif
    XAACursorInfoRec.Flags=USE_HARDWARE_CURSOR
                          |HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
                          |HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
                          |HARDWARE_CURSOR_PROGRAMMED_ORIGIN
                          |HARDWARE_CURSOR_AND_SOURCE_WITH_MASK
                          |HARDWARE_CURSOR_CHAR_BIT_FORMAT
                          |HARDWARE_CURSOR_PROGRAMMED_BITS;
    XAACursorInfoRec.SetCursorColors=RENDITIONSetCursorColors;
    XAACursorInfoRec.SetCursorPosition=RENDITIONSetCursorPosition;
    XAACursorInfoRec.LoadCursorImage=RENDITIONLoadCursorImage;
    XAACursorInfoRec.HideCursor=RENDITIONHideCursor;
    XAACursorInfoRec.ShowCursor=RENDITIONShowCursor;

    vgaHWCursor.Init=XAACursorInit;
    vgaHWCursor.Initialized=TRUE;
    vgaHWCursor.Restore=XAARestoreCursor;
    vgaHWCursor.Warp=XAAWarpCursor;
    vgaHWCursor.QueryBestSize=XAAQueryBestSize;
}



/*
 * local functions
 */


void RENDITIONShowCursor(void)
{
#ifdef DEBUG
    ErrorF( "RENDITION: ShowCursor called\n");
#endif

    /* enable cursor - X11 mode */
    v_enablecursor(&BOARD, V_XCURSOR, 
#ifdef BIGCURSOR
        V_CURSOR64
#else 
        V_CURSOR32
#endif
        );
}



void RENDITIONHideCursor(void)
{
#ifdef DEBUG
    ErrorF( "RENDITION: HideCursor called\n");
#endif

    /* Disable cursor */
    v_enablecursor(&BOARD, V_NOCURSOR, 0);
}



void RENDITIONSetCursorPosition(int x, int y, int xorigin, int yorigin)
{
#ifdef DEBUG
    ErrorF( "RENDITION: SetCursorPosition(%d, %d, %d, %d) called\n", 
        x, y, xorigin, yorigin);
#endif

    v_movecursor(&BOARD, x, y, xorigin, yorigin);
}



void RENDITIONSetCursorColors(int bg, int fg)
{
#ifdef DEBUG
    ErrorF( "RENDITION: SetCursorColors(%x, %x) called\n", fg, bg);
#endif

    v_setcursorcolor(&BOARD, fg, bg);
}



void RENDITIONLoadCursorImage(register unsigned char *src, int xorigin, int yorigin)
{
#ifdef DEBUG
    ErrorF( "RENDITION: loadcursor called\n");
#endif

    v_loadcursor(&BOARD, 
#ifdef BIGCURSOR
        V_CURSOR64, 
#else
        V_CURSOR32, 
#endif
        (vu8 *)src);
}



/*
 * end of file hwcursor.c
 */
