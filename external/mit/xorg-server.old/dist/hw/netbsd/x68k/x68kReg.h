/* $NetBSD: x68kReg.h,v 1.4 2020/11/05 16:06:08 tsutsui Exp $ */
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
    volatile uint16_t r00, r01, r02, r03, r04, r05, r06, r07;
    volatile uint16_t r08, r09, r10, r11, r12, r13, r14, r15;
    volatile uint16_t r16, r17, r18, r19, r20, r21, r22, r23;
    uint8_t pad0[0x450];
    volatile uint16_t ctrl;
    uint8_t pad1[0x1b7e];
} Crtc;

typedef struct {
    uint16_t r00, r01, r02, r03, r04, r05, r06, r07;
    uint16_t r08, r09, r10, r11, r12, r13, r14, r15;
    uint16_t r16, r17, r18, r19, r20, r21, r22, r23;
    uint16_t ctrl;
} X68kCrtc;

/*
 * video controller
 */
typedef struct {
    volatile uint16_t r0;
    uint8_t pad0[0xfe];
    volatile uint16_t r1;
    uint8_t pad1[0xfe];
    volatile uint16_t r2;
    uint8_t pad2[0x19fe];
} Videoc;

typedef struct {
    uint16_t r0;
    uint16_t r1;
    uint16_t r2;
} X68kVideoc;

/* system port */
typedef struct {
    volatile uint16_t r1, r2, r3, r4;
    uint16_t pad0[2];
    volatile uint16_t r5, r6;
    uint16_t pad[0x1ff0];
} Sysport;

/*
 * control registers
 */
typedef struct {
    Crtc crtc;
    volatile uint16_t gpal[256];    /* graphic palette */
    volatile uint16_t tpal[256];     /* text palette */
    Videoc videoc;
    uint16_t pad0[0xa000];
    Sysport sysport;
} FbReg;

typedef struct {
    X68kCrtc crtc;
    X68kVideoc videoc;
    char dotClock;
} X68kFbReg;

/* EOF x68kReg.h */
