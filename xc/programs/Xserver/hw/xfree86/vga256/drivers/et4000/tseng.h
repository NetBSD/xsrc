
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/et4000/tseng.h,v 3.0.2.7 1998/02/27 01:29:28 dawes Exp $ */

#ifndef _TSENG_H
#define _TSENG_H

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86Priv.h"

#include "vga.h"

#define MAX_TSENG_CLOCK 86000

typedef struct {
  unsigned char cmd_reg;
  unsigned char PLL_f2_M;
  unsigned char PLL_f2_N;
  unsigned char PLL_ctrl;
  unsigned char PLL_w_idx;
  unsigned char PLL_r_idx;
  unsigned char timingctrl; /* for STG170x */
  } GenDACstate;


typedef struct {
  vgaHWRec std;                  /* good old IBM VGA */
  unsigned char SegMapComp;      /* CRTC 0x30 */
  unsigned char GenPurp;         /* CRTC 0x31 */
  unsigned char RCConf;          /* CRTC 0x32 */
  unsigned char ExtStart;        /* CRTC 0x33 */
  unsigned char Compatibility;   /* CRTC 0x34 */
  unsigned char OverflowHigh;    /* CRTC 0x35 */
  unsigned char VSConf1;         /* CRTC 0x36 */
  unsigned char VSConf2;         /* CRTC 0x37 */
  unsigned char HorOverflow;     /* CRTC 0x3F */
  unsigned char StateControl;    /* TS  0x06 */
  unsigned char AuxillaryMode;   /* TS  0x07 */
  unsigned char Misc;            /* ATC 0x16 */
  unsigned char SegSel1, SegSel2;  /* 0x3CD, 0x3CB */
  unsigned char ET6KMemBase;     /* ET6000 0x13 -- linear memory mapped address */
  unsigned char ET6KMMAPCtrl;    /* ET6000 0x40 -- used for linear memory mapping */
  unsigned char ET6KPerfContr;   /* ET6000 0x41 -- system performance control */
  unsigned char ET6KRasCas;      /* ET6000 0x44 -- ram delays configuration */
  unsigned char ET6KDispFeat;    /* ET6000 0x46 -- display feature register */
  unsigned char ET6KVidCtrl1;    /* ET6000 0x58 -- used for 15/16 bpp modes */
  unsigned char MClkM, MClkN;    /* memory clock values */
  unsigned char IMAPortCtrl;     /* IMA port control register (0x217B index 0xF7) */
  GenDACstate gendac;
  unsigned char ATTdac_cmd;      /* command register for ATT 49x DACs */
  } vgaET4000Rec, *vgaET4000Ptr;

extern vgaVideoChipRec ET4000;

typedef enum {
    TYPE_UNKNOWN = -1,
    TYPE_ET4000,
    TYPE_ET4000W32,
    TYPE_ET4000W32I,
    TYPE_ET4000W32Ib,
    TYPE_ET4000W32Ic,
    TYPE_ET4000W32P,
    TYPE_ET4000W32Pa,
    TYPE_ET4000W32Pb,
    TYPE_ET4000W32Pc,
    TYPE_ET4000W32Pd,
    TYPE_ET6000,
    TYPE_ET6100,
    TYPE_ET6300
} t_tseng_type;

#define Is_W32_any  ( (et4000_type >= TYPE_ET4000W32) && (et4000_type < TYPE_ET6000) )
#define Is_ET6K     ( et4000_type >= TYPE_ET6000 )
#define Is_W32      ( (et4000_type >= TYPE_ET4000W32) && (et4000_type < TYPE_ET4000W32I) )
#define Is_W32i     ( (et4000_type >= TYPE_ET4000W32I) && (et4000_type < TYPE_ET4000W32P) )
#define Is_W32_W32i ( (et4000_type >= TYPE_ET4000W32) && (et4000_type < TYPE_ET4000W32P) )
#define Is_W32p     ( (et4000_type >= TYPE_ET4000W32P) && (et4000_type < TYPE_ET6000) )
#define Is_W32p_ab  ( (et4000_type >= TYPE_ET4000W32P) && (et4000_type <= TYPE_ET4000W32Pb) )
#define Is_W32p_cd  ( (et4000_type >= TYPE_ET4000W32Pc) && (et4000_type <= TYPE_ET4000W32Pd) )
#define Is_W32p_up  ( et4000_type >= TYPE_ET4000W32P )


extern t_tseng_type et4000_type;
extern unsigned char tseng_save_divide; /* saves state of hibit */
extern unsigned long ET6Kbase;

extern int tseng_bytesperpixel;

extern SymTabRec TsengDacTable[];

typedef enum {
    UNKNOWN_DAC = -1,
    NORMAL_DAC,
    ATT20C47xA_DAC,
    Sierra1502X_DAC,
    ATT20C497_DAC,
    ATT20C490_DAC,
    ATT20C493_DAC,
    ATT20C491_DAC,
    ATT20C492_DAC,
    ICS5341_DAC,
    ICS5301_DAC,
    STG1700_DAC,
    STG1702_DAC,
    STG1703_DAC,
    ET6000_DAC,
    CH8398_DAC,
    MUSIC4910_DAC
} t_ramdactype;

extern t_ramdactype TsengRamdacType;
void Check_Tseng_Ramdac(void);
void tseng_init_clockscale(void);
void TsengHideCursor(void);

#define DAC_IS_ATT49x ( (TsengRamdacType == ATT20C490_DAC) \
                     || (TsengRamdacType == ATT20C491_DAC) \
                     || (TsengRamdacType == ATT20C492_DAC) \
                     || (TsengRamdacType == ATT20C493_DAC) \
                     || (TsengRamdacType == MUSIC4910_DAC) )

#define DAC_is_GenDAC ( (TsengRamdacType == ICS5341_DAC) \
                     || (TsengRamdacType == ICS5301_DAC) )

#define DAC_is_STG170x ( (TsengRamdacType == STG1700_DAC) \
                      || (TsengRamdacType == STG1702_DAC) \
                      || (TsengRamdacType == STG1703_DAC) )

#define CHIP_SUPPORTS_LINEAR (et4000_type >= TYPE_ET4000W32I)

#define Gendac_programmable_clock \
        ( (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) && \
          (   (OFLG_ISSET(CLOCK_OPTION_ICS5341, &vga256InfoRec.clockOptions)) \
           || (OFLG_ISSET(CLOCK_OPTION_ICS5301, &vga256InfoRec.clockOptions)) \
          ) \
        )

#define STG170x_programmable_clock \
        ( (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) && \
          (OFLG_ISSET(CLOCK_OPTION_STG1703, &vga256InfoRec.clockOptions)) \
        )

#define ICD2061a_programmable_clock \
        ( (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) && \
          (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &vga256InfoRec.clockOptions)) \
        )

#define CH8398_programmable_clock \
        ( (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) && \
          (OFLG_ISSET(CLOCK_OPTION_CH8398, &vga256InfoRec.clockOptions)) \
        )

#define ET6000_programmable_clock \
        ( (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) && \
          (OFLG_ISSET(CLOCK_OPTION_ET6000, &vga256InfoRec.clockOptions)) \
        )


extern int Tseng_bus;
#define BUS_ISA 0
#define BUS_MCA 1
#define BUS_VLB 2
#define BUS_PCI 3


/*
 * From tseng_clocks.c
 */

/* pixel multiplexing variables */
extern Bool Tseng_pixMuxPossible;
extern int Tseng_nonMuxMaxClock;
extern int Tseng_pixMuxMinWidth;

Bool Tseng_ET4000ClockSelect(int no);
Bool Tseng_LegendClockSelect(int no);
Bool Tseng_ET6000ClockSelect(int freq);
Bool Tseng_GenDACClockSelect(int freq);
Bool Tseng_STG1703ClockSelect(int freq);
Bool Tseng_ICD2061AClockSelect(int freq);
void tseng_set_dacspeed(void);
void tseng_validate_mode(DisplayModePtr mode, Bool verbose);
void tseng_set_ramdac_bpp(DisplayModePtr mode, vgaET4000Ptr tseng_regs);
#ifdef DPMSExtension
void TsengCrtcDPMSSet(int Mode);
void TsengHVSyncDPMSSet(int Mode);
#endif
#endif
