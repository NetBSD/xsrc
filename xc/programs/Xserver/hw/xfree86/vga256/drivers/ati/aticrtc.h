/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticrtc.h,v 1.1.2.4 2000/05/14 02:02:15 tsi Exp $ */
/*
 * Copyright 1997 through 1999 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef ___ATICRTC_H___
#define ___ATICRTC_H___ 1

#include "atibank.h"
#include "vga.h"

/*
 * CRTC related definitions.
 */
#define ATI_CRTC_VGA    0       /* Use VGA CRTC */
#define ATI_CRTC_8514   1       /* Use 8514/Mach8/Mach32 accelerator CRTC */
#define ATI_CRTC_MACH64 2       /* Use Mach64 accelerator CRTC */
extern CARD8 ATICRTC;

/*
 * Driver data structure.
 */
typedef struct
{
        /* Generic VGA registers */
        vgaHWRec std;

        /* Other generic DAC registers */
        CARD8 dac_read, dac_write, dac_mask;

        /* VGA Wonder registers */
        CARD8             a3,         a6, a7,             ab, ac, ad, ae,
              b0, b1, b2, b3,     b5, b6,     b8, b9, ba,         bd, be, bf;

        /* Mach64 PLL registers */
        CARD8 pll_vclk_cntl, pll_vclk_post_div,
              pll_vclk0_fb_div, pll_vclk1_fb_div,
              pll_vclk2_fb_div, pll_vclk3_fb_div,
              pll_xclk_cntl, pll_ext_vpll_cntl;

        /* Mach64 registers */
        CARD32 crtc_h_total_disp, crtc_h_sync_strt_wid,
               crtc_v_total_disp, crtc_v_sync_strt_wid,
               crtc_off_pitch, crtc_gen_cntl, dsp_config, dsp_on_off,
               ovr_clr, ovr_wid_left_right, ovr_wid_top_bottom,
               clock_cntl, bus_cntl, mem_vga_wp_sel, mem_vga_rp_sel,
               dac_cntl, config_cntl;

        /* LCD registers */
        CARD32 lcd_index, config_panel, lcd_gen_ctrl,
               horz_stretching, vert_stretching, ext_vert_stretch;

        /* Shadow VGA CRTC registers */
        CARD8 shadow_vga[25];

        /* Shadow Mach64 CRTC registers */
        CARD32 shadow_h_total_disp, shadow_h_sync_strt_wid,
               shadow_v_total_disp, shadow_v_sync_strt_wid;

        /*
         * Various things needed by ATISwap:  the function to be called by
         * ATISelectBank, a pointer to a frame buffer save area and the number
         * of banks and planes to contend with.
         */
        BankFunction * bank_function;
        void * frame_buffer;
        unsigned int banks, planes;

        CARD8 crtc;                 /* VGA, 8514 or Mach64 CRTC */
        DisplayModePtr mode;        /* The corresponding mode line */
        const CARD8 *ClockMap;      /* Clock map pointers */
        const CARD8 *ClockUnMap;

        /* Parameters for programming clock frequencies */
        int FeedbackDivider, ReferenceDivider, PostDivider;
} ATIHWRec, *ATIHWPtr;

/* The server's video mode */
#define ATINewHWPtr ((ATIHWPtr)vgaNewVideoState)

/* The current video mode */
extern ATIHWPtr ATICurrentHWPtr;

extern void * ATISave    FunctionPrototype((void *));
extern Bool   ATIInit    FunctionPrototype((DisplayModePtr));
extern void   ATIRestore FunctionPrototype((void *));

#endif /* ___ATICRTC_H___ */
