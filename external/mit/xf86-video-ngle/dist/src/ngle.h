/* $OpenBSD: wsfb_driver.c,v 1.18 2003/04/02 16:42:13 jason Exp $ */
/*
 * Copyright (c) 2024 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dev/wscons/wsconsio.h>

#include "xorg-server.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xorgVersion.h"
#include "xf86Cursor.h"
#include "exa.h"
#include "compat-api.h"

#ifndef NGLE_H
#define NGLE_H

/* private data */
typedef struct {
	int			fd;
	struct wsdisplayio_fbinfo fbi;
	unsigned char*		fbmem;
	size_t			fbmem_len;
	void			*regs;
	size_t			reglen;
	Bool			HWCursor;
	CloseScreenProcPtr	CloseScreen;
	CreateScreenResourcesProcPtr CreateScreenResources;
	EntityInfoPtr		pEnt;
	struct wsdisplay_cursor cursor;
	int			maskoffset;
	xf86CursorInfoPtr	CursorInfoRec;
	OptionInfoPtr		Options;
	ExaDriverPtr		pExa;
	GlyphsProcPtr		glyphs;
	uint32_t		gid, buf, fbacc, creg;
	int 			offset, hwmode, offsetd;
#define HW_FB	0
#define HW_FILL	1
#define HW_BLIT	2
#define HW_BINC	3
	uint32_t		read_mode, write_mode;
	int			need_sync;
	GlyphsProcPtr		glyphs;
} NGLERec, *NGLEPtr;

#define NGLEPTR(p) ((NGLEPtr)((p)->driverPrivate))

Bool NGLESetupCursor(ScreenPtr);
Bool NGLEInitAccel(ScreenPtr);
Bool SummitSetupCursor(ScreenPtr);
Bool SummitInitAccel(ScreenPtr);

static inline void
NGLEWrite4(NGLEPtr fPtr, int offset, uint32_t val)
{
	volatile uint32_t *ptr = (uint32_t *)((uint8_t *)fPtr->regs + offset);
	*ptr = val;
}

static inline void
NGLEWrite1(NGLEPtr fPtr, int offset, uint8_t val)
{
	volatile uint8_t *ptr = (uint8_t *)fPtr->regs + offset;
	*ptr = val;
}

static inline uint32_t
NGLERead4(NGLEPtr fPtr, int offset)
{
	volatile uint32_t *ptr = (uint32_t *)((uint8_t *)fPtr->regs + offset);
	return *ptr;
}

static inline uint8_t
NGLERead1(NGLEPtr fPtr, int offset)
{
	volatile uint8_t *ptr = (uint8_t *)fPtr->regs + offset;
	return *ptr;
}

#endif
