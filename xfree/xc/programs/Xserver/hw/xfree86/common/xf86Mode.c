/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Mode.c,v 1.80 2005/02/26 18:31:48 dawes Exp $ */
/*
 * Copyright (c) 1997-2005 by The XFree86 Project, Inc.
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
 * Copyright © 2003, 2004, 2005 David H. Dawes.
 * Copyright © 2003, 2004, 2005 X-Oz Technologies.
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
 */

/*
 * Authors: Dirk Hohndel <hohndel@XFree86.Org>
 *          David Dawes <dawes@XFree86.Org>
 *          Marc La France <tsi@XFree86.Org>
 *          ... and others
 *
 * This file includes helper functions for mode related things.
 */

#include "X.h"
#include "os.h"
#include "servermd.h"
#include "mibank.h"
#include "globals.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86DDC.h"

/*
 * xf86GetNearestClock --
 *	Find closest clock to given frequency (in kHz).  This assumes the
 *	number of clocks is greater than zero.
 */
int
xf86GetNearestClock(ScrnInfoPtr scrp, int freq, Bool allowDiv2,
    int DivFactor, int MulFactor, int *divider)
{
    int nearestClock = 0, nearestDiv = 1;
    int minimumGap = abs(freq - scrp->clock[0]);
    int i, j, k, gap;

    if (allowDiv2)
	k = 2;
    else
	k = 1;

    /* Must set this here in case the best match is scrp->clock[0] */
    if (divider != NULL)
	*divider = 0;
    
    for (i = 0;  i < scrp->numClocks;  i++) {
	for (j = 1; j <= k; j++) {
	    gap = abs((freq * j) - ((scrp->clock[i] * DivFactor) / MulFactor));
	    if ((gap < minimumGap) ||
		((gap == minimumGap) && (j < nearestDiv))) {
		minimumGap = gap;
		nearestClock = i;
		nearestDiv = j;
		if (divider != NULL)
		    *divider = (j - 1) * V_CLKDIV2;
	    }
	}
    }
    return nearestClock;
}

/*
 * xf86ModeStatusToString
 *
 * Convert a ModeStatus value to a printable message
 */

const char *
xf86ModeStatusToString(ModeStatus status)
{
    switch (status) {
    case MODE_OK:
	return "Mode OK";
    case MODE_HSYNC:
	return "hsync out of range";
    case MODE_VSYNC:
	return "vrefresh out of range";
    case MODE_H_ILLEGAL:
	return "illegal horizontal timings";
    case MODE_V_ILLEGAL:
	return "illegal vertical timings";
    case MODE_BAD_WIDTH:
	return "width requires unsupported line pitch";
    case MODE_NOMODE:
	return "no mode of this name";
    case MODE_NO_INTERLACE:
	return "interlace mode not supported";
    case MODE_NO_DBLESCAN:
	return "doublescan mode not supported";
    case MODE_NO_VSCAN:
	return "multiscan mode not supported";
    case MODE_MEM:
	return "insufficient memory for mode";
    case MODE_VIRTUAL_X:
	return "width too large for virtual size";
    case MODE_VIRTUAL_Y:
	return "height too large for virtual size";
    case MODE_MEM_VIRT:
	return "insufficient memory given virtual size";
    case MODE_NOCLOCK:
	return "no clock available for mode";
    case MODE_CLOCK_HIGH:
	return "mode clock too high";
    case MODE_CLOCK_LOW:
	return "mode clock too low";
    case MODE_CLOCK_RANGE:
	return "bad mode clock/interlace/doublescan";
    case MODE_BAD_HVALUE:
	return "horizontal timing out of range";
    case MODE_BAD_VVALUE:
	return "vertical timing out of range";
    case MODE_BAD_VSCAN:
	return "VScan value out of range";
    case MODE_HSYNC_NARROW:
	return "horizontal sync too narrow";
    case MODE_HSYNC_WIDE:
	return "horizontal sync too wide";
    case MODE_HBLANK_NARROW:
	return "horizontal blanking too narrow";
    case MODE_HBLANK_WIDE:
	return "horizontal blanking too wide";
    case MODE_VSYNC_NARROW:
	return "vertical sync too narrow";
    case MODE_VSYNC_WIDE:
	return "vertical sync too wide";
    case MODE_VBLANK_NARROW:
	return "vertical blanking too narrow";
    case MODE_VBLANK_WIDE:
	return "vertical blanking too wide";
    case MODE_PANEL:
	return "exceeds panel dimensions";
    case MODE_INTERLACE_WIDTH:
	return "width too large for interlaced mode";
    case MODE_ONE_WIDTH:
        return "all modes must have the same width";
    case MODE_ONE_HEIGHT:
        return "all modes must have the same height";
    case MODE_ONE_SIZE:
        return "all modes must have the same resolution";
    case MODE_REFRESH_LOW:
	return "refresh rate is below the target";
    case MODE_TOO_BIG:
	return "size is larger than the preferred mode";
    case MODE_PANEL_NOSCALE:
	return "cannot be scaled by the panel";
    case MODE_ASPECT_RATIO:
	return "aspect ratio is too large";
    case MODE_BAD:
	return "unknown reason";
    case MODE_ERROR:
	return "internal error";
    default:
	return "unknown";
    }
}

const char *
xf86ModeTypeToString(int mType)
{
    if (mType & M_T_BUILTIN)
	return "built-in mode";
    else if (mType & M_T_EDID && mType & M_T_PREFER)
	return "preferred EDID mode";
    else if (mType & M_T_EDID)
	return "EDID mode";
    else if (mType & M_T_DEFAULT)
	return "default mode";
    else
	return "mode";
}

/*
 * xf86ShowClockRanges() -- Print the clock ranges allowed
 * and the clock values scaled by ClockMulFactor and ClockDivFactor
 */
void
xf86ShowClockRanges(ScrnInfoPtr scrp, ClockRangePtr clockRanges)
{
    ClockRangePtr cp;
    int MulFactor = 1;
    int DivFactor = 1;
    int i, j;
    int scaledClock;

    for (cp = clockRanges; cp != NULL; cp = cp->next) {
	DivFactor = max(1, cp->ClockDivFactor);
	MulFactor = max(1, cp->ClockMulFactor);
	if (scrp->progClock) {
	    if (cp->minClock) {
		if (cp->maxClock) {
		    xf86DrvMsg(scrp->scrnIndex, X_INFO,
			"Clock range: %6.2f to %6.2f MHz\n",
			(double)cp->minClock / 1000.0,
			(double)cp->maxClock / 1000.0);
		} else {
		    xf86DrvMsg(scrp->scrnIndex, X_INFO,
			"Minimum clock: %6.2f MHz\n",
			(double)cp->minClock / 1000.0);
		}
	    } else {
		if (cp->maxClock) {
		    xf86DrvMsg(scrp->scrnIndex, X_INFO,
			"Maximum clock: %6.2f MHz\n",
			(double)cp->maxClock / 1000.0);
		}
	    }
	} else if (DivFactor > 1 || MulFactor > 1) {
	    j = 0;
	    for (i = 0; i < scrp->numClocks; i++) {
		scaledClock = (scrp->clock[i] * DivFactor) / MulFactor;
		if (scaledClock >= cp->minClock && scaledClock <= cp->maxClock) {
		    if ((j % 8) == 0) {
			if (j > 0)
			    xf86ErrorF("\n");
			xf86DrvMsg(scrp->scrnIndex, X_INFO, "scaled clocks:");
		    }
		    xf86ErrorF(" %6.2f", (double)scaledClock / 1000.0);
		    j++;
		}
	    }
	    xf86ErrorF("\n");
	}
    }
}


/*
 * xf86FindClockRangeForMode()    [... like the name says ...]
 */
static ClockRangePtr
xf86FindClockRangeForMode(ClockRangePtr clockRanges, DisplayModePtr p)
{
    ClockRangePtr cp;

    for (cp = clockRanges; ; cp = cp->next)
	if (!cp ||
	    ((p->Clock >= cp->minClock) &&
	     (p->Clock <= cp->maxClock) &&
	     (cp->interlaceAllowed || !(p->Flags & V_INTERLACE)) &&
	     (cp->doubleScanAllowed ||
	      ((p->VScan <= 1) && !(p->Flags & V_DBLSCAN)))))
	    return cp;
}


/*
 * xf86HandleBuiltinMode() - handles built-in modes
 */
static ModeStatus
xf86HandleBuiltinMode(ScrnInfoPtr scrp,
		      DisplayModePtr p,
		      DisplayModePtr modep,
		      ClockRangePtr clockRanges,
		      Bool allowDiv2)
{
    ClockRangePtr cp;
    int extraFlags = 0;
    int MulFactor = 1;
    int DivFactor = 1;
    int clockIndex;
    
    /* Reject previously rejected modes */
    if (p->status != MODE_OK)
	return p->status;

    /* Reject previously considered modes */
    if (p->prev)
        return MODE_NOMODE;

    if ((p->type & M_T_CLOCK_C) == M_T_CLOCK_C) {
	/* Check clock is in range */
	cp = xf86FindClockRangeForMode(clockRanges, p);
	if (cp == NULL){
	    modep->type = p->type;
	    p->status = MODE_CLOCK_RANGE;
	    return MODE_CLOCK_RANGE;
	}
	DivFactor = cp->ClockDivFactor;
	MulFactor = cp->ClockMulFactor;
	if (!scrp->progClock) {
	    clockIndex = xf86GetNearestClock(scrp, p->Clock, allowDiv2,
					     cp->ClockDivFactor,
					     cp->ClockMulFactor, &extraFlags);
	    modep->Clock = (scrp->clock[clockIndex] * DivFactor)
		/ MulFactor;
	    modep->ClockIndex	= clockIndex;
	    modep->SynthClock	= scrp->clock[clockIndex];
	    if (extraFlags & V_CLKDIV2) {
		modep->Clock /= 2;
		modep->SynthClock /= 2;
	    }
	} else {
	    modep->Clock = p->Clock;
	    modep->ClockIndex = -1;
	    modep->SynthClock = (modep->Clock * MulFactor)
		/ DivFactor;
	}
	modep->PrivFlags = cp->PrivFlags;
    } else {
	if(!scrp->progClock) {
            modep->Clock = p->Clock;
	    modep->ClockIndex = p->ClockIndex;
	    modep->SynthClock = p->SynthClock;
	} else {
	    modep->Clock = p->Clock;
	    modep->ClockIndex = -1;
	    modep->SynthClock = p->SynthClock;
	}
	modep->PrivFlags = p->PrivFlags;
    }
    modep->type            = p->type;
    modep->HDisplay        = p->HDisplay;
    modep->HSyncStart      = p->HSyncStart;
    modep->HSyncEnd        = p->HSyncEnd;
    modep->HTotal          = p->HTotal;
    modep->HSkew           = p->HSkew;
    modep->VDisplay        = p->VDisplay;
    modep->VSyncStart      = p->VSyncStart;
    modep->VSyncEnd        = p->VSyncEnd;
    modep->VTotal          = p->VTotal;
    modep->VScan           = p->VScan;
    modep->Flags           = p->Flags | extraFlags;
    modep->CrtcHDisplay    = p->CrtcHDisplay;
    modep->CrtcHBlankStart = p->CrtcHBlankStart;
    modep->CrtcHSyncStart  = p->CrtcHSyncStart;
    modep->CrtcHSyncEnd    = p->CrtcHSyncEnd;
    modep->CrtcHBlankEnd   = p->CrtcHBlankEnd;
    modep->CrtcHTotal      = p->CrtcHTotal;
    modep->CrtcHSkew       = p->CrtcHSkew;
    modep->CrtcVDisplay    = p->CrtcVDisplay;
    modep->CrtcVBlankStart = p->CrtcVBlankStart;
    modep->CrtcVSyncStart  = p->CrtcVSyncStart;
    modep->CrtcVSyncEnd    = p->CrtcVSyncEnd;
    modep->CrtcVBlankEnd   = p->CrtcVBlankEnd;
    modep->CrtcVTotal      = p->CrtcVTotal;
    modep->CrtcHAdjusted   = p->CrtcHAdjusted;
    modep->CrtcVAdjusted   = p->CrtcVAdjusted;
    modep->HSync           = p->HSync;
    modep->VRefresh        = p->VRefresh;
    modep->Private         = p->Private;
    modep->PrivSize        = p->PrivSize;

    p->prev = modep;
    
    return MODE_OK;
}

static double
ModeVRefresh(DisplayModePtr mode)
{
    double refresh = 0.0;

    if (mode->VRefresh > 0.0)
	refresh = mode->VRefresh;
    else if (mode->HTotal > 0 && mode->VTotal > 0) {
	refresh = mode->Clock * 1000.0 / mode->HTotal / mode->VTotal;
	if (mode->Flags & V_INTERLACE)
	    refresh *= 2.0;
	if (mode->Flags & V_DBLSCAN)
	    refresh /= 2.0;
	if (mode->VScan > 1)
	    refresh /= mode->VScan;
    }
    return refresh;
}

/*
 * xf86LookupMode
 *
 * This function returns a mode from the given list which matches the
 * given name.  When multiple modes with the same name are available,
 * the method of picking the matching mode is determined by the
 * strategy selected.
 *
 * This function takes the following parameters:
 *    scrp         ScrnInfoPtr
 *    modep        pointer to the returned mode, which must have the name
 *                 field filled in.
 *    clockRanges  a list of clock ranges.   This is optional when all the
 *                 modes are built-in modes.
 *    strategy     how to decide which mode to use from multiple modes with
 *                 the same name
 *
 * In addition, the following fields from the ScrnInfoRec are used:
 *    modePool     the list of monitor modes compatible with the driver
 *    clocks       a list of discrete clocks
 *    numClocks    number of discrete clocks
 *    progClock    clock is programmable
 *
 * If a mode was found, its values are filled in to the area pointed to
 * by modep,  If a mode was not found the return value indicates the
 * reason.
 */

ModeStatus
xf86LookupMode(ScrnInfoPtr scrp, DisplayModePtr modep,
	       ClockRangePtr clockRanges, LookupModeFlags strategy)
{
    DisplayModePtr p, bestMode = NULL;
    ClockRangePtr cp;
    int i, k, gap, minimumGap = CLOCK_TOLERANCE + 1;
    double refresh, bestRefresh = 0.0;
    Bool found = FALSE;
    int extraFlags = 0;
    int clockIndex = -1;
    int MulFactor = 1;
    int DivFactor = 1;
    int ModePrivFlags = 0;
    ModeStatus status = MODE_NOMODE;
    Bool allowDiv2 = (strategy & LOOKUP_CLKDIV2) != 0;
    Bool haveBuiltin;

    strategy &= ~(LOOKUP_CLKDIV2 | LOOKUP_OPTIONAL_TOLERANCES);

    /* Some sanity checking */
    if (scrp == NULL || scrp->modePool == NULL ||
	(!scrp->progClock && scrp->numClocks == 0)) {
	xf86Msg(X_ERROR, "xf86LookupMode: called with invalid scrnInfoRec.\n");
	return MODE_ERROR;
    }
    if (modep == NULL || modep->name == NULL) {
	xf86Msg(X_ERROR, "xf86LookupMode: called with invalid modep.\n");
	return MODE_ERROR;
    }
    for (cp = clockRanges; cp != NULL; cp = cp->next) {
	/* DivFactor and MulFactor must be > 0 */
	cp->ClockDivFactor = max(1, cp->ClockDivFactor);
	cp->ClockMulFactor = max(1, cp->ClockMulFactor);
    }

    haveBuiltin = FALSE;
    /* Scan the mode pool for matching names */
    for (p = scrp->modePool; p != NULL; p = p->next) {
	if (strcmp(p->name, modep->name) == 0) {
	    /*
	     * Requested mode is a built-in mode. Don't let the user
	     * override it.
	     * Since built-in modes always come before user specified
	     * modes it will always be found first.  
	     */
	    if (p->type & M_T_BUILTIN) {
		haveBuiltin = TRUE;
	    }

	    if (haveBuiltin && !(p->type & M_T_BUILTIN))
		continue;

	    /* Skip over previously rejected modes */
	    if (p->status != MODE_OK) {
		if (!found)
		    status = p->status;
		continue;
	    }
		
	    /* Skip over previously considered modes */
	    if (p->prev)
		continue;

	    if (p->type & M_T_BUILTIN) {
		return xf86HandleBuiltinMode(scrp, p,modep, clockRanges,
					     allowDiv2);
	    }

	    /* Check clock is in range */
	    cp = xf86FindClockRangeForMode(clockRanges, p);
	    if (cp == NULL) {
		/*
		 * XXX Could do more here to provide a more detailed
		 * reason for not finding a mode.
		 */
		p->status = MODE_CLOCK_RANGE;
		if (!found)
		    status = MODE_CLOCK_RANGE;
		continue;
	    }

	    /*
	     * If programmable clock and strategy is not LOOKUP_BEST_REFRESH,
	     * the required mode has been found, otherwise record the refresh
	     * and continue looking.
	     */
	    if (scrp->progClock) {
		found = TRUE;
		if (strategy != LOOKUP_BEST_REFRESH) {
		    bestMode = p;
		    DivFactor = cp->ClockDivFactor;
		    MulFactor = cp->ClockMulFactor;
		    ModePrivFlags = cp->PrivFlags;
		    break;
		}
		refresh = ModeVRefresh(p);
		if (p->Flags & V_INTERLACE)
		    refresh /= INTERLACE_REFRESH_WEIGHT;
		/* Force the preferred mode over others. */
		if (p->type & M_T_PREFER) {
		    xf86DrvMsgVerb(scrp->scrnIndex, X_INFO, 4,
			"forcing high priority to the M_T_PREFER mode %s.\n",
			p->name);
		    refresh = 100000000;
		}
		if (refresh > bestRefresh) {
		    bestMode = p;
		    DivFactor = cp->ClockDivFactor;
		    MulFactor = cp->ClockMulFactor;
		    ModePrivFlags = cp->PrivFlags;
		    bestRefresh = refresh;
		}
		continue;
	    }

	    /*
	     * Clock is in range, so if it is not a programmable clock, find
	     * a matching clock.
	     */

	    i = xf86GetNearestClock(scrp, p->Clock, allowDiv2,
		cp->ClockDivFactor, cp->ClockMulFactor, &k);
	    /*
	     * If the clock is too far from the requested clock, this
	     * mode is no good.
	     */
	    if (k & V_CLKDIV2)
		gap = abs((p->Clock * 2) -
		    ((scrp->clock[i] * cp->ClockDivFactor) / cp->ClockMulFactor));
	    else
		gap = abs(p->Clock -
		    ((scrp->clock[i] * cp->ClockDivFactor) / cp->ClockMulFactor));
	    if (gap > minimumGap) {
		p->status = MODE_NOCLOCK;
		if (!found)
		    status = MODE_NOCLOCK;
		continue;
	    }
	    found = TRUE;

	    if (strategy == LOOKUP_BEST_REFRESH) {
		refresh = ModeVRefresh(p);
		if (p->Flags & V_INTERLACE)
		    refresh /= INTERLACE_REFRESH_WEIGHT;
		if (refresh > bestRefresh) {
		    bestMode = p;
		    DivFactor = cp->ClockDivFactor;
		    MulFactor = cp->ClockMulFactor;
		    ModePrivFlags = cp->PrivFlags;
		    extraFlags = k;
		    clockIndex = i;
		    bestRefresh = refresh;
		}
		continue;
	    }
	    if (strategy == LOOKUP_CLOSEST_CLOCK) {
		if (gap < minimumGap) {
		    bestMode = p;
		    DivFactor = cp->ClockDivFactor;
		    MulFactor = cp->ClockMulFactor;
		    ModePrivFlags = cp->PrivFlags;
		    extraFlags = k;
		    clockIndex = i;
		    minimumGap = gap;
		}
		continue;
	    }
	    /*
	     * If strategy is neither LOOKUP_BEST_REFRESH or
	     * LOOKUP_CLOSEST_CLOCK the required mode has been found.
	     */
	    bestMode = p;
	    DivFactor = cp->ClockDivFactor;
	    MulFactor = cp->ClockMulFactor;
	    ModePrivFlags = cp->PrivFlags;
	    extraFlags = k;
	    clockIndex = i;
	    break;
	}
    }
    if (!found || bestMode == NULL)
	return status;

    /* Fill in the mode parameters */
    if (scrp->progClock) {
        modep->Clock		= bestMode->Clock;
	modep->ClockIndex	= -1;
	modep->SynthClock	= (modep->Clock * MulFactor) / DivFactor;
    } else {
	modep->Clock		= (scrp->clock[clockIndex] * DivFactor) / MulFactor;
	modep->ClockIndex	= clockIndex;
	modep->SynthClock	= scrp->clock[clockIndex];
	if (extraFlags & V_CLKDIV2) {
	    modep->Clock /= 2;
	    modep->SynthClock /= 2;
	}
    }
    modep->type                 = bestMode->type;
    modep->PrivFlags		= ModePrivFlags;
    modep->HDisplay		= bestMode->HDisplay;
    modep->HSyncStart		= bestMode->HSyncStart;
    modep->HSyncEnd		= bestMode->HSyncEnd;
    modep->HTotal		= bestMode->HTotal;
    modep->HSkew		= bestMode->HSkew;
    modep->VDisplay		= bestMode->VDisplay;
    modep->VSyncStart		= bestMode->VSyncStart;
    modep->VSyncEnd		= bestMode->VSyncEnd;
    modep->VTotal		= bestMode->VTotal;
    modep->VScan		= bestMode->VScan;
    modep->Flags		= bestMode->Flags | extraFlags;
    modep->CrtcHDisplay		= bestMode->CrtcHDisplay;
    modep->CrtcHBlankStart	= bestMode->CrtcHBlankStart;
    modep->CrtcHSyncStart	= bestMode->CrtcHSyncStart;
    modep->CrtcHSyncEnd		= bestMode->CrtcHSyncEnd;
    modep->CrtcHBlankEnd	= bestMode->CrtcHBlankEnd;
    modep->CrtcHTotal		= bestMode->CrtcHTotal;
    modep->CrtcHSkew		= bestMode->CrtcHSkew;
    modep->CrtcVDisplay		= bestMode->CrtcVDisplay;
    modep->CrtcVBlankStart	= bestMode->CrtcVBlankStart;
    modep->CrtcVSyncStart	= bestMode->CrtcVSyncStart;
    modep->CrtcVSyncEnd		= bestMode->CrtcVSyncEnd;
    modep->CrtcVBlankEnd	= bestMode->CrtcVBlankEnd;
    modep->CrtcVTotal		= bestMode->CrtcVTotal;
    modep->CrtcHAdjusted	= bestMode->CrtcHAdjusted;
    modep->CrtcVAdjusted	= bestMode->CrtcVAdjusted;
    modep->HSync		= bestMode->HSync;
    modep->VRefresh		= bestMode->VRefresh;
    modep->Private		= bestMode->Private;
    modep->PrivSize		= bestMode->PrivSize;

    bestMode->prev = modep;

    return MODE_OK;
}


/*
 * xf86SetModeCrtc
 *
 * Initialises the Crtc parameters for a mode.  The initialisation includes
 * adjustments for interlaced and double scan modes.
 */
static void
xf86SetModeCrtc(DisplayModePtr p, int adjustFlags)
{
    if ((p == NULL) || ((p->type & M_T_CRTC_C) == M_T_BUILTIN))
	return;

    p->CrtcHDisplay             = p->HDisplay;
    p->CrtcHSyncStart           = p->HSyncStart;
    p->CrtcHSyncEnd             = p->HSyncEnd;
    p->CrtcHTotal               = p->HTotal;
    p->CrtcHSkew                = p->HSkew;
    p->CrtcVDisplay             = p->VDisplay;
    p->CrtcVSyncStart           = p->VSyncStart;
    p->CrtcVSyncEnd             = p->VSyncEnd;
    p->CrtcVTotal               = p->VTotal;
    if ((p->Flags & V_INTERLACE) && (adjustFlags & INTERLACE_HALVE_V))
    {
        p->CrtcVDisplay         /= 2;
        p->CrtcVSyncStart       /= 2;
        p->CrtcVSyncEnd         /= 2;
        p->CrtcVTotal           /= 2;
    }
    if (p->Flags & V_DBLSCAN) {
        p->CrtcVDisplay         *= 2;
        p->CrtcVSyncStart       *= 2;
        p->CrtcVSyncEnd         *= 2;
        p->CrtcVTotal           *= 2;
    }
    if (p->VScan > 1) {
        p->CrtcVDisplay         *= p->VScan;
        p->CrtcVSyncStart       *= p->VScan;
        p->CrtcVSyncEnd         *= p->VScan;
        p->CrtcVTotal           *= p->VScan;
    }
    p->CrtcHAdjusted = FALSE;
    p->CrtcVAdjusted = FALSE;

    /*
     * XXX
     *
     * The following is taken from VGA, but applies to other cores as well.
     */
    p->CrtcVBlankStart = min(p->CrtcVSyncStart, p->CrtcVDisplay);
    p->CrtcVBlankEnd = max(p->CrtcVSyncEnd, p->CrtcVTotal);
    if ((p->CrtcVBlankEnd - p->CrtcVBlankStart) >= 127) {
        /* 
         * V Blanking size must be < 127.
         * Moving blank start forward is safer than moving blank end
         * back, since monitors clamp just AFTER the sync pulse (or in
         * the sync pulse), but never before.
         */
        p->CrtcVBlankStart = p->CrtcVBlankEnd - 127;
    }
    p->CrtcHBlankStart = min(p->CrtcHSyncStart, p->CrtcHDisplay);
    p->CrtcHBlankEnd = max(p->CrtcHSyncEnd, p->CrtcHTotal);
    if ((p->CrtcHBlankEnd - p->CrtcHBlankStart) >= 63 * 8) {
        /*
         * H Blanking size must be < 63*8. Same remark as above.
         */
        p->CrtcHBlankStart = p->CrtcHBlankEnd - 63 * 8;
    }
}

/*
 * xf86CheckModeForMonitor
 *
 * This function takes a mode and monitor description, and determines
 * if the mode is valid for the monitor.
 */
ModeStatus
xf86CheckModeForMonitor(DisplayModePtr mode, MonPtr monitor)
{
    int i;
    float hsync, vrefresh;

    /* Sanity checks */
    if (mode == NULL || monitor == NULL) {
	xf86Msg(X_ERROR,
		"xf86CheckModeForMonitor: called with invalid parameters.\n");
	return MODE_ERROR;
    }

    if (monitor->nHsync <= 0 && !(monitor->flags & MON_TOLERANCES_OPTIONAL)) {
	xf86Msg(X_WARNING,
		"xf86CheckModeForMonitor: "
		"called before monitor hsync is set.\n");
    }

    if (monitor->nVrefresh <= 0 &&
	!(monitor->flags & MON_TOLERANCES_OPTIONAL)) {
	xf86Msg(X_WARNING,
		"xf86CheckModeForMonitor: "
		"called before monitor vrefresh is set.\n");
    }

#ifdef DEBUG
    ErrorF("xf86CheckModeForMonitor(%p %s, %p %s)\n",
	   mode, mode->name, monitor, monitor->id);
#endif

    if (monitor->DDC) {
	xf86MonPtr DDC = monitor->DDC;
	struct detailed_monitor_section* detMon;
	struct monitor_ranges *mon_range;
	int i;

	mon_range = NULL;
	for (i = 0; i < 4; i++) {
	    detMon = &DDC->det_mon[i];
	    if(detMon->type == DS_RANGES) {
		mon_range = &detMon->section.ranges;
	    }
	}
	if (mon_range) {
	    /* mode->Clock in kHz, DDC in MHz */
	    if (mon_range->max_clock < 2550 &&
		 mode->Clock / 1000.0 > mon_range->max_clock) {
		xf86Msg(X_WARNING,
		   "(%s,%s) mode clock %gMHz exceeds DDC maximum %dMHz\n",
		   mode->name, monitor->id,
		   mode->Clock/1000.0, mon_range->max_clock);
	    }
	}
    }

    /* Some basic mode validity checks */
    if (0 >= mode->HDisplay || mode->HDisplay > mode->HSyncStart ||
	mode->HSyncStart >= mode->HSyncEnd || mode->HSyncEnd >= mode->HTotal)
	return MODE_H_ILLEGAL;

    if (0 >= mode->VDisplay || mode->VDisplay > mode->VSyncStart ||
	mode->VSyncStart >= mode->VSyncEnd || mode->VSyncEnd >= mode->VTotal)
	return MODE_V_ILLEGAL;

    if (monitor->nHsync > 0) {
	/* Check hsync against the allowed ranges */
	hsync = (float)mode->Clock / (float)mode->HTotal;
	for (i = 0; i < monitor->nHsync; i++)
	    if ((hsync > monitor->hsync[i].lo * (1.0 - SYNC_TOLERANCE)) &&
		(hsync < monitor->hsync[i].hi * (1.0 + SYNC_TOLERANCE)))
		break;

	/* Now see whether we ran out of sync ranges without finding a match */
	if (i == monitor->nHsync)
	    return MODE_HSYNC;
    }

    if (monitor->nVrefresh > 0) {
	/* Check vrefresh against the allowed ranges */
	vrefresh = mode->Clock * 1000.0 / (mode->HTotal * mode->VTotal);
	if (mode->Flags & V_INTERLACE)
	    vrefresh *= 2.0;
	if (mode->Flags & V_DBLSCAN)
	    vrefresh /= 2.0;
	if (mode->VScan > 1)
	    vrefresh /= (float)(mode->VScan);
	for (i = 0; i < monitor->nVrefresh; i++)
	    if ((vrefresh > monitor->vrefresh[i].lo * (1.0 - SYNC_TOLERANCE)) &&
		(vrefresh < monitor->vrefresh[i].hi * (1.0 + SYNC_TOLERANCE)))
		break;

	/* Now see whether we ran out of refresh ranges without finding a match */
	if (i == monitor->nVrefresh)
	    return MODE_VSYNC;
    }

    /* Force interlaced modes to have an odd VTotal */
    if (mode->Flags & V_INTERLACE)
	mode->CrtcVTotal = mode->VTotal |= 1;

    return MODE_OK;
}

/*
 * xf86CheckModeSize
 *
 * An internal routine to check if a mode fits in video memory.  This tries to
 * avoid overflows that would otherwise occur when video memory size is greater
 * than 256MB.
 */
static Bool
xf86CheckModeSize(ScrnInfoPtr scrp, int w, int x, int y)
{
    int bpp = scrp->fbFormat.bitsPerPixel,
	pad = scrp->fbFormat.scanlinePad;
    int lineWidth, lastWidth;

    if (scrp->depth == 4)
	pad *= 4;		/* 4 planes */

    /* Sanity check */
    if ((w < 0) || (x < 0) || (y <= 0))
	return FALSE;

    lineWidth = (((w * bpp) + pad - 1) / pad) * pad;
    lastWidth = x * bpp;

    /*
     * At this point, we need to compare
     *
     *	(lineWidth * (y - 1)) + lastWidth
     *
     * against
     *
     *	scrp->videoRam * (1024 * 8)
     *
     * These are bit quantities.  To avoid overflows, do the comparison in
     * terms of BITMAP_SCANLINE_PAD units.  This assumes BITMAP_SCANLINE_PAD
     * is a power of 2.  We currently use 32, which limits us to a video
     * memory size of 8GB.
     */

    lineWidth = (lineWidth + (BITMAP_SCANLINE_PAD - 1)) / BITMAP_SCANLINE_PAD;
    lastWidth = (lastWidth + (BITMAP_SCANLINE_PAD - 1)) / BITMAP_SCANLINE_PAD;

    if ((lineWidth * (y - 1) + lastWidth) >
	(scrp->videoRam * ((1024 * 8) / BITMAP_SCANLINE_PAD)))
	return FALSE;

    return TRUE;
}

/*
 * xf86InitialCheckModeForDriver
 *
 * This function checks if a mode satisfies a driver's initial requirements:
 *   -  mode size fits within the available pixel area (memory)
 *   -  width lies within the range of supported line pitches
 *   -  mode size fits within virtual size (if fixed)
 *   -  horizontal timings are in range
 *
 * This function takes the following parameters:
 *    scrp         ScrnInfoPtr
 *    mode         mode to check
 *    maxPitch     (optional) maximum line pitch
 *    virtualX     (optional) virtual width requested
 *    virtualY     (optional) virtual height requested
 *
 * In addition, the following fields from the ScrnInfoRec are used:
 *    monitor      pointer to structure for monitor section
 *    fbFormat     pixel format for the framebuffer
 *    videoRam     video memory size (in kB)
 *    maxHValue    maximum horizontal timing value
 *    maxVValue    maximum vertical timing value
 */

ModeStatus
xf86InitialCheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode,
			      ClockRangePtr clockRanges,
			      LookupModeFlags strategy,
			      int maxPitch, int virtualX, int virtualY)
{
    MonPtr monitor;
    ClockRangePtr cp;
    ModeStatus status;
    Bool allowDiv2 = (strategy & LOOKUP_CLKDIV2) != 0;
    int i, needDiv2;

    /* Sanity checks */
    if (!scrp || !mode || !clockRanges) {
	xf86Msg(X_ERROR, "xf86InitialCheckModeForDriver: "
		"called with invalid parameters.\n");
	return MODE_ERROR;
    }

#ifdef DEBUG
    ErrorF("xf86InitialCheckModeForDriver(%p, %p %s, %p, 0x%x, %d, %d, %d)\n",
	   scrp, mode, mode->name , clockRanges, strategy, maxPitch,  virtualX, virtualY);
#endif

    /* Some basic mode validity checks */
    if (0 >= mode->HDisplay || mode->HDisplay > mode->HSyncStart ||
	mode->HSyncStart >= mode->HSyncEnd || mode->HSyncEnd >= mode->HTotal)
	return MODE_H_ILLEGAL;

    if (0 >= mode->VDisplay || mode->VDisplay > mode->VSyncStart ||
	mode->VSyncStart >= mode->VSyncEnd || mode->VSyncEnd >= mode->VTotal)
	return MODE_V_ILLEGAL;

    if (!xf86CheckModeSize(scrp, mode->HDisplay, mode->HDisplay,
				 mode->VDisplay))
        return MODE_MEM;

    if (maxPitch > 0 && mode->HDisplay > maxPitch)
	return MODE_BAD_WIDTH;

    if (virtualX > 0 && mode->HDisplay > virtualX)
	return MODE_VIRTUAL_X;

    if (virtualY > 0 && mode->VDisplay > virtualY)
	return MODE_VIRTUAL_Y;

    if (scrp->maxHValue > 0 && mode->HTotal > scrp->maxHValue)
	return MODE_BAD_HVALUE;

    if (scrp->maxVValue > 0 && mode->VTotal > scrp->maxVValue)
	return MODE_BAD_VVALUE;

    /*
     * The use of the DisplayModeRec's Crtc* and SynthClock elements below is
     * provisional, in that they are later reused by the driver at mode-set
     * time.  Here, they are temporarily enlisted to contain the mode timings
     * as seen by the CRT or panel (rather than the CRTC).  The driver's
     * ValidMode() is allowed to modify these so it can deal with such things
     * as mode stretching and/or centering.  The driver should >NOT< modify the
     * user-supplied values as these are reported back when mode validation is
     * said and done.
     */
    xf86SetModeCrtc(mode, INTERLACE_HALVE_V);

    cp = xf86FindClockRangeForMode(clockRanges, mode);
    if (!cp)
	return MODE_CLOCK_RANGE;

    if (cp->ClockMulFactor < 1)
	cp->ClockMulFactor = 1;
    if (cp->ClockDivFactor < 1)
	cp->ClockDivFactor = 1;

    /*
     * XXX  The effect of clock dividers and multipliers on the monitor's
     *      pixel clock needs to be verified.
     */
    if (scrp->progClock) {
	mode->SynthClock = mode->Clock;
    } else {
	i = xf86GetNearestClock(scrp, mode->Clock, allowDiv2,
				cp->ClockDivFactor, cp->ClockMulFactor,
				&needDiv2);
	mode->SynthClock = (scrp->clock[i] * cp->ClockDivFactor) /
			   cp->ClockMulFactor;
	if (needDiv2 & V_CLKDIV2)
	    mode->SynthClock /= 2;
    }

    if (scrp->ValidMode) {
	status = (*scrp->ValidMode)(scrp->scrnIndex, mode, FALSE,
				    MODECHECK_INITIAL);
	if (status != MODE_OK)
	    return status;
    }

    if (!(monitor = scrp->monitor)) {
	xf86Msg(X_ERROR, "xf86InitialCheckModeForDriver: "
		"called with invalid monitor.\n");
	return MODE_ERROR;
    }

    if (mode->HSync <= 0.0)
	mode->HSync = (float)mode->SynthClock / (float)mode->CrtcHTotal;
    if (monitor->nHsync > 0) {
	/* Check hsync against the allowed ranges */
	for (i = 0; i < monitor->nHsync; i++)
	    if ((mode->HSync > monitor->hsync[i].lo * (1.0 - SYNC_TOLERANCE)) &&
		(mode->HSync < monitor->hsync[i].hi * (1.0 + SYNC_TOLERANCE)))
		break;

	/* Now see whether we ran out of sync ranges without finding a match */
	if (i == monitor->nHsync)
	    return MODE_HSYNC;
    }

    if (mode->VRefresh <= 0.0)
	mode->VRefresh = (mode->SynthClock * 1000.0) /
	    (mode->CrtcHTotal * mode->CrtcVTotal);
    if (monitor->nVrefresh > 0) {
	/* Check vrefresh against the allowed ranges */
	for (i = 0; i < monitor->nVrefresh; i++)
	    if ((mode->VRefresh >
		 monitor->vrefresh[i].lo * (1.0 - SYNC_TOLERANCE)) &&
		(mode->VRefresh <
		 monitor->vrefresh[i].hi * (1.0 + SYNC_TOLERANCE)))
		break;

	/* Now see whether we ran out of refresh ranges without finding a match */
	if (i == monitor->nVrefresh)
	    return MODE_VSYNC;
    }

    /* Force interlaced modes to have an odd VTotal */
    if (mode->Flags & V_INTERLACE)
	mode->CrtcVTotal |= 1;

    /* Assume it is OK */
    return MODE_OK;
}

/*
 * xf86CheckModeForDriver
 *
 * This function is for checking modes while the server is running (for
 * use mainly by the VidMode extension).
 *
 * This function checks if a mode satisfies a driver's requirements:
 *   -  width lies within the line pitch
 *   -  mode size fits within virtual size
 *   -  horizontal/vertical timings are in range
 *
 * This function takes the following parameters:
 *    scrp         ScrnInfoPtr
 *    mode         mode to check
 *    flags        not (currently) used
 *
 * In addition, the following fields from the ScrnInfoRec are used:
 *    maxHValue    maximum horizontal timing value
 *    maxVValue    maximum vertical timing value
 *    virtualX     virtual width
 *    virtualY     virtual height
 *    clockRanges  allowable clock ranges
 */

ModeStatus
xf86CheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode, int flags)
{
    ClockRangesPtr cp;
    int i, k, gap, minimumGap = CLOCK_TOLERANCE + 1;
    int extraFlags = 0;
    int clockIndex = -1;
    int MulFactor = 1;
    int DivFactor = 1;
    int ModePrivFlags = 0;
    Bool allowDiv2;
    ModeStatus status = MODE_NOMODE;

    /* Some sanity checking */
    if (scrp == NULL ||	(!scrp->progClock && scrp->numClocks == 0)) {
	xf86Msg(X_ERROR, "xf86CheckModeForDriver: "
			 "called with invalid scrnInfoRec.\n");
	return MODE_ERROR;
    }
    if (mode == NULL) {
	xf86Msg(X_ERROR,
		"xf86CheckModeForDriver: called with invalid modep.\n");
	return MODE_ERROR;
    }

    /* Check the mode size */
    if (mode->HDisplay > scrp->virtualX)
	return MODE_VIRTUAL_X;

    if (mode->VDisplay > scrp->virtualY)
	return MODE_VIRTUAL_Y;

    if (scrp->maxHValue > 0 && mode->HTotal > scrp->maxHValue)
	return MODE_BAD_HVALUE;

    if (scrp->maxVValue > 0 && mode->VTotal > scrp->maxVValue)
	return MODE_BAD_VVALUE;

    for (cp = scrp->clockRanges; cp != NULL; cp = cp->next) {
	/* DivFactor and MulFactor must be > 0 */
	cp->ClockDivFactor = max(1, cp->ClockDivFactor);
	cp->ClockMulFactor = max(1, cp->ClockMulFactor);
    }

    if (scrp->progClock) {
	/* Check clock is in range */
	for (cp = scrp->clockRanges; cp != NULL; cp = cp->next) {
	    if ((cp->minClock <= mode->Clock) &&
		(cp->maxClock >= mode->Clock) &&
		(cp->interlaceAllowed || !(mode->Flags & V_INTERLACE)) &&
		(cp->doubleScanAllowed ||
		 ((!(mode->Flags & V_DBLSCAN)) && (mode->VScan <= 1))))
	        break;
	}
	if (cp == NULL) {
	    return MODE_CLOCK_RANGE;
	}
	/*
	 * If programmable clock the required mode has been found
	 */
    	DivFactor = cp->ClockDivFactor;
	MulFactor = cp->ClockMulFactor;
	ModePrivFlags = cp->PrivFlags;
    } else {
	 status = MODE_CLOCK_RANGE;
	/* Check clock is in range */
	for (cp = scrp->clockRanges; cp != NULL; cp = cp->next) {
	    if ((cp->minClock <= mode->Clock) &&
		(cp->maxClock >= mode->Clock) &&
		(cp->interlaceAllowed || !(mode->Flags & V_INTERLACE)) &&
		(cp->doubleScanAllowed ||
		 ((!(mode->Flags & V_DBLSCAN)) && (mode->VScan <= 1)))) {

		/*
	 	 * Clock is in range, so if it is not a programmable clock,
		 * find a matching clock.
		 */
    
		allowDiv2 = (cp->strategy & LOOKUP_CLKDIV2) != 0;
		i = xf86GetNearestClock(scrp, mode->Clock, allowDiv2,
			   cp->ClockDivFactor, cp->ClockMulFactor, &k);
		/*
		 * If the clock is too far from the requested clock, this
		 * mode is no good.
		 */
		if (k & V_CLKDIV2)
		    gap = abs((mode->Clock * 2) -
			      ((scrp->clock[i] * cp->ClockDivFactor) /
			       cp->ClockMulFactor));
		else
		    gap = abs(mode->Clock -
			      ((scrp->clock[i] * cp->ClockDivFactor) /
			       cp->ClockMulFactor));
		if (gap > minimumGap) {
		    status = MODE_NOCLOCK;
		    continue;
		}
		
		DivFactor = cp->ClockDivFactor;
		MulFactor = cp->ClockMulFactor;
		ModePrivFlags = cp->PrivFlags;
		extraFlags = k;
		clockIndex = i;
		break;
	    }
	}
	if (cp == NULL)
	    return status;
    }

    /* Fill in the mode parameters */
    if (scrp->progClock) {
	mode->ClockIndex	= -1;
	mode->SynthClock	= (mode->Clock * MulFactor) / DivFactor;
    } else {
	mode->Clock		= (scrp->clock[clockIndex] * DivFactor) / MulFactor;
	mode->ClockIndex	= clockIndex;
	mode->SynthClock	= scrp->clock[clockIndex];
	if (extraFlags & V_CLKDIV2) {
	    mode->Clock /= 2;
	    mode->SynthClock /= 2;
	}
    }
    mode->PrivFlags		= ModePrivFlags;

    return MODE_OK;
}

static struct std_timings est_timings[17] = {
    { 720, 400, 70 },
    { 720, 400, 88 },
    { 640, 480, 60 },
    { 640, 480, 67 },
    { 640, 480, 72 },
    { 640, 480, 75 },
    { 800, 600, 56 },
    { 800, 600, 60 },
    { 800, 600, 72 },
    { 800, 600, 75 },
    { 832, 624, 75 },
    { 1024, 768, 44 },
    { 1024, 768, 60 },
    { 1024, 768, 70 },
    { 1024, 768, 75 },
    { 1280, 1024, 75 },
    { 1152, 870, 75 }
};

Bool
xf86SetMonitorParameters(ScrnInfoPtr pScrn, MonPtr monitor,
			 int hSize, int vSize, int refresh)
{
    int numTimings = 0;
    struct monitor_ranges *mon_range = NULL;
    range hsync[MAX_HSYNC];
    range vrefresh[MAX_VREFRESH];
    const char *type;
    int i;

    if (monitor->flags & MON_PARAMETERS_SET)
	return TRUE;

    /*
     * Probe monitor so that we can enforce/warn about its limits.
     * If one or more DS_RANGES descriptions are present, use the parameters
     * that they provide.  Otherwise, deduce limits based on the modes that
     * are shown as supported via standard and detailed timings.
     */
    if (monitor->DDC) {
	xf86MonPtr DDC = monitor->DDC;
	int i, j;
	float hmin = 1e6, hmax = 0.0, vmin = 1e6, vmax = 0.0;
	float h;
	struct std_timings *t;
	struct detailed_timings *dt;
	int est_mask;

	est_mask = (DDC->timings1.t1 << 9) |
		   (DDC->timings1.t2 << 1) |
		   (DDC->timings1.t_manu & 0x80) >> 7;

	for (i = 0; i < 17; i++) {
	    if (est_mask & (0x10000 >> i)) {
		if (est_timings[i].refresh < vmin)
		    vmin = est_timings[i].refresh;
		if (est_timings[i].refresh > vmax)
		    vmax = est_timings[i].refresh;
		/*
		 * For typical modes this is a reasonable estimate
		 * of the horizontal sync rate.
		 */
		h = est_timings[i].refresh * 1.07 *
			est_timings[i].vsize / 1000.0;
		if (h < hmin)
		    hmin = h;
		if (h > hmax)
		    hmax = h;
	    }
	}

	numTimings = 0;
	for (i = 0; i < DET_TIMINGS; i++) {
	    switch (DDC->det_mon[i].type) {
	    case DS_RANGES:
		mon_range = &DDC->det_mon[i].section.ranges;
		hsync[numTimings].lo = mon_range->min_h;
		hsync[numTimings].hi = mon_range->max_h;
		vrefresh[numTimings].lo = mon_range->min_v;
		vrefresh[numTimings].hi = mon_range->max_v;
		numTimings++;
		break;

	    case DS_STD_TIMINGS:
		t = DDC->det_mon[i].section.std_t;
		for (j = 0; j < 5; j++) {
		    if (t[j].hsize > 256) { /* sanity check */
			if (t[j].refresh < vmin)
			    vmin = t[i].refresh;
			if (t[j].refresh > vmax)
			    vmax = t[i].refresh;
			/*
			 * For typical modes this is a reasonable estimate
			 * of the horizontal sync rate.
			 */
			h = t[j].refresh * 1.07 * t[j].vsize / 1000.0;
			if (h < hmin)
			    hmin = h;
			if (h > hmax)
			    hmax = h;
		    }
		}
		break;

	    case DT:
		dt = &DDC->det_mon[i].section.d_timings;
		if (dt->clock > 15000000) { /* sanity check */
		    float v;
		    h = (float)dt->clock / (dt->h_active + dt->h_blanking);
		    v = h / (dt->v_active + dt->v_blanking);
		    h /= 1000.0;
		    if (dt->interlaced) 
			v /= 2.0;

		    if (v < vmin)
			vmin = v;
		    if (v > vmax)
			vmax = v;
		    if (h < hmin)
			hmin = h;
		    if (h > hmax)
			hmax = h;
		}
		break;
	    }

	    if (numTimings > MAX_HSYNC)
		break;
	}

	t = DDC->timings2;
	for (i = 0; i < STD_TIMINGS; i++) {
	    if (t[i].hsize > 256) { /* sanity check */
		if (t[i].refresh < vmin)
		    vmin = t[i].refresh;
		if (t[i].refresh > vmax)
		    vmax = t[i].refresh;
		/*
		 * For typical modes this is a reasonable estimate
		 * of the horizontal sync rate.
		 */
		h = t[i].refresh * 1.07 * t[i].vsize / 1000.0;
		if (h < hmin)
		    hmin = h;
		if (h > hmax)
		    hmax = h;
	    }
	}

	if (hmax > 0.0 && numTimings == 0) {
	    hsync[numTimings].lo = hmin;
	    hsync[numTimings].hi = hmax;
	    vrefresh[numTimings].lo = vmin;
	    vrefresh[numTimings].hi = vmax;
	    numTimings++;
	}

	if (numTimings > 0) {

#ifdef DEBUG
	    for (i = 0; i < numTimings; i++) {
		ErrorF("DDC - Hsync %.1f-%.1f kHz - Vrefresh %.1f-%.1f Hz\n",
		       hsync[i].lo, hsync[i].hi,
		       vrefresh[i].lo, vrefresh[i].hi);
	    }
#endif

#define DDC_SYNC_TOLERANCE SYNC_TOLERANCE
	    if (monitor->nHsync > 0) {
		for (i = 0; i < monitor->nHsync; i++) {
		    Bool good = FALSE;
		    for (j = 0; j < numTimings; j++) {
			if ((1.0 - DDC_SYNC_TOLERANCE) * hsync[j].lo <=
				monitor->hsync[i].lo &&
			    (1.0 + DDC_SYNC_TOLERANCE) * hsync[j].hi >=
				monitor->hsync[i].hi) {
			    good = TRUE;
			    break;
			}
		    }
		    if (!good) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			  "config file hsync range %g-%gkHz not within DDC "
			  "hsync ranges.\n",
			  monitor->hsync[i].lo, monitor->hsync[i].hi);
		    }
		}
	    }

	    if (monitor->nVrefresh > 0) {
		for (i = 0; i < monitor->nVrefresh; i++) {
		    Bool good = FALSE;
		    for (j = 0; j < numTimings; j++) {
			if ((1.0 - DDC_SYNC_TOLERANCE) * vrefresh[j].lo <=
				monitor->vrefresh[i].lo &&
			    (1.0 + DDC_SYNC_TOLERANCE) * vrefresh[j].hi >=
				monitor->vrefresh[i].hi) {
			    good = TRUE;
			    break;
			}
		    }
		    if (!good) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			  "config file vrefresh range %g-%gHz not within DDC "
			  "vrefresh ranges.\n",
			  monitor->vrefresh[i].lo, monitor->vrefresh[i].hi);
		    }
		}
	    }
        }
    }

    /*
     * If requested by the driver, allow missing hsync and/or vrefresh ranges
     * in the monitor section.
     */
    if (!(monitor->flags & MON_TOLERANCES_OPTIONAL)) {
	type = "";
	if (monitor->nHsync <= 0) {
	    if (numTimings > 0) {
		monitor->nHsync = numTimings;
		for (i = 0; i < numTimings; i++) {
		    monitor->hsync[i].lo = hsync[i].lo;
		    monitor->hsync[i].hi = hsync[i].hi;
		}
	    } else {
		monitor->hsync[0].lo = 28;
		monitor->nHsync = 1;
		if (hSize > 0 && vSize > 0) {
		    if (refresh <= 0)
			refresh = 60;
		    monitor->hsync[0].hi =
			refresh * 1.07 * vSize / 1000.0;
		    type = "native ";
		} else {
		    monitor->hsync[0].hi = 33;
		    type = "default ";
		}
	    }
	}
	for (i = 0; i < monitor->nHsync; i++) {
	    if (monitor->hsync[i].lo == monitor->hsync[i].hi)
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "%s: Using %shsync value of %.2f kHz\n",
			   monitor->id, type,
			   monitor->hsync[i].lo);
	    else
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "%s: Using %shsync range of %.2f-%.2f kHz\n",
			   monitor->id, type,
			   monitor->hsync[i].lo,
			   monitor->hsync[i].hi);
	}

	type = "";
	if (monitor->nVrefresh <= 0) {
	    if (numTimings > 0) {
		monitor->nVrefresh = numTimings;
		for (i = 0; i < numTimings; i++) {
		    monitor->vrefresh[i].lo = vrefresh[i].lo;
		    monitor->vrefresh[i].hi = vrefresh[i].hi;
		}
	    } else {
		monitor->nVrefresh = 1;
		if (hSize > 0 && vSize > 0) {
		    if (refresh <= 0)
			refresh = 60;
		    monitor->vrefresh[0].lo = refresh * 0.95;
		    monitor->vrefresh[0].hi = refresh * 1.05;
		    type = "native ";
		} else {
		    monitor->vrefresh[0].lo = 43;
		    monitor->vrefresh[0].hi = 72;
		    type = "default ";
		}
	    }
	}
	for (i = 0; i < monitor->nVrefresh; i++) {
	    if (monitor->vrefresh[i].lo == monitor->vrefresh[i].hi)
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "%s: Using %svrefresh value of %.2f Hz\n",
			   monitor->id, type,
			   monitor->vrefresh[i].lo);
	    else
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "%s: Using %svrefresh range of %.2f-%.2f Hz\n",
			   monitor->id, type,
			   monitor->vrefresh[i].lo,
			   monitor->vrefresh[i].hi);
	}
    }
    monitor->flags |= MON_PARAMETERS_SET;
    return TRUE;
}

Bool
xf86AddEDIDModes(ScrnInfoPtr pScrn, MonPtr monitor, int flags)
{
    xf86MonPtr DDC;
    int i;
    struct detailed_timings *dt;
    Bool firstPreferred, firstDetailed;
    DisplayModePtr p, new;

    /*
     * Add detailed EDID modes to the monitor's mode list.
     */
    if (!monitor->DDC)
	return TRUE;

    DDC = monitor->DDC;

    /*
     * Check if the monitor's mode list already includes any
     * EDID-derived modes.  If so, there's nothing for us to do here.
     */
    for (p = monitor->Modes; p; p = p->next)
	if (p->type & M_T_EDID)
	    return TRUE;

    firstPreferred = PREFERRED_TIMING_MODE(DDC->features.msc);
    firstDetailed = TRUE;

    for (i = 0; i < DET_TIMINGS; i++) {
	switch (DDC->det_mon[i].type) {
	case DS_RANGES:
		break;

	case DS_STD_TIMINGS:
		break;

	case DT:
	    dt = &DDC->det_mon[i].section.d_timings;
	    if (dt->clock > 15000000) { /* sanity check */
		char *newName = NULL;

		xasprintf(&newName, "%dx%d", dt->h_active, dt->v_active);
		if (newName && !xf86ModeIsPresent(newName, monitor->Modes,
						  0, M_T_DEFAULT)) {
		    new = xcalloc(1, sizeof(DisplayModeRec));
		    if (new) {
			new->type = M_T_DEFAULT | M_T_EDID;
			new->Clock = dt->clock / 1000;
			new->HDisplay = dt->h_active;
			new->HSyncStart = new->HDisplay + dt->h_sync_off;
			new->HSyncEnd = new->HSyncStart + dt->h_sync_width;
			new->HTotal = new->HDisplay + dt->h_blanking;
			new->VDisplay = dt->v_active;
			new->VSyncStart = new->VDisplay + dt->v_sync_off;
			new->VSyncEnd = new->VSyncStart + dt->v_sync_width;
			new->VTotal = new->VDisplay + dt->v_blanking;
			new->name = newName;
			newName = NULL;
			if (firstPreferred && firstDetailed) {
			    new->type |= M_T_PREFER;
			}
			xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
			    "Adding detailed EDID mode %s to monitor "
			    "(preferred: %s).\n",
			    new->name, new->type & M_T_PREFER ? "yes" : "no");
			xf86AddModeToMonitor(monitor, new);
		    }
		}
		if (newName)
		    xfree(newName);
	    }
	    firstDetailed = FALSE;
	    break;
	}
    }

    return TRUE;
}

/*
 * xf86ValidateModes
 *
 * This function takes a set of mode names, modes and limiting conditions,
 * and selects a set of modes and parameters based on those conditions.
 *
 * This function takes the following parameters:
 *    scrp         ScrnInfoPtr
 *    availModes   the list of modes available for the monitor
 *    modeNames    (optional) list of mode names that the screen is requesting
 *    clockRanges  a list of clock ranges
 *    linePitches  (optional) a list of line pitches
 *    minPitch     (optional) minimum line pitch (in pixels)
 *    maxPitch     (optional) maximum line pitch (in pixels)
 *    pitchInc     (mandatory) pitch increment (in bits)
 *    minHeight    (optional) minimum virtual height (in pixels)
 *    maxHeight    (optional) maximum virtual height (in pixels)
 *    virtualX     (optional) virtual width requested (in pixels)
 *    virtualY     (optional) virtual height requested (in pixels)
 *    apertureSize size of video aperture (in bytes)
 *    strategy     how to decide which mode to use from multiple modes with
 *                 the same name
 *
 * In addition, the following fields from the ScrnInfoRec are used:
 *    clocks       a list of discrete clocks
 *    numClocks    number of discrete clocks
 *    progClock    clock is programmable
 *    monitor      pointer to structure for monitor section
 *    fbFormat     format of the framebuffer
 *    videoRam     video memory size
 *    maxHValue    maximum horizontal timing value
 *    maxVValue    maximum vertical timing value
 *    xInc         horizontal timing increment (defaults to 8 pixels)
 *
 * The function fills in the following ScrnInfoRec fields:
 *    modePool     A subset of the modes available to the monitor which
 *		   are compatible with the driver.
 *    modes        one mode entry for each of the requested modes, with the
 *                 status field filled in to indicate if the mode has been
 *                 accepted or not.
 *    virtualX     the resulting virtual width
 *    virtualY     the resulting virtual height
 *    displayWidth the resulting line pitch
 *
 * The function's return value is the number of matching modes found, or -1
 * if an unrecoverable error was encountered.
 */

int
xf86ValidateModes(ScrnInfoPtr scrp, DisplayModePtr availModes,
		  char **modeNames, ClockRangePtr clockRanges,
		  int *linePitches, int minPitch, int maxPitch, int pitchInc,
		  int minHeight, int maxHeight, int virtualX, int virtualY,
		  int apertureSize, LookupModeFlags strategy)
{
    DisplayModePtr p, q, r, new, last, *endp;
    int i, numModes = 0;
    ModeStatus status;
    int linePitch = -1, virtX = 0, virtY = 0;
    int newLinePitch, newVirtX, newVirtY;
    int modeSize;					/* in pixels */
    Bool validateAllDefaultModes = FALSE;
    Bool userModes = FALSE;
    int saveType;
    PixmapFormatRec *BankFormat;
    ClockRangePtr cp;
    ClockRangesPtr storeClockRanges;
    double targetRefresh = 0.0;
    int preferredH = 0, preferredV = 0;
    const char *preferredName = NULL;
    Bool preferredOption = FALSE;
    Bool havePreferredMode = FALSE;
    Bool usePreferred = FALSE;
    int numTimings = 0;

#ifdef DEBUG
    ErrorF("xf86ValidateModes(%p, %p, %p, %p,\n\t\t  %p, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x)\n",
	   scrp, availModes, modeNames, clockRanges,
	   linePitches, minPitch, maxPitch, pitchInc,
	   minHeight, maxHeight, virtualX, virtualY,
	   apertureSize, strategy
	   );
#endif

    /* Some sanity checking */
    if (scrp == NULL || scrp->name == NULL || !scrp->monitor ||
	(!scrp->progClock && scrp->numClocks == 0)) {
	xf86Msg(X_ERROR, "xf86ValidateModes: called with invalid scrnInfoRec.\n");
	return -1;
    }
    if (linePitches != NULL && linePitches[0] <= 0) {
	xf86Msg(X_ERROR,
		"xf86ValidateModes: called with invalid linePitches.\n");
	return -1;
    }
    if (pitchInc <= 0) {
	xf86Msg(X_ERROR,
		"xf86ValidateModes: called with invalid pitchInc.\n");
	return -1;
    }
    if ((virtualX > 0) != (virtualY > 0)) {
	xf86Msg(X_ERROR,
		"xf86ValidateModes: called with invalid virtual resolution.\n");
	return -1;
    }

    /*
     * Probe monitor for preferred DDC/EDID modes.
     */
    if (scrp->monitor->DDC) {
	xf86MonPtr DDC = scrp->monitor->DDC;
	int i, j;
	struct std_timings *t;
	struct detailed_timings *dt;
	Bool digital, firstPreferred, firstDetailed = TRUE;
	int est_mask;

	/*
	 * Check if the monitor's mode list already includes any
	 * EDID-derived modes.
	 */
	xf86AddEDIDModes(scrp, scrp->monitor, 0);

	digital = DIGITAL(DDC->features.input_type);
	firstPreferred = PREFERRED_TIMING_MODE(DDC->features.msc);
	
	est_mask = (DDC->timings1.t1 << 9) |
		   (DDC->timings1.t2 << 1) |
		   (DDC->timings1.t_manu & 0x80) >> 7;

	for (i = 0; i < 17; i++) {
	    if (est_mask & (0x10000 >> i)) {
		if (digital && !firstPreferred) {
		    if (est_timings[i].hsize > preferredH ||
			est_timings[i].vsize > preferredV) {
			preferredH = est_timings[i].hsize;
			preferredV = est_timings[i].vsize;
		    }
		}
	    }
	}

	numTimings = 0;
	for (i = 0; i < DET_TIMINGS; i++) {
	    switch (DDC->det_mon[i].type) {
	    case DS_RANGES:
		break;

	    case DS_STD_TIMINGS:
		t = DDC->det_mon[i].section.std_t;
		for (j = 0; j < 5; j++) {
		    if (t[j].hsize > 256) { /* sanity check */
			if (digital && !firstPreferred) {
			    if (t[j].hsize > preferredH ||
				t[j].vsize > preferredV) {
				preferredH = t[j].hsize;
				preferredV = t[j].vsize;
			    }
			}
		    }
		}
		break;

	    case DT:
		dt = &DDC->det_mon[i].section.d_timings;
		if (dt->clock > 15000000) { /* sanity check */
		    if (digital || (firstPreferred && firstDetailed)) {
			if (dt->h_active > preferredH ||
			    dt->v_active > preferredV) {
			    preferredH = dt->h_active;
			    preferredV = dt->v_active;
			}
		    }
		}
		firstDetailed = FALSE;
		break;
	    }

	    if (numTimings > MAX_HSYNC)
		break;
	}

	t = DDC->timings2;
	for (i = 0; i < STD_TIMINGS; i++) {
	    if (t[i].hsize > 256) { /* sanity check */
		if (digital && !firstPreferred) {
		    if (t[i].hsize > preferredH ||
			t[i].vsize > preferredV) {
			preferredH = t[i].hsize;
			preferredV = t[i].vsize;
		    }
		}
	    }
	}
    }

    /*
     * If requested by the driver, allow missing hsync and/or vrefresh ranges
     * in the monitor section.
     */
    if (strategy & LOOKUP_OPTIONAL_TOLERANCES) {
	strategy &= ~LOOKUP_OPTIONAL_TOLERANCES;
	scrp->monitor->flags |= MON_TOLERANCES_OPTIONAL;
    }
    xf86SetMonitorParameters(scrp, scrp->monitor, 0, 0, 0);

    /*
     * Store the clockRanges for later use by the VidMode extension. Must
     * also store the strategy, since ClockDiv2 flag is stored there.
     */
    storeClockRanges = scrp->clockRanges;
    while (storeClockRanges != NULL) {
	storeClockRanges = storeClockRanges->next;
    }
    for (cp = clockRanges; cp != NULL; cp = cp->next,
	   	storeClockRanges = storeClockRanges->next) {
	storeClockRanges = xnfalloc(sizeof(ClockRanges));
	if (scrp->clockRanges == NULL)
	    scrp->clockRanges = storeClockRanges;
	memcpy(storeClockRanges, cp, sizeof(ClockRange));
	storeClockRanges->strategy = strategy;
    }

    /* Determine which pixmap format to pass to miScanLineWidth() */
    if (scrp->depth > 4)
	BankFormat = &scrp->fbFormat;
    else
	BankFormat = xf86GetPixFormat(scrp, 1);	/* >not< scrp->depth! */

    if (scrp->xInc <= 0)
        scrp->xInc = 8;		/* Suitable for VGA and others */

#define _VIRTUALX(x) ((((x) + scrp->xInc - 1) / scrp->xInc) * scrp->xInc)

    /*
     * Determine maxPitch if it wasn't given explicitly.  Note linePitches
     * always takes precedence if is non-NULL.  In that case the minPitch and
     * maxPitch values passed are ignored.
     */
    if (linePitches) {
	minPitch = maxPitch = linePitches[0];
	for (i = 1; linePitches[i] > 0; i++) {
	    if (linePitches[i] > maxPitch)
		maxPitch = linePitches[i];
	    if (linePitches[i] < minPitch)
		minPitch = linePitches[i];
	}
    }

    /* Initial check of virtual size against other constraints */
    scrp->virtualFrom = X_PROBED;
    /*
     * Initialise virtX and virtY if the values are fixed.
     */
    if (virtualY > 0) {
	if (maxHeight > 0 && virtualY > maxHeight) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		       "Virtual height (%d) is too large for the hardware "
		       "(max %d)\n", virtualY, maxHeight);
	    return -1;
	}

	if (minHeight > 0 && virtualY < minHeight) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		       "Virtual height (%d) is too small for the hardware "
		       "(min %d)\n", virtualY, minHeight);
	    return -1;
	}

	virtualX = _VIRTUALX(virtualX);
	if (linePitches != NULL) {
	    for (i = 0; linePitches[i] != 0; i++) {
		if ((linePitches[i] >= virtualX) &&
		    (linePitches[i] ==
		     miScanLineWidth(virtualX, virtualY, linePitches[i],
				     apertureSize, BankFormat, pitchInc))) {
		    linePitch = linePitches[i];
		    break;
		}
	    }
	} else {
	    linePitch = miScanLineWidth(virtualX, virtualY, minPitch,
					apertureSize, BankFormat, pitchInc);
	}

	if ((linePitch < minPitch) || (linePitch > maxPitch)) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		       "Virtual width (%d) is too large for the hardware "
		       "(max %d)\n", virtualX, maxPitch);
	    return -1;
	}

	if (!xf86CheckModeSize(scrp, linePitch, virtualX, virtualY)) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		      "Virtual size (%dx%d) (pitch %d) exceeds video memory\n",
		      virtualX, virtualY, linePitch);
	    return -1;
	}

	virtX = virtualX;
	virtY = virtualY;
	scrp->virtualFrom = X_CONFIG;
    }

    /* Print clock ranges and scaled clocks */
    xf86ShowClockRanges(scrp, clockRanges);

    /*
     * If scrp->modePool hasn't been setup yet, set it up now.  This allows the
     * modes that the driver definitely can't use to be weeded out early.  Note
     * that a modePool mode's prev field is used to hold a pointer to the
     * member of the scrp->modes list for which a match was considered.
     */
    if (scrp->modePool == NULL) {
	q = NULL;
	for (p = availModes; p != NULL; p = p->next) {
	    status = xf86InitialCheckModeForDriver(scrp, p, clockRanges,
						   strategy, maxPitch,
						   virtualX, virtualY);

	    if (status == MODE_OK)
		status = xf86CheckModeForMonitor(p, scrp->monitor);

	    if (status == MODE_OK) {
		new = xnfalloc(sizeof(DisplayModeRec));
		*new = *p;
		new->next = NULL;
		if (!q) {
		    scrp->modePool = new;
		} else {
		    q->next = new;
		}
		new->prev = NULL;
		q = new;
		q->name = xnfstrdup(p->name);
	        q->status = MODE_OK;
	    } else {
		xf86DrvMsg(scrp->scrnIndex, X_INFO,
			   "Not using %s \"%s\" (%s)\n",
			   xf86ModeTypeToString(p->type),
			   p->name, xf86ModeStatusToString(status));
	    }
	}

	if (scrp->modePool == NULL) {
	    xf86DrvMsg(scrp->scrnIndex, X_WARNING, "Mode pool is empty\n");
	    return 0;
	}
    } else {
	for (p = scrp->modePool; p != NULL; p = p->next) {
	    p->prev = NULL;
	    p->status = MODE_OK;
	    /*
	     * Mark modes with no clock that match the preferred EDID mode
	     * name as preferred.
	     */
	    if (preferredName && p->Clock == 0 && p->name &&
		strcmp(preferredName, p->name) == 0) {
		p->type |= M_T_PREFER;
	    }
	}
    }

    /*
     * Go through the mode pool and see if any modes match the target
     * refresh rate, (if specified).  If no modes match, abandon the target.
     */
    targetRefresh = xf86SetRealOption(scrp->options,
				      "TargetRefresh", 0.0);
    if (targetRefresh > 0.0) {
	for (p = scrp->modePool; p != NULL; p = p->next) {
	    if (ModeVRefresh(p) > targetRefresh * (1.0 - SYNC_TOLERANCE))
		break;
	}
	if (!p)
	    targetRefresh = 0.0;
    }

    if (targetRefresh > 0.0) {
	xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
		   "Target refresh rate is %.1f Hz\n", targetRefresh);
    }

    usePreferred = xf86SetBoolOption(scrp->options, "UsePreferredMode", TRUE);
    if (usePreferred) {
	char *s, *end;
	int newH = 0, newV = 0;

	s = xf86SetStrOption(scrp->options, "PreferredMode", NULL);
	if (s) {
	    while (*s && isspace(*s))
		s++;
	    newH = strtol(s, &end, 10);
	    s = end;
	    while (*s && isspace(*s))
		s++;
	    if (*s == 'x' || *s == 'X') {
		s++;
		while (*s && isspace(*s))
		    s++;
		newV = strtol(s, &end, 10);
		s = end;
		while (*s && isspace(*s))
		    s++;
	    }
	    if (!*s && newH > 0 && newV > 0) {
		preferredH = newH;
		preferredV = newV;
	    }
	    preferredOption = TRUE;
	}
	/*
	 * If there is no preferredMode option, check the mode pool for a mode
	 * tagged as preferred.
	 */
	if (!preferredOption) {
	    for (p = scrp->modePool; p != NULL; p = p->next) {
		if (p->type & M_T_PREFER) {
		    havePreferredMode = TRUE;
		    xf86DrvMsgVerb(scrp->scrnIndex, X_INFO, 4,
			"preferredMode is %s (%dx%d) vs preferredHxV %dx%d.\n",
			p->name, p->HDisplay, p->VDisplay,
			preferredH, preferredV);
		}
	    }
	}

	/*
	 * If there is no mode tagged as preferred, go through the mode pool
	 * and see if any modes match the preferred mode size, (if specified).
	 * If no modes match, abandon it.
	 */
	if (!havePreferredMode && preferredH > 0 && preferredV > 0) {
	    for (p = scrp->modePool; p != NULL; p = p->next) {
		if (p->HDisplay == preferredH && p->VDisplay == preferredV)
		    break;
	    }
	    if (!p) {
		xf86DrvMsgVerb(scrp->scrnIndex, X_INFO, 4,
			       "No preferred mode match found.\n");
		preferredH = 0;
		preferredV = 0;
	    }
	}

	if (preferredH > 0 && preferredV > 0) {
	    xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
		       "Preferred mode size is %dx%d\n",
		       preferredH, preferredV);
	}
    } else {
	xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
		   "Use of a preferred mode has been disabled.\n");
	preferredH = 0;
	preferredV = 0;
    }

    xf86DrvMsgVerb(scrp->scrnIndex, X_INFO, 4,
		   "usePreferred is %s havePreferredMode is %s, "
		   "preferredH is %d, preferredV is %d.\n",
		   BOOLTOSTRING(usePreferred), BOOLTOSTRING(havePreferredMode),
		   preferredH, preferredV);

    /*
     * Check the mode pool against a preferred refresh rate and preferred
     * mode size.
     */
    for (q = scrp->modePool;  q != NULL;  q = q->next) {
	if (q->status != MODE_OK)
	    continue;

	if (ModeVRefresh(q) < (1.0 - SYNC_TOLERANCE) * targetRefresh) {
	    if (preferredH <= 0 || preferredV <= 0) {
		xf86DrvMsg(scrp->scrnIndex, X_INFO,
			   "Not using %s \"%s\" because its refresh (%.1f) "
			   "is below the target (%.1f).\n",
			   xf86ModeTypeToString(q->type),
			   q->name, ModeVRefresh(q), targetRefresh);
		q->status = MODE_REFRESH_LOW;
	    }
	}

	if (preferredH > 0 && preferredV > 0 &&
	    (q->HDisplay > preferredH || q->VDisplay > preferredV)) {
	    xf86DrvMsg(scrp->scrnIndex, X_INFO,
		       "Not using %s \"%s\" because it is "
		       "larger than the preferred mode (%dx%d).\n",
		       xf86ModeTypeToString(q->type), q->name,
		       preferredH, preferredV);
	    q->status = MODE_TOO_BIG;
	}
    }

    /*
     * Allocate one entry in scrp->modes for each named mode.
     */
    while (scrp->modes)
	xf86DeleteMode(&scrp->modes, scrp->modes);
    endp = &scrp->modes;
    last = NULL;
    if (modeNames != NULL) {
	for (i = 0; modeNames[i] != NULL; i++) {
	    userModes = TRUE;
	    new = xnfcalloc(1, sizeof(DisplayModeRec));
	    new->prev = last;
	    new->type = M_T_USERDEF;
	    new->name = xnfalloc(strlen(modeNames[i]) + 1);
	    strcpy(new->name, modeNames[i]);
	    if (new->prev)
		new->prev->next = new;
	    *endp = last = new;
	    endp = &new->next;
	}
    }

    /* Lookup each mode */
#ifdef RANDR
    if (!xf86Info.disableRandR	
#ifdef PANORAMIX
	&& noPanoramiXExtension
#endif
	)
	validateAllDefaultModes = TRUE;
#endif

    for (p = scrp->modes; ; p = p->next) {
	Bool repeat;

	/*
	 * If the supplied mode names don't produce a valid mode, scan through
	 * unconsidered modePool members until one survives validation.  This
	 * is done in decreasing order by mode pixel area.
	 */

	if (p == NULL) {
	    if ((numModes > 0) && !validateAllDefaultModes)
		break;

	    validateAllDefaultModes = TRUE;
	    r = NULL;
	    modeSize = 0;
	    for (q = scrp->modePool;  q != NULL;  q = q->next) {
		if ((q->prev == NULL) && (q->status == MODE_OK)) {
		    /*
		     * Deal with the case where this mode wasn't considered
		     * because of a builtin mode of the same name.
		     */
		    for (p = scrp->modes; p != NULL; p = p->next) {
			if ((p->status != MODE_OK) &&
			    !strcmp(p->name, q->name))
			    break;
		    }

		    if (p != NULL)
			q->prev = p;
		    else {
			/*
			 * A quick check to not allow default modes with
			 * horizontal timing parameters that CRTs may have
			 * problems with.
			 */
			if ((q->type & M_T_DEFAULT) &&
			    ((double)q->HTotal / (double)q->HDisplay) < 1.15)
			    continue;

			if (modeSize < (q->HDisplay * q->VDisplay)) {
			    r = q;
			    modeSize = q->HDisplay * q->VDisplay;
			}
		    }
		}
	    }

	    if (r == NULL)
		break;

	    p = xnfcalloc(1, sizeof(DisplayModeRec));
	    p->prev = last;
	    p->name = xnfalloc(strlen(r->name) + 1);
	    if (!userModes && (!last || strcmp(last->name, r->name) != 0)) {
		p->type = M_T_USERDEF;
	    }
	    strcpy(p->name, r->name);
	    if (p->prev)
		p->prev->next = p;
	    *endp = last = p;
	    endp = &p->next;
	}

	repeat = FALSE;
    lookupNext:
	if (repeat && ((status = p->status) != MODE_OK)) {
	    xf86DrvMsg(scrp->scrnIndex, X_INFO,
		       "Not using %s \"%s\" (%s)\n",
		       xf86ModeTypeToString(p->type),
		       p->name, xf86ModeStatusToString(status));
	}
	saveType = p->type;
	status = xf86LookupMode(scrp, p, clockRanges, strategy);
	if (repeat && status == MODE_NOMODE) {
	    continue;
	}
	if (status != MODE_OK) {
	    xf86DrvMsg(scrp->scrnIndex, X_INFO,
		       "Not using %s \"%s\" (%s)\n",
		       xf86ModeTypeToString(p->type),
		       p->name, xf86ModeStatusToString(status));
	}
	if (status == MODE_ERROR) {
	    xf86Msg(X_ERROR, "xf86ValidateModes: "
		   "unexpected result from xf86LookupMode().\n");
	    return -1;
	}
	if (status != MODE_OK) {
	    if (p->status == MODE_OK)
		p->status = status;
	    continue;
	}
	p->type |= saveType;
	repeat = TRUE;

	newLinePitch = linePitch;
	newVirtX = virtX;
	newVirtY = virtY;

	/*
	 * Don't let non-user defined modes increase the virtual size
	 */
	if (!(p->type & M_T_USERDEF) && (numModes > 0)) {
	    if (p->HDisplay > virtX) {
		p->status = MODE_VIRTUAL_X;
		goto lookupNext;
	    }
	    if (p->VDisplay > virtY) {
		p->status = MODE_VIRTUAL_Y;
		goto lookupNext;
	    }
	}
	/*
	 * Adjust virtual width and height if the mode is too large for the
	 * current values and if they are not fixed.
	 */
	if (virtualX <= 0 && p->HDisplay > newVirtX)
	    newVirtX = _VIRTUALX(p->HDisplay);
	if (virtualY <= 0 && p->VDisplay > newVirtY) {
	    if (maxHeight > 0 && p->VDisplay > maxHeight) {
		p->status = MODE_VIRTUAL_Y;	/* ? */
		goto lookupNext;
	    }
	    newVirtY = p->VDisplay;
	}

	/*
	 * If virtual resolution is to be increased, revalidate it.
	 */
	if ((virtX != newVirtX) || (virtY != newVirtY)) {
	    if (linePitches != NULL) {
		newLinePitch = -1;
		for (i = 0; linePitches[i] != 0; i++) {
		    if ((linePitches[i] >= newVirtX) &&
			(linePitches[i] >= linePitch) &&
			(linePitches[i] ==
			 miScanLineWidth(newVirtX, newVirtY, linePitches[i],
					 apertureSize, BankFormat, pitchInc))) {
			newLinePitch = linePitches[i];
			break;
		    }
		}
	    } else {
		if (linePitch < minPitch)
		    linePitch = minPitch;
		newLinePitch = miScanLineWidth(newVirtX, newVirtY, linePitch,
					       apertureSize, BankFormat,
					       pitchInc);
	    }
	    if ((newLinePitch < minPitch) || (newLinePitch > maxPitch)) {
		p->status = MODE_BAD_WIDTH;
		goto lookupNext;
	    }

	    /*
	     * Check that the pixel area required by the new virtual height
	     * and line pitch isn't too large.
	     */
	    if (!xf86CheckModeSize(scrp, newLinePitch, newVirtX, newVirtY)) {
		p->status = MODE_MEM_VIRT;
		goto lookupNext;
	    }
	}

	if (scrp->ValidMode) {
	    /*
	     * Give the driver a final say, passing it the proposed virtual
	     * geometry.
	     */
	    scrp->virtualX = newVirtX;
	    scrp->virtualY = newVirtY;
	    scrp->displayWidth = newLinePitch;
	    p->status = (scrp->ValidMode)(scrp->scrnIndex, p, FALSE,
					  MODECHECK_FINAL);

	    if (p->status != MODE_OK) {
	        goto lookupNext;
	    }
	}

	/* Mode has passed all the tests */
	virtX = newVirtX;
	virtY = newVirtY;
	linePitch = newLinePitch;
	p->status = MODE_OK;
	numModes++;
    }

#undef _VIRTUALX

    /* Update the ScrnInfoRec parameters */
    
    scrp->virtualX = virtX;
    scrp->virtualY = virtY;
    scrp->displayWidth = linePitch;

    if (numModes <= 0)
	return 0;
    
    /* Make the mode list into a circular list by joining up the ends */
    p = scrp->modes;
    while (p->next != NULL)
	p = p->next;
    /* p is now the last mode on the list */
    p->next = scrp->modes;
    scrp->modes->prev = p;

    if (minHeight > 0 && virtY < minHeight) {
	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Virtual height (%d) is too small for the hardware "
		   "(min %d)\n", virtY, minHeight);
	return -1;
    }

    return numModes;
}

/*
 * xf86DeleteMode
 *
 * This function removes a mode from a list of modes.
 *
 * There are different types of mode lists:
 *
 *  - singly linked linear lists, ending in NULL
 *  - doubly linked linear lists, starting and ending in NULL
 *  - doubly linked circular lists
 *
 */
 
void
xf86DeleteMode(DisplayModePtr *modeList, DisplayModePtr mode)
{
    /* Catch the easy/insane cases */
    if (modeList == NULL || *modeList == NULL || mode == NULL)
	return;

    /* If the mode is at the start of the list, move the start of the list */
    if (*modeList == mode)
	*modeList = mode->next;

    /* If mode is the only one on the list, set the list to NULL */
    if ((mode == mode->prev) && (mode == mode->next)) {
	*modeList = NULL;
    } else {
	if ((mode->prev != NULL) && (mode->prev->next == mode))
	    mode->prev->next = mode->next;
	if ((mode->next != NULL) && (mode->next->prev == mode))
	    mode->next->prev = mode->prev;
    }

    xfree(mode->name);
    xfree(mode);
}

/*
 * xf86PruneDriverModes
 *
 * Remove modes from the driver's mode list which have been marked as
 * invalid.
 */

void
xf86PruneDriverModes(ScrnInfoPtr scrp)
{
    DisplayModePtr first, p, n;

    p = scrp->modes;
    if (p == NULL)
	return;

    do {
	if (!(first = scrp->modes))
	    return;
	n = p->next;
	if (p->status != MODE_OK) {
#if 0
	    xf86DrvMsg(scrp->scrnIndex, X_INFO,
		       "Not using %s \"%s\" (%s)\n", p->name,
		       xf86ModeTypeToString(p->type),
		       xf86ModeStatusToString(p->status));
#endif
	    xf86DeleteMode(&(scrp->modes), p);
	}
	p = n;
    } while (p != NULL && p != first);

    /* modePool is no longer needed, turf it */
    while (scrp->modePool) {
	/*
	 * A modePool mode's prev field is used to hold a pointer to the
	 * member of the scrp->modes list for which a match was considered.
	 * Clear that pointer first, otherwise xf86DeleteMode might get 
	 * confused 
	 */
	scrp->modePool->prev = NULL;
	xf86DeleteMode(&scrp->modePool, scrp->modePool);
    }
}


/*
 * xf86SetCrtcForModes
 *
 * Goes through the screen's mode list, and initialises the Crtc
 * parameters for each mode.  The initialisation includes adjustments
 * for interlaced and double scan modes.
 */
void
xf86SetCrtcForModes(ScrnInfoPtr scrp, int adjustFlags)
{
    DisplayModePtr p;

    /*
     * Store adjustFlags for use with the VidMode extension. There is an
     * implicit assumption here that SetCrtcForModes is called once.
     */
    scrp->adjustFlags = adjustFlags;

    p = scrp->modes;
    if (p == NULL)
	return;

    do {
	xf86SetModeCrtc(p, adjustFlags);
#ifdef DEBUG
	ErrorF("%sMode %s: %d (%d) %d %d (%d) %d %d (%d) %d %d (%d) %d\n",
	       (p->type & M_T_DEFAULT) ? "Default " : "",
	       p->name, p->CrtcHDisplay, p->CrtcHBlankStart,
	       p->CrtcHSyncStart, p->CrtcHSyncEnd, p->CrtcHBlankEnd,
	       p->CrtcHTotal, p->CrtcVDisplay, p->CrtcVBlankStart,
	       p->CrtcVSyncStart, p->CrtcVSyncEnd, p->CrtcVBlankEnd,
	       p->CrtcVTotal);
#endif
	p = p->next;
    } while (p != NULL && p != scrp->modes);
}


static void
add(char **p, char *new)
{
    *p = xnfrealloc(*p, strlen(*p) + strlen(new) + 2);
    strcat(*p, " ");
    strcat(*p, new);
}

static void
PrintModeline(int scrnIndex,DisplayModePtr mode)
{
    char *tmp;
    char *flags = xnfcalloc(1, 1);

    if (mode->HSkew) { 
	xasprintf(&tmp, "hskew %i", mode->HSkew); 
	if (tmp) {
	    add(&flags, tmp);
	    xfree(tmp);
	}
    }
    if (mode->VScan) { 
	xasprintf(&tmp, "vscan %i", mode->VScan); 
	if (tmp) {
	    add(&flags, tmp);
	    xfree(tmp);
	}
    }
    if (mode->Flags & V_INTERLACE) add(&flags, "interlace");
    if (mode->Flags & V_CSYNC) add(&flags, "composite");
    if (mode->Flags & V_DBLSCAN) add(&flags, "doublescan");
    if (mode->Flags & V_BCAST) add(&flags, "bcast");
    if (mode->Flags & V_PHSYNC) add(&flags, "+hsync");
    if (mode->Flags & V_NHSYNC) add(&flags, "-hsync");
    if (mode->Flags & V_PVSYNC) add(&flags, "+vsync");
    if (mode->Flags & V_NVSYNC) add(&flags, "-vsync");
    if (mode->Flags & V_PCSYNC) add(&flags, "+csync");
    if (mode->Flags & V_NCSYNC) add(&flags, "-csync");
#if 0
    if (mode->Flags & V_CLKDIV2) add(&flags, "vclk/2");
#endif
    xf86DrvMsgVerb(scrnIndex, X_INFO, 3,
		   "Modeline \"%s\"  %6.2f  %i %i %i %i  %i %i %i %i%s\n",
		   mode->name, mode->Clock/1000., mode->HDisplay,
		   mode->HSyncStart, mode->HSyncEnd, mode->HTotal,
		   mode->VDisplay, mode->VSyncStart, mode->VSyncEnd,
		   mode->VTotal, flags);
    xfree(flags);
}

void
xf86PrintModes(ScrnInfoPtr scrp)
{
    DisplayModePtr p;
    float hsync, refresh = 0;
    char *desc, *desc2, *prefix, *uprefix;

    if (scrp == NULL)
	return;

    xf86DrvMsg(scrp->scrnIndex, scrp->virtualFrom, "Virtual size is %dx%d "
	       "(pitch %d)\n", scrp->virtualX, scrp->virtualY,
	       scrp->displayWidth);
    
    p = scrp->modes;
    if (p == NULL)
	return;

    do {
	desc = desc2 = "";
	if (p->HSync > 0.0)
	    hsync = p->HSync;
	else if (p->HTotal > 0)
	    hsync = (float)p->Clock / (float)p->HTotal;
	else
	    hsync = 0.0;
	refresh = ModeVRefresh(p);
	if (p->Flags & V_INTERLACE) {
	    desc = " (I)";
	}
	if (p->Flags & V_DBLSCAN) {
	    desc = " (D)";
	}
	if (p->VScan > 1) {
	    desc2 = " (VScan)";
	}
	prefix = xstrdup(xf86ModeTypeToString(p->type));
	if (!prefix)
	    continue;
	if (islower(prefix[0]))
	    prefix[0] = toupper(prefix[0]);
	if (p->type & M_T_USERDEF)
	    uprefix = "*";
	else
	    uprefix = " ";
	if (hsync == 0 || refresh == 0) {
	    if (p->name)
		xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
			   "%s%s \"%s\"\n", uprefix, prefix, p->name);
	    else
		xf86DrvMsg(scrp->scrnIndex, X_PROBED,
			   "%s%s %dx%d (unnamed)\n",
			   uprefix, prefix, p->HDisplay, p->VDisplay);
	} else if (p->Clock == p->SynthClock) {
	    xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
			"%s%s \"%s\": %.1f MHz, %.1f kHz, %.1f Hz%s%s\n",
			uprefix, prefix, p->name, p->Clock / 1000.0,
			hsync, refresh, desc, desc2);
	} else {
	    xf86DrvMsg(scrp->scrnIndex, X_CONFIG,
			"%s%s \"%s\": %.1f MHz (scaled from %.1f MHz), "
			"%.1f kHz, %.1f Hz%s%s\n",
			uprefix, prefix, p->name, p->Clock / 1000.0,
			p->SynthClock / 1000.0, hsync, refresh, desc, desc2);
	}
	xfree(prefix);
	if (hsync != 0 && refresh != 0)
	    PrintModeline(scrp->scrnIndex,p);
	p = p->next;
    } while (p != NULL && p != scrp->modes);
}

Bool
xf86ModeIsPresent(const char *modeName, const DisplayModeRec *modeList,
		  int inclTypeMask, int exclTypeMask)
{
    const DisplayModeRec *p;

    p = modeList;
    while (p) {
	if (p->name && strcmp(modeName, p->name) == 0) {
	    if (!inclTypeMask && !exclTypeMask) {
		return TRUE;
	    } else {
		if (inclTypeMask) {
		    if (p->type & inclTypeMask) {
			return TRUE;
		    }
		}
		if (exclTypeMask) {
		    if (!(p->type & exclTypeMask)) {
			return TRUE;
		    }
		}
	    }
	}
	p = p->next;
    }
    return FALSE;
}

/* This is intended for non-circular mode lists. */
void
xf86AddModeAfter(DisplayModePtr *ppOld, DisplayModePtr pNew)
{
    /* First check for an empty list. */
    if (!*ppOld) {
	*ppOld = pNew;
	pNew->prev = NULL;
	pNew->next = NULL;
    } else {
	if ((*ppOld)->next)
	    (*ppOld)->next->prev = pNew;
	pNew->next = (*ppOld)->next;
	(*ppOld)->next = pNew;
	pNew->prev = *ppOld;
    }
}

/* This is intended for non-circular mode lists. */
void
xf86AddModeBefore(DisplayModePtr *ppOld, DisplayModePtr pNew)
{
    /* First check for an empty list. */
    if (!*ppOld) {
	*ppOld = pNew;
	pNew->prev = NULL;
	pNew->next = NULL;
    } else {
	if ((*ppOld)->prev)
	    (*ppOld)->prev->next = pNew;
	pNew->prev = (*ppOld)->prev;
	(*ppOld)->prev = pNew;
	pNew->next = *ppOld;
    }
}

void
xf86AddModeToMonitor(MonPtr pMonitor, DisplayModePtr pNew)
{
    if (pMonitor->Last)
	xf86AddModeAfter(&pMonitor->Last, pNew);
    else
	xf86AddModeAfter(&pMonitor->Modes, pNew);
    pMonitor->Last = pNew;
}

