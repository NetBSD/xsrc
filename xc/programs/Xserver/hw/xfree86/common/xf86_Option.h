/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_Option.h,v 3.60 1996/10/18 15:02:40 dawes Exp $ */
/*
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: xf86_Option.h /main/17 1996/01/12 12:02:03 kaleb $ */

#ifndef _XF86_OPTION_H
#define _XF86_OPTION_H

/*
 * Structures and macros for handling option flags.
 */
#define MAX_OFLAGS	192
#define FLAGBITS	sizeof(unsigned long)
typedef struct {
	unsigned long flag_bits[MAX_OFLAGS/FLAGBITS];
} OFlagSet;

#define OFLG_SET(f,p)	((p)->flag_bits[(f)/FLAGBITS] |= (1 << ((f)%FLAGBITS)))
#define OFLG_CLR(f,p)	((p)->flag_bits[(f)/FLAGBITS] &= ~(1 << ((f)%FLAGBITS)))
#define OFLG_ISSET(f,p)	((p)->flag_bits[(f)/FLAGBITS] & (1 << ((f)%FLAGBITS)))
#define OFLG_ZERO(p)	memset((char *)(p), 0, sizeof(*(p)))

/*
 * Option flags.  Define these in numeric order.
 */
/* SVGA clock-related options */
#define OPTION_LEGEND		 0  /* Legend board with 32 clocks */
#define OPTION_SWAP_HIBIT	 1  /* WD90Cxx-swap high-order clock sel bit */
#define OPTION_8CLKS		 2  /* Probe 8 clocks instead of 4 (PVGA1) */
#define OPTION_16CLKS		 3  /* probe for 16 clocks instead of 8 */
#define OPTION_PROBE_CLKS	 4  /* Force clock probe for cards where a
				       set of preset clocks is used */
#define OPTION_HIBIT_HIGH	 5  /* Initial state of high order clock bit */
#define OPTION_HIBIT_LOW	 6
#define OPTION_CLKDIV2		 7  /* allow using clocks divided by 2 
				       in addition to bare clocks */
#define OPTION_HW_CLKS		 8  /* (ct) Hardware clocks */

/* Laptop display options */
#define OPTION_INTERN_DISP	10  /* Laptops - enable internal display (WD)*/
#define OPTION_EXTERN_DISP	11  /* Laptops - enable external display (WD)*/
#define OPTION_CLGD6225_LCD	12  /* Option to avoid setting the DAC to */
				   /* white on a clgd6225 with the LCD */
				   /* enabled */
#define OPTION_STN              13  /* DSTN option (CT)*/
#define OPTION_EXT_FRAM_BUF     14 /* external frame accelerator (CT) */
#define OPTION_LCD_STRETCH      15 /* disable LCD stretching */
#define OPTION_LCD_CENTER	16 /* enable LCD centering */
#define OPTION_PANEL_SIZE	17 /* (CT) Fix wrong panel size set in registers */

/* Memory options */
#define OPTION_FAST_DRAM	20 /* fast DRAM (for ET4000, S3, AGX) */
#define OPTION_MED_DRAM		21 /* medium speed DRAM (for S3, AGX) */
#define OPTION_SLOW_DRAM	22 /* slow DRAM (for Cirrus, S3, AGX) */
#define OPTION_NO_MEM_ACCESS	23 /* Unable to access video ram directly */
#define OPTION_NOLINEAR_MODE	24 /* chipset has broken linear access mode */
#define OPTION_INTEL_GX		25 /* Linear fb on an Intel GX/Pro (Mach32) */
#define OPTION_NO_2MB_BANKSEL	26 /* For cirrus cards with 512kx8 memory */
#define OPTION_FIFO_CONSERV	27 /* (cirrus) (agx) */
#define OPTION_FIFO_AGGRESSIVE	28 /* (cirrus) (agx) */
#define OPTION_MMIO		29 /* Use MMIO for Cirrus 543x */
#define OPTION_LINEAR		30 /* Use linear fb for Cirrus */
#define OPTION_FIFO_MODERATE  	31 /* (agx) */
#define OPTION_SLOW_VRAM	32 /* (s3) */
#define OPTION_SLOW_DRAM_REFRESH 33 /* (s3) */
#define OPTION_FAST_VRAM	34 /* (s3) */
#define OPTION_PCI_BURST_ON	35 /* W32/SVGA */
#define OPTION_PCI_BURST_OFF	36 /* W32/SVGA */
#define OPTION_W32_INTERLEAVE_ON  37 /* W32/SVGA */
#define OPTION_W32_INTERLEAVE_OFF 38 /* W32/SVGA */
#define OPTION_SLOW_EDODRAM	39 /* slow EDO-DRAM (for S3) */

/* Accel/cursor features */
#define OPTION_NOACCEL		40 /* Disable accel support in SVGA server */
#define OPTION_HW_CURSOR	41 /* Turn on HW cursor */
#define OPTION_SW_CURSOR	42 /* Turn off HW cursor (Mach32) */
#define OPTION_NO_BITBLT	43 /* Disable hardware bitblt (cirrus) */
#define OPTION_FAVOUR_BITBLT	44 /* Favour use of BitBLT (cirrus) */
#define OPTION_NO_IMAGEBLT	45 /* Avoid system-to-video BitBLT (cirrus) */
#define OPTION_NO_FONT_CACHE	46 /* Don't enable the font cache */
#define OPTION_NO_PIXMAP_CACHE	47 /* Don't enable the pixmap cache */
#define OPTION_TRIO32_FC_BUG	48 /* Workaround Trio32 font cache bug */
#define OPTION_S3_968_DASH_BUG	49 /* Workaround S3 968 dashed line bug */

/* RAMDAC options */
#define OPTION_BT485_CURS	50 /* Override Bt485 RAMDAC probe */
#define OPTION_TI3020_CURS	51 /* Use 3020 RAMDAC cursor (default) */
#define OPTION_NO_TI3020_CURS	52 /* Override 3020 RAMDAC cursor use */
#define OPTION_DAC_8_BIT	53 /* 8-bit DAC operation */
#define OPTION_SYNC_ON_GREEN	54 /* Set Sync-On-Green in RAMDAC */
#define OPTION_BT482_CURS       55 /* Use Bt482 RAMDAC cursor */
#define OPTION_S3_964_BT485_VCLK	56 /* probe/invert VCLK for 964 + Bt485 */
#define OPTION_TI3026_CURS	57 /* Use 3026 RAMDAC cursor (default) */
#define OPTION_IBMRGB_CURS	58 /* Use IBM RGB52x RAMDAC cursor (default) */
#define OPTION_DAC_6_BIT	59 /* 6-bit DAC operation */

/* Vendor specific options */
#define OPTION_SPEA_MERCURY	70 /* pixmux for SPEA Mercury (S3) */
#define OPTION_NUMBER_NINE	71 /* pixmux for #9 with Bt485 (S3) */
#define OPTION_STB_PEGASUS	72 /* pixmux for STB Pegasus (S3) */
#define OPTION_ELSA_W1000PRO	73 /* pixmux for ELSA Winner 1000PRO (S3) */
#define OPTION_ELSA_W2000PRO	74 /* pixmux for ELSA Winner 2000PRO (S3) */
#define OPTION_DIAMOND		75 /* Diamond boards (S3) */
#define OPTION_GENOA		76 /* Genoa boards (S3) */
#define OPTION_STB		77 /* STB boards (S3) */
#define OPTION_HERCULES		78 /* Hercules boards (S3) */
#define OPTION_MIRO_MAGIC_S4	79 /* miroMagic S4 with (S3) 928 and BT485 */

/* Misc options */
#define OPTION_CSYNC		90 /* Composite sync */
#define OPTION_SECONDARY	91 /* Use secondary address (HGC1280) */
#define OPTION_PCI_HACK		92 /* (S3) */
#define OPTION_POWER_SAVER	93 /* Power-down screen saver */
#define OPTION_OVERRIDE_BIOS	94 /* Override BIOS for Mach64 */
#define OPTION_NO_BLOCK_WRITE	95 /* No block write mode for Mach64 */
#define OPTION_BLOCK_WRITE	96 /* Block write mode for Mach64 */
#define OPTION_NO_BIOS_CLOCKS	97 /* Override BIOS clocks for Mach64 */
#define OPTION_S3_INVERT_VCLK	98 /* invert VLCK (CR67:0) (S3) */
#define OPTION_NO_PROGRAM_CLOCKS 99 /* Turn off clock programming */
#define OPTION_NO_PCI_PROBE	100 /* Disable PCI probe (VGA) */
#define OPTION_TRIO64VP_BUG1	101 /* Trio64V+ bug hack #1 */
#define OPTION_TRIO64VP_BUG2	102 /* Trio64V+ bug hack #2 */
#define OPTION_TRIO64VP_BUG3	103 /* Trio64V+ bug hack #3 */
#define OPTION_USE_MODELINE	104 /* use modeline for LCD instead of preset (ct)*/
#define OPTION_SUSPEND_HACK	105 /* (CT) Use different suspend/resume scheme */
#define OPTION_18_BIT_BUS	106 /* (CT) Use 18bit TFT bus for 24bpp mode */

/* Debugging options */
#define OPTION_SHOWCACHE	108 /* Allow cache to be seen (S3) */
#define OPTION_FB_DEBUG		109 /* Linear fb debug for S3 */

/* Some AGX Tuning/Debugging options -- several are for development testing */
#define OPTION_8_BIT_BUS        110 /* Force 8-bit CPU interface - MR1:0 */
#define OPTION_WAIT_STATE       111 /* Force 1 bus wait state - MR1:1<-1 */
#define OPTION_NO_WAIT_STATE    112 /* Force no bus wait state - MR:1<-0 */
#define OPTION_CRTC_DELAY       113 /* Select XGA Mode Delay - MR1:3 */
#define OPTION_VRAM_128         114 /* VRAM shift every 128 cycles - MR2:0 */
#define OPTION_VRAM_256         115 /* VRAM shift every 256 cycles - MR2:0 */
#define OPTION_REFRESH_20       116 /* # clocks between scr refreshs - MR3:5 */
#define OPTION_REFRESH_25       117 /* # clocks between scr refreshs - MR3:5 */
#define OPTION_VLB_A            118 /* VESA VLB transaction type A   - MR7:2 */
#define OPTION_VLB_B            119 /* VESA VLB transaction type B   - MR7:2 */
#define OPTION_SPRITE_REFRESH   120 /* Sprite refresh every hsync    - MR8:4 */
#define OPTION_SCREEN_REFRESH   121 /* Screen refresh during blank   - MR8:5 */
#define OPTION_VRAM_DELAY_LATCH	122 /* Delay Latch                   - MR7:3 */
#define OPTION_VRAM_DELAY_RAS	123 /* Delay RAS signal              - MR7:4 */
#define OPTION_VRAM_EXTEND_RAS  124 /* Extend the RAS signal         - MR8:6 */
#define OPTION_ENGINE_DELAY     125 /* Wait state for some VLB's     - MR5:3 */

/* Some options for oti087, debugging and fine tunning */
#define OPTION_CLOCK_50         130
#define OPTION_CLOCK_66         131
#define OPTION_NO_WAIT          132
#define OPTION_FIRST_WWAIT      133
#define OPTION_WRITE_WAIT       134
#define OPTION_ONE_WAIT         135
#define OPTION_READ_WAIT        136
#define OPTION_ALL_WAIT         137
#define OPTION_ENABLE_BITBLT    138


/* #ifdef PC98 */
#define OPTION_PCSKB		 140 /* SELECT EPSON PCSKB for S3 Server */
#define OPTION_PCSKB4		 141 /* SELECT EPSON PCSKB for S3 Server */
#define OPTION_PCHKB		 142 /* SELECT EPSON PCHKB for S3 Server */
#define OPTION_NECWAB		 143 /* SELECT NEC WAB-A/B for S3 Server */
#define OPTION_PW805I		 144 /* SELECT Canopus PW805i for S3 Server */
#define OPTION_PWLB		 145 /* SELECT Canopus PW_LB for S3 Server */
#define OPTION_PW968		 146 /* SELECT Canopus PW968 for S3 Server */
#define OPTION_GA98NB1           150 /* SELECT IO DATA GA-98NB1 for SVGA */
#define OPTION_GA98NB2           151 /* SELECT IO DATA GA-98NB2 for SVGA */
#define OPTION_GA98NB4           152 /* SELECT IO DATA GA-98NB4 for SVGA */
#define OPTION_WAP               153 /* SELECT MELCO WAP-2000/4000 for SVGA */
#define OPTION_NEC_CIRRUS        154 /* SELECT NEC Internal Server for SVGA */
#define OPTION_EPSON_MEM_WIN	 161 /* ENABLE mem-window 0xF00000 for EPSON */
#define OPTION_PW_MUX            162 /* ENABLE MUX on PW928II */
#define OPTION_NOINIT		 163 /* Not Initialize SDAC & VGA Registers */
#define OPTION_PC98TGUI		 170 /* SELECT NEC TGUI9660 */
/* #endif */

#define OPTION_TGUI_PCI_READ_OFF 171 /* Trident TGUI PCI burst read */
#define OPTION_TGUI_PCI_WRITE_OFF 172 /* Trident TGUI PCI burst write */

#define CLOCK_OPTION_PROGRAMABLE 0 /* has a programable clock */
#define CLOCK_OPTION_ICD2061A	 1 /* use ICD 2061A programable clocks      */
#define CLOCK_OPTION_SC11412     3 /* use SC11412 programmable clocks */
#define CLOCK_OPTION_S3GENDAC    4 /* use S3 Gendac programmable clocks */
#define CLOCK_OPTION_TI3025      5 /* use TI3025 programmable clocks */
#define CLOCK_OPTION_ICS2595     6 /* use ICS2595 programmable clocks */
#define CLOCK_OPTION_CIRRUS      7 /* use Cirrus programmable clocks */
#define CLOCK_OPTION_CH8391      8 /* use Chrontel 8391 programmable clocks */
#define CLOCK_OPTION_ICS5342     9 /* use ICS 5342 programmable clocks */
#define CLOCK_OPTION_S3TRIO     10 /* use S3 Trio32/64 programmable clocks */
#define CLOCK_OPTION_TI3026     11 /* use TI3026 programmable clocks */
#define CLOCK_OPTION_IBMRGB     12 /* use IBM RGB52x programmable clocks */
#define CLOCK_OPTION_STG1703    13 /* use STG1703 programmable clocks */
#define CLOCK_OPTION_ICS5341    14 /* use ICS 5341 (ET4000W32p) */
#define CLOCK_OPTION_TRIDENT    15 /* use programmable clock on TGUI */
#define CLOCK_OPTION_ATT409     16 /* use ATT20C409 programmable clock */
#define CLOCK_OPTION_CH8398     17 /* use Chrontel 8398 programmable clock */
#define CLOCK_OPTION_GLORIA8    18 /* use ELSA Gloria-8 TVP3030/ICS9161 clock */
#define CLOCK_OPTION_ET6000     19 /* use ET6000 built-in programmable clock */
#define CLOCK_OPTION_ICS1562    20 /* used for TGA server */

/*
 * Table to map option strings to tokens.
 */
typedef struct {
  char *name;
  int  token;
} OptFlagRec, *OptFlagPtr;

#ifdef INIT_OPTIONS
OptFlagRec xf86_OptionTab[] = {
  { "legend",		OPTION_LEGEND },
  { "swap_hibit",	OPTION_SWAP_HIBIT },
  { "8clocks",		OPTION_8CLKS },
  { "16clocks",		OPTION_16CLKS },
  { "probe_clocks",	OPTION_PROBE_CLKS },
  { "hibit_high",	OPTION_HIBIT_HIGH },
  { "hibit_low",	OPTION_HIBIT_LOW },
  { "clkdiv2",		OPTION_CLKDIV2 },
  { "hw_clocks",        OPTION_HW_CLKS },

  { "intern_disp",	OPTION_INTERN_DISP },
  { "extern_disp",	OPTION_EXTERN_DISP },
  { "clgd6225_lcd",	OPTION_CLGD6225_LCD },
  { "stn",              OPTION_STN},
  { "ext_fram_buf",	OPTION_EXT_FRAM_BUF },
  { "no_stretch",	OPTION_LCD_STRETCH },
  { "lcd_center",	OPTION_LCD_CENTER },
  { "lcd_centre",	OPTION_LCD_CENTER },
  { "fix_panel_size",	OPTION_PANEL_SIZE },

  { "fast_dram",	OPTION_FAST_DRAM },
  { "med_dram",		OPTION_MED_DRAM },
  { "slow_dram",	OPTION_SLOW_DRAM },
  { "slow_edodram",	OPTION_SLOW_EDODRAM },
  { "nomemaccess",	OPTION_NO_MEM_ACCESS },
  { "nolinear",		OPTION_NOLINEAR_MODE },
  { "intel_gx",		OPTION_INTEL_GX },
  { "no_2mb_banksel",	OPTION_NO_2MB_BANKSEL },
  { "fifo_conservative",OPTION_FIFO_CONSERV },
  { "fifo_moderate",    OPTION_FIFO_MODERATE },
  { "fifo_aggressive",	OPTION_FIFO_AGGRESSIVE },
  { "mmio",		OPTION_MMIO },
  { "linear",		OPTION_LINEAR },
  { "slow_vram",	OPTION_SLOW_VRAM },
  { "s3_slow_vram",	OPTION_SLOW_VRAM },
  { "slow_dram_refresh",OPTION_SLOW_DRAM_REFRESH },
  { "s3_slow_dram_refresh",OPTION_SLOW_DRAM_REFRESH },
  { "fast_vram",	OPTION_FAST_VRAM },
  { "s3_fast_vram",	OPTION_FAST_VRAM },
  { "pci_burst_on",	OPTION_PCI_BURST_ON },
  { "pci_burst_off",	OPTION_PCI_BURST_OFF },
  { "w32_interleave_on",OPTION_W32_INTERLEAVE_ON },
  { "w32_interleave_off",OPTION_W32_INTERLEAVE_OFF },
  { "tgui_pci_read_off",OPTION_TGUI_PCI_READ_OFF },
  { "tgui_pci_write_off",OPTION_TGUI_PCI_WRITE_OFF },

  { "noaccel",		OPTION_NOACCEL },
  { "hw_cursor",	OPTION_HW_CURSOR },
  { "sw_cursor",	OPTION_SW_CURSOR },
  { "no_bitblt",	OPTION_NO_BITBLT },
  { "favour_bitblt",	OPTION_FAVOUR_BITBLT },
  { "favor_bitblt",	OPTION_FAVOUR_BITBLT },
  { "no_imageblt",	OPTION_NO_IMAGEBLT },
  { "no_font_cache",	OPTION_NO_FONT_CACHE },
  { "no_pixmap_cache",	OPTION_NO_PIXMAP_CACHE },
  { "trio32_fc_bug",	OPTION_TRIO32_FC_BUG },
  { "s3_968_dash_bug",	OPTION_S3_968_DASH_BUG },

  { "bt485_curs",	OPTION_BT485_CURS },
  { "ti3020_curs",	OPTION_TI3020_CURS },
  { "no_ti3020_curs",	OPTION_NO_TI3020_CURS },
  { "ti3026_curs",	OPTION_TI3026_CURS },
  { "ibmrgb_curs",	OPTION_IBMRGB_CURS },
  { "dac_8_bit",	OPTION_DAC_8_BIT },
  { "sync_on_green",    OPTION_SYNC_ON_GREEN },
  { "bt482_curs",	OPTION_BT482_CURS },
  { "s3_964_bt485_vclk",OPTION_S3_964_BT485_VCLK },
  { "dac_6_bit",	OPTION_DAC_6_BIT },

  { "spea_mercury",	OPTION_SPEA_MERCURY },
  { "number_nine",	OPTION_NUMBER_NINE },
  { "stb_pegasus",	OPTION_STB_PEGASUS },
  { "elsa_w1000pro",	OPTION_ELSA_W1000PRO },
  { "elsa_w1000isa",	OPTION_ELSA_W1000PRO }, /* These are treated the same */
  { "elsa_w2000pro",	OPTION_ELSA_W2000PRO },
  { "diamond",		OPTION_DIAMOND },
  { "genoa",		OPTION_GENOA },
  { "stb",		OPTION_STB },
  { "hercules",		OPTION_HERCULES },
  { "miro_magic_s4",	OPTION_MIRO_MAGIC_S4},

  { "composite",	OPTION_CSYNC },
  { "secondary",	OPTION_SECONDARY },
  { "pci_hack",		OPTION_PCI_HACK },
  { "power_saver",	OPTION_POWER_SAVER },
  { "override_bios",	OPTION_OVERRIDE_BIOS },
  { "no_block_write",	OPTION_NO_BLOCK_WRITE },
  { "block_write",	OPTION_BLOCK_WRITE },
  { "no_bios_clocks",	OPTION_NO_BIOS_CLOCKS },
  { "s3_invert_vclk",	OPTION_S3_INVERT_VCLK },
  { "no_program_clocks",OPTION_NO_PROGRAM_CLOCKS },
  { "no_pci_probe",	OPTION_NO_PCI_PROBE },
  { "trio64v+_bug1",	OPTION_TRIO64VP_BUG1 },
  { "trio64v+_bug2",	OPTION_TRIO64VP_BUG2 },
  { "trio64v+_bug3",	OPTION_TRIO64VP_BUG3 },
  { "use_modeline",	OPTION_USE_MODELINE },
  { "suspend_hack",	OPTION_SUSPEND_HACK },
  { "use_18bit_bus",	OPTION_18_BIT_BUS },

  { "showcache",	OPTION_SHOWCACHE },
  { "fb_debug",		OPTION_FB_DEBUG },

/* #ifdef PC98 */
  { "pcskb",		OPTION_PCSKB },
  { "pcskb4",		OPTION_PCSKB4 },
  { "pchkb",		OPTION_PCHKB },
  { "necwab",		OPTION_NECWAB },
  { "pw805i",		OPTION_PW805I },
  { "pw_localbus",	OPTION_PWLB },
  { "pw968",		OPTION_PW968 },
  { "ga98nb1",		OPTION_GA98NB1 },
  { "ga98nb2",		OPTION_GA98NB2 },
  { "ga98nb4",		OPTION_GA98NB4 },
  { "wap",		OPTION_WAP },
  { "nec_cirrus",	OPTION_NEC_CIRRUS },
  { "epsonmemwin",	OPTION_EPSON_MEM_WIN },
  { "pw_mux",		OPTION_PW_MUX },
  { "noinit",		OPTION_NOINIT },
  { "pc98_tgui",	OPTION_PC98TGUI },
/* #endif */

  { "8_bit_bus",        OPTION_8_BIT_BUS },
  { "wait_state",       OPTION_WAIT_STATE },
  { "no_wait_state",    OPTION_NO_WAIT_STATE },
  { "crtc_delay",       OPTION_CRTC_DELAY },
  { "vram_128",         OPTION_VRAM_128 },
  { "vram_256",         OPTION_VRAM_256 },
  { "refresh_20",       OPTION_REFRESH_20 },
  { "refresh_25",       OPTION_REFRESH_25 },
  { "vlb_a",            OPTION_VLB_A },
  { "vlb_b",            OPTION_VLB_B },
  { "sprite_refresh",   OPTION_SPRITE_REFRESH },
  { "screen_refresh",   OPTION_SPRITE_REFRESH },
  { "vram_delay_latch", OPTION_VRAM_DELAY_LATCH },
  { "vram_delay_ras",   OPTION_VRAM_DELAY_RAS },
  { "vram_extend_ras",  OPTION_VRAM_EXTEND_RAS },
  { "engine_delay",     OPTION_ENGINE_DELAY },
  { "clock_50",         OPTION_CLOCK_50 },
  { "clock_66",         OPTION_CLOCK_66 },
  { "no_wait",          OPTION_NO_WAIT },
  { "first_wwait",      OPTION_FIRST_WWAIT },
  { "write_wait",       OPTION_WRITE_WAIT },
  { "one_wait",         OPTION_ONE_WAIT },
  { "read_wait",        OPTION_READ_WAIT },
  { "all_wait",         OPTION_ALL_WAIT },
  { "enable_bitblt",    OPTION_ENABLE_BITBLT },

  { "",			-1 },
};

OptFlagRec xf86_ClockOptionTab [] = {
  { "icd2061a",		CLOCK_OPTION_ICD2061A },  /* generic ICD2061A */
  { "ics9161a",		CLOCK_OPTION_ICD2061A },  /* ICD2061A compatible */
  { "dcs2824",		CLOCK_OPTION_ICD2061A },  /* ICD2061A compatible */
  { "sc11412", 		CLOCK_OPTION_SC11412 },   /* Sierra SC11412 */
  { "s3gendac",		CLOCK_OPTION_S3GENDAC },  /* S3 gendac */
  { "s3_sdac",		CLOCK_OPTION_S3GENDAC },  /* S3 SDAC */
  { "ics5300",		CLOCK_OPTION_S3GENDAC },  /* S3 gendac compatible */
  { "ics5342",		CLOCK_OPTION_ICS5342 },   /* not completely S3 SDAC compatible */
  { "s3_trio",		CLOCK_OPTION_S3TRIO },    /* S3 Trio32/64 */
  { "s3_trio32",	CLOCK_OPTION_S3TRIO },    /* S3 Trio32/64 */
  { "s3_trio64",	CLOCK_OPTION_S3TRIO },    /* S3 Trio32/64 */
  { "ti3025",		CLOCK_OPTION_TI3025 },    /* TI3025 */
  { "ti3026",		CLOCK_OPTION_TI3026 },    /* TI3026 */
  { "ti3030",		CLOCK_OPTION_TI3026 },    /* TI3030 is TI3026 compatible */
  { "ibm_rgb514",	CLOCK_OPTION_IBMRGB },    /* IBM RGB514 */
  { "ibm_rgb524",	CLOCK_OPTION_IBMRGB },    /* IBM RGB524 */
  { "ibm_rgb525",	CLOCK_OPTION_IBMRGB },    /* IBM RGB525 */
  { "ibm_rgb528",	CLOCK_OPTION_IBMRGB },    /* IBM RGB528 */
  { "ibm_rgb51x",	CLOCK_OPTION_IBMRGB },    /* IBM RGB51x */
  { "ibm_rgb52x",	CLOCK_OPTION_IBMRGB },    /* IBM RGB52x */
  { "ibm_rgb5xx",	CLOCK_OPTION_IBMRGB },    /* IBM RGB52x */
  { "ics2595",		CLOCK_OPTION_ICS2595 },   /* ICS2595 */
  { "cirrus",		CLOCK_OPTION_CIRRUS }, 	  /* Cirrus built-in */
  { "ch8391",		CLOCK_OPTION_CH8391 }, 	  /* Chrontel 8391  */
  { "stg1703",		CLOCK_OPTION_STG1703 },   /* STG1703 */
  { "ics5341",		CLOCK_OPTION_ICS5341 },   /* ET4000 W32p version of S3 SDAC/ICS5341 */
  { "tgui",		CLOCK_OPTION_TRIDENT },   /* Trident TGUI built-in */
  { "att20c409",	CLOCK_OPTION_ATT409 },    /* ATT20C409 */
  { "att20c499",	CLOCK_OPTION_ATT409 },    /* ATT20C499, 409 compatible */
  { "att20c408",	CLOCK_OPTION_ATT409 },    /* ATT20C408, 409 compatible */
  { "ch8398",		CLOCK_OPTION_CH8398 },    /* Chrontel 8398 */
  { "ati18818",		CLOCK_OPTION_ICS2595 },   /* ATI18818, ICS2595 compatible */
  { "et6000",		CLOCK_OPTION_ET6000 },    /* ET6000 */
  { "",			-1 },
};

#else
extern OptFlagRec xf86_OptionTab[];
extern OptFlagRec xf86_ClockOptionTab[];
#endif

#endif /* _XF86_OPTION_H */

