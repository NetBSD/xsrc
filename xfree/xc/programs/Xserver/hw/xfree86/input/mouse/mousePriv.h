/* $XFree86: xc/programs/Xserver/hw/xfree86/input/mouse/mousePriv.h,v 1.5 2000/05/31 07:15:06 eich Exp $ */
/*
 * Copyright (c) 1997-1999 by The XFree86 Project, Inc.
 */

#ifndef _X_MOUSEPRIV_H
#define _X_MOUSEPRIV_H

#include "mouse.h"
#include "xf86Xinput.h"                                                                                              
/* Private interface for the mouse driver. */

typedef struct {
    const char *	name;
    int			class;
    const char **	defaults;
    MouseProtocolID	id;
} MouseProtocolRec, *MouseProtocolPtr;

/* mouse proto flags */
#define MPF_NONE		0x00
#define MPF_SAFE		0x01

/* pnp.c */
int MouseGetPnpProtocol(InputInfoPtr pInfo);

#endif /* _X_MOUSE_H */
