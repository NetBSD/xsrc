/* $NetBSD: x68kFb.c,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
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

static void x68kRegSetup(X68kScreenRec *pPriv);

DevPrivateKeyRec x68kScreenPrivateKeyRec;

/*-------------------------------------------------------------------------
 * function "x68kFbCommonOpen"
 *
 *  purpose:  open and map frame buffer, and obtain some FB-specific
 *            infomation. then set controller to approppriate mode as
 *            configured in X68kConfig. 
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *            (const char *)device   : name of frame buffer device
 *  returns:  (Bool): TRUE  if succeeded
 *                    FALSE otherwise
 *-----------------------------------------------------------------------*/
Bool
x68kFbCommonOpen(X68kScreenRec *pPriv, const char *device)
{
    struct grfinfo gi;
    
    /* open frame buffer */
    if ( ( pPriv->fd = open(device, O_RDWR, 0)) < 0) {
        Error( "Can't open frame buffer" );
        return FALSE;
    }
    /* get frame buffer infomation */
    if ( ioctl( pPriv->fd, GRFIOCGINFO, &gi ) == -1 ) {
        Error( "Can't get grfinfo" );
        return FALSE;
    }
    pPriv->mapsize = gi.gd_regsize + gi.gd_fbsize;
    
    /* map control registers and frame buffer */
    pPriv->reg = (FbReg *)mmap(0, pPriv->mapsize, PROT_READ | PROT_WRITE,
                               MAP_FILE | MAP_SHARED, pPriv->fd, 0 );
    if ( pPriv->reg == (FbReg *)-1) {
        Error( "Can't map frame buffer" );
        return FALSE;
    }
    pPriv->fb = (uint8_t *)((uint32_t)pPriv->reg + gi.gd_regsize);

    x68kRegSetup( pPriv );

    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "x68kFbCommonClose"
 *
 *  purpose:  unmap and close frame buffer, and
 *            change video mode to normal ITE mode
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
x68kFbCommonClose(X68kScreenRec *pPriv)
{
    X68kFbReg graphNone = {
        { 137,14, 28, 124,
          567, 5, 40, 552,
           27, 0,  0,   0,
            0, 0,  0,   0,
            0, 0,  0,   0,
       0x0416, 0,  0,   0, 0 },
        { 0x0004, 0x21e4, 0x0020 },
        0
    };
    /* change video mode */
    pPriv->x68kreg = graphNone;
    x68kRegSetup(pPriv);
    
    /* unmap and close frame buffer */
    if ( munmap(__UNVOLATILE(pPriv->reg), pPriv->mapsize) == -1 )
        Error("Can't unmap frame buffer");
    close(pPriv->fd);
}

/*-------------------------------------------------------------------------
 * function "x68kRegSetup"
 *
 *  purpose:  set up CRT controller and Video controller
 *            with register values in pPriv->x68kreg
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
#define CRTCSET(r) pPriv->reg->crtc.r = pPriv->x68kreg.crtc.r
#define VIDEOCSET(r) pPriv->reg->videoc.r = pPriv->x68kreg.videoc.r

static void
x68kRegSetup(X68kScreenRec *pPriv)
{
    u_short pr20 = pPriv->reg->crtc.r20;

    /* timing registers */
    if ( (pr20 & 0x0003) < (pPriv->x68kreg.crtc.r20 & 0x0003) ||
         ( (pr20 & 0x0003) == (pPriv->x68kreg.crtc.r20 & 0x0003) &&
           (pr20 & 0x0010) < (pPriv->x68kreg.crtc.r20 & 0x0010) ) ) {
        
        /* to higher resolution */
        CRTCSET(r00); CRTCSET(r01); CRTCSET(r02);CRTCSET(r03);
        CRTCSET(r04); CRTCSET(r05); CRTCSET(r06);CRTCSET(r07);
        CRTCSET(r20);
    } else {
        /* to lower resolution */
        CRTCSET(r20); CRTCSET(r01); CRTCSET(r02);CRTCSET(r03);
        CRTCSET(r04); CRTCSET(r05); CRTCSET(r06);CRTCSET(r07);
        CRTCSET(r00);
    }
    CRTCSET(r08);

    /* scroll registers */
    CRTCSET(r12); CRTCSET(r13); CRTCSET(r14);CRTCSET(r15);
    CRTCSET(r16); CRTCSET(r17); CRTCSET(r18);CRTCSET(r19);

    /* mode controlling registers */
    VIDEOCSET(r0); VIDEOCSET(r1); VIDEOCSET(r2);

    /* dot clock bit */
    pPriv->reg->sysport.r4 = (pPriv->x68kreg.dotClock)? 0x000e: 0x000c;
}

/*-------------------------------------------------------------------------
 * function "x68kSaveScreen"                      [ screen function ]
 *
 *  purpose:  activate/deactivate screen saver
 *  argument: (ScreenPtr)ipScreen : target screen to save
 *            (int)mode          : on/off
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
Bool
x68kSaveScreen(ScreenPtr pScreen, Bool on)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(pScreen);
    static int status = FALSE;
    static u_short r2;

    if (on == SCREEN_SAVER_ON || on == SCREEN_SAVER_CYCLE) {
        if (!status) {
            r2 = pPriv->reg->videoc.r2;
            pPriv->reg->videoc.r2 = 0x0000;
            status = TRUE;
        }
    } else {
        if (status) {
            pPriv->reg->videoc.r2 = r2;
            status = FALSE;
        }
    }
    return TRUE;
}

/* EOF x68kFb.c */
