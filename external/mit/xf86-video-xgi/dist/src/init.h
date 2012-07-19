/*
 * Data and prototypes for init.c
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License as published by
 * * the Free Software Foundation; either version 2 of the named License,
 * * or any later version.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) The name of the author may not be used to endorse or promote products
 * *    derived from this software without specific prior written permission.
 * *
 * * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: 	Thomas Winischhofer <thomas@winischhofer.net>
 *
 */

#ifndef _INIT_
#define _INIT_

#include "osdef.h"

#include "vgatypes.h"
#include "vb_struct.h"

#ifdef LINUX_XF86
#include "xf86.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xf86_OSproc.h"
#include "xgi.h"
#include "xgi_regs.h"
#endif

#ifdef LINUX_KERNEL
#ifdef XGI_CP
#undef XGI_CP
#endif
#include <linux/config.h>
#include <linux/version.h>
#include <linux/types.h>
#include <asm/io.h>
#include <linux/fb.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <linux/xgifb.h>
#else
#include <video/xgifb.h>
#endif
#endif

/* Mode numbers */
static const USHORT  ModeIndex_320x200[]      = {0x59, 0x41, 0x00, 0x4f};
static const USHORT  ModeIndex_320x240[]      = {0x50, 0x56, 0x00, 0x53};
static const USHORT  ModeIndex_320x240_FSTN[] = {0x5a, 0x5b, 0x00, 0x00};  /* FSTN */
static const USHORT  ModeIndex_400x300[]      = {0x51, 0x57, 0x00, 0x54};
static const USHORT  ModeIndex_512x384[]      = {0x52, 0x58, 0x00, 0x5c};
static const USHORT  ModeIndex_640x400[]      = {0x2f, 0x5d, 0x00, 0x5e};
static const USHORT  ModeIndex_640x480[]      = {0x2e, 0x44, 0x00, 0x62};
static const USHORT  ModeIndex_720x480[]      = {0x31, 0x33, 0x00, 0x35};
static const USHORT  ModeIndex_720x576[]      = {0x32, 0x34, 0x00, 0x36};
static const USHORT  ModeIndex_768x576[]      = {0x5f, 0x60, 0x00, 0x61};
static const USHORT  ModeIndex_800x480[]      = {0x70, 0x7a, 0x00, 0x76};
static const USHORT  ModeIndex_800x600[]      = {0x30, 0x47, 0x00, 0x63};
static const USHORT  ModeIndex_848x480[]      = {0x39, 0x3b, 0x00, 0x3e};
static const USHORT  ModeIndex_856x480[]      = {0x3f, 0x42, 0x00, 0x45};
static const USHORT  ModeIndex_1024x768[]     = {0x38, 0x4a, 0x00, 0x64};
static const USHORT  ModeIndex_1024x576[]     = {0x71, 0x74, 0x00, 0x77};
static const USHORT  ModeIndex_1024x600[]     = {0x20, 0x21, 0x00, 0x22};  /* 300 series only */
static const USHORT  ModeIndex_1280x1024[]    = {0x3a, 0x4d, 0x00, 0x65};
static const USHORT  ModeIndex_1280x960[]     = {0x7b, 0x7c, 0x00, 0x7d};
static const USHORT  ModeIndex_1152x768[]     = {0x23, 0x24, 0x00, 0x25};  /* 300 series only */
static const USHORT  ModeIndex_1152x864[]     = {0x20, 0x21, 0x00, 0x22};
static const USHORT  ModeIndex_310_1280x768[] = {0x23, 0x24, 0x00, 0x25};
static const USHORT  ModeIndex_1280x720[]     = {0x79, 0x75, 0x00, 0x78};
static const USHORT  ModeIndex_1280x800[]     = {0x14, 0x15, 0x00, 0x16};
static const USHORT  ModeIndex_1360x768[]     = {0x48, 0x4b, 0x00, 0x4e};
static const USHORT  ModeIndex_1400x1050[]    = {0x26, 0x27, 0x00, 0x28};  /* 315 series only */
/* static const USHORT  ModeIndex_1440x900[]	  = {0x26, 0x27, 0x00, 0x28}; */ /* Jong@09292009 */
static const USHORT  ModeIndex_1680x1050[]    = {0x17, 0x18, 0x00, 0x19};  /* 315 series only */
static const USHORT  ModeIndex_1600x1200[]    = {0x3c, 0x3d, 0x00, 0x66};
static const USHORT  ModeIndex_1920x1440[]    = {0x68, 0x69, 0x00, 0x6b};
static const USHORT  ModeIndex_310_2048x1536[]= {0x6c, 0x6d, 0x00, 0x6e};


void	XGI_New_DisplayOn(VB_DEVICE_INFO *XGI_Pr);
void	XGI_New_DisplayOff(VB_DEVICE_INFO *XGI_Pr);

void	XGI_New_GetVBType(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo);
USHORT	XGI_GetMCLK(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo);
USHORT	XGI_New_GetOffset(VB_DEVICE_INFO *XGI_Pr,USHORT ModeNo,USHORT ModeIdIndex,
              USHORT RefreshRateTableIndex,PXGI_HW_DEVICE_INFO HwInfo);
void	XGI_New_LoadDAC(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo, USHORT ModeNo, USHORT ModeIdIndex);
#ifdef LINUX_XF86
BOOLEAN	XGIBIOSSetMode(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
    ScrnInfoPtr pScrn, DisplayModePtr mode);

BOOLEAN	XGIBIOSSetModeCRT1(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
    ScrnInfoPtr pScrn, DisplayModePtr mode);
#endif

#ifdef LINUX_XF86
extern unsigned char XGI_GetSetBIOSScratch(ScrnInfoPtr pScrn, USHORT offset, unsigned char value);
extern unsigned char XGI_GetSetModeID(ScrnInfoPtr pScrn, unsigned char id);
extern USHORT XGI_CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode,
    unsigned long VBFlags);
#endif

#endif

