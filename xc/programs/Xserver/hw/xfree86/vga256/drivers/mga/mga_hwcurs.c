/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_hwcurs.c,v 1.1.2.4 1998/12/20 11:12:07 hohndel Exp $ */
/*
 * Copyright 1994 by Robin Cutshaw <robin@XFree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ROBIN CUTSHAW BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * Modified for TVP3026 by Harald Koenig <koenig@tat.physik.uni-tuebingen.de>
 * 
 * Modified for MGA Millennium by Xavier Ducoin <xavier@rd.lectra.fr>
 *
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

#include "mga.h"

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

extern vgaHWCursorRec vgaHWCursor;

/*
 * This is top-level initialization funtion called from mga_driver
 */
Bool MGAHwCursorInit()
{
    int RamUsed;

    if (!MGAdac.isHwCursor) 
	return FALSE;
    RamUsed = vga256InfoRec.virtualY * vga256InfoRec.displayWidth 
                 * vgaBitsPerPixel / 8;
#ifdef DEBUG
    ErrorF("Ram %d Used %d Cursor %d\n",vga256InfoRec.videoRam*1024,RamUsed,
    		MGAdac.CursorMaxWidth * MGAdac.CursorMaxHeight*
		vgaBitsPerPixel / 8);
#endif
    if ((vga256InfoRec.videoRam * 1024 - RamUsed) < 
	    (MGAdac.CursorMaxWidth * MGAdac.CursorMaxHeight 
		* vgaBitsPerPixel / 8))
        return FALSE;
    XAACursorInfoRec.MaxWidth = MGAdac.CursorMaxWidth;
    XAACursorInfoRec.MaxHeight = MGAdac.CursorMaxHeight;
    XAACursorInfoRec.Flags = MGAdac.CursorFlags;
    XAACursorInfoRec.SetCursorColors = MGAdac.SetCursorColors;
    XAACursorInfoRec.SetCursorPosition = MGAdac.SetCursorPosition;
    XAACursorInfoRec.LoadCursorImage = MGAdac.LoadCursorImage;
    XAACursorInfoRec.HideCursor = MGAdac.HideCursor;
    XAACursorInfoRec.ShowCursor = MGAdac.ShowCursor;

    vgaHWCursor.Init = XAACursorInit;
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Restore = XAARestoreCursor;
    vgaHWCursor.Warp = XAAWarpCursor;
    vgaHWCursor.QueryBestSize = XAAQueryBestSize;

    return TRUE;
}
