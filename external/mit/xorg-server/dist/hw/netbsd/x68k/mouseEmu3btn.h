/*
 * Copyright (c) 1999-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/*
 * 3 button emulation stuff
 * pulled from xf86-input-mouse/dist/src/mouse.h
 */

typedef struct _MouseEmu3btn {
    DeviceIntPtr device;

    int emulateState;
    Bool emulate3Buttons;
    Bool emulate3ButtonsSoft;
    Bool emulate3Pending;
    int emulate3Timeout;
    CARD32 emulate3Expires;
} MouseEmu3btn, *MouseEmu3btnPtr;

/* default timeout value for 3 button emulation */
#define EMU3B_DEF_TIMEOUT	100	/* ms */

void Emulate3ButtonsEnable(MouseEmu3btnPtr pEmu3btn, DeviceIntPtr device, int timeout);
void Emulate3ButtonsQueueEvent(MouseEmu3btnPtr pEmu3btn, int type, int buttons, int bmask);
