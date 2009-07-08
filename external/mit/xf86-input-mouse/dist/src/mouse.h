/*
 * Copyright (c) 1997-1999 by The XFree86 Project, Inc.
 */

#ifndef MOUSE_H_
#define MOUSE_H_

#include "xf86OSmouse.h"

#ifdef __NetBSD__
_X_EXPORT const char * xf86MouseProtocolIDToName(MouseProtocolID id);
MouseProtocolID xf86MouseProtocolNameToID(const char *name);
#endif

#endif
