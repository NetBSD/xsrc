/* $XConsortium: XItest.h,v 1.1 94/01/29 15:13:44 rws Exp $ */
/*
 * Copyright 1992 by Hewlett-Packard.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and Hewlett-Packard
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and Hewlett-Packard
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#define	KeyMask		(1 << 0)
#define	BtnMask		(1 << 1)
#define	ValMask		(1 << 2)
#define	AnyMask		(1 << 3)
#define	KFeedMask	(1 << 4)
#define	PFeedMask	(1 << 5)
#define	IFeedMask	(1 << 6)
#define	SFeedMask	(1 << 7)
#define	BFeedMask	(1 << 8)
#define	LFeedMask	(1 << 9)
#define	NKeysMask	(1 << 10)
#define	NBtnsMask	(1 << 11)
#define	ModMask		(1 << 12)
#define	NFeedMask	(1 << 13)
#define	NValsMask	(1 << 14)
#define	DCtlMask	(1 << 15)
#define	DModMask	(1 << 16)
#define	DValMask	(1 << 17)
#define	FCtlMask	(1 << 18)
#define	FocusMask	(1 << 19)
#define	NDvCtlMask	(1 << 20)

typedef struct _ExtDeviceInfo
    {
    XDevice *Key;
    XDevice *Button;
    XDevice *Valuator;
    XDevice *Any;
    XDevice *KbdFeed;
    XDevice *PtrFeed;
    XDevice *IntFeed;
    XDevice *StrFeed;
    XDevice *BelFeed;
    XDevice *LedFeed;
    XDevice *NoKeys;
    XDevice *NoButtons;
    XDevice *Mod;
    XDevice *NoFeedback;
    XDevice *NoValuators;
    XDevice *DvCtl;
    XDevice *DvMod;
    XDevice *DvVal;
    XDevice *Focus;
    XDevice *NDvCtl;
    } ExtDeviceInfo;

