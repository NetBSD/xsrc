/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_rop.h,v 1.1.2.2 1999/12/11 17:43:22 hohndel Exp $ */

#include "regsmi.h"

/* For source copies */
static int smiAlu[16] =
{
	ROP_0,				/* GXclear */
	ROP_DSa,			/* GXand */
	ROP_SDna,			/* GXandReverse */
	ROP_S,				/* GXcopy */
	ROP_DSna,			/* GXandInverted */
	ROP_D,				/* GXnoop */
	ROP_DSx,			/* GXxor */
	ROP_DSo,			/* GXor */
	ROP_DSon,			/* GXnor */
	ROP_DSxn,			/* GXequiv */
	ROP_Dn,				/* GXinvert*/
	ROP_SDno,			/* GXorReverse */
	ROP_Sn,				/* GXcopyInverted */
	ROP_DSno,			/* GXorInverted */
	ROP_DSan,			/* GXnand */
	ROP_1,				/* GXset */
};

/* For pattern fills */
static int smiAlu_sp[16] =
{
	ROP_0,				/* GXclear */
	ROP_DPa,			/* GXand */
	ROP_PDna,			/* GXandReverse */
	ROP_P,				/* GXcopy */
	ROP_DPna,			/* GXandInverted */
	ROP_D,				/* GXnoop */
	ROP_DPx,			/* GXxor */
	ROP_DPo,			/* GXor */
	ROP_DPon,			/* GXnor */
	ROP_DPxn,			/* GXequiv */
	ROP_Dn,				/* GXinvert*/
	ROP_PDno,			/* GXorReverse */
	ROP_Pn,				/* GXcopyInverted */
	ROP_DPno,			/* GXorInverted */
	ROP_DPan,			/* GXnand */
	ROP_1,				/* GXset */
};
