/* $NetBSD: x68kText.c,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
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

#include "x68k.h"
#include "mi.h"
#include "micmap.h"
#include "fb.h"

/*-------------------------------------------------------------------------
 * function "x68kTextOpen"                          [ X68kFBProc function ]
 *
 *  purpose:  call common frame buffer opening procedure
 *            and enable TVRAM simultaneous access mode
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *  returns:  (Bool): TRUE  if succeeded
 *                    FALSE otherwise
 *-----------------------------------------------------------------------*/
static u_short r21;
static u_short tpal0;
static u_short tpal15;

Bool
x68kTextOpen(X68kScreenRec *pPriv)
{
    if( !x68kFbCommonOpen(pPriv, "/dev/grf0") )
        return FALSE;

    /* enable TVRAM simultaneous access mode */
    r21 = pPriv->reg->crtc.r21;
    pPriv->reg->crtc.r21 = 0x01f0;

    /* initialize scroll registers */
    pPriv->reg->crtc.r10 = pPriv->reg->crtc.r11 = 0;

    tpal0 = pPriv->reg->tpal[0];
    tpal15 = pPriv->reg->tpal[15];
    
    pPriv->reg->tpal[0] = 0;
    pPriv->reg->tpal[15] = 0xFFFE;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "x68kTextClose"                        [ X68kFBProc function ]
 *
 *  purpose:  close text frame buffer
 *  argument: nothing
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
x68kTextClose(X68kScreenRec *pPriv)
{
    pPriv->reg->crtc.r21 = r21;  /* recover TVRAM mode */
    pPriv->reg->tpal[0] = tpal0;
    pPriv->reg->tpal[15] = tpal15;
    x68kFbCommonClose(pPriv);
}

/*-------------------------------------------------------------------------
 * function "x68kTextInit"                     [ called by DIX AddScreen ]
 *
 *  purpose:  initialize text frame buffer
 *  argument: (int)screen              : screen index
 *            (ScreenPtr)pScreen       : DIX screen record
 *            (int)argc, (char **)argv : standard C arguments
 *  returns:  (Bool) TRUE  if succeeded
 *                   FALSE otherwise
 *-----------------------------------------------------------------------*/
Bool
x68kTextInit(int screen, ScreenPtr pScreen, int argc, char *argv[])
{
    X68kScreenRec *pPriv;

    /* get private screen record set by X68KConfig */
    pPriv = x68kGetScreenRecByType(X68K_FB_TEXT);

    if ( !dixRegisterPrivateKey(&x68kScreenPrivateKeyRec, PRIVATE_SCREEN, 0) ) {
            Error("dixRegisterPrivateKey failed");
            return FALSE;
    }
    x68kSetScreenPrivate(pScreen, pPriv);

    if ( !fbScreenInit(pScreen, pPriv->fb,
                        pPriv->scr_width, pPriv->scr_height,
                        pPriv->dpi, pPriv->dpi, pPriv->fb_width, 1) )
        return FALSE;
    pScreen->whitePixel = 1;
    pScreen->blackPixel = 0;
    if ( !miDCInitialize(pScreen, &x68kPointerScreenFuncs) )
        return FALSE;
    if ( !miCreateDefColormap(pScreen) )
        return FALSE;
    pScreen->SaveScreen = x68kSaveScreen;
    
    return TRUE;
}

/* EOF x68kText.c */
