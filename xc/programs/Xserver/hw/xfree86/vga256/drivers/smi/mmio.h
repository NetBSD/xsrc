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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/mmio.h,v 1.1.2.2 1999/12/11 17:43:19 hohndel Exp $ */

#ifndef _MMIO_H_
#define _MMIO_H_

#include <Xmd.h>

#define int16	CARD16
#define int32	CARD32

typedef struct
{
	int16 vendor_ID;
	int16 device_ID;
} pci_id;

typedef struct
{
	int16 cmd;
	int16 devsel;
} cmd_devsel;

typedef struct
{
	pci_id	   pci_ident;
	cmd_devsel cmd_device_sel;
	int32 	   class_code;
	char  	   dummy_0c;
	char  	   latency_timer;
	int16 	   dummy_0e;
	int32 	   base0;
	int32	   dummy_14;
	int32	   dummy_18;
	int32	   dummy_1c;
	int32	   dummy_20;
	int32	   dummy_24;
	int32	   dummy_28;
	pci_id     subsystem_ident;
	int32 	   bios_base;
	int32 	   power_cap;
	int32 	   dummy_38;
	char  	   int_line;
	char  	   int_pin;
	int16 	   dummy_3e;
	int32	   power_reg;
	int32	   power_data;
} pci_conf_regs;

typedef struct
{
	int32 dpr00;
	int32 dpr04;
	int32 dpr08;
	int32 dpr0C;
	int32 dpr10;
	int32 dpr14;
	int32 dpr18;
	int32 dpr1C;
	int32 dpr20;
	int32 dpr24;
	int32 dpr28;
	int32 dpr2C;
	int32 dpr30;
	int32 dpr34;
	int32 dpr38;
	int32 dpr3C;
	int32 dpr40;
	int32 dpr44;
} DPR;

typedef struct
{
	int32 vpr00;
	int32 vpr04;
	int32 vpr08;
	int32 vpr0C;
	int32 vpr10;
	int32 vpr14;
	int32 vpr18;
	int32 vpr1C;
	int32 vpr20;
	int32 vpr24;
	int32 vpr28;
	int32 vpr2C;
	int32 vpr30;
	int32 vpr34;
	int32 vpr38;
	int32 vpr3C;
	int32 vpr40;
	int32 vpr44;
	int32 vpr48;
	int32 vpr4C;
	int32 vpr50;
	int32 vpr54;
	int32 vpr58;
	int32 vpr5C;
	int32 vpr60;
	int32 vpr64;
	int32 vpr68;
	int32 vpr6C;
} VPR;

#define SMI_STARTENGINE		(1 << 31)
#define SMI_MONOPATTERN		(0 << 30)
#define SMI_COLORPATTERN	(1 << 30)
#define SMI_QUICKSTART		(1 << 28)
#define SMI_LEFTTORIGHT		(0 << 27)
#define SMI_RIGHTTOLEFT		(1 << 27)
#define SMI_MONOSOURCE		(1 << 22)
#define SMI_COLORSOURCE		(0 << 22)
#define SMI_BITBLT			(0x0 << 16)
#define SMI_FILL			(0x1 << 16)
#define SMI_BLTWRITE		(0x8 << 16)
#define SMI_TRANSPARENT		(1 << 8)

#endif
