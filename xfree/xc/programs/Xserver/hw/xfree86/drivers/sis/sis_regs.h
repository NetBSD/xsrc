/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_regs.h,v 1.15 2001/04/19 12:40:33 alanh Exp $ */

#define inSISREG(base)          inb(base)
#define outSISREG(base,val)     outb(base,val)
#define orSISREG(base,val)      do { \
                      unsigned char __Temp = inb(base); \
                      outSISREG(base, __Temp | (val)); \
                    } while (0)
#define andSISREG(base,val)     do { \
                      unsigned char __Temp = inb(base); \
                      outSISREG(base, __Temp & (val)); \
                    } while (0)

#define inSISIDXREG(base,idx,var)   do { \
                      outb(base,idx); var=inb((base)+1); \
                    } while (0)
#if 0
#define outSISIDXREG(base,idx,val)  outw(base, (val)<<8 | (idx));
#endif
#define outSISIDXREG(base,idx,val)  do { \
                      outb(base,idx); outb((base)+1,val); \
                    } while (0)
#define orSISIDXREG(base,idx,val)   do { \
                      unsigned char __Temp; \
                      outb(base,idx);   \
                      __Temp = inb((base)+1)|(val); \
                      outSISIDXREG(base,idx,__Temp); \
                    } while (0)
#define andSISIDXREG(base,idx,and)  do { \
                      unsigned char __Temp; \
                      outb(base,idx);   \
                      __Temp = inb((base)+1)&(and); \
                      outSISIDXREG(base,idx,__Temp); \
                    } while (0)
#define setSISIDXREG(base,idx,and,or)   do { \
                      unsigned char __Temp; \
                      outb(base,idx);   \
                      __Temp = (inb((base)+1)&(and))|(or); \
                      outSISIDXREG(base,idx,__Temp); \
                    } while (0)

#define BITMASK(h,l)    (((unsigned)(1U << ((h)-(l)+1))-1)<<(l))
#define GENMASK(mask)   BITMASK(1?mask,0?mask)

#define GETBITS(var,mask)   (((var) & GENMASK(mask)) >> (0?mask))
#define SETBITS(val,mask)   ((val) << (0?mask))
#define SETBIT(n)       (1<<(n))

#define GETBITSTR(val,from,to)  ((GETBITS(val,from)) << (0?to))
#define SETVARBITS(var,val,from,to) (((var)&(~(GENMASK(to)))) | \
                    GETBITSTR(val,from,to))
#define GETVAR8(var)        ((var)&0xFF)
#define SETVAR8(var,val)    (var) =  GETVAR8(val)

#define VGA_RELIO_BASE  0x380

#define AROFFSET    VGA_ATTR_INDEX - VGA_RELIO_BASE
#define ARROFFSET   VGA_ATTR_DATA_R - VGA_RELIO_BASE
#define GROFFSET    VGA_GRAPH_INDEX - VGA_RELIO_BASE
#define SROFFSET    VGA_SEQ_INDEX - VGA_RELIO_BASE
#define CROFFSET    VGA_CRTC_INDEX_OFFSET + VGA_IOBASE_COLOR-VGA_RELIO_BASE
#define MISCROFFSET VGA_MISC_OUT_R - VGA_RELIO_BASE
#define MISCWOFFSET VGA_MISC_OUT_W - VGA_RELIO_BASE

#define SISAR       pSiS->RElIO+AROFFSET
#define SISARR      pSiS->RELIO+ARROFFSET
#define SISGR       pSiS->RELIO+GROFFSET
#define SISSR       pSiS->RelIO+SROFFSET
#define SISCR       pSiS->RelIO+CROFFSET
#define SISMISCR    pSiS->RelIO+MISCROFFSET
#define SISMISCW    pSiS->RelIO+MISCWOFFSET
#define SISPART1        pSiS->RelIO+0x04
#define SISPART2        pSiS->RelIO+0x10
#define SISPART3        pSiS->RelIO+0x12
#define SISPART4        pSiS->RelIO+0x14
#define SISPART5        pSiS->RelIO+0x16

/* 3C4 */
#define BankReg 0x06
#define ClockReg 0x07
#define CPUThreshold 0x08
#define CRTThreshold 0x09
#define CRTCOff 0x0A
#define DualBanks 0x0B
#define MMIOEnable 0x0B
#define RAMSize 0x0C
#define Mode64 0x0C
#define ExtConfStatus1 0x0E
#define ClockBase 0x13
#define LinearAdd0 0x20
#define LinearAdd1 0x21
#define GraphEng 0x27
#define MemClock0 0x28
#define MemClock1 0x29
#define XR2A 0x2A
#define XR2B 0x2B
#define TurboQueueBase 0x2C
#define FBSize 0x2F
#define ExtMiscCont5 0x34
#define ExtMiscCont9 0x3C

/* 3x4 */
#define Offset 0x13

#define read_xr(num,var) do {outb(0x3c4, num);var=inb(0x3c5);} while (0)

/* Definitions for the SIS engine communication. */

extern int sisReg32MMIO[];
#define BR(x) sisReg32MMIO[x]

/* These are done using Memory Mapped IO, of the registers */
/* 
 * Modified for Sis by Xavier Ducoin (xavier@rd.lectra.fr) 
 */

#define sisLEFT2RIGHT       0x10
#define sisRIGHT2LEFT       0x00
#define sisTOP2BOTTOM       0x20
#define sisBOTTOM2TOP       0x00

#define sisSRCSYSTEM        0x03
#define sisSRCVIDEO         0x02
#define sisSRCFG            0x01
#define sisSRCBG            0x00

#define sisCMDBLT           0x0000
#define sisCMDBLTMSK        0x0100
#define sisCMDCOLEXP        0x0200
#define sisCMDLINE          0x0300

#define sisCMDENHCOLEXP     0x2000

#define sisXINCREASE        0x10
#define sisYINCREASE        0x20
#define sisCLIPENABL        0x40
#define sisCLIPINTRN        0x80 
#define sisCLIPEXTRN        0x00


#define sisPATREG           0x08
#define sisPATFG            0x04
#define sisPATBG            0x00

#define sisLASTPIX          0x0800
#define sisXMAJOR           0x0400


/* Macros to do useful things with the SIS BitBLT engine */

#define sisBLTSync \
  while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)+2) & \
    (0x4000)){}

/* According to SiS 6326 2D programming guide, 16 bits position at   */
/* 0x82A8 returns queue free. But this don't work, so don't wait     */
/* anything when turbo-queue is enabled. If there are frequent syncs */
/* this should work. But not for xaa_benchmark :-(     */

#define sisBLTWAIT \
  if (!pSiS->TurboQueue) {\
    while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)+2) & \
    (0x4000)){}} /* \
    else {while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)) < \
    63){}} */

#define sisSETPATREG()\
   ((unsigned char *)(pSiS->IOBase + BR(11)))

#define sisSETPATREGL()\
   ((unsigned long *)(pSiS->IOBase + BR(11)))

#define sisSETCMD(op) \
  *(volatile unsigned short *)(pSiS->IOBase + BR(10) +2 ) = op

#define sisSETROPFG(op) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = ((*(volatile unsigned int *)(pSiS->IOBase + BR(4)))&0xffffff) | (op<<24)

#define sisSETROPBG(op) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = ((*(volatile unsigned int *)(pSiS->IOBase + BR(5)))&0xffffff) | (op<<24)

#define sisSETROP(op) \
   sisSETROPFG(op);sisSETROPBG(op);


#define sisSETSRCADDR(srcAddr) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(0)) = srcAddr&0x3FFFFFL

#define sisSETDSTADDR(dstAddr) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(1)) = dstAddr&0x3FFFFFL

#define sisSETPITCH(srcPitch,dstPitch) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(2)) = ((dstPitch&0xFFFF)<<16)| \
      (srcPitch&0xFFFF)

/* according to SIS 2D Engine Programming Guide 
 * width -1 independant of Bpp
 */ 
#define sisSETHEIGHTWIDTH(Height,Width)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(3)) = (((Height)&0xFFFF)<<16)| \
      ((Width)&0xFFFF)

#define sisSETCLIPTOP(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(8)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETCLIPBOTTOM(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(9)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETBGCOLOR(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor)

#define sisSETBGCOLOR8(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFF)

#define sisSETBGCOLOR16(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFFFF)

#define sisSETBGCOLOR24(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFFFFFF)


#define sisSETFGCOLOR(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor)

#define sisSETFGCOLOR8(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFF)

#define sisSETFGCOLOR16(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFFFF)

#define sisSETFGCOLOR24(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFFFFFF)

/* Line drawing */

#define sisSETXStart(XStart) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(0)) = XStart&0xFFFF

#define sisSETYStart(YStart) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(1)) = YStart&0xFFFF

#define sisSETLineMajorCount(MajorAxisCount) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(3)) = MajorAxisCount&0xFFFF

#define sisSETLineSteps(K1,K2) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(6)) = (((K1)&0xFFFF)<<16)| \
      ((K2)&0xFFFF)

#define sisSETLineErrorTerm(ErrorTerm) \
  *(volatile unsigned short *)(pSiS->IOBase + BR(7)) = ErrorTerm


/* SiS Registers for Xv */

/* VGA standard register */
#define  Index_SR_Graphic_Mode                  0x06
#define  Index_SR_RAMDAC_Ctrl                   0x07
#define  Index_SR_Threshold_Ctrl1               0x08
#define  Index_SR_Threshold_Ctrl2               0x09
#define  Index_SR_Misc_Ctrl                     0x0F
#define  Index_SR_DDC                           0x11
#define  Index_SR_Feature_Connector_Ctrl        0x12
#define  Index_SR_DRAM_Sizing                   0x14
#define  Index_SR_DRAM_State_Machine_Ctrl       0x15
#define  Index_SR_AGP_PCI_State_Machine         0x21
#define  Index_SR_Internal_MCLK0                0x28
#define  Index_SR_Internal_MCLK1                0x29
#define  Index_SR_Internal_DCLK1                0x2B
#define  Index_SR_Internal_DCLK2                0x2C
#define  Index_SR_Internal_DCLK3                0x2D
#define  Index_SR_Ext_Clock_Sel                 0x32
#define  Index_SR_Int_Status                    0x34
#define  Index_SR_Int_Enable                    0x35
#define  Index_SR_Int_Reset                     0x36
#define  Index_SR_Power_On_Trap                 0x38
#define  Index_SR_Power_On_Trap2                0x39
#define  Index_SR_Power_On_Trap3                0x3A

/* video registers */
#define  Index_VI_Passwd                        0x00
#define  Index_VI_Win_Hor_Disp_Start_Low        0x01
#define  Index_VI_Win_Hor_Disp_End_Low          0x02
#define  Index_VI_Win_Hor_Over                  0x03

#define  Index_VI_Win_Ver_Disp_Start_Low        0x04
#define  Index_VI_Win_Ver_Disp_End_Low          0x05
#define  Index_VI_Win_Ver_Over                  0x06

#define  Index_VI_Disp_Y_Buf_Start_Low          0x07
#define  Index_VI_Disp_Y_Buf_Start_Middle       0x08
#define  Index_VI_Disp_Y_Buf_Start_High         0x09

#define  Index_VI_U_Buf_Start_Low               0x0A
#define  Index_VI_U_Buf_Start_Middle            0x0B
#define  Index_VI_U_Buf_Start_High              0x0C

#define  Index_VI_V_Buf_Start_Low               0x0D
#define  Index_VI_V_Buf_Start_Middle            0x0E
#define  Index_VI_V_Buf_Start_High              0x0F

#define  Index_VI_Disp_Y_Buf_Pitch_Low          0x10
#define  Index_VI_Disp_UV_Buf_Pitch_Low         0x11
#define  Index_VI_Disp_Y_UV_Buf_Pitch_High      0x12

#define  Index_VI_Disp_Y_Buf_Preset_Low         0x13
#define  Index_VI_Disp_Y_Buf_Preset_Middle      0x14
#define  Index_VI_UV_Buf_Preset_Low             0x15
#define  Index_VI_UV_Buf_Preset_Middle          0x16
#define  Index_VI_Disp_Y_UV_Buf_Preset_High     0x17

#define  Index_VI_Hor_Post_Up_Scale_Low         0x18
#define  Index_VI_Hor_Post_Up_Scale_High        0x19
#define  Index_VI_Ver_Up_Scale_Low              0x1A
#define  Index_VI_Ver_Up_Scale_High             0x1B
#define  Index_VI_Scale_Control                 0x1C

#define  Index_VI_Play_Threshold_Low            0x1D
#define  Index_VI_Play_Threshold_High           0x1E
#define  Index_VI_Line_Buffer_Size              0x1F

/* Destination color key */
#define  Index_VI_Overlay_ColorKey_Red_Min      0x20
#define  Index_VI_Overlay_ColorKey_Green_Min    0x21
#define  Index_VI_Overlay_ColorKey_Blue_Min     0x22
#define  Index_VI_Overlay_ColorKey_Red_Max      0x23
#define  Index_VI_Overlay_ColorKey_Green_Max    0x24
#define  Index_VI_Overlay_ColorKey_Blue_Max     0x25

/* Source color key */
#define  Index_VI_Overlay_ChromaKey_Red_Y_Min   0x26
#define  Index_VI_Overlay_ChromaKey_Green_U_Min 0x27
#define  Index_VI_Overlay_ChromaKey_Blue_V_Min  0x28
#define  Index_VI_Overlay_ChromaKey_Red_Y_Max   0x29
#define  Index_VI_Overlay_ChromaKey_Green_U_Max 0x2A
#define  Index_VI_Overlay_ChromaKey_Blue_V_Max  0x2B

#define  Index_VI_Contrast_Factor               0x2C

#define  Index_VI_Brightness                    0x2D
#define  Index_VI_Contrast_Enh_Ctrl             0x2E

#define  Index_VI_Key_Overlay_OP                0x2F

#define  Index_VI_Control_Misc0                 0x30
#define  Index_VI_Control_Misc1                 0x31
#define  Index_VI_Control_Misc2                 0x32

#define  Index_MPEG_Read_Ctrl0                  0x60
#define  Index_MPEG_Read_Ctrl1                  0x61
#define  Index_MPEG_Read_Ctrl2                  0x62
#define  Index_MPEG_Read_Ctrl3                  0x63
#define  Index_MPEG_Ver_Up_Scale_Low            0x64
#define  Index_MPEG_Ver_Up_Scale_High           0x65

/*
   CRT_2 function control register
 */
#define  Index_CRT2_FC_CONTROL                  0x00
#define  Index_CRT2_FC_SCREEN_HIGH              0x04
#define  Index_CRT2_FC_SCREEN_MID               0x05
#define  Index_CRT2_FC_SCREEN_LOW               0x06
#define  Index_CRT2_FC_ENABLE_WRITE             0x24
#define  Index_CRT2_FC_VR                       0x25
#define  Index_CRT2_FC_VCount                   0x27
#define  Index_CRT2_FC_VCount1                  0x28

/* video attributes - these should probably be configurable on the fly
 *                    so users with different desktop sizes can keep
 *                    captured data off the desktop
 */
#define _VINWID                                  704
#define _VINHGT                         _VINHGT_NTSC
#define _VINHGT_NTSC                             240
#define _VINHGT_PAL                              290
#define _VIN_WINDOW                  (704 * 291 * 2)
#define _VBI_WINDOW                   (704 * 64 * 2)

#define _VIN_FIELD_EVEN                            1
#define _VIN_FIELD_ODD                             2
#define _VIN_FIELD_BOTH                            4

#define vc_index_offset                         0x00
#define vc_data_offset                          0x01
#define vi_index_offset                         0x02
#define vi_data_offset                          0x03
#define crt2_index_offset                       0x04
#define crt2_port_offset                        0x05
#define sr_index_offset                         0x44
#define sr_data_offset                          0x45
#define cr_index_offset                         0x54
#define cr_data_offset                          0x55
#define input_stat                              0x5A

/* i2c registers */
#define X_INDEXREG      0x14
#define X_PORTREG       0x15
#define X_DATA          0x0f
#define I2C_SCL         0x00
#define I2C_SDA         0x01
#define I2C_DELAY       10

/* mmio registers */
#define REG_PRIM_CRT_COUNTER    0x8514
