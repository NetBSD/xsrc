/* $NetBSD: ffb_accel_common.c,v 1.2 2016/08/19 19:16:01 mrg Exp $ */
/*
 * Copyright (C) 1998,1999,2000 Jakub Jelinek (jakub@redhat.com)
 * Copyright (C) 1998 Michal Rehacek (majkl@iname.com)
 * Copyright (C) 1999,2000 David S. Miller (davem@redhat.com)
 * Copyright (C) 2015 Michael Lorenz (macallan@netbsd.org)
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
 * JAKUB JELINEK, MICHAL REHACEK, OR DAVID MILLER BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#include <sys/types.h>

#include "ffb_fifo.h"
#include "ffb_rcache.h"
#include "ffb.h"
#include "ffb_regs.h"

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "exa.h"

void FFB_SetupTextureAttrs(FFBPtr pFfb)
{
       ffb_fbcPtr ffb = pFfb->regs;
       unsigned int ppc = FFB_PPC_APE_DISABLE | FFB_PPC_CS_VAR | FFB_PPC_XS_VAR;
       unsigned int ppc_mask = FFB_PPC_APE_MASK | FFB_PPC_CS_MASK | FFB_PPC_XS_MASK;
       unsigned int rop = FFB_ROP_NEW | (FFB_ROP_NEW << 8);
       unsigned int fbc = pFfb->fbc;
       unsigned int wid = pFfb->wid;

       ppc |= FFB_PPC_ABE_ENABLE;
       ppc_mask |= FFB_PPC_ABE_MASK;

       if ((pFfb->ppc_cache & ppc_mask) != ppc ||
           pFfb->fbc_cache != fbc ||
           pFfb->wid_cache != wid ||
           pFfb->rop_cache != rop ||
           pFfb->pmask_cache != 0xffffffff)
               __FFB_Attr_SFB_VAR(pFfb, ppc, ppc_mask, fbc,
                                  wid, rop, 0xffffffff);
       FFBWait(pFfb, ffb);
}

void FFB_HardwareSetup(FFBPtr pFfb)
{
       ffb_fbcPtr ffb = pFfb->regs;

	/* Determine the current screen resolution type.  This is
	 * needed to figure out the fastfill/pagefill parameters.
	 */
	switch(ffb->fbcfg0 & FFB_FBCFG0_RES_MASK) {
	default:
	case FFB_FBCFG0_RES_STD:
		pFfb->ffb_res = ffb_res_standard;
		break;
	case FFB_FBCFG0_RES_HIGH:
		pFfb->ffb_res = ffb_res_high;
		break;
	case FFB_FBCFG0_RES_STEREO:
		pFfb->ffb_res = ffb_res_stereo;
		break;
	case FFB_FBCFG0_RES_PRTRAIT:
		pFfb->ffb_res = ffb_res_portrait;
		break;
	};
	CreatorAlignTabInit(pFfb);

	/* Next, determine the hwbug workarounds and feature enables
	 * we should be using on this board.
	 */
	pFfb->disable_pagefill = 0;
	pFfb->disable_vscroll = 0;
	pFfb->has_brline_bug = 0;
	pFfb->use_blkread_prefetch = 0;
	if (pFfb->ffb_type == ffb1_prototype ||
	    pFfb->ffb_type == ffb1_standard ||
	    pFfb->ffb_type == ffb1_speedsort) {
		pFfb->has_brline_bug = 1;
		if (pFfb->ffb_res == ffb_res_high)
			pFfb->disable_vscroll = 1;
		if (pFfb->ffb_res == ffb_res_high ||
		    pFfb->ffb_res == ffb_res_stereo)
			pFfb->disable_pagefill = 1;

	} else {
		/* FFB2 has blkread prefetch.  AFB supposedly does too
		 * but the chip locks up on me when I try to use it. -DaveM
		 */
#define AFB_PREFETCH_IS_BUGGY	1
		if (!AFB_PREFETCH_IS_BUGGY ||
		    (pFfb->ffb_type != afb_m3 &&
		     pFfb->ffb_type != afb_m6)) {
			pFfb->use_blkread_prefetch = 1;
		}
		/* XXX I still cannot get page/block fast fills
		 * XXX to work reliably on any of my AFB boards. -DaveM
		 */
#define AFB_FASTFILL_IS_BUGGY	1
		if (AFB_FASTFILL_IS_BUGGY &&
		    (pFfb->ffb_type == afb_m3 ||
		     pFfb->ffb_type == afb_m6))
			pFfb->disable_pagefill = 1;
	}
	pFfb->disable_fastfill_ap = 0;
	if (pFfb->ffb_res == ffb_res_stereo ||
	    pFfb->ffb_res == ffb_res_high)
		pFfb->disable_fastfill_ap = 1;
}

/* Multiplies and divides suck... */
void CreatorAlignTabInit(FFBPtr pFfb)
{
	struct fastfill_parms *ffp = &FFB_FFPARMS(pFfb);
	short *tab = pFfb->Pf_AlignTab;
	int i;

	for(i = 0; i < 0x800; i++) {
		int alignval;

		alignval = (i / ffp->pagefill_width) * ffp->pagefill_width;
		*tab++ = alignval;
	}
}

