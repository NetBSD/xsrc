/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_vidregs.h,v 1.2 2000/12/04 18:50:01 dawes Exp $ */

#ifndef SISREG_H
#define SISREG_H

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
#define  Index_SR_Power_On_Trap			0x38
#define  Index_SR_Power_On_Trap2		0x39
#define  Index_SR_Power_On_Trap3		0x3A

/* video registers */
#define  Index_VI_Passwd			0x00
#define  Index_VI_Win_Hor_Disp_Start_Low	0x01
#define  Index_VI_Win_Hor_Disp_End_Low		0x02
#define  Index_VI_Win_Hor_Over			0x03

#define  Index_VI_Win_Ver_Disp_Start_Low	0x04
#define  Index_VI_Win_Ver_Disp_End_Low		0x05
#define  Index_VI_Win_Ver_Over			0x06

#define  Index_VI_Disp_Y_Buf_Start_Low		0x07
#define  Index_VI_Disp_Y_Buf_Start_Middle	0x08
#define  Index_VI_Disp_Y_Buf_Start_High 	0x09

#define  Index_VI_U_Buf_Start_Low		0x0A
#define  Index_VI_U_Buf_Start_Middle		0x0B
#define  Index_VI_U_Buf_Start_High		0x0C

#define  Index_VI_V_Buf_Start_Low		0x0D
#define  Index_VI_V_Buf_Start_Middle		0x0E
#define  Index_VI_V_Buf_Start_High		0x0F

#define  Index_VI_Disp_Y_Buf_Pitch_Low		0x10
#define  Index_VI_Disp_UV_Buf_Pitch_Low		0x11
#define  Index_VI_Disp_Y_UV_Buf_Pitch_High	0x12

#define  Index_VI_Disp_Y_Buf_Preset_Low 	0x13
#define  Index_VI_Disp_Y_Buf_Preset_Middle	0x14
#define  Index_VI_UV_Buf_Preset_Low		0x15
#define  Index_VI_UV_Buf_Preset_Middle		0x16
#define  Index_VI_Disp_Y_UV_Buf_Preset_High	0x17

#define  Index_VI_Hor_Post_Up_Scale_Low 	0x18
#define  Index_VI_Hor_Post_Up_Scale_High	0x19
#define  Index_VI_Ver_Up_Scale_Low		0x1A
#define  Index_VI_Ver_Up_Scale_High		0x1B
#define  Index_VI_Scale_Control 		0x1C

#define  Index_VI_Play_Threshold_Low		0x1D
#define  Index_VI_Play_Threshold_High		0x1E
#define  Index_VI_Line_Buffer_Size		0x1F

/* Destination color key */
#define  Index_VI_Overlay_ColorKey_Red_Min	0x20
#define  Index_VI_Overlay_ColorKey_Green_Min	0x21
#define  Index_VI_Overlay_ColorKey_Blue_Min	0x22
#define  Index_VI_Overlay_ColorKey_Red_Max	0x23
#define  Index_VI_Overlay_ColorKey_Green_Max	0x24
#define  Index_VI_Overlay_ColorKey_Blue_Max	0x25

/* Source color key */
#define  Index_VI_Overlay_ChromaKey_Red_Y_Min	0x26
#define  Index_VI_Overlay_ChromaKey_Green_U_Min 0x27
#define  Index_VI_Overlay_ChromaKey_Blue_V_Min	0x28
#define  Index_VI_Overlay_ChromaKey_Red_Y_Max	0x29
#define  Index_VI_Overlay_ChromaKey_Green_U_Max 0x2A
#define  Index_VI_Overlay_ChromaKey_Blue_V_Max	0x2B

#define  Index_VI_Contrast_Factor		0x2C

#define  Index_VI_Brightness			0x2D
#define  Index_VI_Contrast_Enh_Ctrl		0x2E

#define  Index_VI_Key_Overlay_OP		0x2F

#define  Index_VI_Control_Misc0 		0x30
#define  Index_VI_Control_Misc1 		0x31
#define  Index_VI_Control_Misc2 		0x32

#define  Index_MPEG_Read_Ctrl0			0x60
#define  Index_MPEG_Read_Ctrl1			0x61
#define  Index_MPEG_Read_Ctrl2			0x62
#define  Index_MPEG_Read_Ctrl3			0x63
#define  Index_MPEG_Ver_Up_Scale_Low		0x64
#define  Index_MPEG_Ver_Up_Scale_High		0x65

/*
   CRT_2 function control register
 */
#define  Index_CRT2_FC_CONTROL			0x00
#define  Index_CRT2_FC_SCREEN_HIGH		0x04
#define  Index_CRT2_FC_SCREEN_MID		0x05
#define  Index_CRT2_FC_SCREEN_LOW		0x06
#define  Index_CRT2_FC_ENABLE_WRITE		0x24
#define  Index_CRT2_FC_VR			0x25
#define  Index_CRT2_FC_VCount			0x27
#define  Index_CRT2_FC_VCount1			0x28

/* video attributes - these should probably be configurable on the fly
 *		      so users with different desktop sizes can keep
 *		      captured data off the desktop
 */
#define _VINWID                                  704
#define _VINHGT                         _VINHGT_NTSC
#define _VINHGT_NTSC                             240
#define _VINHGT_PAL                              290
#define _VIN_WINDOW	             (704 * 291 * 2)
#define _VBI_WINDOW            	      (704 * 64 * 2)

#define _VIN_FIELD_EVEN                            1
#define _VIN_FIELD_ODD	                           2
#define _VIN_FIELD_BOTH                            4

#define vc_index_offset				0x00
#define vc_data_offset				0x01
#define vi_index_offset				0x02
#define vi_data_offset				0x03
#define crt2_index_offset			0x04
#define crt2_port_offset			0x05
#define sr_index_offset				0x44
#define sr_data_offset				0x45
#define cr_index_offset				0x54
#define cr_data_offset				0x55
#define input_stat				0x5A

/* i2c registers */
#define X_INDEXREG	0x14
#define X_PORTREG       0x15
#define X_DATA		0x0f
#define I2C_SCL		0x00
#define I2C_SDA		0x01
#define I2C_DELAY       10

/* mmio registers */
#define REG_PRIM_CRT_COUNTER    0x8514

#endif /* SISREG_H */
