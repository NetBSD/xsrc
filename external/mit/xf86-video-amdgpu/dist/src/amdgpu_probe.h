/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *
 * Modified by Marc Aurele La France <tsi@xfree86.org> for ATI driver merge.
 */

#ifndef _AMDGPU_PROBE_H_
#define _AMDGPU_PROBE_H_ 1

#include <stdint.h>
#include "xorg-server.h"
#include "xf86str.h"
#include "xf86DDC.h"
#include "randrstr.h"

#include "xf86Crtc.h"

#ifdef XSERVER_PLATFORM_BUS
#include "xf86platformBus.h"
#endif

#include <amdgpu.h>

#include "compat-api.h"

extern DriverRec AMDGPU;

typedef enum {
	CHIP_FAMILY_UNKNOW,
	CHIP_FAMILY_LEGACY,
	CHIP_FAMILY_AMDGPU,
	CHIP_FAMILY_TAHITI,
	CHIP_FAMILY_PITCAIRN,
	CHIP_FAMILY_VERDE,
	CHIP_FAMILY_OLAND,
	CHIP_FAMILY_HAINAN,
	CHIP_FAMILY_BONAIRE,
	CHIP_FAMILY_KAVERI,
	CHIP_FAMILY_KABINI,
	CHIP_FAMILY_HAWAII,
	CHIP_FAMILY_MULLINS,
	CHIP_FAMILY_TOPAZ,
	CHIP_FAMILY_TONGA,
	CHIP_FAMILY_CARRIZO,
	CHIP_FAMILY_FIJI,
	CHIP_FAMILY_STONEY,
	CHIP_FAMILY_POLARIS10,
	CHIP_FAMILY_POLARIS11,
	CHIP_FAMILY_LAST
} AMDGPUChipFamily;

typedef struct {
	uint32_t pci_device_id;
	AMDGPUChipFamily chip_family;
} AMDGPUCardInfo;

typedef struct {
	Bool HasCRTC2;		/* All cards except original Radeon  */

	amdgpu_device_handle pDev;

	int fd;			/* for sharing across zaphod heads   */
	int fd_ref;
	unsigned long fd_wakeup_registered;	/* server generation for which fd has been registered for wakeup handling */
	int fd_wakeup_ref;
	unsigned int assigned_crtcs;
	ScrnInfoPtr primary_scrn;
	ScrnInfoPtr secondary_scrn;
	struct xf86_platform_device *platform_dev;
} AMDGPUEntRec, *AMDGPUEntPtr;

extern const OptionInfoRec *AMDGPUOptionsWeak(void);

extern Bool AMDGPUPreInit_KMS(ScrnInfoPtr, int);
extern Bool AMDGPUScreenInit_KMS(SCREEN_INIT_ARGS_DECL);
extern Bool AMDGPUSwitchMode_KMS(SWITCH_MODE_ARGS_DECL);
extern void AMDGPUAdjustFrame_KMS(ADJUST_FRAME_ARGS_DECL);
extern Bool AMDGPUEnterVT_KMS(VT_FUNC_ARGS_DECL);
extern void AMDGPULeaveVT_KMS(VT_FUNC_ARGS_DECL);
extern void AMDGPUFreeScreen_KMS(FREE_SCREEN_ARGS_DECL);

extern ModeStatus AMDGPUValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode,
				  Bool verbose, int flag);
#endif /* _AMDGPU_PROBE_H_ */
