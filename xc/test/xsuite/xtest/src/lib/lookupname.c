/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: lookupname.c,v 1.4 94/04/17 21:00:51 rws Exp $
 */
#include "xtest.h"
#include "Xlib.h"
#include "Xproto.h"
#include "extensions/XIproto.h"
#include "Xutil.h"
#include "Xatom.h"
#include "xtestlib.h"

#define	   XInputNumErrors	5
extern int XInputMajorOpcode;
extern int XInputFirstError;
extern int XInputFirstEvent;

static char	buf[100];
static char	*bp;

struct valname S_bool[] = {
	True, "True",
	False, "False",
};
int 	NS_bool = NELEM(S_bool);

/*
 * Return a character representation of the given bool value.
 */
char *
boolname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_bool; vp < &S_bool[NELEM(S_bool)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_eventmask[] = {
	NoEventMask, "NoEventMask",
	KeyPressMask, "KeyPressMask",
	KeyReleaseMask, "KeyReleaseMask",
	ButtonPressMask, "ButtonPressMask",
	ButtonReleaseMask, "ButtonReleaseMask",
	EnterWindowMask, "EnterWindowMask",
	LeaveWindowMask, "LeaveWindowMask",
	PointerMotionMask, "PointerMotionMask",
	PointerMotionHintMask, "PointerMotionHintMask",
	Button1MotionMask, "Button1MotionMask",
	Button2MotionMask, "Button2MotionMask",
	Button3MotionMask, "Button3MotionMask",
	Button4MotionMask, "Button4MotionMask",
	Button5MotionMask, "Button5MotionMask",
	ButtonMotionMask, "ButtonMotionMask",
	KeymapStateMask, "KeymapStateMask",
	ExposureMask, "ExposureMask",
	VisibilityChangeMask, "VisibilityChangeMask",
	StructureNotifyMask, "StructureNotifyMask",
	ResizeRedirectMask, "ResizeRedirectMask",
	SubstructureNotifyMask, "SubstructureNotifyMask",
	SubstructureRedirectMask, "SubstructureRedirectMask",
	FocusChangeMask, "FocusChangeMask",
	PropertyChangeMask, "PropertyChangeMask",
	ColormapChangeMask, "ColormapChangeMask",
	OwnerGrabButtonMask, "OwnerGrabButtonMask",
};
int 	NS_eventmask = NELEM(S_eventmask);

/*
 * Return a character representation of the given eventmask value.
 */
char *
eventmaskname(val)
unsigned long	val;
{
struct valname *vp;
int	size;
unsigned long	masks;

	size = 0;
	for (vp = S_eventmask; vp < &S_eventmask[NELEM(S_eventmask)]; vp++)
		size += strlen(vp->name)+1;

	bp = (char*)malloc(size+sizeof("UNDEFINED BITS(0xffffffff)"));
	if (bp == (char*)0) {
		/* Just return the value */
		sprintf(buf, "(0x%x)", val);
		return(buf);
	}

	bp[0] = 0;
	masks = 0;
	for (vp = S_eventmask; vp < &S_eventmask[NELEM(S_eventmask)]; vp++) {
		if (vp->val & val) {
			if (*bp != 0)
				strcat(bp, "|");
			strcat(bp, vp->name);
			masks |= vp->val;
		}
	}
	/*
	 * Any bits set in val that are not in masks have been
	 * missed by the above.
	 */
	if (val & (~masks)) {
		if (*bp != 0)
			strcat(bp, "|");
		sprintf(buf, "UNDEFINED BITS(0x%x)", val & (~masks));
		strcat(bp, buf);
	}
	return(bp);
}

struct valname S_event[] = {
	KeyPress, "KeyPress",
	KeyRelease, "KeyRelease",
	ButtonPress, "ButtonPress",
	ButtonRelease, "ButtonRelease",
	MotionNotify, "MotionNotify",
	EnterNotify, "EnterNotify",
	LeaveNotify, "LeaveNotify",
	FocusIn, "FocusIn",
	FocusOut, "FocusOut",
	KeymapNotify, "KeymapNotify",
	Expose, "Expose",
	GraphicsExpose, "GraphicsExpose",
	NoExpose, "NoExpose",
	VisibilityNotify, "VisibilityNotify",
	CreateNotify, "CreateNotify",
	DestroyNotify, "DestroyNotify",
	UnmapNotify, "UnmapNotify",
	MapNotify, "MapNotify",
	MapRequest, "MapRequest",
	ReparentNotify, "ReparentNotify",
	ConfigureNotify, "ConfigureNotify",
	ConfigureRequest, "ConfigureRequest",
	GravityNotify, "GravityNotify",
	ResizeRequest, "ResizeRequest",
	CirculateNotify, "CirculateNotify",
	CirculateRequest, "CirculateRequest",
	PropertyNotify, "PropertyNotify",
	SelectionClear, "SelectionClear",
	SelectionRequest, "SelectionRequest",
	SelectionNotify, "SelectionNotify",
	ColormapNotify, "ColormapNotify",
	ClientMessage, "ClientMessage",
	MappingNotify, "MappingNotify",
	LASTEvent, "LASTEvent",
};
int 	NS_event = NELEM(S_event);

struct valname XI_event[] = {
	XI_DeviceValuator, "DeviceValuator",
	XI_DeviceKeyPress, "DeviceKeyPress",
	XI_DeviceKeyRelease, "DeviceKeyRelease",
	XI_DeviceButtonPress, "DeviceButtonPress",
	XI_DeviceButtonRelease, "DeviceButtonRelease",
	XI_DeviceMotionNotify, "DeviceMotionNotify",
	XI_DeviceFocusIn, "DeviceFocusIn",
	XI_DeviceFocusOut, "DeviceFocusOut",
	XI_ProximityIn, "ProximityIn",
	XI_ProximityOut, "ProximityOut",
	XI_DeviceStateNotify, "DeviceStateNotify",
	XI_DeviceMappingNotify, "DeviceMappingNotify",
	XI_ChangeDeviceNotify, "ChangeDeviceNotify",
	XI_DeviceKeystateNotify, "DeviceKeyStateNotify",
	XI_DeviceButtonstateNotify, "DeviceButtonStateNotify",
};
int 	NXI_event = NELEM(XI_event);

/*
 * Return a character representation of the given event value.
 */
char *
eventname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_event; vp < &S_event[NELEM(S_event)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	for (vp = XI_event; vp < &XI_event[NELEM(XI_event)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_keymask[] = {
	ShiftMask, "ShiftMask",
	LockMask, "LockMask",
	ControlMask, "ControlMask",
	Mod1Mask, "Mod1Mask",
	Mod2Mask, "Mod2Mask",
	Mod3Mask, "Mod3Mask",
	Mod4Mask, "Mod4Mask",
	Mod5Mask, "Mod5Mask",
};
int 	NS_keymask = NELEM(S_keymask);

/*
 * Return a character representation of the given keymask value.
 */
char *
keymaskname(val)
unsigned long	val;
{
struct valname *vp;
int	size;
unsigned long	masks;

	size = 0;
	for (vp = S_keymask; vp < &S_keymask[NELEM(S_keymask)]; vp++)
		size += strlen(vp->name)+1;

	bp = (char*)malloc(size+sizeof("UNDEFINED BITS(0xffffffff)"));
	if (bp == (char*)0) {
		/* Just return the value */
		sprintf(buf, "(0x%x)", val);
		return(buf);
	}

	bp[0] = 0;
	masks = 0;
	for (vp = S_keymask; vp < &S_keymask[NELEM(S_keymask)]; vp++) {
		if (vp->val & val) {
			if (*bp != 0)
				strcat(bp, "|");
			strcat(bp, vp->name);
			masks |= vp->val;
		}
	}
	/*
	 * Any bits set in val that are not in masks have been
	 * missed by the above.
	 */
	if (val & (~masks)) {
		if (*bp != 0)
			strcat(bp, "|");
		sprintf(buf, "UNDEFINED BITS(0x%x)", val & (~masks));
		strcat(bp, buf);
	}
	return(bp);
}

struct valname S_modifier[] = {
	ShiftMapIndex, "ShiftMapIndex",
	LockMapIndex, "LockMapIndex",
	ControlMapIndex, "ControlMapIndex",
	Mod1MapIndex, "Mod1MapIndex",
	Mod2MapIndex, "Mod2MapIndex",
	Mod3MapIndex, "Mod3MapIndex",
	Mod4MapIndex, "Mod4MapIndex",
	Mod5MapIndex, "Mod5MapIndex",
};
int 	NS_modifier = NELEM(S_modifier);

/*
 * Return a character representation of the given modifier value.
 */
char *
modifiername(val)
int 	val;
{
struct valname *vp;

	for (vp = S_modifier; vp < &S_modifier[NELEM(S_modifier)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_buttonmask[] = {
	Button1Mask, "Button1Mask",
	Button2Mask, "Button2Mask",
	Button3Mask, "Button3Mask",
	Button4Mask, "Button4Mask",
	Button5Mask, "Button5Mask",
};
int 	NS_buttonmask = NELEM(S_buttonmask);

/*
 * Return a character representation of the given buttonmask value.
 */
char *
buttonmaskname(val)
unsigned long	val;
{
struct valname *vp;
int	size;
unsigned long	masks;

	size = 0;
	for (vp = S_buttonmask; vp < &S_buttonmask[NELEM(S_buttonmask)]; vp++)
		size += strlen(vp->name)+1;

	bp = (char*)malloc(size+sizeof("UNDEFINED BITS(0xffffffff)"));
	if (bp == (char*)0) {
		/* Just return the value */
		sprintf(buf, "(0x%x)", val);
		return(buf);
	}

	bp[0] = 0;
	masks = 0;
	for (vp = S_buttonmask; vp < &S_buttonmask[NELEM(S_buttonmask)]; vp++) {
		if (vp->val & val) {
			if (*bp != 0)
				strcat(bp, "|");
			strcat(bp, vp->name);
			masks |= vp->val;
		}
	}
	/*
	 * Any bits set in val that are not in masks have been
	 * missed by the above.
	 */
	if (val & (~masks)) {
		if (*bp != 0)
			strcat(bp, "|");
		sprintf(buf, "UNDEFINED BITS(0x%x)", val & (~masks));
		strcat(bp, buf);
	}
	return(bp);
}

struct valname S_button[] = {
	Button1, "Button1",
	Button2, "Button2",
	Button3, "Button3",
	Button4, "Button4",
	Button5, "Button5",
};
int 	NS_button = NELEM(S_button);

/*
 * Return a character representation of the given button value.
 */
char *
buttonname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_button; vp < &S_button[NELEM(S_button)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_notifymode[] = {
	NotifyNormal, "NotifyNormal",
	NotifyGrab, "NotifyGrab",
	NotifyUngrab, "NotifyUngrab",
	NotifyWhileGrabbed, "NotifyWhileGrabbed",
};
int 	NS_notifymode = NELEM(S_notifymode);

/*
 * Return a character representation of the given notifymode value.
 */
char *
notifymodename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_notifymode; vp < &S_notifymode[NELEM(S_notifymode)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_notifydetail[] = {
	NotifyAncestor, "NotifyAncestor",
	NotifyVirtual, "NotifyVirtual",
	NotifyInferior, "NotifyInferior",
	NotifyNonlinear, "NotifyNonlinear",
	NotifyNonlinearVirtual, "NotifyNonlinearVirtual",
	NotifyPointer, "NotifyPointer",
	NotifyPointerRoot, "NotifyPointerRoot",
};
int 	NS_notifydetail = NELEM(S_notifydetail);

/*
 * Return a character representation of the given notifydetail value.
 */
char *
notifydetailname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_notifydetail; vp < &S_notifydetail[NELEM(S_notifydetail)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_visibility[] = {
	VisibilityUnobscured, "VisibilityUnobscured",
	VisibilityPartiallyObscured, "VisibilityPartiallyObscured",
	VisibilityFullyObscured, "VisibilityFullyObscured",
};
int 	NS_visibility = NELEM(S_visibility);

/*
 * Return a character representation of the given visibility value.
 */
char *
visibilityname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_visibility; vp < &S_visibility[NELEM(S_visibility)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_grabreply[] = {
	GrabSuccess, "GrabSuccess",
	AlreadyGrabbed, "AlreadyGrabbed",
	GrabInvalidTime, "GrabInvalidTime",
	GrabNotViewable, "GrabNotViewable",
	GrabFrozen, "GrabFrozen",
};
int 	NS_grabreply = NELEM(S_grabreply);

/*
 * Return a character representation of the given grabreply value.
 */
char *
grabreplyname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_grabreply; vp < &S_grabreply[NELEM(S_grabreply)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_alloweventmode[] = {
	AsyncPointer, "AsyncPointer",
	SyncPointer, "SyncPointer",
	ReplayPointer, "ReplayPointer",
	AsyncKeyboard, "AsyncKeyboard",
	SyncKeyboard, "SyncKeyboard",
	ReplayKeyboard, "ReplayKeyboard",
	AsyncBoth, "AsyncBoth",
	SyncBoth, "SyncBoth",
};
int 	NS_alloweventmode = NELEM(S_alloweventmode);

/*
 * Return a character representation of the given alloweventmode value.
 */
char *
alloweventmodename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_alloweventmode; vp < &S_alloweventmode[NELEM(S_alloweventmode)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_revertto[] = {
	RevertToNone, "RevertToNone",
	RevertToPointerRoot, "RevertToPointerRoot",
	RevertToParent, "RevertToParent",
};
int 	NS_revertto = NELEM(S_revertto);

/*
 * Return a character representation of the given revertto value.
 */
char *
reverttoname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_revertto; vp < &S_revertto[NELEM(S_revertto)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_error[] = {
	Success, "Success",
	BadRequest, "BadRequest",
	BadValue, "BadValue",
	BadWindow, "BadWindow",
	BadPixmap, "BadPixmap",
	BadAtom, "BadAtom",
	BadCursor, "BadCursor",
	BadFont, "BadFont",
	BadMatch, "BadMatch",
	BadDrawable, "BadDrawable",
	BadAccess, "BadAccess",
	BadAlloc, "BadAlloc",
	BadColor, "BadColor",
	BadGC, "BadGC",
	BadIDChoice, "BadIDChoice",
	BadName, "BadName",
	BadLength, "BadLength",
	BadImplementation, "BadImplementation",
};
int 	NS_error = NELEM(S_error);

struct valname S_XIerror[] = {
	0, "BadDevice",
	1, "BadEvent",
	2, "BadMode",
	3, "DeviceBusy",
	4, "BadClass",
};
int 	NS_XIerror = NELEM(S_XIerror);

/*
 * Return a character representation of the given error value.
 */
char *
errorname(val)
int val;
{
struct valname *vp;

	if (val < FirstExtensionError)
	    for (vp = S_error; vp < &S_error[NELEM(S_error)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	    }
	else if (val >= XInputFirstError &&
	    val < XInputFirstError + XInputNumErrors)
	    for (vp = S_XIerror; vp < &S_XIerror[NELEM(S_XIerror)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	    }
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_class[] = {
	InputOutput, "InputOutput",
	InputOnly, "InputOnly",
};
int 	NS_class = NELEM(S_class);

/*
 * Return a character representation of the given class value.
 */
char *
classname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_class; vp < &S_class[NELEM(S_class)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_bitgravity[] = {
	ForgetGravity, "ForgetGravity",
	NorthWestGravity, "NorthWestGravity",
	NorthGravity, "NorthGravity",
	NorthEastGravity, "NorthEastGravity",
	WestGravity, "WestGravity",
	CenterGravity, "CenterGravity",
	EastGravity, "EastGravity",
	SouthWestGravity, "SouthWestGravity",
	SouthGravity, "SouthGravity",
	SouthEastGravity, "SouthEastGravity",
	StaticGravity, "StaticGravity",
};
int 	NS_bitgravity = NELEM(S_bitgravity);

/*
 * Return a character representation of the given bitgravity value.
 */
char *
bitgravityname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_bitgravity; vp < &S_bitgravity[NELEM(S_bitgravity)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_wingravity[] = {
	UnmapGravity, "UnmapGravity",
	NorthWestGravity, "NorthWestGravity",
	NorthGravity, "NorthGravity",
	NorthEastGravity, "NorthEastGravity",
	WestGravity, "WestGravity",
	CenterGravity, "CenterGravity",
	EastGravity, "EastGravity",
	SouthWestGravity, "SouthWestGravity",
	SouthGravity, "SouthGravity",
	SouthEastGravity, "SouthEastGravity",
	StaticGravity, "StaticGravity",
};
int 	NS_wingravity = NELEM(S_wingravity);

/*
 * Return a character representation of the given wingravity value.
 */
char *
wingravityname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_wingravity; vp < &S_wingravity[NELEM(S_wingravity)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_backingstore[] = {
	NotUseful, "NotUseful",
	WhenMapped, "WhenMapped",
	Always, "Always",
};
int 	NS_backingstore = NELEM(S_backingstore);

/*
 * Return a character representation of the given backingstore value.
 */
char *
backingstorename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_backingstore; vp < &S_backingstore[NELEM(S_backingstore)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_gcfunction[] = {
	GXclear, "GXclear",
	GXand, "GXand",
	GXandReverse, "GXandReverse",
	GXcopy, "GXcopy",
	GXandInverted, "GXandInverted",
	GXnoop, "GXnoop",
	GXxor, "GXxor",
	GXor, "GXor",
	GXnor, "GXnor",
	GXequiv, "GXequiv",
	GXinvert, "GXinvert",
	GXorReverse, "GXorReverse",
	GXcopyInverted, "GXcopyInverted",
	GXorInverted, "GXorInverted",
	GXnand, "GXnand",
	GXset, "GXset",
};
int 	NS_gcfunction = NELEM(S_gcfunction);

/*
 * Return a character representation of the given gcfunction value.
 */
char *
gcfunctionname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_gcfunction; vp < &S_gcfunction[NELEM(S_gcfunction)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_linestyle[] = {
	LineSolid, "LineSolid",
	LineOnOffDash, "LineOnOffDash",
	LineDoubleDash, "LineDoubleDash",
};
int 	NS_linestyle = NELEM(S_linestyle);

/*
 * Return a character representation of the given linestyle value.
 */
char *
linestylename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_linestyle; vp < &S_linestyle[NELEM(S_linestyle)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_capstyle[] = {
	CapNotLast, "CapNotLast",
	CapButt, "CapButt",
	CapRound, "CapRound",
	CapProjecting, "CapProjecting",
};
int 	NS_capstyle = NELEM(S_capstyle);

/*
 * Return a character representation of the given capstyle value.
 */
char *
capstylename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_capstyle; vp < &S_capstyle[NELEM(S_capstyle)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_joinstyle[] = {
	JoinMiter, "JoinMiter",
	JoinRound, "JoinRound",
	JoinBevel, "JoinBevel",
};
int 	NS_joinstyle = NELEM(S_joinstyle);

/*
 * Return a character representation of the given joinstyle value.
 */
char *
joinstylename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_joinstyle; vp < &S_joinstyle[NELEM(S_joinstyle)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_fillstyle[] = {
	FillSolid, "FillSolid",
	FillTiled, "FillTiled",
	FillStippled, "FillStippled",
	FillOpaqueStippled, "FillOpaqueStippled",
};
int 	NS_fillstyle = NELEM(S_fillstyle);

/*
 * Return a character representation of the given fillstyle value.
 */
char *
fillstylename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_fillstyle; vp < &S_fillstyle[NELEM(S_fillstyle)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_displayclass[] = {
	StaticGray, "StaticGray",
	GrayScale, "GrayScale",
	StaticColor, "StaticColor",
	PseudoColor, "PseudoColor",
	TrueColor, "TrueColor",
	DirectColor, "DirectColor",
};
int 	NS_displayclass = NELEM(S_displayclass);

/*
 * Return a character representation of the given displayclass value.
 */
char *
displayclassname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_displayclass; vp < &S_displayclass[NELEM(S_displayclass)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_proto[] = {
	X_CreateWindow, "X_CreateWindow",
	X_ChangeWindowAttributes, "X_ChangeWindowAttributes",
	X_GetWindowAttributes, "X_GetWindowAttributes",
	X_DestroyWindow, "X_DestroyWindow",
	X_DestroySubwindows, "X_DestroySubwindows",
	X_ChangeSaveSet, "X_ChangeSaveSet",
	X_ReparentWindow, "X_ReparentWindow",
	X_MapWindow, "X_MapWindow",
	X_MapSubwindows, "X_MapSubwindows",
	X_UnmapWindow, "X_UnmapWindow",
	X_UnmapSubwindows, "X_UnmapSubwindows",
	X_ConfigureWindow, "X_ConfigureWindow",
	X_CirculateWindow, "X_CirculateWindow",
	X_GetGeometry, "X_GetGeometry",
	X_QueryTree, "X_QueryTree",
	X_InternAtom, "X_InternAtom",
	X_GetAtomName, "X_GetAtomName",
	X_ChangeProperty, "X_ChangeProperty",
	X_DeleteProperty, "X_DeleteProperty",
	X_GetProperty, "X_GetProperty",
	X_ListProperties, "X_ListProperties",
	X_SetSelectionOwner, "X_SetSelectionOwner",
	X_GetSelectionOwner, "X_GetSelectionOwner",
	X_ConvertSelection, "X_ConvertSelection",
	X_SendEvent, "X_SendEvent",
	X_GrabPointer, "X_GrabPointer",
	X_UngrabPointer, "X_UngrabPointer",
	X_GrabButton, "X_GrabButton",
	X_UngrabButton, "X_UngrabButton",
	X_ChangeActivePointerGrab, "X_ChangeActivePointerGrab",
	X_GrabKeyboard, "X_GrabKeyboard",
	X_UngrabKeyboard, "X_UngrabKeyboard",
	X_GrabKey, "X_GrabKey",
	X_UngrabKey, "X_UngrabKey",
	X_AllowEvents, "X_AllowEvents",
	X_GrabServer, "X_GrabServer",
	X_UngrabServer, "X_UngrabServer",
	X_QueryPointer, "X_QueryPointer",
	X_GetMotionEvents, "X_GetMotionEvents",
	X_TranslateCoords, "X_TranslateCoords",
	X_WarpPointer, "X_WarpPointer",
	X_SetInputFocus, "X_SetInputFocus",
	X_GetInputFocus, "X_GetInputFocus",
	X_QueryKeymap, "X_QueryKeymap",
	X_OpenFont, "X_OpenFont",
	X_CloseFont, "X_CloseFont",
	X_QueryFont, "X_QueryFont",
	X_QueryTextExtents, "X_QueryTextExtents",
	X_ListFonts, "X_ListFonts",
	X_ListFontsWithInfo, "X_ListFontsWithInfo",
	X_SetFontPath, "X_SetFontPath",
	X_GetFontPath, "X_GetFontPath",
	X_CreatePixmap, "X_CreatePixmap",
	X_FreePixmap, "X_FreePixmap",
	X_CreateGC, "X_CreateGC",
	X_ChangeGC, "X_ChangeGC",
	X_CopyGC, "X_CopyGC",
	X_SetDashes, "X_SetDashes",
	X_SetClipRectangles, "X_SetClipRectangles",
	X_FreeGC, "X_FreeGC",
	X_ClearArea, "X_ClearArea",
	X_CopyArea, "X_CopyArea",
	X_CopyPlane, "X_CopyPlane",
	X_PolyPoint, "X_PolyPoint",
	X_PolyLine, "X_PolyLine",
	X_PolySegment, "X_PolySegment",
	X_PolyRectangle, "X_PolyRectangle",
	X_PolyArc, "X_PolyArc",
	X_FillPoly, "X_FillPoly",
	X_PolyFillRectangle, "X_PolyFillRectangle",
	X_PolyFillArc, "X_PolyFillArc",
	X_PutImage, "X_PutImage",
	X_GetImage, "X_GetImage",
	X_PolyText8, "X_PolyText8",
	X_PolyText16, "X_PolyText16",
	X_ImageText8, "X_ImageText8",
	X_ImageText16, "X_ImageText16",
	X_CreateColormap, "X_CreateColormap",
	X_FreeColormap, "X_FreeColormap",
	X_CopyColormapAndFree, "X_CopyColormapAndFree",
	X_InstallColormap, "X_InstallColormap",
	X_UninstallColormap, "X_UninstallColormap",
	X_ListInstalledColormaps, "X_ListInstalledColormaps",
	X_AllocColor, "X_AllocColor",
	X_AllocNamedColor, "X_AllocNamedColor",
	X_AllocColorCells, "X_AllocColorCells",
	X_AllocColorPlanes, "X_AllocColorPlanes",
	X_FreeColors, "X_FreeColors",
	X_StoreColors, "X_StoreColors",
	X_StoreNamedColor, "X_StoreNamedColor",
	X_QueryColors, "X_QueryColors",
	X_LookupColor, "X_LookupColor",
	X_CreateCursor, "X_CreateCursor",
	X_CreateGlyphCursor, "X_CreateGlyphCursor",
	X_FreeCursor, "X_FreeCursor",
	X_RecolorCursor, "X_RecolorCursor",
	X_QueryBestSize, "X_QueryBestSize",
	X_QueryExtension, "X_QueryExtension",
	X_ListExtensions, "X_ListExtensions",
	X_ChangeKeyboardMapping, "X_ChangeKeyboardMapping",
	X_GetKeyboardMapping, "X_GetKeyboardMapping",
	X_ChangeKeyboardControl, "X_ChangeKeyboardControl",
	X_GetKeyboardControl, "X_GetKeyboardControl",
	X_Bell, "X_Bell",
	X_ChangePointerControl, "X_ChangePointerControl",
	X_GetPointerControl, "X_GetPointerControl",
	X_SetScreenSaver, "X_SetScreenSaver",
	X_GetScreenSaver, "X_GetScreenSaver",
	X_ChangeHosts, "X_ChangeHosts",
	X_ListHosts, "X_ListHosts",
	X_SetAccessControl, "X_SetAccessControl",
	X_SetCloseDownMode, "X_SetCloseDownMode",
	X_KillClient, "X_KillClient",
	X_RotateProperties, "X_RotateProperties",
	X_ForceScreenSaver, "X_ForceScreenSaver",
	X_SetPointerMapping, "X_SetPointerMapping",
	X_GetPointerMapping, "X_GetPointerMapping",
	X_SetModifierMapping, "X_SetModifierMapping",
	X_GetModifierMapping, "X_GetModifierMapping",
	X_NoOperation, "X_NoOperation",
};
int 	NS_proto = NELEM(S_proto);

struct valname XI_proto[] = {
	X_GetExtensionVersion, "X_GetExtensionVersion",
	X_ListInputDevices, "X_ListInputDevices",
	X_OpenDevice, "X_OpenDevice",
	X_CloseDevice, "X_CloseDevice",
	X_SetDeviceMode, "X_SetDeviceMode",
	X_SelectExtensionEvent, "X_SelectExtensionEvent",
	X_GetSelectedExtensionEvents, "X_GetSelectedExtensionEvents",
	X_ChangeDeviceDontPropagateList, "X_ChangeDeviceDontPropagateList",
	X_GetDeviceDontPropagateList, "X_GetDeviceDontPropagateList",
	X_GetDeviceMotionEvents, "X_GetDeviceMotionEvents ",
	X_ChangeKeyboardDevice, "X_ChangeKeyboardDevice",
	X_ChangePointerDevice, "X_ChangePointerDevice",
	X_GrabDevice, "X_GrabDevice",
	X_UngrabDevice, "X_UngrabDevice",
	X_GrabDeviceKey, "X_GrabDeviceKey",
	X_UngrabDeviceKey, "X_UngrabDeviceKey",
	X_GrabDeviceButton, "X_GrabDeviceButton",
	X_UngrabDeviceButton, "X_UngrabDeviceButton",
	X_AllowDeviceEvents, "X_AllowDeviceEvents",
	X_GetDeviceFocus, "X_GetDeviceFocus",
	X_SetDeviceFocus, "X_SetDeviceFocus",
	X_GetFeedbackControl, "X_GetFeedbackControl",
	X_ChangeFeedbackControl, "X_ChangeFeedbackControl",
	X_GetDeviceKeyMapping, "X_GetDeviceKeyMapping",
	X_ChangeDeviceKeyMapping, "X_ChangeDeviceKeyMapping",
	X_GetDeviceModifierMapping, "X_GetDeviceModifierMapping",
	X_SetDeviceModifierMapping, "X_SetDeviceModifierMapping",
	X_GetDeviceButtonMapping, "X_GetDeviceButtonMapping",
	X_SetDeviceButtonMapping, "X_SetDeviceButtonMapping",
	X_QueryDeviceState, "X_QueryDeviceState",
	X_SendExtensionEvent, "X_SendExtensionEvent",
	X_DeviceBell, "X_DeviceBell",
	X_SetDeviceValuators, "X_SetDeviceValuators",
	X_GetDeviceControl, "X_GetDeviceControl",
	X_ChangeDeviceControl, "X_ChangeDeviceControl",
};
int 	NXI_proto = NELEM(XI_proto);

/*
 * Return a character representation of the given proto value.
 */
char *
protoname(val)
int val;
{
struct valname *vp;

	if (val < X_NoOperation)
	    for (vp = S_proto; vp < &S_proto[NELEM(S_proto)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	    }
	else if ((val & 0x0ff) == XInputMajorOpcode)
	    for (vp = XI_proto; vp < &XI_proto[NELEM(XI_proto)]; vp++) {
		if (vp->val == val >> 8)
			return(vp->name);
	    }
	sprintf(buf, "UNDEFINED (major=%d, minor=%d)", val & 0x0ff, val >> 8);
	return(buf);
}

struct valname S_atom[] = {
	XA_PRIMARY, "XA_PRIMARY",
	XA_SECONDARY, "XA_SECONDARY",
	XA_ARC, "XA_ARC",
	XA_ATOM, "XA_ATOM",
	XA_BITMAP, "XA_BITMAP",
	XA_CARDINAL, "XA_CARDINAL",
	XA_COLORMAP, "XA_COLORMAP",
	XA_CURSOR, "XA_CURSOR",
	XA_CUT_BUFFER0, "XA_CUT_BUFFER0",
	XA_CUT_BUFFER1, "XA_CUT_BUFFER1",
	XA_CUT_BUFFER2, "XA_CUT_BUFFER2",
	XA_CUT_BUFFER3, "XA_CUT_BUFFER3",
	XA_CUT_BUFFER4, "XA_CUT_BUFFER4",
	XA_CUT_BUFFER5, "XA_CUT_BUFFER5",
	XA_CUT_BUFFER6, "XA_CUT_BUFFER6",
	XA_CUT_BUFFER7, "XA_CUT_BUFFER7",
	XA_DRAWABLE, "XA_DRAWABLE",
	XA_FONT, "XA_FONT",
	XA_INTEGER, "XA_INTEGER",
	XA_PIXMAP, "XA_PIXMAP",
	XA_POINT, "XA_POINT",
	XA_RECTANGLE, "XA_RECTANGLE",
	XA_RESOURCE_MANAGER, "XA_RESOURCE_MANAGER",
	XA_RGB_COLOR_MAP, "XA_RGB_COLOR_MAP",
	XA_RGB_BEST_MAP, "XA_RGB_BEST_MAP",
	XA_RGB_BLUE_MAP, "XA_RGB_BLUE_MAP",
	XA_RGB_DEFAULT_MAP, "XA_RGB_DEFAULT_MAP",
	XA_RGB_GRAY_MAP, "XA_RGB_GRAY_MAP",
	XA_RGB_GREEN_MAP, "XA_RGB_GREEN_MAP",
	XA_RGB_RED_MAP, "XA_RGB_RED_MAP",
	XA_STRING, "XA_STRING",
	XA_VISUALID, "XA_VISUALID",
	XA_WINDOW, "XA_WINDOW",
	XA_WM_COMMAND, "XA_WM_COMMAND",
	XA_WM_HINTS, "XA_WM_HINTS",
	XA_WM_CLIENT_MACHINE, "XA_WM_CLIENT_MACHINE",
	XA_WM_ICON_NAME, "XA_WM_ICON_NAME",
	XA_WM_ICON_SIZE, "XA_WM_ICON_SIZE",
	XA_WM_NAME, "XA_WM_NAME",
	XA_WM_NORMAL_HINTS, "XA_WM_NORMAL_HINTS",
	XA_WM_SIZE_HINTS, "XA_WM_SIZE_HINTS",
	XA_WM_ZOOM_HINTS, "XA_WM_ZOOM_HINTS",
	XA_MIN_SPACE, "XA_MIN_SPACE",
	XA_NORM_SPACE, "XA_NORM_SPACE",
	XA_MAX_SPACE, "XA_MAX_SPACE",
	XA_END_SPACE, "XA_END_SPACE",
	XA_SUPERSCRIPT_X, "XA_SUPERSCRIPT_X",
	XA_SUPERSCRIPT_Y, "XA_SUPERSCRIPT_Y",
	XA_SUBSCRIPT_X, "XA_SUBSCRIPT_X",
	XA_SUBSCRIPT_Y, "XA_SUBSCRIPT_Y",
	XA_UNDERLINE_POSITION, "XA_UNDERLINE_POSITION",
	XA_UNDERLINE_THICKNESS, "XA_UNDERLINE_THICKNESS",
	XA_STRIKEOUT_ASCENT, "XA_STRIKEOUT_ASCENT",
	XA_STRIKEOUT_DESCENT, "XA_STRIKEOUT_DESCENT",
	XA_ITALIC_ANGLE, "XA_ITALIC_ANGLE",
	XA_X_HEIGHT, "XA_X_HEIGHT",
	XA_QUAD_WIDTH, "XA_QUAD_WIDTH",
	XA_WEIGHT, "XA_WEIGHT",
	XA_POINT_SIZE, "XA_POINT_SIZE",
	XA_RESOLUTION, "XA_RESOLUTION",
	XA_COPYRIGHT, "XA_COPYRIGHT",
	XA_NOTICE, "XA_NOTICE",
	XA_FONT_NAME, "XA_FONT_NAME",
	XA_FAMILY_NAME, "XA_FAMILY_NAME",
	XA_FULL_NAME, "XA_FULL_NAME",
	XA_CAP_HEIGHT, "XA_CAP_HEIGHT",
	XA_WM_CLASS, "XA_WM_CLASS",
	XA_WM_TRANSIENT_FOR, "XA_WM_TRANSIENT_FOR",
};
int 	NS_atom = NELEM(S_atom);

/*
 * Return a character representation of the given atom value.
 */
char *
atomname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_atom; vp < &S_atom[NELEM(S_atom)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_mapstate[] = {
	IsUnmapped, "IsUnmapped",
	IsUnviewable, "IsUnviewable",
	IsViewable, "IsViewable",
};
int 	NS_mapstate = NELEM(S_mapstate);

/*
 * Return a character representation of the given mapstate value.
 */
char *
mapstatename(val)
int 	val;
{
struct valname *vp;

	for (vp = S_mapstate; vp < &S_mapstate[NELEM(S_mapstate)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

struct valname S_visualmask[] = {
	VisualNoMask, "VisualNoMask",
	VisualIDMask, "VisualIDMask",
	VisualScreenMask, "VisualScreenMask",
	VisualDepthMask, "VisualDepthMask",
	VisualClassMask, "VisualClassMask",
	VisualRedMaskMask, "VisualRedMaskMask",
	VisualGreenMaskMask, "VisualGreenMaskMask",
	VisualBlueMaskMask, "VisualBlueMaskMask",
	VisualColormapSizeMask, "VisualColormapSizeMask",
	VisualBitsPerRGBMask, "VisualBitsPerRGBMask",
};
int 	NS_visualmask = NELEM(S_visualmask);

/*
 * Return a character representation of the given visualmask value.
 */
char *
visualmaskname(val)
unsigned long	val;
{
struct valname *vp;
int	size;
unsigned long	masks;

	size = 0;
	for (vp = S_visualmask; vp < &S_visualmask[NELEM(S_visualmask)]; vp++)
		size += strlen(vp->name)+1;

	bp = (char*)malloc(size+sizeof("UNDEFINED BITS(0xffffffff)"));
	if (bp == (char*)0) {
		/* Just return the value */
		sprintf(buf, "(0x%x)", val);
		return(buf);
	}

	bp[0] = 0;
	masks = 0;
	for (vp = S_visualmask; vp < &S_visualmask[NELEM(S_visualmask)]; vp++) {
		if (vp->val & val) {
			if (*bp != 0)
				strcat(bp, "|");
			strcat(bp, vp->name);
			masks |= vp->val;
		}
	}
	/*
	 * Any bits set in val that are not in masks have been
	 * missed by the above.
	 */
	if (val & (~masks)) {
		if (*bp != 0)
			strcat(bp, "|");
		sprintf(buf, "UNDEFINED BITS(0x%x)", val & (~masks));
		strcat(bp, buf);
	}
	return(bp);
}

struct valname S_contexterror[] = {
	XCSUCCESS, "XCSUCCESS",
	XCNOMEM, "XCNOMEM",
	XCNOENT, "XCNOENT",
};
int 	NS_contexterror = NELEM(S_contexterror);

/*
 * Return a character representation of the given contexterror value.
 */
char *
contexterrorname(val)
int 	val;
{
struct valname *vp;

	for (vp = S_contexterror; vp < &S_contexterror[NELEM(S_contexterror)]; vp++) {
		if (vp->val == val)
			return(vp->name);
	}
	sprintf(buf, "UNDEFINED (%d)", val);
	return(buf);
}

