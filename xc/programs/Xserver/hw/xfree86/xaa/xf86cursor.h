/* $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xf86cursor.h,v 3.5.2.1 1998/02/01 16:42:14 robin Exp $ */
typedef struct {
    int Flags;
    int MaxWidth;
    int MaxHeight;
    int CursorDataX;
    int CursorDataY;
    void (*SetCursorColors)(int bg, int fg);
    void (*SetCursorPosition)(int x, int y, int xorigin, int yorigin);
    void (*LoadCursorImage)(void *bits, int xorigin, int yorigin);
    void (*HideCursor)();
    void (*ShowCursor)();
    int  (*GetInstalledColormaps)();
} XAACursorInfoRecType;

enum {
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP		= 0x1,
    HARDWARE_CURSOR_PROGRAMMED_BITS		= 0x2,
    HARDWARE_CURSOR_BIT_ORDER_MSBFIRST		= 0x4,
    HARDWARE_CURSOR_PROGRAMMED_ORIGIN		= 0x8,
    HARDWARE_CURSOR_PLANES_NOT_INTERLEAVED	= 0x10,
    HARDWARE_CURSOR_AND_SOURCE_WITH_MASK	= 0x20,
    HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK	= 0x40,
    USE_HARDWARE_CURSOR				= 0x80,
    HARDWARE_CURSOR_INT64_BIT_FORMAT		= 0x100,
    HARDWARE_CURSOR_LONG_BIT_FORMAT		= 0x200,
    HARDWARE_CURSOR_SHORT_BIT_FORMAT		= 0x400,
    HARDWARE_CURSOR_CHAR_BIT_FORMAT		= 0x800,
    HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE	= 0x1000,
    HARDWARE_CURSOR_SYNC_NEEDED			= 0x2000,
    HARDWARE_CURSOR_INVERT_MASK			= 0x4000
};

extern XAACursorInfoRecType XAACursorInfoRec;
