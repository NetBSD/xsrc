/* $XFree86: xc/programs/Xserver/include/dixevents.h,v 3.1 1996/04/15 11:34:24 dawes Exp $ */
/************************************************************

Copyright 1996 by Thomas E. Dickey <dickey@clark.net>

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the above listed
copyright holder(s) not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifndef DIXEVENTS_H
#define DIXEVENTS_H

Mask
GetNextEventMask(
#if NeedFunctionPrototypes
	void
#endif
	);

void
SetMaskForEvent(
#if NeedFunctionPrototypes
	Mask                   /* mask */,
	int                    /* event */
#endif
	);

void
SetCriticalEvent(
#if NeedFunctionPrototypes
	int                    /* event */
#endif
	);

void
ConfineCursorToWindow(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	Bool                   /* generateEvents */,
	Bool                   /* confineToScreen */
#endif
	);

Bool
PointerConfinedToScreen(
#if NeedFunctionPrototypes
	void
#endif
	);

Bool
IsParent(
#if NeedFunctionPrototypes
	WindowPtr              /* a */,
	WindowPtr              /* b */
#endif
	);

WindowPtr
GetCurrentRootWindow(
#if NeedFunctionPrototypes
	void
#endif
	);

WindowPtr
GetSpriteWindow(
#if NeedFunctionPrototypes
	void
#endif
	);

CursorPtr
GetSpriteCursor(
#if NeedFunctionPrototypes
	void
#endif
	);

void
GetSpritePosition(
#if NeedFunctionPrototypes
	int *                  /* px */,
	int *                  /* py */
#endif
	);

void
NoticeEventTime(
#if NeedFunctionPrototypes
	xEvent *               /* xE */
#endif
	);

void
EnqueueEvent(
#if NeedFunctionPrototypes
	xEvent *               /* xE */,
	DeviceIntPtr           /* device */,
	int                    /* count */
#endif
	);

void
ComputeFreezes(
#if NeedFunctionPrototypes
	void
#endif
	);

void
CheckGrabForSyncs(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* thisDev */,
	Bool                   /* thisMode */,
	Bool                   /* otherMode */
#endif
	);

void
ActivatePointerGrab(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* mouse */,
	GrabPtr                /* grab */,
	TimeStamp              /* time */,
	Bool                   /* autoGrab */
#endif
	);

void
DeactivatePointerGrab(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* mouse */
#endif
	);

void
ActivateKeyboardGrab(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* keybd */,
	GrabPtr                /* grab */,
	TimeStamp              /* time */,
	Bool                   /* passive */
#endif
	);

void
DeactivateKeyboardGrab(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* keybd */
#endif
	);

void
AllowSome(
#if NeedFunctionPrototypes
	ClientPtr              /* client */,
	TimeStamp              /* time */,
	DeviceIntPtr           /* thisDev */,
	int                    /* newState */
#endif
	);

int
ProcAllowEvents(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

void
ReleaseActiveGrabs(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
TryClientEvents (
#if NeedFunctionPrototypes
	ClientPtr              /* client */,
	xEvent *               /* pEvents */,
	int                    /* count */,
	Mask                   /* mask */,
	Mask                   /* filter */,
	GrabPtr                /* grab */
#endif
	);

int
DeliverEventsToWindow(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	xEvent *               /* pEvents */,
	int                    /* count */,
	Mask                   /* filter */,
	GrabPtr                /* grab */,
	int                    /* mskidx */
#endif
	);

int
MaybeDeliverEventsToClient(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	xEvent *               /* pEvents */,
	int                    /* count */,
	Mask                   /* filter */,
	ClientPtr              /* dontClient */
#endif
	);

int
DeliverDeviceEvents(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	xEvent *               /* xE */,
	GrabPtr                /* grab */,
	WindowPtr              /* stopAt */,
	DeviceIntPtr           /* dev */,
	int                    /* count */
#endif
	);

int
DeliverEvents(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	xEvent *               /* xE */,
	int                    /* count */,
	WindowPtr              /* otherParent */
#endif
	);

void
WindowsRestructured(
#if NeedFunctionPrototypes
	void
#endif
	);

void
DefineInitialRootWindow(
#if NeedFunctionPrototypes
	WindowPtr              /* win */
#endif
	);

void
WindowHasNewCursor(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */
#endif
	);

void
NewCurrentScreen(
#if NeedFunctionPrototypes
	ScreenPtr              /* newScreen */,
	int                    /* x */,
	int                    /* y */
#endif
	);

int
ProcWarpPointer(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

Bool
CheckDeviceGrabs(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* device */,
	xEvent *               /* xE */,
	int                    /* checkFirst */,
	int                    /* count */
#endif
	);

void
DeliverFocusedEvent(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* keybd */,
	xEvent *               /* xE */,
	WindowPtr              /* window */,
	int                    /* count */
#endif
	);

void
DeliverGrabbedEvent(
#if NeedFunctionPrototypes
	xEvent *               /* xE */,
	DeviceIntPtr           /* thisDev */,
	Bool                   /* deactivateGrab */,
	int                    /* count */
#endif
	);

void
#ifdef XKB
CoreProcessKeyboardEvent (
#else
ProcessKeyboardEvent (
#endif
#if NeedFunctionPrototypes
	xEvent *               /* xE */,
	DeviceIntPtr           /* keybd */,
	int                    /* count */
#endif
	);

void
#ifdef XKB
CoreProcessPointerEvent (
#else
ProcessPointerEvent (
#endif
#if NeedFunctionPrototypes
	xEvent *               /* xE */,
	DeviceIntPtr           /* mouse */,
	int                    /* count */
#endif
	);

void
RecalculateDeliverableEvents(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */
#endif
	);

int
OtherClientGone(
#if NeedFunctionPrototypes
	pointer                /* value */,
	XID                    /* id */
#endif
	);

int
EventSelectForWindow(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	ClientPtr              /* client */,
	Mask                   /* mask */
#endif
	);

int
EventSuppressForWindow(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	ClientPtr              /* client */,
	Mask                   /* mask */,
	Bool *                 /* checkOptional */
#endif
	);

void
DoFocusEvents(
#if NeedFunctionPrototypes
	DeviceIntPtr           /* dev */,
	WindowPtr              /* fromWin */,
	WindowPtr              /* toWin */,
	int                    /* mode */
#endif
	);

int
SetInputFocus(
#if NeedFunctionPrototypes
	ClientPtr              /* client */,
	DeviceIntPtr           /* dev */,
	Window                 /* focusID */,
	CARD8                  /* revertTo */,
	Time                   /* ctime */,
	Bool                   /* followOK */
#endif
	);

int
ProcSetInputFocus(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcGetInputFocus(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcGrabPointer(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcChangeActivePointerGrab(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcUngrabPointer(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
GrabDevice(
#if NeedFunctionPrototypes
	ClientPtr              /* client */,
	DeviceIntPtr           /* dev */,
	unsigned               /* this_mode */,
	unsigned               /* other_mode */,
	Window                 /* grabWindow */,
	unsigned               /* ownerEvents */,
	Time                   /* ctime */,
	Mask                   /* mask */,
	CARD8 *                /* status */
#endif
	);

int
ProcGrabKeyboard(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcUngrabKeyboard(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcQueryPointer(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

void
InitEvents(
#if NeedFunctionPrototypes
	void
#endif
	);

int
ProcSendEvent(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcUngrabKey(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcGrabKey(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcGrabButton(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

int
ProcUngrabButton(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

void
DeleteWindowFromAnyEvents(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	Bool                   /* freeResources */
#endif
	);

void
CheckCursorConfinement(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */
#endif
	);

Mask
EventMaskForClient(
#if NeedFunctionPrototypes
	WindowPtr              /* pWin */,
	ClientPtr              /* client */
#endif
	);

int
ProcRecolorCursor(
#if NeedFunctionPrototypes
	ClientPtr              /* client */
#endif
	);

void
WriteEventsToClient(
#if NeedFunctionPrototypes
	ClientPtr              /* pClient */,
	int                    /* count */,
	xEvent *               /* events */
#endif
	);

#endif /* DIXEVENTS_H */
