/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/solx86/solx86_mouse.c,v 1.2 1999/06/12 15:37:12 dawes Exp $ */

/*
 * Copyright 1999 by The XFree86 Project, Inc.
 */

#include "X.h"
#include "xf86.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"

static int
SupportedInterfaces(void)
{
    /* XXX This needs to be checked. */
    return MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_AUTO | MSE_XPS2;
}

OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
    OSMouseInfoPtr p;

    p = xcalloc(sizeof(OSMouseInfoRec), 1);
    if (!p)
	return NULL;
    p->SupportedInterfaces = SupportedInterfaces;
    return p;
}

