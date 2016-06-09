/* $NetBSD: x68kReg.h,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
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
 * CRT controller
 */
typedef struct {
    u_short r00, r01, r02, r03, r04, r05, r06, r07;
    u_short r08, r09, r10, r11, r12, r13, r14, r15;
    u_short r16, r17, r18, r19, r20, r21, r22, r23;
    char pad0[0x450];
    u_short ctrl;
    char pad1[0x1b7e];
} Crtc;

typedef struct {
    u_short r00, r01, r02, r03, r04, r05, r06, r07;
    u_short r08, r09, r10, r11, r12, r13, r14, r15;
    u_short r16, r17, r18, r19, r20, r21, r22, r23;
    u_short ctrl;
} X68kCrtc;
    
/*
 * video controller
 */
typedef struct {
    u_short r0;
    char pad0[0xfe];
    u_short r1;
    char pad1[0xfe];
    u_short r2;
    char pad2[0x19fe];
} Videoc;

typedef struct {
    u_short r0;
    u_short r1;
    u_short r2;
} X68kVideoc;

/* system port */
typedef struct {
    u_short r1, r2, r3, r4;
    u_short pad0[2];
    u_short r5, r6;
    u_short pad[0x1ff0];
} Sysport;

/*
 * control registers
 */
typedef struct {
    Crtc crtc;
    u_short gpal[256];    /* graphic palette */
    u_short tpal[256];     /* text palette */
    Videoc videoc;
    u_short pad0[0xa000];
    Sysport sysport;
} FbReg;

typedef struct {
    X68kCrtc crtc;
    X68kVideoc videoc;
    char dotClock;
} X68kFbReg;

/* EOF x68kReg.h */
