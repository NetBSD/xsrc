/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86DPMS.c,v 1.13 2004/06/01 01:23:49 dawes Exp $ */

/*
 * Copyright (c) 1997-2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Automatic configuration support is
 * Copyright 2003, 2004 by X-Oz Technologies.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 * 
 *  3. The end-user documentation included with the redistribution,
 *     if any, must include the following acknowledgment: "This product
 *     includes software developed by X-Oz Technologies
 *     (http://www.x-oz.com/)."  Alternately, this acknowledgment may
 *     appear in the software itself, if and wherever such third-party
 *     acknowledgments normally appear.
 *
 *  4. Except as contained in this notice, the name of X-Oz
 *     Technologies shall not be used in advertising or otherwise to
 *     promote the sale, use or other dealings in this Software without
 *     prior written authorization from X-Oz Technologies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL X-OZ TECHNOLOGIES OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*
 * This file contains the DPMS functions required by the extension.
 */

#include "X.h"
#include "os.h"
#include "globals.h"
#include "xf86.h"
#include "xf86Priv.h"
#ifdef DPMSExtension
#define DPMS_SERVER
#include "extensions/dpms.h"
#include "dpmsproc.h"
#endif
#include "edid.h"


#ifdef DPMSExtension
static int DPMSGeneration = 0;
static int DPMSIndex = -1;
static Bool DPMSClose(int i, ScreenPtr pScreen);
static int DPMSCount = 0;
#endif


Bool
xf86DPMSInit(ScreenPtr pScreen, DPMSSetProcPtr set, int flags)
{
#ifdef DPMSExtension
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    DPMSPtr pDPMS;
    pointer DPMSOpt;
    MessageType DPMSFrom = X_DEFAULT;

    if (serverGeneration != DPMSGeneration) {
	if ((DPMSIndex = AllocateScreenPrivateIndex()) < 0)
	    return FALSE;
	DPMSGeneration = serverGeneration;
    }

    /*
     * The logic here is as follows:
     *
     * DPMS can be enabled or disabled per-screen.  This can be done
     * in the following order of preference:
     *    1. The "dpms" option (enable or disable)
     *    2. The global 'dpms' command line option (enable)
     *    3. The probed EDID data showing DPMS capabilities (enable or disable)
     *    4. Default (disabled).
     *
     * In addition to that, the initial global DPMS enable/disable state
     * can be set as follows, in the following order of preference:
     *    1. The '-dpms' command line option (disable)
     *    2. At least one screen with DPMS enabled. (enable)
     *    3. Default (disabled).
     */

    if (!(pScreen->devPrivates[DPMSIndex].ptr = xcalloc(sizeof(DPMSRec), 1)))
	return FALSE;

    pDPMS = (DPMSPtr)pScreen->devPrivates[DPMSIndex].ptr;
    pScrn->DPMSSet = set;
    pDPMS->Flags = flags;

    /* Per-screen setting. */
    DPMSOpt = xf86FindOption(pScrn->options, "dpms");
    if (DPMSOpt) {
	pDPMS->Enabled = xf86SetBoolOption(pScrn->options, "dpms", FALSE);
	xf86MarkOptionUsed(DPMSOpt);
	DPMSFrom = X_CONFIG;
    } else if (DPMSEnabledSwitch) {
	pDPMS->Enabled = TRUE;
	DPMSFrom = X_CMDLINE;
    } else if (pScrn->monitor->DDC) {
	xf86MonPtr DDC = (xf86MonPtr)(pScrn->monitor->DDC);
	if (DDC->features.dpms) {
	    pDPMS->Enabled = TRUE;
	}
	DPMSFrom = X_PROBED;
    } else {
	pDPMS->Enabled = FALSE;
    }

    /* Initial global state. */
    if (DPMSDisabledSwitch) {
	DPMSEnabled = FALSE;
    } else if (pDPMS->Enabled) {
	DPMSEnabled = TRUE;
    }

    xf86DrvMsg(pScreen->myNum, DPMSFrom, "DPMS %s%s\n",
	       pDPMS->Enabled ?  "enabled" : "disabled",
	       DPMSDisabledSwitch ? " (disabled globally)" : "");

    pDPMS->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = DPMSClose;
    DPMSCount++;
    return TRUE;
#else
    return FALSE;
#endif
}


#ifdef DPMSExtension

static Bool
DPMSClose(int i, ScreenPtr pScreen)
{
    DPMSPtr pDPMS;

    /* This shouldn't happen */
    if (DPMSIndex < 0)
	return FALSE;

    pDPMS = (DPMSPtr)pScreen->devPrivates[DPMSIndex].ptr;

    /* This shouldn't happen */
    if (!pDPMS)
	return FALSE;

    pScreen->CloseScreen = pDPMS->CloseScreen;

    xfree((pointer)pDPMS);
    pScreen->devPrivates[DPMSIndex].ptr = NULL;
    if (--DPMSCount == 0)
	DPMSIndex = -1;
    return pScreen->CloseScreen(i, pScreen);
}


/*
 * DPMSSet --
 *	Device dependent DPMS mode setting hook.  This is called whenever
 *	the DPMS mode is to be changed.
 */
void
DPMSSet(int level)
{
    int i;
    DPMSPtr pDPMS;
    ScrnInfoPtr pScrn;

    DPMSPowerLevel = level;

    if (DPMSIndex < 0)
	return;

    if (level != DPMSModeOn)
	SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverActive);

    /* For each screen, set the DPMS level */
    for (i = 0; i < xf86NumScreens; i++) {
    	pScrn = xf86Screens[i];
	pDPMS = (DPMSPtr)screenInfo.screens[i]->devPrivates[DPMSIndex].ptr;
	if (pDPMS && pScrn->DPMSSet && pDPMS->Enabled && pScrn->vtSema) { 
	    xf86EnableAccess(pScrn);
	    pScrn->DPMSSet(pScrn, level, 0);
	}
    }
}


/*
 * DPMSSupported --
 *	Return TRUE if any screen supports DPMS.
 */
Bool
DPMSSupported(void)
{
    int i;
    DPMSPtr pDPMS;
    ScrnInfoPtr pScrn;

    if (DPMSIndex < 0) {
	return FALSE;
    }

    /* For each screen, check if DPMS is supported */
    for (i = 0; i < xf86NumScreens; i++) {
    	pScrn = xf86Screens[i];
	pDPMS = (DPMSPtr)screenInfo.screens[i]->devPrivates[DPMSIndex].ptr;
	if (pDPMS && pScrn->DPMSSet)
	    return TRUE;
    }
    return FALSE;
}


/*
 * DPMSGet --
 *	Device dependent DPMS mode getting hook.  This returns the current
 *	DPMS mode, or -1 if DPMS is not supported.
 *
 *	This should hook in to the appropriate driver-level function, which
 *	will be added to the ScrnInfoRec.
 *
 *	NOTES:
 *	 1. the calling interface should be changed to specify which
 *	    screen to check.
 *	 2. It isn't clear that this function is ever used or what it should
 *	    return.
 */
int
DPMSGet(int *level)
{
    return DPMSPowerLevel;
}

#endif /* DPMSExtension */
