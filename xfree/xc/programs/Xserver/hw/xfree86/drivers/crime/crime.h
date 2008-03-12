/* $NetBSD: crime.h,v 1.2 2008/03/12 20:00:07 macallan Exp $ */
/*
 * Copyright (c) 2008 Michael Lorenz
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
#include <machine/crmfbreg.h>

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86RamDac.h"
#include "xaa.h"

#ifndef CRIME_H
#define CRIME_H

#define CRIME_DEBUG

#define CRIME_DEBUG_LINES		0x00000001
#define CRIME_DEBUG_BITBLT		0x00000002
#define CRIME_DEBUG_RECTFILL		0x00000004
#define CRIME_DEBUG_IMAGEWRITE		0x00000008
#define CRIME_DEBUG_COLOUREXPAND	0x00000010
#define CRIME_DEBUG_CLIPPING		0x00000020
#define CRIME_DEBUG_SYNC		0x00000040
#define CRIME_DEBUG_XRENDER		0x00000080
#define CRIME_DEBUG_ALL			0xffffffff
#define CRIME_DEBUG_MASK (/*CRIME_DEBUG_XRENDER*/0)

#ifdef CRIME_DEBUG
#define LOG(x) if (x & CRIME_DEBUG_MASK) xf86Msg(X_ERROR, "%s\n", __func__)
#define DONE(x) if (x & CRIME_DEBUG_MASK) \
		 xf86Msg(X_ERROR, "%s done\n", __func__)
#else
#define LOG(x)
#define DONE(x)
#endif

/* private data */
typedef struct {
	int			fd; /* file descriptor of open device */
	struct wsdisplay_fbinfo info; /* frame buffer characteristics */
	Bool			HWCursor;
	CloseScreenProcPtr	CloseScreen;
	EntityInfoPtr		pEnt;

	struct wsdisplay_cursor cursor;
	int			maskoffset;
	xf86CursorInfoPtr	CursorInfoRec;
	OptionInfoPtr		Options;

	XAAInfoRecPtr		pXAA;
	void			*engine;
	char			*linear;
	void			*fb;
	unsigned char		*buffers[8];
	unsigned char		*expandbuffers[1];
	int			ux, uy, uw, uh, us;
	int			start, xdir, ydir;
	int			format;
	uint32_t		expand[2048];
	uint32_t		alpha_color;
	unsigned char		*alpha_texture;
} CrimeRec, *CrimePtr;

#define CRIMEPTR(p) ((CrimePtr)((p)->driverPrivate))

Bool CrimeSetupCursor(ScreenPtr);
int CrimeAccelInit(ScrnInfoPtr);

#endif
