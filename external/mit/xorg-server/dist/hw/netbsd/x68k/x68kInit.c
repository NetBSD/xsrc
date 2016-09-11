/* $NetBSD: x68kInit.c,v 1.3 2016/09/11 03:55:57 tsutsui Exp $ */
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

/*
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

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

*******************************************************/

#include "Xos.h"
#include "x68k.h"
#include "mi.h"

static int nscreens;

void
OsVendorInit(void)
{
}

/*-------------------------------------------------------------------------
 * function "InitOutput"                                [ called by DIX ]
 *
 *  purpose:  initialize outputs ( screens )
 *  argument: (ScreenInfo *)pScreenInfo  : screen info struct
 *            (int)argc, (char *)argv    : standard arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
InitOutput(ScreenInfo *pScreenInfo, int argc, char *argv[])
{
    int i;
    X68kScreenRec *screen;
    X68kFbProcRec *fb;
    
    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    /* read configuration file */
    nscreens = x68kConfig();

    /* register pixmap formats */
    x68kRegisterPixmapFormats(pScreenInfo);
    
    /* open and initialize frame buffer for each screen */
    for (i = 0; i < nscreens; i++) {
        screen = x68kGetScreenRec(i);
        fb = x68kGetFbProcRec(i);
        if ( !(*fb->open)(screen) )
            return;
        if ( AddScreen(fb->init, argc, argv) < 0 )
            FatalError("AddScreen failed\n");
    }
}

/*-------------------------------------------------------------------------
 * function "InitInput"                                 [ called by DIX ]
 *
 *  purpose:  initialize inputs ( keyboard and mouse )
 *  argument: (int)argc, (char *)argv    : standard arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
InitInput(int argc, char *argv[]) 
{
    x68kPointerDevice = AddInputDevice(serverClient, x68kMouseProc, TRUE);
    x68kKeyboardDevice = AddInputDevice(serverClient, x68kKbdProc, TRUE);

    if ( !mieqInit() )
        FatalError("mieqInit failed\n");

    /* setup SIGIO handler for asynchronous event handling */
    (void)OsSignal(SIGIO, x68kSigIOHandler);
}

void
CloseInput(void)
{
    mieqFini();
}

/*-------------------------------------------------------------------------
 * function "AbortDDX"                                 [ called by OS ]
 *
 *  purpose:  free signal handler and close frame buffers
 *  argument: ExitCode
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
AbortDDX(enum ExitCode error)
{
    int i;
    X68kScreenRec *screen;
    X68kFbProcRec *fb;

    /* give up SIGIO handling */
    (void) OsSignal(SIGIO, SIG_IGN);

    /* close all frame buffers */
    for (i = 0; i < nscreens; i++) {
        screen = x68kGetScreenRec(i);
        fb = x68kGetFbProcRec(i);
        (*fb->close)(screen);
    }
}

/*-------------------------------------------------------------------------
 * function "ddxGiveUp"                                 [ called by DIX ]
 *
 *  purpose:  do nothing but call AbortDDX.
 *  argument: nothing
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
ddxGiveUp(enum ExitCode error)
{
    AbortDDX(error);
}

/*-------------------------------------------------------------------------
 * function "ddxProcessArgument"                        [ called by OS ]
 *
 *  purpose:  process X68k dependent arguments
 *            currently only `x68kconfig' will be recognized.
 *  argument: (int)argc, (char **)argv: standard C arguments
 *            (int)i                  : index of current argument
 *  returns:  number of arguments eaten
 *-----------------------------------------------------------------------*/
int
ddxProcessArgument(int argc, char *argv[], int i)
{

    if (strcmp(argv[i], "-x68kconfig") == 0) {
        if (++i >= argc)
            UseMsg();
        configFilename = strdup(argv[i]);
        return 2;
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * function "ddxUseMsg"                                 [ called by OS ]
 *
 *  purpose:  print X68k dependent usage
 *  argument: nothing
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
ddxUseMsg(void)
{
    ErrorF("\nX68k dependent options\n");
    ErrorF("-x68kconfig filename   specify configuration file\n");
}

_X_EXPORT void
OsVendorFatalError(const char *f, va_list args)
{
}

/* EOF x68kInit.c */
