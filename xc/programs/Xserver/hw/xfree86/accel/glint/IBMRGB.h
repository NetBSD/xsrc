/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/IBMRGB.h,v 1.5.2.1 1998/07/30 06:23:38 hohndel Exp $ */
/*
 * IBM RGB526 registers
 */


/* #define COMPATIBELMODE */

#define STANDARTMODE

#define IBMRGB_REF_FREQ_1       14.31818
#define IBMRGB_REF_FREQ_2       50.00000

#ifdef GLINT_SERVER
#define IBMRGB_WRITE_ADDR           0x4000  
#define IBMRGB_RAMDAC_DATA          0x4008 
#define IBMRGB_PIXEL_MASK           0x4010 
#define IBMRGB_READ_ADDR            0x4018  
#define IBMRGB_INDEX_LOW            0x4020 
#define IBMRGB_INDEX_HIGH           0x4028 
#define IBMRGB_INDEX_DATA           0x4030  
#define IBMRGB_INDEX_CONTROL        0x4038 
#endif

#define IBMRGB_rev		0x00
#define IBMRGB_id		0x01
#define IBMRGB_misc_clock	0x02
#define IBMRGB_sync		0x03
#define IBMRGB_hsync_pos	0x04
#define IBMRGB_pwr_mgmt		0x05
#define IBMRGB_dac_op		0x06
#define IBMRGB_pal_ctrl		0x07
#define IBMRGB_sysclk		0x08  /* not RGB525 */
#define IBMRGB_pix_fmt		0x0a
#define IBMRGB_8bpp		0x0b
#define IBMRGB_16bpp		0x0c
#define IBMRGB_24bpp		0x0d
#define IBMRGB_32bpp		0x0e
#define IBMRGB_pll_ctrl1	0x10
#define IBMRGB_pll_ctrl2	0x11
#define IBMRGB_pll_ref_div_fix	0x14
#define IBMRGB_sysclk_ref_div	0x15  /* not RGB525 */
#define IBMRGB_sysclk_vco_div	0x16  /* not RGB525 */
/* #define IBMRGB_f0		0x20 */

#define IBMRGB_sysclk_n		0x15
#define IBMRGB_sysclk_m		0x16
#define IBMRGB_sysclk_p		0x17
#define IBMRGB_sysclk_c		0x18

#define IBMRGB_m0		0x20
#define IBMRGB_n0		0x21
#define IBMRGB_p0		0x22
#define IBMRGB_c0		0x23
#define IBMRGB_m1		0x24
#define IBMRGB_n1		0x25
#define IBMRGB_p1		0x26
#define IBMRGB_c1		0x27
#define IBMRGB_m2		0x28
#define IBMRGB_n2		0x29
#define IBMRGB_p2		0x2a
#define IBMRGB_c2		0x2b
#define IBMRGB_m3		0x2c
#define IBMRGB_n3		0x2d
#define IBMRGB_p3		0x2e
#define IBMRGB_c3		0x2f

#define IBMRGB_curs		0x30
#define IBMRGB_curs_xl		0x31
#define IBMRGB_curs_xh		0x32
#define IBMRGB_curs_yl		0x33
#define IBMRGB_curs_yh		0x34
#define IBMRGB_curs_hot_x	0x35
#define IBMRGB_curs_hot_y	0x36
#define IBMRGB_curs_col1_r	0x40
#define IBMRGB_curs_col1_g	0x41
#define IBMRGB_curs_col1_b	0x42
#define IBMRGB_curs_col2_r	0x43
#define IBMRGB_curs_col2_g	0x44
#define IBMRGB_curs_col2_b	0x45
#define IBMRGB_curs_col3_r	0x46
#define IBMRGB_curs_col3_g	0x47
#define IBMRGB_curs_col3_b	0x48
#define IBMRGB_border_col_r	0x60
#define IBMRGB_border_col_g	0x61
#define IBMRGB_botder_col_b	0x62
#define IBMRGB_misc1		0x70
#define IBMRGB_misc2		0x71
#define IBMRGB_misc3		0x72
#define IBMRGB_misc4		0x73  /* not RGB525 */
#define IBMRGB_dac_sense	0x82
#define IBMRGB_misr_r		0x84
#define IBMRGB_misr_g		0x86
#define IBMRGB_misr_b		0x88
#define IBMRGB_pll_vco_div_in	0x8e
#define IBMRGB_pll_ref_div_in	0x8f
#define IBMRGB_vram_mask_0	0x90
#define IBMRGB_vram_mask_1	0x91
#define IBMRGB_vram_mask_2	0x92
#define IBMRGB_vram_mask_3	0x93
#define IBMRGB_curs_array	0x100



/* Constants rgb525.h */  

/* RGB525_REVISION_LEVEL */
#define RGB525_PRODUCT_REV_LEVEL        0xf0

/* RGB525_ID */
#define RGB525_PRODUCT_ID               0x01

/* RGB525_MISC_CTRL_1 */
#define MISR_CNTL_ENABLE                0x80
#define VMSK_CNTL_ENABLE                0x40
#define PADR_RDMT_RDADDR                0x0
#define PADR_RDMT_PAL_STATE             0x20
#define SENS_DSAB_DISABLE               0x10
#define SENS_SEL_BIT3                   0x0
#define SENS_SEL_BIT7                   0x08
#define VRAM_SIZE_32                    0x0
#define VRAM_SIZE_64                    0x01

/* RGB525_MISC_CTRL_2 */
#define PCLK_SEL_LCLK                   0x0
#define PCLK_SEL_PLL                    0x40
#define PCLK_SEL_EXT                    0x80
#define INTL_MODE_ENABLE                0x20
#define BLANK_CNTL_ENABLE               0x10
#define COL_RES_6BIT                    0x0
#define COL_RES_8BIT                    0x04
#define PORT_SEL_VGA                    0x0
#define PORT_SEL_VRAM                   0x01

/* RGB525_MISC_CTRL_3 */
#define SWAP_RB                         0x80
#define SWAP_WORD_LOHI                  0x0
#define SWAP_WORD_HILO                  0x10
#define SWAP_NIB_HILO                   0x0
#define SWAP_NIB_LOHI                   0x02

/* RGB525_MISC_CLK_CTRL */
#define DDOT_CLK_ENABLE                 0x0
#define DDOT_CLK_DISABLE                0x80
#define SCLK_ENABLE                     0x0
#define SCLK_DISABLE                    0x40
#define B24P_DDOT_PLL                   0x0
#define B24P_DDOT_SCLK                  0x20
#define DDOT_DIV_PLL_1                  0x0
#define DDOT_DIV_PLL_2                  0x02
#define DDOT_DIV_PLL_4                  0x04
#define DDOT_DIV_PLL_8                  0x06
#define DDOT_DIV_PLL_16                 0x08
#define PLL_DISABLE                     0x0
#define PLL_ENABLE                      0x01

/* RGB525_SYNC_CTRL */
#define DLY_CNTL_ADD                    0x0
#define DLY_SYNC_NOADD                  0x80
#define CSYN_INVT_DISABLE               0x0
#define CSYN_INVT_ENABLE                0x40
#define VSYN_INVT_DISABLE               0x0
#define VSYN_INVT_ENABLE                0x20
#define HSYN_INVT_DISABLE               0x0
#define HSYN_INVT_ENABLE                0x10
#define VSYN_CNTL_NORMAL                0x0
#define VSYN_CNTL_HIGH                  0x04
#define VSYN_CNTL_LOW                   0x08
#define VSYN_CNTL_DISABLE               0x0C
#define HSYN_CNTL_NORMAL                0x0
#define HSYN_CNTL_HIGH                  0x01
#define HSYN_CNTL_LOW                   0x02
#define HSYN_CNTL_DISABLE               0x03

/* RGB525_HSYNC_CTRL */
#define HSYN_POS(n)                     (n)

/* RGB525_POWER_MANAGEMENT */
#define SCLK_PWR_NORMAL                 0x0
#define SCLK_PWR_DISABLE                0x10
#define DDOT_PWR_NORMAL                 0x0
#define DDOT_PWR_DISABLE                0x08
#define SYNC_PWR_NORMAL                 0x0
#define SYNC_PWR_DISABLE                0x04
#define ICLK_PWR_NORMAL                 0x0
#define ICLK_PWR_DISABLE                0x02
#define DAC_PWR_NORMAL                  0x0
#define DAC_PWR_DISABLE                 0x01

/* RGB525_DAC_OPERATION */
#define SOG_DISABLE                     0x0
#define SOG_ENABLE                      0x08
#define BRB_NORMAL                      0x0
#define BRB_ALWAYS                      0x04
#define DSR_DAC_SLOW                    0x02
#define DSR_DAC_FAST                    0x0
#define DPE_DISABLE                     0x0
#define DPE_ENABLE                      0x01

/* RGB525_PALETTE_CTRL */
#define SIXBIT_LINEAR_ENABLE            0x0
#define SIXBIT_LINEAR_DISABLE           0x80
#define PALETTE_PARITION(n)             (n)

/* RGB525_PIXEL_FORMAT */
#define PIXEL_FORMAT_4BPP               0x02
#define PIXEL_FORMAT_8BPP               0x03
#define PIXEL_FORMAT_16BPP              0x04
#define PIXEL_FORMAT_24BPP              0x05
#define PIXEL_FORMAT_32BPP              0x06

/* RGB525_8BPP_CTRL */
#define B8_DCOL_INDIRECT                0x0
#define B8_DCOL_DIRECT                  0x01

/* RGB525_16BPP_CTRL */
#define B16_DCOL_INDIRECT               0x0
#define B16_DCOL_DYNAMIC                0x40
#define B16_DCOL_DIRECT                 0xC0
#define B16_POL_FORCE_BYPASS            0x0
#define B16_POL_FORCE_LOOKUP            0x20
#define B16_ZIB                         0x0
#define B16_LINEAR                      0x04
#define B16_555                         0x0
#define B16_565                         0x02
#define B16_SPARSE                      0x0
#define B16_CONTIGUOUS                  0x01

/* RGB525_24BPP_CTRL */
#define B24_DCOL_INDIRECT               0x0
#define B24_DCOL_DIRECT                 0x01

/* RGB525_32BPP_CTRL */
#define B32_POL_FORCE_BYPASS            0x0
#define B32_POL_FORCE_LOOKUP            0x04
#define B32_DCOL_INDIRECT               0x0
#define B32_DCOL_DYNAMIC                0x01
#define B32_DCOL_DIRECT                 0x03

/* RGB525_PLL_CTRL_1 */
#define REF_SRC_REFCLK                  0x0
#define REF_SRC_EXTCLK                  0x10
#define PLL_EXT_FS_3_0                  0x0
#define PLL_EXT_FS_2_0                  0x01
#define PLL_CNTL2_3_0                   0x02
#define PLL_CNTL2_2_0                   0x03

/* RGB525_PLL_CTRL_2 */
#define PLL_INT_FS_3_0(n)               (n)
#define PLL_INT_FS_2_0(n)               (n)

/* RGB525_PLL_REF_DIV_COUNT */
#define REF_DIV_COUNT(n)                (n)

/* RGB525_F0 - RGB525_F15 */
#define VCO_DIV_COUNT(n)                (n)

/* RGB525_PLL_REFCLK values */
#define RGB525_PLL_REFCLK_MHz(n)        ((n)/2)

/* RGB525_CURSOR_CONTROL */
#define SMLC_PART_0                     0x0
#define SMLC_PART_1                     0x40
#define SMLC_PART_2                     0x80
#define SMLC_PART_3                     0xC0
#define PIX_ORDER_RL                    0x0
#define PIX_ORDER_LR                    0x20
#define LOC_READ_LAST                   0x0
#define LOC_READ_ACTUAL                 0x10
#define UPDT_CNTL_DELAYED               0x0
#define UPDT_CNTL_IMMEDIATE             0x08
#define CURSOR_SIZE_32                  0x0
#define CURSOR_SIZE_64                  0x40
#define CURSOR_MODE_OFF                 0x0
#define CURSOR_MODE_3_COLOR             0x01
#define CURSOR_MODE_2_COLOR_HL          0x02
#define CURSOR_MODE_2_COLOR             0x03

/* RGB525_REVISION_LEVEL */
#define REVISION_LEVEL                  0xF0    /* predefined */

/* RGB525_ID */
#define ID_CODE                         0x01    /* predefined */

/* MISR status */
#define RGB525_MISR_DONE                0x01

/* the IBMRGB640 is rather different from the rest of the RAMDACs,
   so we define a completely new set of register names for it */
#define RGB640_SER_07_00		0x02
#define RGB640_SER_15_08		0x03
#define RGB640_SER_23_16		0x04
#define RGB640_SER_31_24		0x05
#define RGB640_SER_WID_03_00		0x06
#define RGB640_SER_WID_07_04		0x07
#define RGB640_SER_MODE			0x08
#define		IBM640_SER_2_1	0x00
#define		IBM640_SER_4_1	0x01
#define		IBM640_SER_8_1	0x02
#define		IBM640_SER_16_1	0x03
#define		IBM640_SER_16_3	0x05
#define		IBM640_SER_5_1	0x06
#define RGB640_PIXEL_INTERLEAVE		0x09
#define RGB640_MISC_CONF		0x0a
#define		IBM640_PCLK		0x00
#define		IBM640_PCLK_2		0x40
#define		IBM640_PCLK_4		0x80
#define		IBM640_PCLK_8		0xc0
#define		IBM640_PSIZE10		0x10
#define		IBM640_LCI		0x08
#define		IBM640_WIDCTL_MASK	0x07
#define RGB640_VGA_CONTROL		0x0b
#define 	IBM640_RDBK	0x04
#define 	IBM640_PSIZE8	0x02
#define		IBM640_VRAM	0x01
#define RGB640_DAC_CONTROL		0x0d
#define		IBM640_MONO	0x08
#define		IBM640_DACENBL	0x04
#define		IBM640_SHUNT	0x02
#define		IBM640_SLOWSLEW	0x01
#define RGB640_OUTPUT_CONTROL		0x0e
#define		IBM640_RDAI	0x04
#define		IBM640_WDAI	0x02
#define		IBM640_WATCTL	0x01
#define RGB640_SYNC_CONTROL		0x0f
#define		IBM640_PWR	0x20
#define		IBM640_VSP	0x10
#define		IBM640_HSP	0x08
#define		IBM640_CSE	0x04
#define		IBM640_CSG	0x02
#define		IBM640_BPE	0x01
#define RGB640_PLL_N			0x10
#define RGB640_PLL_M			0x11
#define RGB640_PLL_P			0x12
#define RGB640_PLL_CTL			0x13
#define 	IBM640_PLL_EN	0x04
#define		IBM640_PLL_HIGH	0x10
#define		IBM640_PLL_LOW	0x01
#define RGB640_AUX_PLL_CTL		0x17
#define		IBM640_AUXPLL	0x04
#define		IBM640_AUX_HI	0x02
#define		IBM640_AUX_LO	0x01
#define RGB640_VRAM_MASK0		0xf0
#define RGB640_VRAM_MASK1		0xf1
#define RGB640_VRAM_MASK2		0xf2
/*
 default clock speed in Hz for the reference board. The actual speed
 is looked up in the registry. Use this if no registry entry is found
 or the registry entry is zero.
*/

#define GLINT_DEFAULT_CLOCK_SPEED_SX_REV1   (40 * (1000*1000))
#define GLINT_DEFAULT_CLOCK_SPEED_SX_REV2   (50 * (1000*1000))
#define GLINT_DEFAULT_CLOCK_SPEED_DELTA     (40 * (1000*1000))
#define GLINT_DEFAULT_CLOCK_SPEED           (50 * (1000*1000))  /* TX, Dual TX and SX rev 2 */ 
#define GLINT_DEFAULT_CLOCK_SPEED_MX        (66 * (1000*1000))  /* MX */
#define GLINT_DEFAULT_CLOCK_SPEED_DUAL_MX   (62 * (1000*1000))  /* Dual MX */

#define PERMEDIA_DEFAULT_CLOCK_SPEED        ( 60 * (1000*1000))
#define PERMEDIA_4MB_DEFAULT_CLOCK_SPEED    ( 70 * (1000*1000))
#define PERMEDIA_8MB_DEFAULT_CLOCK_SPEED    ( 60 * (1000*1000))
#define PERMEDIA_LC_DEFAULT_CLOCK_SPEED     ( 83 * (1000*1000))
#define MAX_PERMEDIA_CLOCK_SPEED            (100 * (1000*1000))
#define PERMEDIA_REF_CLOCK_SPEED            14318200

#define PERMEDIA2_DEFAULT_CLOCK_SPEED		( 80 * (1000*1000))

#define PERMEDIA_VGA_CTRL_INDEX     5
#define PERMEDIA_VGA_ENABLE         0x08
#define PERMEDIA_VGA_MEMORYACCESS   0x01
/* #define PERMEDIA_VGA_HOSTDACACCESS  0x02 */
/* #define PERMEDIA_VGA_IRQ            0x04 */
#define PERMEDIA_VGA_DAC3           0x20
#define PERMEDIA_VGA_DAC4           0x10


/* PERMEDIA memory mapped VGA access */
#define PERMEDIA_MMVGA_INDEX_REG    0x63C4
#define PERMEDIA_MMVGA_DATA_REG     0x63C5
