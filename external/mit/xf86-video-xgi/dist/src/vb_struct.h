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
 
 
#ifndef _VB_STRUCT_
#define _VB_STRUCT_


typedef struct _XGI_PanelDelayTblStruct
{
 UCHAR timer[2];
} XGI_PanelDelayTblStruct;

typedef struct _XGI_LCDDataStruct
{
 USHORT RVBHCMAX;
 USHORT RVBHCFACT;
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT LCDHT;
 USHORT LCDVT;
} XGI_LCDDataStruct;


typedef struct _XGI_LVDSCRT1HDataStruct
{
 UCHAR Reg[8];
} XGI_LVDSCRT1HDataStruct;
typedef struct _XGI_LVDSCRT1VDataStruct
{
 UCHAR Reg[7];
} XGI_LVDSCRT1VDataStruct;


typedef struct _XGI_TVDataStruct
{
 USHORT RVBHCMAX;
 USHORT RVBHCFACT;
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT TVHDE;
 USHORT TVVDE;
 USHORT RVBHRS;
 UCHAR FlickerMode;
 USHORT HALFRVBHRS;
 UCHAR RY1COE;
 UCHAR RY2COE;
 UCHAR RY3COE;
 UCHAR RY4COE;
} XGI_TVDataStruct;

typedef struct _XGI_LVDSDataStruct
{
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT LCDHT;
 USHORT LCDVT;
} XGI_LVDSDataStruct;

typedef struct _XGI_LVDSDesStruct
{
 USHORT LCDHDES;
 USHORT LCDVDES;
} XGI_LVDSDesStruct;

typedef struct _XGI_LVDSCRT1DataStruct
{
 UCHAR CR[15];
} XGI_LVDSCRT1DataStruct;

/*add for LCDA*/


typedef struct _XGI_StStruct
{
 UCHAR St_ModeID;
 USHORT St_ModeFlag;
 UCHAR St_StTableIndex;
 UCHAR St_CRT2CRTC;
 UCHAR St_CRT2CRTC2;
 UCHAR St_ResInfo;
 UCHAR VB_StTVFlickerIndex;
 UCHAR VB_StTVEdgeIndex;
 UCHAR VB_StTVYFilterIndex;
} XGI_StStruct;

typedef struct _XGI_StandTableStruct
{
 UCHAR CRT_COLS;
 UCHAR ROWS;
 UCHAR CHAR_HEIGHT;
 USHORT CRT_LEN;
 UCHAR SR[4];
 UCHAR MISC;
 UCHAR CRTC[0x19];
 UCHAR ATTR[0x14];
 UCHAR GRC[9];
} XGI_StandTableStruct;

typedef struct _XGI_ExtStruct
{
 UCHAR Ext_ModeID;
 USHORT Ext_ModeFlag;
 USHORT Ext_ModeInfo;
 USHORT Ext_Point;
 USHORT Ext_VESAID;
 UCHAR Ext_VESAMEMSize;
 UCHAR Ext_RESINFO;
 UCHAR VB_ExtTVFlickerIndex;
 UCHAR VB_ExtTVEdgeIndex;
 UCHAR VB_ExtTVYFilterIndex;
 UCHAR REFindex;
} XGI_ExtStruct;

typedef struct _XGI_Ext2Struct
{
 USHORT Ext_InfoFlag;
 UCHAR Ext_CRT1CRTC;
 UCHAR Ext_CRTVCLK;
 UCHAR Ext_CRT2CRTC;
 UCHAR Ext_CRT2CRTC2;
 UCHAR  ModeID;
 USHORT XRes;
 USHORT YRes;
 /* USHORT ROM_OFFSET; */
} XGI_Ext2Struct;


typedef struct _XGI_MCLKDataStruct
{
 UCHAR SR28,SR29,SR2A;
 USHORT CLOCK;
} XGI_MCLKDataStruct;

typedef struct _XGI_ECLKDataStruct
{
 UCHAR SR2E,SR2F,SR30;
 USHORT CLOCK;
} XGI_ECLKDataStruct;

typedef struct _XGI_VCLKDataStruct
{
 UCHAR SR2B,SR2C;
 USHORT CLOCK;
} XGI_VCLKDataStruct;

typedef struct _XGI_VBVCLKDataStruct
{
 UCHAR Part4_A,Part4_B;
 USHORT CLOCK;
} XGI_VBVCLKDataStruct;

typedef struct _XGI_StResInfoStruct
{
 USHORT HTotal;
 USHORT VTotal;
} XGI_StResInfoStruct;

typedef struct _XGI_ModeResInfoStruct
{
 USHORT HTotal;
 USHORT VTotal;
 UCHAR  XChar;
 UCHAR  YChar;
} XGI_ModeResInfoStruct;

typedef struct _XGI_LCDNBDesStruct
{
  UCHAR NB[12];
} XGI_LCDNBDesStruct;
 /*add for new UNIVGABIOS*/
typedef struct _XGI_LCDDesStruct
{
 USHORT LCDHDES;
 USHORT LCDHRS;
 USHORT LCDVDES;
 USHORT LCDVRS;
} XGI_LCDDesStruct;

typedef struct _XGI_LCDDataTablStruct
{
 UCHAR  PANELID;
 USHORT MASK;
 USHORT CAP;
 USHORT DATAPTR;
} XGI_LCDDataTablStruct;

typedef struct _XGI_TVTablDataStruct
{
 USHORT MASK;
 USHORT CAP;
 USHORT DATAPTR;
} XGI_TVDataTablStruct;

typedef struct _XGI330_LCDDesDataStruct
{
 USHORT LCDHDES;
 USHORT LCDHRS;
 USHORT LCDVDES;
 USHORT LCDVRS;
} XGI330_LCDDataDesStruct;


typedef struct _XGI330_LVDSDataStruct
{
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT LCDHT;
 USHORT LCDVT;
} XGI330_LVDSDataStruct;

typedef struct _XGI330_LCDDesDataStruct2
{
 USHORT LCDHDES;
 USHORT LCDHRS;
 USHORT LCDVDES;
 USHORT LCDVRS;
 USHORT LCDHSync;
 USHORT LCDVSync;
} XGI330_LCDDataDesStruct2;

typedef struct _XGI330_LCDDataStruct
{
 USHORT RVBHCMAX;
 USHORT RVBHCFACT;
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT LCDHT;
 USHORT LCDVT;
} XGI330_LCDDataStruct;


typedef struct _XGI330_TVDataStruct
{
 USHORT RVBHCMAX;
 USHORT RVBHCFACT;
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT TVHDE;
 USHORT TVVDE;
 USHORT RVBHRS;
 UCHAR FlickerMode;
 USHORT HALFRVBHRS;
} XGI330_TVDataStruct;

typedef struct _XGI330_LCDDataTablStruct
{
 UCHAR  PANELID;
 USHORT MASK;
 USHORT CAP;
 USHORT DATAPTR;
} XGI330_LCDDataTablStruct;

typedef struct _XGI330_TVDataTablStruct
{
 USHORT MASK;
 USHORT CAP;
 USHORT DATAPTR;
} XGI330_TVDataTablStruct;


typedef struct _XGI330_CHTVDataStruct
{
 USHORT VGAHT;
 USHORT VGAVT;
 USHORT LCDHT;
 USHORT LCDVT;
} XGI330_CHTVDataStruct;

typedef struct _XGI_TimingHStruct
{
  UCHAR data[8];
} XGI_TimingHStruct;

typedef struct _XGI_TimingVStruct
{
  UCHAR data[7];
} XGI_TimingVStruct;

/* Jong 10/04/2007; merge code */
typedef struct _XGI_CH7007TV_TimingHStruct
{
  UCHAR data[10];
} XGI_CH7007TV_TimingHStruct;

/* Jong 10/04/2007; merge code */
typedef struct _XGI_CH7007TV_TimingVStruct
{
  UCHAR data[10];
} XGI_CH7007TV_TimingVStruct;

/* Jong 10/04/2007; merge code */
typedef struct _XGI_XG21CRT1Struct
{
 UCHAR ModeID,CR02,CR03,CR15,CR16;
} XGI_XG21CRT1Struct;

typedef struct _XGI330_CHTVRegDataStruct
{
 UCHAR Reg[16];
} XGI330_CHTVRegDataStruct;

typedef struct _XGI330_LCDCapStruct
{
 		UCHAR      LCD_ID;
                USHORT     LCD_Capability;
                UCHAR      LCD_SetFlag;
                UCHAR      LCD_DelayCompensation;
                UCHAR      LCD_HSyncWidth;
                UCHAR      LCD_VSyncWidth;
                UCHAR      LCD_VCLK;
                UCHAR      LCDA_VCLKData1;
                UCHAR      LCDA_VCLKData2;
                UCHAR      LCUCHAR_VCLKData1;
                UCHAR      LCUCHAR_VCLKData2;
                UCHAR      PSC_S1;
                UCHAR      PSC_S2;
                UCHAR      PSC_S3;
                UCHAR      PSC_S4;
                UCHAR      PSC_S5;
                UCHAR      PWD_2B;
                UCHAR      PWD_2C;
                UCHAR      PWD_2D;
                UCHAR      PWD_2E;
                UCHAR      PWD_2F;
                UCHAR      Spectrum_31;
                UCHAR      Spectrum_32;
                UCHAR      Spectrum_33;
                UCHAR      Spectrum_34;
} XGI330_LCDCapStruct;

/* Jong 10/04/2007; merge code */
typedef struct _XGI21_LVDSCapStruct
{
                USHORT     LVDS_Capability;
                USHORT     LVDSHT;
                USHORT     LVDSVT;
                USHORT     LVDSHDE;
                USHORT     LVDSVDE;
                USHORT     LVDSHFP;
                USHORT     LVDSVFP;
                USHORT     LVDSHSYNC;
                USHORT     LVDSVSYNC;
                UCHAR      VCLKData1;
                UCHAR      VCLKData2;
                UCHAR      PSC_S1;
                UCHAR      PSC_S2;
                UCHAR      PSC_S3;
                UCHAR      PSC_S4;
                UCHAR      PSC_S5;
} XGI21_LVDSCapStruct;

typedef struct _XGI_CRT1TableStruct
{
  UCHAR CR[15];
} XGI_CRT1TableStruct;


typedef struct _XGI330_VCLKDataStruct
{
    UCHAR SR2B,SR2C;
    USHORT CLOCK;
} XGI330_VCLKDataStruct;

typedef struct _XGI301C_Tap4TimingStruct
{
    USHORT DE;
    UCHAR  Reg[64];   /* C0-FF */
} XGI301C_Tap4TimingStruct;

typedef struct _XGI_New_StandTableStruct
{
	UCHAR  CRT_COLS;
	UCHAR  ROWS;
	UCHAR  CHAR_HEIGHT;
	USHORT CRT_LEN;
	UCHAR  SR[4];
	UCHAR  MISC;
	UCHAR  CRTC[0x19];
	UCHAR  ATTR[0x14];
	UCHAR  GRC[9];
} XGI_New_StandTableStruct;

typedef UCHAR DRAM8Type[8];
typedef UCHAR DRAM4Type[4];
typedef UCHAR DRAM32Type[32];
typedef UCHAR DRAM2Type[2];

typedef struct _VB_DEVICE_INFO  VB_DEVICE_INFO,*PVB_DEVICE_INFO;

#define AGP_REG_SIZE 12
#define CR40_SIZE    24
#define CR6B_SIZE    8
#define CR6E_SIZE    8
#define CR6F_SIZE    8
#define CR89_SIZE    8
#define SR15_SIZE    4
#define MCLK_SIZE    8
#define ECLK_SIZE    8

struct _VB_DEVICE_INFO
{
    BOOLEAN  ISXPDOS;

    ULONG   P3c4,P3d4,P3c0,P3ce,P3c2,P3cc;
    ULONG   P3ca,P3c6,P3c7,P3c8,P3c9,P3da;
    ULONG   Part0Port,Part1Port,Part2Port;
    ULONG   Part3Port,Part4Port,Part5Port;
    ULONG   RVBHCFACT,RVBHCMAX,RVBHRS;
    ULONG   VGAVT,VGAHT,VGAVDE,VGAHDE;
    ULONG   VT,HT,VDE,HDE;
    ULONG   LCDHRS,LCDVRS,LCDHDES,LCDVDES;
	/*
    USHORT   P3c4,P3d4,P3c0,P3ce,P3c2,P3cc;
    USHORT   P3ca,P3c6,P3c7,P3c8,P3c9,P3da;
    USHORT   Part0Port,Part1Port,Part2Port;
    USHORT   Part3Port,Part4Port,Part5Port;
    USHORT   RVBHCFACT,RVBHCMAX,RVBHRS;
    USHORT   VGAVT,VGAHT,VGAVDE,VGAHDE;
    USHORT   VT,HT,VDE,HDE;
    USHORT   LCDHRS,LCDVRS,LCDHDES,LCDVDES; */

    USHORT   ModeType;
    USHORT   IF_DEF_TRUMPION,IF_DEF_DSTN;
    USHORT   IF_DEF_CRT2Monitor,IF_DEF_VideoCapture;
    USHORT   IF_DEF_CH7017,IF_DEF_LCDA,IF_DEF_YPbPr,IF_DEF_ScaleLCD,IF_DEF_OEMUtil,IF_DEF_PWD;
    USHORT   IF_DEF_ExpLink;
    USHORT   IF_DEF_CH7005,IF_DEF_HiVision; /* Jong 10/08/2007; merge code */
    USHORT   IF_DEF_CH7007; /* Jong 10/04/2007; merge code */
    USHORT   LCDResInfo,LCDTypeInfo, VBType;/*301b*/
    USHORT   VBInfo,TVInfo,LCDInfo;
    USHORT   VBExtInfo;/*301lv*/
    USHORT   SetFlag;
    USHORT   NewFlickerMode;
    USHORT   SelectCRT2Rate;

    PUCHAR ROMAddr;
    PUCHAR FBAddr;
    ULONG BaseAddr;
    /* USHORT BaseAddr; */
    XGIIOADDRESS RelIO;

    DRAM4Type  CR6B[CR6B_SIZE];

    UCHAR  XG45CR6E[CR6E_SIZE];
    UCHAR  XG45CR6F[CR6F_SIZE];
    DRAM4Type  CR6E[CR6E_SIZE];
    DRAM32Type CR6F[CR6F_SIZE];
    DRAM2Type  CR89[CR89_SIZE];

    DRAM8Type  SR15[SR15_SIZE]; /* pointer : point to array */
    DRAM8Type  CR40[CR40_SIZE];
    UCHAR  SoftSetting;
    UCHAR  OutputSelect;

	USHORT IF_DEF_LVDS; /* Jong 10/05/2007; merge code */

    const USHORT *pRGBSenseData;
    const USHORT *pRGBSenseData2; /*301b*/
    const USHORT *pVideoSenseData;
    const USHORT *pVideoSenseData2;
    const USHORT *pYCSenseData;
    const USHORT *pYCSenseData2;

    UCHAR  SR07;
    UCHAR  CR49[2];
    UCHAR  SR1F;
    UCHAR  AGPReg[AGP_REG_SIZE];
    UCHAR  SR16[4];
    UCHAR  SR21;
    UCHAR  SR22;
    UCHAR  SR23;
    UCHAR  SR24;
    UCHAR  SR25[2];
    UCHAR  SR31;
    UCHAR  SR32;
    UCHAR  SR33;

	/* Jong 10/05/2007; merge code */
    UCHAR  *pSR36;      
    UCHAR  CRCF;
    UCHAR  *pCRD0;      
    UCHAR  *pCRDE;      
    UCHAR  *pCR8F;      
    UCHAR  *pSR40;      
    UCHAR  *pSR41;      
    UCHAR  *pDVOSetting;
    UCHAR  *pCR2E;
    UCHAR  *pCR2F;
    UCHAR  *pCR46;
    UCHAR  *pCR47;

    UCHAR  CRT2Data_1_2;
    UCHAR  CRT2Data_4_D;
    UCHAR  CRT2Data_4_E;
    UCHAR  CRT2Data_4_10;
    XGI_MCLKDataStruct  MCLKData[MCLK_SIZE];
    XGI_ECLKDataStruct  ECLKData[ECLK_SIZE];

    const UCHAR   *XGI_TVDelayList;
    const UCHAR   *XGI_TVDelayList2;
    const UCHAR   *CHTVVCLKUNTSC;
    const UCHAR   *CHTVVCLKONTSC;
    const UCHAR   *CHTVVCLKUPAL;
    const UCHAR   *CHTVVCLKOPAL;
    const UCHAR   *NTSCTiming;
    const UCHAR   *PALTiming;
    const UCHAR   *HiTVExtTiming;
    const UCHAR   *HiTVSt1Timing;
    const UCHAR   *HiTVSt2Timing;
    const UCHAR   *HiTVTextTiming;
    const UCHAR   *YPbPr750pTiming;
    const UCHAR   *YPbPr525pTiming;
    const UCHAR   *YPbPr525iTiming;
    const UCHAR   *HiTVGroup3Data;
    const UCHAR   *HiTVGroup3Simu;
    const UCHAR   *HiTVGroup3Text;
    const UCHAR   *Ren525pGroup3;
    const UCHAR   *Ren750pGroup3;
    const UCHAR   *ScreenOffset;
    UCHAR   DRAMTypeDefinition;
    UCHAR   I2CDefinition;
    UCHAR   CR97;

    const XGI330_LCDCapStruct  *LCDCapList;
    XGI21_LVDSCapStruct  *XG21_LVDSCapList; /* Jong 10/05/2007; merge code */

    XGI_TimingHStruct  TimingH;
    XGI_TimingVStruct  TimingV;

    const XGI_StStruct          *SModeIDTable;
    const XGI_StandTableStruct  *StandTable;
    const XGI_ExtStruct         *EModeIDTable;
    const XGI_Ext2Struct        *RefIndex;
    /* XGINew_CRT1TableStruct *CRT1Table; */
    const XGI_CRT1TableStruct    *XGINEWUB_CRT1Table;
    const XGI_VCLKDataStruct    *VCLKData;
    const XGI_VBVCLKDataStruct  *VBVCLKData;
    const XGI_StResInfoStruct   *StResInfo;
    const XGI_ModeResInfoStruct *ModeResInfo;
    XGI_XG21CRT1Struct			*UpdateCRT1;  /* Jong 10/05/2007; merge code */
};  /* _VB_DEVICE_INFO */

/* Jong 10/04/2007; merge code */
typedef struct 
{
    USHORT    Horizontal_ACTIVE;
    USHORT    Horizontal_FP;
    USHORT    Horizontal_SYNC;
    USHORT    Horizontal_BP;
    USHORT    Vertical_ACTIVE;
    USHORT    Vertical_FP;
    USHORT    Vertical_SYNC;
    USHORT    Vertical_BP;
    double    DCLK;
    UCHAR     FrameRate;
    UCHAR     Interlace;
    USHORT    Margin;    
} TimingInfo;

#define _VB_STRUCT_
#endif /* _VB_STRUCT_ */
