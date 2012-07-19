/* Copyright (C) 2003-2006 by XGI Technology, Taiwan.
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL XGI AND/OR
 *  ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _XGI_VIDEOHW_H_
#define _XGI_VIDEOHW_H_

#include "xgi_video.h"

typedef struct {

   int     pixelFormat;

   CARD32  pitch;

   CARD8   keyOP;
   CARD16  HUSF;
   CARD16  VUSF;
   CARD8   IntBit;
   CARD8   wHPre;
   float   f_scale;

   CARD16  srcW;
   CARD16  srcH;

   BoxRec  dstBox;

   CARD32  PSY;
   CARD32  PSV;
   CARD32  PSU;
   CARD8   bobEnable;

   CARD32  lineBufSize;

   CARD32  dwContrastFactor;
   CARD32  SamplePixel;

} XGIOverlayRec, *XGIOverlayPtr;

/******************************************************************************/
/* BIT OPERATION */
/******************************************************************************/
#define LOBYTE(x)       ((CARD8)(x&0xFF))
#define HIBYTE(x)       ((CARD8)((x>>8)&0xFF))
#define LOWORD(x)   ((CARD16)(x&0xFFFF))
#define HIWORD(x)   ((CARD16)((x>>16)&0xFFFF))

/******************************************************************************/
/* DEFINITIONS FOR VIDEO PORT */
/******************************************************************************/
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

#define  Index_VI_Disp_U_Buf_Start_Low          0x0A
#define  Index_VI_Disp_U_Buf_Start_Middle       0x0B
#define  Index_VI_Disp_U_Buf_Start_High         0x0C

#define  Index_VI_Disp_V_Buf_Start_Low          0x0D
#define  Index_VI_Disp_V_Buf_Start_Middle       0x0E
#define  Index_VI_Disp_V_Buf_Start_High         0x0F

#define  Index_VI_Disp_Y_Buf_Pitch_Low          0x10
#define  Index_VI_Disp_UV_Buf_Pitch_Low         0x11
#define  Index_VI_Disp_Y_UV_Buf_Pitch_High      0x12

#define  Index_VI_Disp_Y_Buf_Preset_Low         0x13
#define  Index_VI_Disp_Y_Buf_Preset_Middle      0x14
#define  Index_VI_Disp_UV_Buf_Preset_Low        0x15
#define  Index_VI_Disp_UV_Buf_Preset_Middle     0x16
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
#define  Index_SP_Buf_Preset_High               0x35
#define  Index_SP_Buf_Preset_Low                0x36
#define  Index_SP_Buf_Preset_Middle             0x37
#define  Index_SP_FIFO_Max_Size                 0x3F
#define  Index_MPEG_Flip_Ctrl_Reg0              0x60
#define  Index_MPEG_Ver_Up_Scale_Low            0x64
#define  Index_MPEG_Ver_Up_Scale_High           0x65

/* For MPEG AutoFlip preset in SiS630 and SiS540 */
#define  Index_MPEG_Y_Buf_Preset_Low            0x66
#define  Index_MPEG_Y_Buf_Preset_Middle         0x67
#define  Index_MPEG_UV_Buf_Preset_Low           0x68
#define  Index_MPEG_UV_Buf_Preset_Middle        0x69
#define  Index_MPEG_Y_UV_Buf_Preset_High        0x6A

/* For Chip higher than 325 */
#define  Index_VI_Disp_Y_Buf_EXT_High           0x6B
#define  Index_VI_Disp_U_Buf_EXT_High           0x6C
#define  Index_VI_Disp_V_Buf_EXT_High           0x6D
#define  Index_VI_Disp_Y_Buf_Pitch_EXT_High     0x6E
#define  Index_VI_Disp_UV_Buf_Pitch_EXT_High    0x6F

#define  Index_VI_Hue                           0x70
#define  Index_VI_Saturation                    0x71
#define  Index_VI_Control_Misc3                 0x74

/* 4_tap_dda Weighting Matrix in XGI340 */
#define  Index_DDA_Weighting_Matrix_A0          0x75
#define  Index_DDA_Weighting_Matrix_A1          0x76
#define  Index_DDA_Weighting_Matrix_A2          0x77
#define  Index_DDA_Weighting_Matrix_A3          0x78
#define  Index_DDA_Weighting_Matrix_B0          0x79
#define  Index_DDA_Weighting_Matrix_B1          0x7A
#define  Index_DDA_Weighting_Matrix_B2          0x7B
#define  Index_DDA_Weighting_Matrix_B3          0x7C
#define  Index_DDA_Weighting_Matrix_C0          0x7D
#define  Index_DDA_Weighting_Matrix_C1          0x7E
#define  Index_DDA_Weighting_Matrix_C2          0x7F
#define  Index_DDA_Weighting_Matrix_C3          0x80
#define  Index_DDA_Weighting_Matrix_D0          0x81
#define  Index_DDA_Weighting_Matrix_D1          0x82
#define  Index_DDA_Weighting_Matrix_D2          0x83
#define  Index_DDA_Weighting_Matrix_D3          0x84
#define  Index_DDA_Weighting_Matrix_E0          0x85
#define  Index_DDA_Weighting_Matrix_E1          0x86
#define  Index_DDA_Weighting_Matrix_E2          0x87
#define  Index_DDA_Weighting_Matrix_E3          0x88
#define  Index_DDA_Weighting_Matrix_F0          0x89
#define  Index_DDA_Weighting_Matrix_F1          0x8A
#define  Index_DDA_Weighting_Matrix_F2          0x8B
#define  Index_DDA_Weighting_Matrix_F3          0x8C
#define  Index_DDA_Weighting_Matrix_G0          0x8D
#define  Index_DDA_Weighting_Matrix_G1          0x8E
#define  Index_DDA_Weighting_Matrix_G2          0x8F
#define  Index_DDA_Weighting_Matrix_G3          0x90
#define  Index_DDA_Weighting_Matrix_H0          0x91
#define  Index_DDA_Weighting_Matrix_H1          0x92
#define  Index_DDA_Weighting_Matrix_H2          0x93
#define  Index_DDA_Weighting_Matrix_H3          0x94
#define  Index_DDA_Weighting_Matrix_I0          0x95
#define  Index_DDA_Weighting_Matrix_I1          0x96
#define  Index_DDA_Weighting_Matrix_I2          0x97
#define  Index_DDA_Weighting_Matrix_I3          0x98
#define  Index_DDA_Weighting_Matrix_J0          0x99
#define  Index_DDA_Weighting_Matrix_J1          0x9A
#define  Index_DDA_Weighting_Matrix_J2          0x9B
#define  Index_DDA_Weighting_Matrix_J3          0x9C
#define  Index_DDA_Weighting_Matrix_K0          0x9D
#define  Index_DDA_Weighting_Matrix_K1          0x9E
#define  Index_DDA_Weighting_Matrix_K2          0x9F
#define  Index_DDA_Weighting_Matrix_K3          0xA0
#define  Index_DDA_Weighting_Matrix_L0          0xA1
#define  Index_DDA_Weighting_Matrix_L1          0xA2
#define  Index_DDA_Weighting_Matrix_L2          0xA3
#define  Index_DDA_Weighting_Matrix_L3          0xA4
#define  Index_DDA_Weighting_Matrix_M0          0xA5
#define  Index_DDA_Weighting_Matrix_M1          0xA6
#define  Index_DDA_Weighting_Matrix_M2          0xA7
#define  Index_DDA_Weighting_Matrix_M3          0xA8
#define  Index_DDA_Weighting_Matrix_N0          0xA9
#define  Index_DDA_Weighting_Matrix_N1          0xAA
#define  Index_DDA_Weighting_Matrix_N2          0xAB
#define  Index_DDA_Weighting_Matrix_N3          0xAC
#define  Index_DDA_Weighting_Matrix_O0          0xAD
#define  Index_DDA_Weighting_Matrix_O1          0xAE
#define  Index_DDA_Weighting_Matrix_O2          0xAF
#define  Index_DDA_Weighting_Matrix_O3          0xB0
#define  Index_DDA_Weighting_Matrix_P0          0xB1
#define  Index_DDA_Weighting_Matrix_P1          0xB2
#define  Index_DDA_Weighting_Matrix_P2          0xB3
#define  Index_DDA_Weighting_Matrix_P3          0xB4

#define  Index_VI_Play_Threshold_Low_Ext        0xB5
#define  Index_VI_Play_Threshold_High_Ext       0xB6
#define  Index_VI_Line_Buffer_Size_Ext          0xB7

/******************************************************************************/
/* DEFINITIONS FOR SEQUENCER */
/******************************************************************************/
#define  Index_SR_Graphic_Mode                 0x06
#define  Index_SR_RAMDAC_Ctrl                  0x07
#define  Index_SR_Threshold_Ctrl1              0x08
#define  Index_SR_Threshold_Ctrl2              0x09
#define  Index_SR_FC_SCREEN_HIGH               0x0D
#define  Index_SR_CRT_Misc_Ctrl                0x0F
#define  Index_SR_DDC                          0x11
#define  Index_SR_Feature_Connector_Ctrl       0x12
#define  Index_SR_DRAM_Sizing                  0x14
#define  Index_SR_DRAM_State_Machine_Ctrl      0x15

#define  Index_SR_Module_Enable                0x1E
#define  Index_SR_Power_Management             0x1F
#define  Index_SR_AGP_PCI_State_Machine        0x21
#define  Index_SR_Internal_MCLK0               0x28
#define  Index_SR_Internal_MCLK1               0x29
#define  Index_SR_Internal_DCLK1               0x2B
#define  Index_SR_Internal_DCLK2               0x2C
#define  Index_SR_Internal_DCLK3               0x2D
#define  Index_SR_Internal_ECLK0               0x2E
#define  Index_SR_Internal_ECLK1               0x2F
#define  Index_SR_Ext_Clock_Sel                0x32
#define  Index_SR_Int_Status                   0x34
#define  Index_SR_Int_Enable                   0x35
#define  Index_SR_Int_Reset                    0x36

#define  INDEX_SR_Synchronous_Reset            0x3C
#define  Index_SR_SW_Flip_1                    0x3E
#define  Index_Video_Process                   0x3F

/******************************************************************************/
/* DEFINITIONS FOR Default Color in HW */
/******************************************************************************/
#define  Default_Brightness      0x00
#define  Default_Contrast        0x04
#define  Default_Hue             0x00
#define  Default_Saturation      0x00

/******************************************************************************/
/* DEFINITIONS FOR Capture Register */
/******************************************************************************/
#define  Index_VC_Ver_Down_Scale_Factor_Over   0x10

/******************************************************************************/
/* DEFINITIONS FOR MMIO Register */
/******************************************************************************/

#define REG_PRIM_CRT_COUNTER                   0x8514
#define REG_LEFT_FLIP_1                        0x8540
#define REG_STATUS0                            0x8240
#define REG_GAMMA_PALETTE                      0x8570

/*
   CRT_2 function control register
 */
#define  Index_CRT2_FC_CONTROL         0x00
#define  Index_CRT2_FC_SCREEN_HIGH     0x04
#define  Index_CRT2_FC_SCREEN_MID      0x05
#define  Index_CRT2_FC_SCREEN_LOW      0x06
#define  Index_CRT2_FC_ENABLE_WRITE    0x24
#define  Index_CRT2_FC_VR        0x25
#define  Index_CRT2_FC_VCount       0x27
#define  Index_CRT2_FC_VCount1         0x28

/* video attributes - these should probably be configurable on the fly
 *          so users with different desktop sizes can keep
 *          captured data off the desktop
 */
#define _VINWID                                  704
#define _VINHGT                         _VINHGT_NTSC
#define _VINHGT_NTSC                             240
#define _VINHGT_PAL                              290
#define _VIN_WINDOW               (704 * 291 * 2)
#define _VBI_WINDOW                    (704 * 64 * 2)

#define _VIN_FIELD_EVEN                            1
#define _VIN_FIELD_ODD                             2
#define _VIN_FIELD_BOTH                            4

#define vc_index_offset          0x00
#define vc_data_offset           0x01
#define vi_index_offset          0x02
#define vi_data_offset           0x03
#define crt2_index_offset        0x04
#define crt2_port_offset         0x05
#define sr_index_offset          0x44
#define sr_data_offset           0x45
#define cr_index_offset          0x54
#define cr_data_offset           0x55
#define input_stat            0x5A

/* i2c registers */
#define X_INDEXREG   0x14
#define X_PORTREG       0x15
#define X_DATA    0x0f
#define I2C_SCL      0x00

#define I2C_SDA      0x01
#define I2C_DELAY       10

/*******************************
*       Function               *
*******************************/
/* static CARD8 vblank_active_CRT1(XGIPtr); */
void SetOverlayReg(XGIPtr, XGIOverlayPtr);
void SetColorkeyReg(XGIPtr, CARD32);
void SetSelectOverlayReg(XGIPtr, CARD8);
void SetEnableOverlayReg(XGIPtr, Bool);
void SetCloseOverlayReg(XGIPtr);

void XGIResetVideo(ScrnInfoPtr);

void SetVideoContrastReg(XGIPtr, INT32);
void SetVideoBrightnessReg(XGIPtr, INT32);
void SetVideoSaturationReg(XGIPtr, INT32);
void SetVideoHueReg(XGIPtr, INT32);

void EnableCaptureAutoFlip(XGIPtr, Bool);
#endif /* _XGI_VIDEOHW_H_ */

