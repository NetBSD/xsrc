/*
 * Southland Media MGX - defines and such.
 *
 * Copyright (C) 2021 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * MICHAEL LORENZ BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MGX_H
#define MGX_H

#define MMIO_IS_BE

#include "xorg-server.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86RamDac.h"
#include <X11/Xmd.h>
#include "gcstruct.h"
#include "xf86sbusBus.h"
#include "exa.h"

#include <dev/wscons/wsconsio.h>

#include "compat-api.h"

typedef struct {
	uint8_t 	*fb;
	uint8_t 	*blt;
	sbusDevicePtr	psdp;
	CloseScreenProcPtr CloseScreen;
	struct wsdisplay_cursor cursor;
	Bool		HWCursor, NoAccel;
	int		vramsize;	/* size of the fb */
	int		maskoffset;
	xf86CursorInfoPtr CursorInfoRec;
	OptionInfoPtr	Options;
	ExaDriverPtr	pExa;
	uint32_t 	dec;
	int 		offset;
} MgxRec, *MgxPtr;

Bool MgxInitAccel(ScreenPtr);
Bool MgxSetupCursor(ScreenPtr);

#define GET_MGX_FROM_SCRN(p)    ((MgxPtr)((p)->driverPrivate))

static inline void MgxWrite1(MgxPtr pMgx, int offset, uint8_t val)
{
	MMIO_OUT8(pMgx->blt, offset ^ 3, val);
}

static inline void MgxWrite2(MgxPtr pMgx, int offset, uint16_t val)
{
	MMIO_OUT16(pMgx->blt, offset ^ 2, val);
}

static inline void MgxWrite4(MgxPtr pMgx, int offset, uint32_t val)
{
	MMIO_OUT32(pMgx->blt, offset, val);
}

static inline uint8_t MgxRead1(MgxPtr pMgx, int offset)
{
	uint8_t val = MMIO_IN8(pMgx->blt, offset ^ 3);
	return val;
}

static inline uint32_t MgxRead4(MgxPtr pMgx, int offset)
{
	uint32_t val = MMIO_IN32(pMgx->blt, offset);
	return val;
}

#endif /* MGX_H */
