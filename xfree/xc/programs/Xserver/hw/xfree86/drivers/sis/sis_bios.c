/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_bios.c,v 1.8 2001/04/19 14:11:37 alanh Exp $ */

#include "xf86.h"
#include "xf86PciInfo.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_bios.h"


UShort DRAMType[17][5]={{0x0C,0x0A,0x02,0x40,0x39},{0x0D,0x0A,0x01,0x40,0x48},
                     {0x0C,0x09,0x02,0x20,0x35},{0x0D,0x09,0x01,0x20,0x44},
                     {0x0C,0x08,0x02,0x10,0x31},{0x0D,0x08,0x01,0x10,0x40},
                     {0x0C,0x0A,0x01,0x20,0x34},{0x0C,0x09,0x01,0x08,0x32},
                     {0x0B,0x08,0x02,0x08,0x21},{0x0C,0x08,0x01,0x08,0x30},
                     {0x0A,0x08,0x02,0x04,0x11},{0x0B,0x0A,0x01,0x10,0x28},
                     {0x09,0x08,0x02,0x02,0x01},{0x0B,0x09,0x01,0x08,0x24},
                     {0x0B,0x08,0x01,0x04,0x20},{0x0A,0x08,0x01,0x02,0x10},
                     {0x09,0x08,0x01,0x01,0x00}};

UShort MDA_DAC[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F};

UShort CGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F};

UShort EGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x05,0x15,
               0x20,0x30,0x24,0x34,0x21,0x31,0x25,0x35,
               0x08,0x18,0x0C,0x1C,0x09,0x19,0x0D,0x1D,
               0x28,0x38,0x2C,0x3C,0x29,0x39,0x2D,0x3D,
               0x02,0x12,0x06,0x16,0x03,0x13,0x07,0x17,
               0x22,0x32,0x26,0x36,0x23,0x33,0x27,0x37,
               0x0A,0x1A,0x0E,0x1E,0x0B,0x1B,0x0F,0x1F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F};

UShort VGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x00,0x05,0x08,0x0B,0x0E,0x11,0x14,0x18,
               0x1C,0x20,0x24,0x28,0x2D,0x32,0x38,0x3F,

               0x00,0x10,0x1F,0x2F,0x3F,0x1F,0x27,0x2F,
               0x37,0x3F,0x2D,0x31,0x36,0x3A,0x3F,0x00,
               0x07,0x0E,0x15,0x1C,0x0E,0x11,0x15,0x18,
               0x1C,0x14,0x16,0x18,0x1A,0x1C,0x00,0x04,
               0x08,0x0C,0x10,0x08,0x0A,0x0C,0x0E,0x10,
               0x0B,0x0C,0x0D,0x0F,0x10};

UShort  ModeIndex_640x480[]   = {0x2E, 0x44, 0x45, 0x62};
UShort  ModeIndex_720x480[]   = {0x31, 0x33, 0x00, 0x35};
UShort  ModeIndex_720x576[]   = {0x32, 0x34, 0x00, 0x36};
UShort  ModeIndex_800x600[]   = {0x30, 0x47, 0x48, 0x63};
UShort  ModeIndex_1024x768[]  = {0x38, 0x4A, 0x4B, 0x64};
UShort  ModeIndex_1280x1024[] = {0x3A, 0x4D, 0x4E, 0x65};
UShort  ModeIndex_1600x1200[] = {0x3C, 0x3D, 0x3E, 0x66};
UShort  RefreshRate[7][8] = { 
                        {60, 72, 75, 85, 100, 120, 160, 200},
                        {56, 60, 72, 75,  85, 100, 120, 160},
                        {43, 60, 70, 75,  85, 100, 120,   0},
                        {43, 60, 75, 85,   0,   0,   0,   0},
                        {60, 65, 70, 75,  85,   0,   0,   0},
                        {60, 0 , 0 , 0 , 0  , 0  , 0  , 0  },
                        {60, 0 , 0 , 0 , 0  , 0  , 0  , 0  }};

UShort StResInfo[5][2]={{640,400},{640,350},{720,400},{720,350},{640,480}};
UShort ModeResInfo[15][4]={{320,200,8,8},{320,240,8,8},{320,400,8,8},
                       {400,300,8,8},{512,384,8,8},{640,400,8,16},
                       {640,480,8,16},{800,600,8,16},{1024,768,8,16},
                       {1280,1024,8,16},{1600,1200,8,16},{1920,1440,8,16},
                       {720,480,8,16},{720,576,8,16},{1280,960,8,16}};

UShort HiTVExtTiming[61]={0x32,0x65,0x2C,0x5F,0x08,0x31,0x3A,0x64,
                          0x28,0x02,0x01,0x3D,0x06,0x3E,0x35,0x6D,
                          0x06,0x14,0x3E,0x35,0x6D,0x00,0xC5,0x3F,
                          0x64,0x90,0x33,0x8C,0x18,0x36,0x3E,0x13,
                          0x2A,0xDE,0x2A,0x44,0x40,0x2A,0x44,0x40,
                          0x8E,0x8E,0x82,0x07,0x0B,
                          0x92,0x0F,0x40,0x60,0x80,0x14,0x90,0x8C,
                          0x60,0x14,0x3D,0x63,0x4F,
                          0x027,0xFFFC,0x6A};

UShort HiTVSt1Timing[61]={0x32,0x65,0x2C,0x5F,0x08,0x31,0x3A,0x65,
                          0x28,0x02,0x01,0x3D,0x06,0x3E,0x35,0x6D,
                          0x06,0x14,0x3E,0x35,0x6D,0x00,0xC5,0x3F,
                          0x65,0x90,0x7B,0xA8,0x03,0xF0,0x87,0x03,
                          0x11,0x15,0x11,0xCF,0x10,0x11,0xCF,0x10,
                          0x35,0x35,0x3B,0x69,0x1D,
                          0x92,0x0F,0x40,0x60,0x80,0x14,0x90,0x8C,
                          0x60,0x04,0x86,0xAF,0x5D,
                          0xE,0xFFFC,0x2D};

UShort HiTVSt2Timing[61]={0x32,0x65,0x2C,0x5F,0x08,0x31,0x3A,0x64,
                          0x28,0x02,0x01,0x3D,0x06,0x3E,0x35,0x6D,
                          0x06,0x14,0x3E,0x35,0x6D,0x00,0xC5,0x3F,
                          0x64,0x90,0x33,0x8C,0x18,0x36,0x3E,0x13,
                          0x2A,0xDE,0x2A,0x44,0x40,0x2A,0x44,0x40,
                          0x8E,0x8E,0x82,0x07,0x0B,
                          0x92,0x0F,0x40,0x60,0x80,0x14,0x90,0x8C,
                          0x60,0x14,0x3D,0x63,0x4F,
                          0x27,0xFFFC,0x6A};

UShort HiTVTextTiming[61]={0x32,0x65,0x2C,0x5F,0x08,0x31,0x3A,0x65,
                           0x28,0x02,0x01,0x3D,0x06,0x3E,0x35,0x6D,
                           0x06,0x14,0x3E,0x35,0x6D,0x00,0xC5,0x3F,
                           0x65,0x90,0xE7,0xBC,0x03,0x0C,0x97,0x03,
                           0x14,0x78,0x14,0x08,0x20,0x14,0x08,0x20,
                           0xC8,0xC8,0x3B,0xD2,0x26,
                           0x92,0x0F,0x40,0x60,0x80,0x14,0x90,0x8C,
                           0x60,0x04,0x96,0x72,0x5C,
                           0x11,0xFFFC,0x32};

UShort HiTVGroup3Data[63]={0x00,0x1A,0x22,0x63,0x62,0x22,0x08,0x5F,
                           0x05,0x21,0xB2,0xB2,0x55,0x77,0x2A,0xA6,
                           0x25,0x2F,0x47,0xFA,0xC8,0xFF,0x8E,0x20,
                           0x8C,0x6E,0x60,0x2E,0x58,0x48,0x72,0x44,
                           0x56,0x36,0x4F,0x6E,0x3F,0x80,0x00,0x80,
                           0x4F,0x7F,0x03,0xA8,0x7D,0x20,0x1A,0xA9,
                           0x14,0x05,0x03,0x7E,0x64,0x31,0x14,0x75,
                           0x18,0x05,0x18,0x05,0x4C,0xA8,0x01};

UShort HiTVGroup3Simu[63]={0x00,0x1A,0x22,0x63,0x62,0x22,0x08,0x95,
                           0xDB,0x20,0xB8,0xB8,0x55,0x47,0x2A,0xA6,
                           0x25,0x2F,0x47,0xFA,0xC8,0xFF,0x8E,0x20,
                           0x8C,0x6E,0x60,0x15,0x26,0xD3,0xE4,0x11,
                           0x56,0x36,0x4F,0x6E,0x3F,0x80,0x00,0x80,
                           0x67,0x36,0x01,0x47,0x0E,0x10,0xBE,0xB4,
                           0x01,0x05,0x03,0x7E,0x65,0x31,0x14,0x75,
                           0x18,0x05,0x18,0x05,0x4C,0xA8,0x01};

UShort HiTVGroup3Text[63]={0x00,0x1A,0x22,0x63,0x62,0x22,0x08,0xA7,
                           0xF5,0x20,0xCE,0xCE,0x55,0x47,0x2A,0xA6,
                           0x25,0x2F,0x47,0xFA,0xC8,0xFF,0x8E,0x20,
                           0x8C,0x6E,0x60,0x18,0x2C,0x0C,0x20,0x22,
                           0x56,0x36,0x4F,0x6E,0x3F,0x80,0x00,0x80,
                           0x93,0x3C,0x01,0x50,0x2F,0x10,0xF4,0xCA,
                           0x01,0x05,0x03,0x7E,0x65,0x31,0x14,0x75,
                           0x18,0x05,0x18,0x05,0x4C,0xA8,0x01};


UShort NTSCTiming[61]={0x017,0x01D,0x003,0x009,0x005,0x006,0x00C,0x00C,
                    0x094,0x049,0x001,0x00A,0x006,0x00D,0x004,0x00A,
                    0x006,0x014,0x00D,0x004,0x00A,0x000,0x085,0x01B,
                    0x00C,0x050,0x000,0x099,0x000,0x0EC,0x04A,0x017,
                    0x088,0x000,0x04B,0x000,0x000,0x0E2,0x000,0x002,
                    0x003,0x00A,0x065,0x09D,0x008,
                    0x092,0x08F,0x040,0x060,0x080,0x014,0x090,0x08C,
                    0x060,0x014,0x050,0x000,0x040,
                    0x00044,0x002DB,0x0003B};    /* Ajust xxx */

UShort PALTiming[61]={ 0x019,0x052,0x035,0x06E,0x004,0x038,0x03D,0x070,
                    0x094,0x049,0x001,0x012,0x006,0x03E,0x035,0x06D,
                    0x006,0x014,0x03E,0x035,0x06D,0x000,0x045,0x02B,
                    0x070,0x050,0x000,0x097,0x000,0x0D7,0x05D,0x017,
                    0x088,0x000,0x045,0x000,0x000,0x0E8,0x000,0x002,
                    0x00D,0x000,0x068,0x0B0,0x00B,
                    0x092,0x08F,0x040,0x060,0x080,0x014,0x090,0x08C,
                    0x060,0x014,0x063,0x000,0x040,
                    0x0003E,0x002E1,0x00028};    /* Ajust xxx */

UShort NTSCGroup3Data[63]= {0x000,0x014,0x015,0x025,0x055,0x015,0x00B,0x089,
                         0x0D7,0x040,0x0B0,0x0B0,0x0FF,0x0C4,0x045,0x0A6,
                         0x025,0x02F,0x067,0x0F6,0x0BF,0x0FF,0x08E,0x020,
                         0x08C,0x0DA,0x060,0x092,0x0C8,0x055,0x08B,0x000,
                         0x051,0x004,0x018,0x00A,0x0F8,0x087,0x000,0x080,
                         0x03B,0x03B,0x000,0x0F0,0x0F0,0x000,0x0F0,0x0F0,
                         0x000,0x051,0x00F,0x00F,0x008,0x00F,0x008,0x06F,
                         0x018,0x005,0x005,0x005,0x04C,0x0AA,0x001};

UShort PALGroup3Data[63]={0x000,0x01A,0x022,0x063,0x062,0x022,0x008,0x085,
                       0x0C3,0x020,0x0A4,0x0A4,0x055,0x047,0x02A,0x0A6,
                       0x025,0x02F,0x047,0x0FA,0x0C8,0x0FF,0x08E,0x020,
                       0x08C,0x0DC,0x060,0x092,0x0C8,0x04F,0x085,0x000,
                       0x056,0x036,0x04F,0x06E,0x0FE,0x083,0x054,0x081,
                       0x030,0x030,0x000,0x0F3,0x0F3,0x000,0x0A2,0x0A2,
                       0x000,0x048,0x0FE,0x07E,0x008,0x040,0x008,0x091,
                       0x018,0x005,0x018,0x005,0x04C,0x0A8,0x001};
UShort Part1[41]={0x30, 0x16, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x3f, 0x30, 0x8c, 0xbc, 0x22, 0x1c, 0x05, 0xdf,
                  0xf3, 0x16, 0x0a, 0x20, 0x89, 0x93, 0x00, 0x80,
                  0x14, 0xf7, 0x58, 0x00, 0x00, 0x18, 0x28, 0x32,
                  0x15, 0xff, 0xa0, 0x00, 0x01, 0x03, 0x2c, 0x11, 
                  0x92};

UShort   P3c4,P3d4,P3c0,P3ce,P3c2,P3ca,P3c6,P3c7,P3c8,P3c9,P3da;
UShort   CRT1VCLKLen;      /*VCLKData table length of bytes of each entry*/
UShort   flag_clearbuffer; /*0: no clear frame buffer 1:clear frame buffer*/
int      RAMType;
int      ModeIDOffset,StandTable,CRT1Table,ScreenOffset,VCLKData,MCLKData, ECLKData;
int      REFIndex,ModeType;
UShort   IF_DEF_LVDS;
UShort   IF_DEF_HiVision;
UShort   IF_DEF_CH7005;
UShort VBInfo, SetFlag,RVBHCFACT,RVBHCMAX,VGAVT,VGAHT,VT,HT,VGAVDE,VGAHDE;
UShort VDE,HDE,RVBHRS,NewFlickerMode,RY1COE,RY2COE,RY3COE,RY4COE;                
UShort LCDResInfo,LCDTypeInfo,LCDInfo,VCLKLen;
UShort LCDHDES,LCDVDES;
UShort DDC_Port;
UShort DDC_Index;
UShort DDC_DataShift;
UShort DDC_DeviceAddr;
UShort DDC_Flag;
UShort DDC_ReadAddr;
UShort DDC_Buffer;

static Bool SearchModeID(ULong ROMAddr, UShort ModeNo);
static Bool CheckMemorySize(ULong ROMAddr);
static void GetModePtr(ULong ROMAddr, UShort ModeNo);
static void SetSeqRegs(ULong ROMAddr);
static void SetMiscRegs(ULong ROMAddr);
static void SetCRTCRegs(ULong ROMAddr);
static void SetATTRegs(ULong ROMAddr);
static void SetGRCRegs(ULong ROMAddr);
static void ClearExt1Regs(void);
static Bool GetRatePtr(ULong ROMAddr, UShort ModeNo);
static void SetSync(ULong ROMAddr);
static void SetCRT1CRTC(ULong ROMAddr);
static void SetCRT1Offset(ULong ROMAddr);
static void SetCRT1VCLK(ULong ROMAddr);
static void SetCRT1ModeRegs(ULong ROMAddr, UShort ModeNo);
static void SetVCLKState(ULong ROMAddr, UShort ModeNo);
static void LoadDAC(ULong ROMAddr);
static void WriteDAC(UShort dl, UShort ah, UShort al, UShort dh);
static void DisplayOn(void);
static void DisplayOff(void);
static void SetReg3(UShort port, UShort data);
static UShort GetReg1(UShort port, UShort index);
static UShort GetReg2(UShort port);
static UShort GetModeIDLength(ULong ROMAddr, UShort ModeNo);
static UShort GetRefindexLength(ULong ROMAddr, UShort ModeNo);
static void SetInterlace(ULong ROMAddr, UShort ModeNo);
static void SetCRT1FIFO(ULong ROMAddr);
static UShort CalcDelay(ULong ROMAddr,UShort key);
static void SetCRT1FIFO2(ULong ROMAddr);
static UShort CalcDelay2(ULong ROMAddr,UShort key);
static void SetReg4(UShort port, ULong data);
static ULong GetReg3(UShort port);
static void SetPitch(ScrnInfoPtr pScrn, UShort BaseAddr);
static UShort CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode);
static UShort CalcRefreshRate(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void WaitVertical(void);
static Bool SetCRT2Group(UShort BaseAddr,ULong ROMAddr,UShort ModeNo, ScrnInfoPtr pScrn);
static void SetDefCRT2ExtRegs(UShort BaseAddr);
static UShort GetRatePtrCRT2(ULong ROMAddr, UShort ModeNo);
static Bool AjustCRT2Rate(ULong ROMAddr);
static void SaveCRT2Info(UShort ModeNo);
static void DisableLockRegs(void);
static void DisableCRT2(void);
static void GetCRT2Data(ULong ROMAddr,UShort ModeNo);
static void GetCRT2DataLVDS(ULong ROMAddr,UShort ModeNo);
static void GetCRT2Data301(ULong ROMAddr,UShort ModeNo);
static void GetResInfo(ULong ROMAddr,UShort ModeNo);
static void GetLVDSDesData(ULong ROMAddr,UShort ModeNo);
static void GetRAMDAC2DATA(ULong ROMAddr,UShort ModeNo);
static void GetCRT2Ptr(ULong ROMAddr,UShort ModeNo);
static void SetCRT2ModeRegs(UShort BaseAddr,UShort ModeNo);
static void SetGroup1(UShort BaseAddr,ULong ROMAddr,UShort ModeNo,
                    ScrnInfoPtr pScrn);
static void SetGroup1_LVDS(UShort  BaseAddr,ULong ROMAddr,UShort ModeNo,
                    ScrnInfoPtr pScrn);
static void SetTPData(void);
static void SetGroup1_301(UShort BaseAddr,ULong ROMAddr,UShort ModeNo,ScrnInfoPtr pScrn);
static void SetCRT2Offset(UShort Part1Port,ULong ROMAddr);
static UShort GetOffset(ULong ROMAddr);
static UShort GetColorDepth(ULong ROMAddr);
static void SetCRT2FIFO(UShort  Part1Port,ULong ROMAddr,UShort ModeNo,ScrnInfoPtr pScrn);
static UShort GetVCLK(ULong ROMAddr,UShort ModeNo);
static UShort GetQueueConfig(void);
static UShort GetVCLKPtr(ULong ROMAddr,UShort ModeNo);
static UShort GetColorTh(ULong ROMAddr);
static UShort GetMCLK(ULong ROMAddr);
static UShort GetMCLKPtr(ULong ROMAddr);
static UShort GetDRAMType(ULong ROMAddr);
static UShort CalcDelayVB(void);
static UShort GetVCLK2Ptr(ULong ROMAddr,UShort ModeNo);
static UShort GetVCLKLen(ULong ROMAddr);
static void SetCRT2Sync(UShort BaseAddr,ULong ROMAddr,UShort ModeNo);
static void GetCRT1Ptr(ULong ROMAddr);
static void SetRegANDOR(UShort Port,UShort Index,UShort DataAND,UShort DataOR);
static UShort GetVGAHT2(void);
static void SetGroup2(UShort  BaseAddr,ULong ROMAddr, UShort ModeNo);
static void SetGroup3(UShort  BaseAddr,ULong ROMAddr);
static void SetGroup4(UShort  BaseAddr,ULong ROMAddr,UShort ModeNo);
static void SetCRT2VCLK(UShort BaseAddr,ULong ROMAddr,UShort ModeNo);
static void SetGroup5(UShort  BaseAddr,ULong ROMAddr);
static void EnableCRT2(void);
static void LoadDAC2(ULong ROMAddr,UShort Part5Port);
static void WriteDAC2(UShort Pdata,UShort dl, UShort ah, UShort al, UShort dh);
static void SetLockRegs(void);
static void GetVBInfo(UShort BaseAddr,ULong ROMAddr);
static Bool BridgeIsEnable(UShort BaseAddr);
static Bool BridgeInSlave(void);
static Bool GetLCDResInfo(ULong ROMAddr,UShort P3d4);
static void PresetScratchregister(UShort P3d4);
static Bool GetLCDDDCInfo(ScrnInfoPtr pScrn);
static void SetTVSystem(void);
static void LongWait(void);
static void VBLongWait(void);
static Bool WaitVBRetrace(UShort  BaseAddr);
static void ModCRT1CRTC(ULong ROMAddr,UShort ModeNo);
static void SetCRT2ECLK(ULong ROMAddr, UShort ModeNo);
static UShort GetLVDSDesPtr(ULong ROMAddr,UShort ModeNo);
static Bool GetLVDSCRT1Ptr(ULong ROMAddr,UShort ModeNo);
static void SetCHTVReg(ULong ROMAddr,UShort ModeNo);
static void SetCHTVRegANDOR(UShort tempax,UShort tempbh);
static void GetCHTVRegPtr(ULong ROMAddr,UShort ModeNo);
static void SetSwitchDDC2(void);
static void SetStart(void);
static void SetStop(void);
static UShort WriteDDC2Data(UShort tempax);
static UShort ReadDDC2Data(UShort tempax);
static void SetSCLKLow(void);
static void SetSCLKHigh(void);
static void DDC2Delay(void);
static UShort CheckACK(void);

Bool SiSSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   SISPtr  pSiS = SISPTR(pScrn);
   ULong   temp;
   UShort  cr30flag,cr31flag;
   ULong   ROMAddr  = (ULong) SISPTR(pScrn)->BIOS;
   UShort  BaseAddr = (UShort) (SISPTR(pScrn)->RelIO +0x30);
   UShort  ModeNo=0;
   UShort  Rate;

   P3c4=BaseAddr+0x14;
   P3d4=BaseAddr+0x24;
   P3c0=BaseAddr+0x10;
   P3ce=BaseAddr+0x1e;
   P3c2=BaseAddr+0x12;
   P3ca=BaseAddr+0x1a;
   P3c6=BaseAddr+0x16;
   P3c7=BaseAddr+0x17;
   P3c8=BaseAddr+0x18;
   P3c9=BaseAddr+0x19;
   P3da=BaseAddr+0x2A;

   ModeNo = CalcModeIndex(pScrn, mode);
   Rate = CalcRefreshRate(pScrn, mode);
   SetReg1(P3d4, 0x33, Rate);

   SetReg1(P3c4, 0x20, 0xa1);
   SetReg1(P3c4, 0x1E, 0x5A);

   if(pSiS->VBFlags & VB_LVDS)
      IF_DEF_LVDS = 1;
   else
      IF_DEF_LVDS = 0;
   if(pSiS->VBFlags & VB_CHRONTEL)
      IF_DEF_CH7005 = 1;
   else
      IF_DEF_CH7005 = 0;


/* ynlai begin */
   IF_DEF_HiVision=0;
/* ynlai end */

   PresetScratchregister(P3d4); /* add for CRT2 */
   /* replace GetSenseStatus,SetTVSystem,SetDisplayInfo */

   DisplayOff();
   SetReg1(P3c4,0x05,0x86);                     /* 1.Openkey */
   temp=SearchModeID(ROMAddr,ModeNo);           /* 2.Get ModeID Table */
   if(temp==0)  return(0);

   /*  SetTVSystem();   */                            /* add for CRT2 */
   /*GetLCDDDCInfo(pScrn);*/                        /* add for CRT2 */
   GetVBInfo(BaseAddr,ROMAddr);                 /* add for CRT2 */
   GetLCDResInfo(ROMAddr, P3d4);                /* add for CRT2 */

   temp=CheckMemorySize(ROMAddr);               /* 3.Check memory size */
   if(temp==0) return(0);
   cr30flag=(UChar)GetReg1(P3d4,0x30);
   if(((cr30flag&0x01)==1)||((cr30flag&0x02)==0)){
     /* if cr30 d[0]=1 or d[1]=0 set crt1 */
     SetReg1(P3d4,0x34,ModeNo);
     /* set CR34->CRT1 ModeNofor CRT2 FIFO */
     GetModePtr(ROMAddr,ModeNo);                  /* 4.GetModePtr */
     SetSeqRegs(ROMAddr);                         /* 5.SetSeqRegs */
     SetMiscRegs(ROMAddr);                        /* 6.SetMiscRegs */
     SetCRTCRegs(ROMAddr);                        /* 7.SetCRTCRegs */
     SetATTRegs(ROMAddr);                         /* 8.SetATTRegs */
     SetGRCRegs(ROMAddr);                         /* 9.SetGRCRegs */
     ClearExt1Regs();                             /* 10.Clear Ext1Regs */
     temp=GetRatePtr(ROMAddr,ModeNo);             /* 11.GetRatePtr */
     if(temp) {
       SetSync(ROMAddr);                          /* 12.SetSync */
       SetCRT1CRTC(ROMAddr);                      /* 13.SetCRT1CRTC */
       SetCRT1Offset(ROMAddr);                    /* 14.SetCRT1Offset */
       SetCRT1VCLK(ROMAddr);                      /* 15.SetCRT1VCLK */
       SetVCLKState(ROMAddr, ModeNo);
       if( (pSiS->Chipset == PCI_CHIP_SIS630) || (pSiS->Chipset == PCI_CHIP_SIS540) )
          SetCRT1FIFO2(ROMAddr);
       else
          SetCRT1FIFO(ROMAddr);
     }
     SetCRT1ModeRegs(ROMAddr, ModeNo);
     if( (pSiS->Chipset == PCI_CHIP_SIS630) || (pSiS->Chipset == PCI_CHIP_SIS540) )
         SetInterlace(ROMAddr,ModeNo);
     LoadDAC(ROMAddr);
   }
   cr31flag=(UChar)GetReg1(P3d4,0x31);
   if(((cr30flag&0x01)==1)||((cr30flag&0x03)==0x02)||
     (((cr30flag&0x03)==0x00)&&((cr31flag&0x20)==0x20))) {
     /* if CR30 d[0]=1 or d[1:0]=10, set CRT2 or cr30 cr31== 0x00 0x20 */
     SetCRT2Group(BaseAddr,ROMAddr,ModeNo, pScrn);   /*    add for CRT2   */
   }

/* ynlai begin test */
/* ynlai end test */

   SetPitch(pScrn, BaseAddr);                     /* 16.SetPitch */
   WaitVertical();
   DisplayOn();                                   /* 17.DisplayOn */
   return TRUE;
}

static Bool SearchModeID(ULong ROMAddr, UShort ModeNo)
{
   UChar ModeID;
   UShort  usIDLength;

   ModeIDOffset=*((UShort *)(ROMAddr+0x20A));      /* Get EModeIDTable */
   ModeID=*((UChar *)(ROMAddr+ModeIDOffset));      /* Offset 0x20A  */
   usIDLength = GetModeIDLength(ROMAddr, ModeNo);
   while(ModeID!=0xff && ModeID!=ModeNo) {
/*    ModeIDOffset=ModeIDOffset+10;   */           /*StructSize  */
      ModeIDOffset=ModeIDOffset+usIDLength;
      ModeID=*((UChar *)(ROMAddr+ModeIDOffset));
   }
   if(ModeID==0xff) return(FALSE);
   else return(TRUE);
}

static Bool CheckMemorySize(ULong ROMAddr)
{
  UShort memorysize;
  UShort modeflag;
  UShort temp;

  modeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag   */
  ModeType=modeflag&ModeInfoFlag;                      /* Get mode type    */

  memorysize=modeflag&MemoryInfoFlag;
  memorysize=memorysize>MemorySizeShift;
  memorysize++;                                        /* Get memory size  */

  temp=GetReg1(P3c4,0x14);                             /* Get DRAM Size    */
  temp=temp&0x3F;
  temp++;

  if(temp<memorysize) return(FALSE);
  else return(TRUE);
}

static void GetModePtr(ULong ROMAddr, UShort ModeNo)
{
   UChar index;

   StandTable=*((UShort *)(ROMAddr+0x202));          /* Get First  0x202  */
                                                     /* StandTable Offset */
   if(ModeNo<=13) {
     index=*((UChar *)(ROMAddr+ModeIDOffset+0x03));  /* si+St_ModeFlag    */
   }
   else {
     if(ModeType <= 0x02) index=0x1B;                /* 02 -> ModeEGA     */
     else index=0x0F;
   }
   StandTable=StandTable+64*index;
}

static void SetSeqRegs(ULong ROMAddr)
{
   UChar SRdata;
   UShort i;

   SetReg1(P3c4,0x00,0x03);                        /* Set SR0               */
   StandTable=StandTable+0x05;
   SRdata=*((UChar *)(ROMAddr+StandTable));        /* Get SR01 from file    */
   if(IF_DEF_LVDS==1){
     if(IF_DEF_CH7005==1) {
       if(VBInfo&SetCRT2ToTV) {
         if(VBInfo&SetInSlaveMode) {
           SRdata=SRdata|0x01;
         }      
       }
     }
     if(VBInfo&SetCRT2ToLCD){
       if(VBInfo&SetInSlaveMode){
         if(LCDInfo&LCDNonExpanding){
           SRdata=SRdata|0x01;
         }
       }
     }
   }
   SRdata=SRdata|0x20;
   SetReg1(P3c4,0x01,SRdata);                      /* Set SR1               */
   for(i=02;i<=04;i++) {
     StandTable++;
     SRdata=*((UChar *)(ROMAddr+StandTable));      /* Get SR2,3,4 from file */
     SetReg1(P3c4,i,SRdata);                       /* Set SR2 3 4           */
   }
}

static void SetMiscRegs(ULong ROMAddr)
{
   UChar Miscdata;

   StandTable++;
   Miscdata=*((UChar *)(ROMAddr+StandTable));      /* Get Misc from file  */
   SetReg3(P3c2,Miscdata);                         /* Set Misc(3c2)       */
}

static void SetCRTCRegs(ULong ROMAddr)
{
  UChar CRTCdata;
  UShort i;

  CRTCdata=(UChar)GetReg1(P3d4,0x11);
  CRTCdata=CRTCdata&0x7f;
  SetReg1(P3d4,0x11,CRTCdata);                     /* Unlock CRTC        */

  for(i=0;i<=0x18;i++) {
     StandTable++;
     CRTCdata=*((UChar *)(ROMAddr+StandTable));    /* Get CRTC from file */
     SetReg1(P3d4,i,CRTCdata);                     /* Set CRTC(3d4)      */
  }
}

static void SetATTRegs(ULong ROMAddr)
{
   UChar ARdata;
   UShort i;

   for(i=0;i<=0x13;i++) {
     StandTable++;
     ARdata=*((UChar *)(ROMAddr+StandTable));    /* Get AR for file  */
     if(IF_DEF_LVDS==1){  /*for LVDS*/
       if(IF_DEF_CH7005==1) {
         if(VBInfo&SetCRT2ToTV) {
           if(VBInfo&SetInSlaveMode) {
             if(i==0x13) ARdata=0;              
           }
         }
       }
       if(VBInfo&SetCRT2ToLCD){
         if(VBInfo&SetInSlaveMode){
           if(i==0x13) ARdata=0;
         }
       }
     }
     GetReg2(P3da);                              /* reset 3da        */
     SetReg3(P3c0,i);                            /* set index        */
     SetReg3(P3c0,ARdata);                       /* set data         */
   }

   GetReg2(P3da);                                /* reset 3da        */
   SetReg3(P3c0,0x14);                           /* set index        */
   SetReg3(P3c0,0x00);                           /* set data         */
   GetReg2(P3da);                                /* Enable Attribute */
   SetReg3(P3c0,0x20);
}

static void SetGRCRegs(ULong ROMAddr)
{
   UChar GRdata;
   UShort i;

   for(i=0;i<=0x08;i++) {
     StandTable++;
     GRdata=*((UChar *)(ROMAddr+StandTable));    /* Get GR from file */
     SetReg1(P3ce,i,GRdata);                     /* Set GR(3ce)      */
   }
   if(ModeType>ModeVGA){
     GRdata=(UChar)GetReg1(P3ce,0x05);
     GRdata=GRdata&0xBF;
     SetReg1(P3ce,0x05,GRdata);
   }
}

static void ClearExt1Regs()
{
  UShort i;

  for(i=0x0A;i<=0x0E;i++) SetReg1(P3c4,i,0x00);      /* Clear SR0A-SR0E */
}


static Bool GetRatePtr(ULong ROMAddr, UShort ModeNo)
{
  short  index;
  UShort temp;
  UShort ulRefIndexLength;

  if(ModeNo<0x14) return(FALSE);                       /* Mode No <= 13h then return */

  index=GetReg1(P3d4,0x33);                            /* Get 3d4 CRTC33   */
  index=index&0x0F;                                    /* Frame rate index */
  if(index!=0) index--;
  REFIndex=*((UShort *)(ROMAddr+ModeIDOffset+0x04));   /* si+Ext_point   */

  ulRefIndexLength = GetRefindexLength(ROMAddr, ModeNo);
  do {
    temp=*((UShort *)(ROMAddr+REFIndex));              /* di => REFIndex */
    if(temp==0xFFFF) break;
    temp=temp&ModeInfoFlag;
    if(temp<ModeType) break;

    REFIndex=REFIndex+ulRefIndexLength;                /* rate size   */
    index--;
  } while(index>=0);

  REFIndex=REFIndex-ulRefIndexLength;                  /* rate size   */
  return(TRUE);
}

static void SetSync(ULong ROMAddr)
{
  UShort sync;
  UShort temp;

   sync=*((UShort *)(ROMAddr+REFIndex));               /* di+0x00 */
   sync=sync&0xC0;
   temp=0x2F;
   temp=temp|sync;
   SetReg3(P3c2,temp);                                 /* Set Misc(3c2) */
}

static void SetCRT1CRTC(ULong ROMAddr)
{
  UChar  index;
  UChar  data;
  UShort i;

  index=*((UChar *)(ROMAddr+REFIndex+0x02)) & 0x3F;   /* Get index */
  CRT1Table=*((UShort *)(ROMAddr+0x204));             /* Get CRT1Table */
  CRT1Table=CRT1Table+index*CRT1Len;

  data=(UChar)GetReg1(P3d4,0x11);
  data=data&0x7F;
  SetReg1(P3d4,0x11,data);                            /* Unlock CRTC */

  CRT1Table--;
  for(i=0;i<=0x05;i++) {
    CRT1Table++;
    data=*((UChar *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x06;i<=0x07;i++) {
    CRT1Table++;
    data=*((UChar *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x10;i<=0x12;i++) {
    CRT1Table++;
    data=*((UChar *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x15;i<=0x16;i++) {
    CRT1Table++;
    data=*((UChar *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x0A;i<=0x0C;i++) {
    CRT1Table++;
    data=*((UChar *)(ROMAddr+CRT1Table));
    SetReg1(P3c4,i,data);
  }

  CRT1Table++;
   data=*((UChar *)(ROMAddr+CRT1Table));
  data=data&0xE0;
  SetReg1(P3c4,0x0E,data);

  data=(UChar)GetReg1(P3d4,0x09);
  data=data&0xDF;
  i=*((UChar *)(ROMAddr+CRT1Table));
  i=i&0x01;
  i=i<<5;
  data=data|i;
  i=*((UShort *)(ROMAddr+ModeIDOffset+0x01));
  i=i&DoubleScanMode;
  if(i) data=data|0x80;
  SetReg1(P3d4,0x09,data);

  if(ModeType>0x03) SetReg1(P3d4,0x14,0x4F);
}

static void SetCRT1Offset(ULong ROMAddr)
{
   UShort temp,ah,al;
   UShort temp2,i;
   UShort DisplayUnit;

   temp=*((UChar *)(ROMAddr+ModeIDOffset+0x03));         /* si+Ext_ModeInfo  */
   temp=temp>>4;                                         /* index            */
   ScreenOffset=*((UShort *)(ROMAddr+0x206));            /* ScreenOffset     */
   temp=*((UChar *)(ROMAddr+ScreenOffset+temp));         /* data             */

   temp2=*((UShort *)(ROMAddr+REFIndex+0x00));
   temp2=temp2&InterlaceMode;
   if(temp2) temp=temp<<1;
   temp2=ModeType-ModeEGA;
   switch (temp2) {
     case 0 : temp2=1; break;
     case 1 : temp2=2; break;
     case 2 : temp2=4; break;
     case 3 : temp2=4; break;
     case 4 : temp2=6; break;
     case 5 : temp2=8; break;
   }
   temp=temp*temp2;
   DisplayUnit=temp;

   temp2=temp;
   temp=temp>>8;                                         /* ah */
   temp=temp&0x0F;
   i=GetReg1(P3c4,0x0E);
   i=i&0xF0;
   i=i|temp;
   SetReg1(P3c4,0x0E,i);

   temp=(UChar)temp2;
   temp=temp&0xFF;                                        /* al */
   SetReg1(P3d4,0x13,temp);

   temp2=*((UShort *)(ROMAddr+REFIndex+0x00));
   temp2=temp2&InterlaceMode;
   if(temp2) DisplayUnit>>=1;

   DisplayUnit=DisplayUnit<<5;
   ah=(DisplayUnit&0xff00)>>8;
   al=DisplayUnit&0x00ff;
   if(al==0) ah=ah+1;
   else ah=ah+2;
   SetReg1(P3c4,0x10,ah);
}

static void SetCRT1VCLK(ULong ROMAddr)
{
  UShort i;
  UChar  index,data;

  index=*((UChar *)(ROMAddr+REFIndex+0x03)) & 0x3F; 
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((UShort *)(ROMAddr+0x208));
  VCLKData=VCLKData+data;

  SetReg1(P3c4,0x31,0);
  for(i=0x2B;i<=0x2C;i++) {
     data=*((UChar *)(ROMAddr+VCLKData));
     SetReg1(P3c4,i,data);
     VCLKData++;
  }
  SetReg1(P3c4,0x2D,0x80);
}


static void SetCRT1ModeRegs(ULong ROMAddr, UShort ModeNo)
{
  UShort data,data2,data3;

  if(ModeNo>0x13)   data=*((UShort *)(ROMAddr+REFIndex+0x00));
  else data=0;
  data2=0;
  if(ModeNo>0x13)
    if(ModeType>0x02) {
       data2=data2|0x02;
       data3=ModeType-ModeVGA;
       data3=data3<<2;
       data2=data2|data3;
    }

   data=data&InterlaceMode;
   if(data) data2=data2|0x20;
   SetReg1(P3c4,0x06,data2);

   data=GetReg1(P3c4,0x01);
   data=data&0xF7;
   data2=*((UShort *)(ROMAddr+ModeIDOffset+0x01));
   data2=data2&HalfDCLK;
   if(data2) data=data|0x08;
   SetReg1(P3c4,0x01,data);

   data=GetReg1(P3c4,0x0F);
   data=data&0xF7;
   data2=*((UShort *)(ROMAddr+ModeIDOffset+0x01));
   data2=data2&LineCompareOff;
   if(data2) data=data|0x08;
   SetReg1(P3c4,0x0F,data);

   data=GetReg1(P3c4,0x21);
   data=data&0x1F;
   if(ModeType==0x00) data=data|0x60;                /* Text Mode */
   else if(ModeType<=0x02) data=data|0x00;           /* EGA Mode  */
   else data=data|0xA0;                              /* VGA Mode  */
   SetReg1(P3c4,0x21,data);
}

static void SetVCLKState(ULong ROMAddr, UShort ModeNo)
{
   UShort data,data2;
   UShort VCLK;
   UChar  index;

  index=*((UChar *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((UShort *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((UShort *)(ROMAddr+VCLKData));
  if(ModeNo<=0x13) VCLK=0;

   data=GetReg1(P3c4,0x07);
   data=data&0x7B;
   if(VCLK>=150) data=data|0x80;                     /* VCLK > 150 */
   SetReg1(P3c4,0x07,data);

   data=GetReg1(P3c4,0x32);
   data=data&0xD7;
   if(VCLK>=150) data=data|0x08;                     /* VCLK > 150 */
   SetReg1(P3c4,0x32,data);

   data2=0x03;
   if(VCLK>135) data2=0x02;
   if(VCLK>160) data2=0x01;
   if(VCLK>260) data2=0x00;
   data=GetReg1(P3c4,0x07);
   data=data&0xFC;
   data=data|data2;
   SetReg1(P3c4,0x07,data);
}

static void LoadDAC(ULong ROMAddr)
{
   UShort data,data2;
   UShort time,i,j,k;
   UShort m,n,o;
   UShort si,di,bx,dl;
   UShort al,ah,dh;
   UShort *table=0;

   data=*((UShort *)(ROMAddr+ModeIDOffset+0x01));
   data=data&DACInfoFlag;
   time=64;
   if(data==0x00) table=MDA_DAC;
   if(data==0x08) table=CGA_DAC;
   if(data==0x10) table=EGA_DAC;
   if(data==0x18) {
     time=256;
     table=VGA_DAC;
   }
   if(time==256) j=16;
   else j=time;

   SetReg3(P3c6,0xFF);
   SetReg3(P3c8,0x00);

   for(i=0;i<j;i++) {
      data=table[i];
      for(k=0;k<3;k++) {
        data2=0;
        if(data&0x01) data2=0x2A;
        if(data&0x02) data2=data2+0x15;
        SetReg3(P3c9,data2);
        data=data>>2;
      }
   }

   if(time==256) {
      for(i=16;i<32;i++) {
         data=table[i];
         for(k=0;k<3;k++) SetReg3(P3c9,data);
      }
      si=32;
      for(m=0;m<9;m++) {
         di=si;
         bx=si+0x04;
         dl=0;
         for(n=0;n<3;n++) {
            for(o=0;o<5;o++) {
              dh=table[si];
              ah=table[di];
              al=table[bx];
              si++;
              WriteDAC(dl,ah,al,dh);
            }         /* for 5 */
            si=si-2;
            for(o=0;o<3;o++) {
              dh=table[bx];
              ah=table[di];
              al=table[si];
              si--;
              WriteDAC(dl,ah,al,dh);
            }         /* for 3 */
            dl++;
         }            /* for 3 */
         si=si+5;
      }               /* for 9 */
   }
}

static void WriteDAC(UShort dl, UShort ah, UShort al, UShort dh)
{
  UShort temp;
  UShort bh,bl;

  bh=ah;
  bl=al;
  if(dl!=0) {
    temp=bh;
    bh=dh;
    dh=temp;
    if(dl==1) {
       temp=bl;
       bl=dh;
       dh=temp;
    }
    else {
       temp=bl;
       bl=bh;
       bh=temp;
    }
  }
  SetReg3(P3c9,(UShort)dh);
  SetReg3(P3c9,(UShort)bh);
  SetReg3(P3c9,(UShort)bl);
}

static void DisplayOn(void)
{
   UShort data;

   data=GetReg1(P3c4,0x01);
   data=data&0xDF;
   SetReg1(P3c4,0x01,data);
}

static void DisplayOff(void)
{
   UShort data;

   data=GetReg1(P3c4,0x01);
   data=data|0x20;
   SetReg1(P3c4,0x01,data);
}

void SetReg1(UShort port, UShort index, UShort  data)
{
    outb(port ,(UChar)(index & 0xff));
    port++;
    outb(port ,(UChar)(data  & 0xff));
}

static void SetReg3(UShort port, UShort data)
{
    outb(port, (UChar)(data & 0xff));
}

static UShort GetReg1(UShort port, UShort index)
{
    UChar   data;

    outb(port, (UChar)(index & 0xff));
    port += 1;
    data = inb(port);
   return(data);
}

static UShort GetReg2(UShort port)
{
    UChar   data;

    data = inb(port);

    return(data);
}

static UShort GetModeIDLength(ULong ROMAddr, UShort ModeNo)
{
   UChar  ModeID;
   UShort modeidlength;
   UShort usModeIDOffset;

   return(10);
   modeidlength=0;
   usModeIDOffset=*((UShort *)(ROMAddr+0x20A));      /* Get EModeIDTable    */
   ModeID=*((UChar *)(ROMAddr+usModeIDOffset));      /* Offset 0x20A        */
   while(ModeID!=0x2E) {
      modeidlength++;
      usModeIDOffset=usModeIDOffset+1;               /* 10 <= ExtStructSize */
      ModeID=*((UChar *)(ROMAddr+usModeIDOffset));
   }
   return(modeidlength);
}

static UShort GetRefindexLength(ULong ROMAddr, UShort ModeNo)
{
   UChar ModeID;
   UChar temp;
   UShort refindexlength;
   UShort usModeIDOffset;
   UShort usREFIndex;
   UShort usIDLength;

   usModeIDOffset=*((UShort *)(ROMAddr+0x20A));           /* Get EModeIDTable  */
   ModeID=*((UChar *)(ROMAddr+usModeIDOffset));           /* Offset 0x20A      */
   usIDLength = GetModeIDLength(ROMAddr, ModeNo);
   while(ModeID!=0x40) {
      usModeIDOffset=usModeIDOffset+usIDLength;           /*10 <= ExtStructSize */
      ModeID=*((UChar *)(ROMAddr+usModeIDOffset));
   }

   refindexlength=1;
   usREFIndex=*((UShort *)(ROMAddr+usModeIDOffset+0x04)); /* si+Ext_point      */
   usREFIndex++;
   temp=*((UChar *)(ROMAddr+usREFIndex));                 /* di => REFIndex    */
   while(temp!=0xFF) {
      refindexlength++;
      usREFIndex++;
      temp=*((UChar *)(ROMAddr+usREFIndex));              /* di => REFIndex    */
   }
   return(refindexlength);
}

static void SetInterlace(ULong ROMAddr, UShort ModeNo)
{
  ULong Temp;
  UShort data,Temp2;

  Temp = (ULong)GetReg1(P3d4, 0x01);
  Temp++;
  Temp=Temp*8;

  if(Temp==1024) data=0x0035;
  else if(Temp==1280) data=0x0048;
  else data=0x0000;

  Temp2=*((UShort *)(ROMAddr+REFIndex+0x00));
  Temp2 &= InterlaceMode;
  if(Temp2 == 0) data=0x0000;

  SetReg1(P3d4,0x19,data);

  Temp = (ULong)GetReg1(P3d4, 0x1A);
  Temp2= (UShort)(Temp & 0xFC);
  SetReg1(P3d4,0x1A,(UShort)Temp);

  Temp = (ULong)GetReg1(P3c4, 0x0f);
  Temp2= (UShort)Temp & 0xBF;
  if(ModeNo==0x37) Temp2=Temp2|0x40;
  SetReg1(P3d4,0x1A,(UShort)Temp2);
}

static void SetCRT1FIFO(ULong ROMAddr)
{
  UShort  index,data,VCLK,data2,MCLKOffset,MCLK,colorth=0;
  UShort  ah,bl,A,B;

  index=*((UChar *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((UShort *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((UShort *)(ROMAddr+VCLKData));           /* Get VCLK */

  MCLKOffset=*((UShort *)(ROMAddr+0x20C));
  index=GetReg1(P3c4,0x3A);
  index=index&07;
  MCLKOffset=MCLKOffset+index*5;
  MCLK=*((UChar *)(ROMAddr+MCLKOffset+0x03));     /* Get MCLK */

  data2=ModeType-0x02;
    switch (data2) {
      case 0 : colorth=1; break;
      case 1 : colorth=2; break;
      case 2 : colorth=4; break;
      case 3 : colorth=4; break;
      case 4 : colorth=6; break;
      case 5 : colorth=8; break;
    }

  do{
/*==============*/
     B=(UShort)(CalcDelay(ROMAddr,0)*VCLK*colorth);
     B=B/(16*MCLK);
     B++;

     A=(CalcDelay(ROMAddr,1)*VCLK*colorth);
     A=A/(16*MCLK);
     A++;

     if(A<4) A=0;
     else A=A-4;

     if(A>B)  bl=A;
     else bl=B;

     bl++;
     if(bl>0x13) {
        data=GetReg1(P3c4,0x16);
        data=data>>6;
        if(data!=0) {
          data--;
          data=data<<6;
          data2=GetReg1(P3c4,0x16);
          data2=(data2&0x3f)|data;
          SetReg1(P3c4,0x16,data2);
        }
        else bl=0x13;
     }
/*==============*/
  } while(bl>0x13);

  ah=bl;
  ah=ah<<4;
  ah=ah|0x0f;
  SetReg1(P3c4,0x08,ah);

  data=bl;
  data=data&0x10;
  data=data<<1;
  data2=GetReg1(P3c4,0x0F);
  data2=data2&0x9f;
  data2=data2|data;
  SetReg1(P3c4,0x0F,data2);

  data=bl+3;
  if(data>0x0f) data=0x0f;
  SetReg1(P3c4,0x3b,0x00);
  data2=GetReg1(P3c4,0x09);
  data2=data2&0xF0;
  data2=data2|data;
  SetReg1(P3c4,0x09,data2);
}

static UShort CalcDelay(ULong ROMAddr,UShort key)
{
  UShort data,data2,temp0,temp1;
  UChar   ThLowA[]=   {61,3,52,5,68,7,100,11,
                     43,3,42,5,54,7, 78,11,
                     34,3,37,5,47,7, 67,11};
  UChar   ThLowB[]=   {81,4,72,6,88,8,120,12,
                     55,4,54,6,66,8, 90,12,
                     42,4,45,6,55,8, 75,12};
  UChar   ThTiming[]= {1,2,2,3,0,1,1,2};

  data=GetReg1(P3c4,0x16);
  data=data>>6;
  data2=GetReg1(P3c4,0x14);
  data2=(data2>>4)&0x0C;
  data=data|data2;
  data=data<1;
  if(key==0) {
    temp0=(UShort)ThLowA[data];
    temp1=(UShort)ThLowA[data+1];
  }
  else {
    temp0=(UShort)ThLowB[data];
    temp1=(UShort)ThLowB[data+1];
  }

  data2=0;
  data=GetReg1(P3c4,0x18);
  if(data&0x02) data2=data2|0x01;
  if(data&0x20) data2=data2|0x02;
  if(data&0x40) data2=data2|0x04;

  data=temp1*ThTiming[data2]+temp0;
  return(data);
}

static void SetCRT1FIFO2(ULong ROMAddr)
{
  UShort  index,data,VCLK,data2,MCLKOffset,MCLK,colorth=0;
  UShort  ah,bl,B;
  ULong   eax;

  index=*((UChar *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((UShort *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((UShort *)(ROMAddr+VCLKData));           /* Get VCLK */

  MCLKOffset=*((UShort *)(ROMAddr+0x20C));
  index=GetReg1(P3c4,0x1A);
  index=index&07;
  MCLKOffset=MCLKOffset+index*5;
  MCLK=*((UShort *)(ROMAddr+MCLKOffset+0x03));     /* Get MCLK */

  data2=ModeType-0x02;
    switch (data2) {
      case 0 : colorth=1; break;
      case 1 : colorth=1; break;
      case 2 : colorth=2; break;
      case 3 : colorth=2; break;
      case 4 : colorth=3; break;
      case 5 : colorth=4; break;
    }

  do{
/*==============*/
     B=(CalcDelay2(ROMAddr,0)*VCLK*colorth);
     if (B%(16*MCLK) == 0)
     {
       B=B/(16*MCLK);
       bl=B+1;
     }
     else
     {
       B=B/(16*MCLK);
       bl=B+2;
     }

     if(bl>0x13) {
        data=GetReg1(P3c4,0x15);
        data=data&0xf0;
        if(data!=0xb0) {
          data=data+0x20;
          if(data==0xa0) data=0x30;

          data2=GetReg1(P3c4,0x15);
          data2=(data2&0x0f)|data;
          SetReg1(P3c4,0x15,data2);
        }
        else bl=0x13;
     }
/*==============*/
  } while(bl>0x13);

  data2=GetReg1(P3c4,0x15);
  data2=(data2&0xf0)>>4;
  data2=data2<<24;

/* ========================*/
  SetReg4(0xcf8,0x80000050);
  eax=GetReg3(0xcfc);
  eax=eax&0x0f0ffffff;
  eax=eax|data2;
  SetReg4(0xcfc,eax);
/* ========================*/

  ah=bl;
  ah=ah<<4;
  ah=ah|0x0f;
  SetReg1(P3c4,0x08,ah);

  data=bl;
  data=data&0x10;
  data=data<<1;
  data2=GetReg1(P3c4,0x0F);
  data2=data2&0x9f;
  data2=data2|data;
  SetReg1(P3c4,0x0F,data2);

  data=bl+3;
  if(data>0x0f) data=0x0f;
  SetReg1(P3c4,0x3b,0x00);
  data2=GetReg1(P3c4,0x09);
  data2=data2&0xF0;
  data2=data2|data;
  SetReg1(P3c4,0x09,data2);
}

static UShort CalcDelay2(ULong ROMAddr,UShort key)
{
  UShort data,index;
  UChar  LatencyFactor[]={88,80,78,72,70,00,
                          00,79,77,71,69,49,
                          88,80,78,72,70,00,
                          00,72,70,64,62,44};

  index=0;
  data=GetReg1(P3c4,0x14);
  if(data&0x80) index=index+12;

  data=GetReg1(P3c4,0x15);
  data=(data&0xf0)>>4;
  if(data&0x01) index=index+6;

  data=data>>1;
  index=index+data;
  data=LatencyFactor[index];

  return(data);
}

static void SetReg4(UShort port, ULong data)
{
    outl(port, (ULong)(data & 0xffffffff));
}

ULong GetReg3(UShort port)
{
    ULong   data;
    
    data = inl(port);
    return(data);
}

static void SetPitch(ScrnInfoPtr pScrn, UShort BaseAddr)
{
    SISPtr pSiS = SISPTR(pScrn);
    ULong  HDisplay;
    ULong  temp;
    UShort Port = BaseAddr + IND_SIS_CRT2_PORT_04;

    HDisplay = pSiS->scrnOffset / 8;
    SetReg1(P3d4, 0x13, HDisplay);
    temp = (GetReg1(P3c4, 0x0E) & 0xF0) | (HDisplay>>8);
    SetReg1(P3c4, 0x0E, temp);
    
    SetReg1(Port, 0x24, 1);
    SetReg1(Port, 0x07, HDisplay); 
    temp = (GetReg1(Port, 0x09) & 0xF0) | (HDisplay>>8);
    SetReg1(Port, 0x09, temp);


}
static UShort CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   UShort i = (pScrn->bitsPerPixel+7)/8 - 1;
   UShort ModeIndex = 0;
   switch(mode->HDisplay)
   {
     case 640:
          ModeIndex = ModeIndex_640x480[i];
          break;
     case 720:
          if(mode->VDisplay == 480)
            ModeIndex = ModeIndex_720x480[i];
          else  
            ModeIndex = ModeIndex_720x576[i];
          break;
     case 800:
          ModeIndex = ModeIndex_800x600[i];
          break;
     case 1024:
          ModeIndex = ModeIndex_1024x768[i];
          break;
     case 1280:
          ModeIndex = ModeIndex_1280x1024[i];
          break;
     case 1600:
          ModeIndex = ModeIndex_1600x1200[i];
          break;
   }

   return(ModeIndex);
}

static UShort CalcRefreshRate(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   SISPtr pSiS = SISPTR(pScrn);
   UShort Index=0;
   UShort i=0;
   UShort Rate=1;
   UShort temp = (int)(mode->VRefresh+0.5);
   
   switch(mode->HDisplay)
   {
     case 640:
          Index = 0;
          break;
     case 800:
          Index = 1;
          break;
     case 1024:
          Index = 2;
          Rate  = 2;
          break;
     case 1280:
          Index = 3;
          Rate  = 2;
          break;
     case 1600:
          Index = 4;
          break;
     case 720:
          if(mode->VDisplay == 480)
            Index = 5;
          else  
            Index = 6;
          break;
    
   }
   while(RefreshRate[Index][i] != 0)
   {
      if(temp == RefreshRate[Index][i])
      {  
         Rate=i+1;
         break;
      }
      else
         i++;
   } 
   if(pSiS->VBFlags & CRT2_VGA)
      Rate |= Rate << 4;
   return(Rate);
}

static void WaitVertical(void)
{
#if 0
  UShort tempax,tempdx;

  tempdx=0x3da;
  do {
    tempax=GetReg2(tempdx);
  } while(!(tempax&01));

  do {
    tempax=GetReg2(tempdx);
  } while(!(tempax&01));
#endif
}

static Bool SetCRT2Group(UShort BaseAddr,ULong ROMAddr,UShort ModeNo, ScrnInfoPtr pScrn)
{
   UShort temp;

   SetFlag=SetFlag|ProgrammingCRT2;
   SearchModeID(ROMAddr,ModeNo);
   temp=GetRatePtrCRT2(ROMAddr,ModeNo);
   if(((temp&0x02)==0) && ((VBInfo&CRT2DisplayFlag)==0))
     return(FALSE);
   SaveCRT2Info(ModeNo);
   DisableBridge(BaseAddr);   
   UnLockCRT2(BaseAddr);
   SetDefCRT2ExtRegs(BaseAddr);    
   SetCRT2ModeRegs(BaseAddr,ModeNo);   
   if(IF_DEF_LVDS==0) {
    if(VBInfo&CRT2DisplayFlag){
      LockCRT2(BaseAddr);
      return 0;
    }
   }
   GetCRT2Data(ROMAddr,ModeNo); 
   if(IF_DEF_LVDS==1) {
     GetLVDSDesData(ROMAddr,ModeNo);  
  }
   SetGroup1(BaseAddr,ROMAddr,ModeNo,pScrn);    
   if(IF_DEF_LVDS==0) {
     SetGroup2(BaseAddr,ROMAddr,ModeNo);  
     SetGroup3(BaseAddr,ROMAddr);  
     SetGroup4(BaseAddr,ROMAddr,ModeNo);
     SetGroup5(BaseAddr,ROMAddr);   
   }
   else {
     if(IF_DEF_CH7005==1) SetCHTVReg(ROMAddr,ModeNo);   
     ModCRT1CRTC(ROMAddr,ModeNo);   
     SetCRT2ECLK(ROMAddr,ModeNo);
   }
   EnableCRT2();
   EnableBridge(BaseAddr);
   if(IF_DEF_LVDS==0) {
  /*   SetLockRegs();  */
   }
   LockCRT2(BaseAddr);
   return 1;
}

static void SetDefCRT2ExtRegs(UShort BaseAddr)
{
  UShort  Part1Port,Part2Port,Part4Port;
  UShort  temp;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  Part2Port=BaseAddr+IND_SIS_CRT2_PORT_10;
  Part4Port=BaseAddr+IND_SIS_CRT2_PORT_14;
  if(IF_DEF_LVDS==0) {
    SetReg1(Part1Port,0x02,0x40);
    SetReg1(Part4Port,0x10,0x80);
    temp=(UChar)GetReg1(P3c4,0x16);
    temp=temp&0xC3;
    SetReg1(P3d4,0x35,temp);
  }
  else {
    SetReg1(P3d4,0x32,0x02);
    SetReg1(Part1Port,0x02,0x00);
  }
}

static UShort GetRatePtrCRT2(ULong ROMAddr, UShort ModeNo)
{                           /* return bit0=>0:standard mode 1:extended mode */
  short  index;             /*        bit1=>0:crt2 no support this mode     */
  UShort temp;             /*              1:crt2 support this mode        */
  UShort ulRefIndexLength;
  UShort temp1,modeflag1,Flag;
  short  LCDRefreshIndex[2]={0x03,0x01};

  if(IF_DEF_CH7005==1) {
    if(VBInfo&SetCRT2ToTV) {
      modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
      if(modeflag1&HalfDCLK) return(0);
    }
  }
  if(ModeNo<0x14) return(0);       /* Mode No <= 13h then return              */

  index=GetReg1(P3d4,0x33);        /* Get 3d4 CRTC33                          */
  index=index>>SelectCRT2Rate;     /* For CRT2,cl=SelectCRT2Rate=4, shr ah,cl */
  index=index&0x0F;                /* Frame rate index                        */
  if(index!=0) index--;

  if(SetFlag&ProgrammingCRT2){
    Flag=1;
    if(IF_DEF_CH7005==1) {
      if(VBInfo&SetCRT2ToTV) {
        index=0;
        Flag=0;
      }
    }
    if((Flag)&&(VBInfo&SetCRT2ToLCD)){
      if(IF_DEF_LVDS==0) {
        temp=LCDResInfo;
        temp1=LCDRefreshIndex[temp];
        if(index>temp1){
          index=temp1;
        }
      }
      else {
        index=0;
      }
    }
  }

  REFIndex=*((UShort *)(ROMAddr+ModeIDOffset+0x04));   /* si+Ext_point */

  ulRefIndexLength =Ext2StructSize;
  do {
    temp=*((UShort *)(ROMAddr+REFIndex));              /* di => REFIndex */
    if(temp==0xFFFF) break;
    temp=temp&ModeInfoFlag;
    if(temp<ModeType) break;

    REFIndex=REFIndex+ulRefIndexLength;                /* rate size */
    index--;
    if(index<0){
      if(!(VBInfo&SetCRT2ToRAMDAC)){
        if(VBInfo&SetInSlaveMode){
          temp1=*((UShort *)(ROMAddr+REFIndex+0-Ext2StructSize));
          if(temp1&InterlaceMode){
            index=0;
          }
        }
      }
    }
  } while(index>=0);
  REFIndex=REFIndex-ulRefIndexLength;                    /* rate size */

  if((SetFlag&ProgrammingCRT2)){
    temp1=AjustCRT2Rate(ROMAddr);
  }else{
    temp1=0;
  }

  return(0x01|(temp1<<1));
}

static Bool AjustCRT2Rate(ULong ROMAddr)
{
  UShort tempax,tempbx=0,temp,resinfo;
  UShort tempextinfoflag,Flag;
  tempax=0;
  if(IF_DEF_LVDS==0) {
    if(VBInfo&SetCRT2ToRAMDAC){
      tempax=tempax|SupportRAMDAC2;
    }
    if(VBInfo&SetCRT2ToLCD){
      tempax=tempax|SupportLCD;
      if(LCDResInfo!=Panel1280x1024){
        temp=*((UChar *)(ROMAddr+ModeIDOffset+0x09)); /* si+Ext_ResInfo */
        if(temp>=9) tempax=0;
      }
    }
/* ynlai begin */
    if(IF_DEF_HiVision==1) {
      tempax=tempax|SupportHiVisionTV;
      if(VBInfo&SetInSlaveMode){
        resinfo=*((UChar *)(ROMAddr+ModeIDOffset+0x09)); /*si+Ext_ResInfo  */
        if(resinfo==4) return(0);
        if(resinfo==3) {
          if(SetFlag&TVSimuMode) return(0);
        }
        if(resinfo>7) return(0);
      }
    }
    else {
      if(VBInfo&(SetCRT2ToAVIDEO|SetCRT2ToSVIDEO|SetCRT2ToSCART)){
        tempax=tempax|SupportTV;
        if(!(VBInfo&SetPALTV)){
          tempextinfoflag=*((UShort *)(ROMAddr+REFIndex+0x0)); /* di+Ext_InfoFlag */
          if(tempextinfoflag&NoSupportSimuTV){
            if(VBInfo&SetInSlaveMode){
              if(!(VBInfo&SetNotSimuMode)){
                return 0;
              }
            }
          }
        }
      }
    }
/* ynlai end   */
    tempbx=*((UShort *)(ROMAddr+ModeIDOffset+0x04));   /* si+Ext_point */
  }
  else {  /* for LVDS */
    Flag=1;
    if(IF_DEF_CH7005==1) {
      if(VBInfo&SetCRT2ToTV) {
        tempax=tempax|SupportCHTV;
        Flag=0;
      }
    }                    
    tempbx=*((UShort *)(ROMAddr+ModeIDOffset+0x04));   
    if((Flag)&&(VBInfo&SetCRT2ToLCD)){
      tempax=tempax|SupportLCD;
      temp=*((UChar *)(ROMAddr+ModeIDOffset+0x09)); /*si+Ext_ResInfo  */
      if(temp>0x08)   return(0);       /*1024x768  */
      if(LCDResInfo<Panel1024x768){
        if(temp>0x07) return(0);       /*800x600  */
        if(temp==0x04) return(0);      /*512x384  */
      }
    }
  }
  for(;REFIndex>tempbx;REFIndex-=Ext2StructSize){
     tempextinfoflag=*((UShort *)(ROMAddr+REFIndex+0x0)); /* di+Ext_InfoFlag */
     if(tempextinfoflag&tempax){
       return 1;
     }
  }
  for(REFIndex=tempbx;;REFIndex+=Ext2StructSize){
     tempextinfoflag=*((UShort *)(ROMAddr+REFIndex+0x0)); /* di+Ext_InfoFlag */
     if(tempextinfoflag==0x0FFFF){
       return 0;
     }
     if(tempextinfoflag&tempax){
       return 1;
     }
  }
  return(FALSE);
}

static void SaveCRT2Info(UShort ModeNo)
{
  UShort temp1,temp2,temp3;
  temp1=(VBInfo&SetInSlaveMode)>>8;
  temp2=~(SetInSlaveMode>>8);
  temp3=(UChar)GetReg1(P3d4,0x31);
  temp3=((temp3&temp2)|temp1);
  SetReg1(P3d4,0x31,(UShort)temp3);
  temp3=(UChar)GetReg1(P3d4,0x35);
  temp3=temp3&0xF3;
  SetReg1(P3d4,0x35,(UShort)temp3);
}

static void DisableLockRegs(void)
{
  UChar temp3;
  temp3=(UChar)GetReg1(P3c4,0x32);
  temp3=temp3&0xDF;
  SetReg1(P3c4,0x32,(UShort)temp3);
}

static void DisableCRT2(void)
{
  UChar temp3;
  temp3=(UChar)GetReg1(P3c4,0x1E);
  temp3=temp3&0xDF;
  SetReg1(P3c4,0x1E,(UShort)temp3);
}

void DisableBridge(UShort  BaseAddr)
{
  UChar   temp3,part2_02,part2_05;
  UShort  Part2Port,Part1Port=0;
  Part2Port=BaseAddr+IND_SIS_CRT2_PORT_10;

  if(IF_DEF_LVDS==0) {
    part2_02=(UChar)GetReg1(Part2Port,0x02);
    part2_05=(UChar)GetReg1(Part2Port,0x05);
/*    if(!WaitVBRetrace(BaseAddr))  */        /* return 0:no enable read dram */
    {
      LongWait();
      DisableLockRegs();
    }
    SetReg1(Part2Port,0x02,0x38);
    SetReg1(Part2Port,0x05,0xFF);
    temp3=(UChar)GetReg1(Part2Port,0x00);
    temp3=temp3&0xDF;
    SetReg1(Part2Port,0x00,(UShort)temp3);
    SetReg1(Part2Port,0x02,part2_02);
    SetReg1(Part2Port,0x05,part2_05);
    DisableCRT2();
  }
  else {
    DisableLockRegs();
    DisableCRT2();
    UnLockCRT2(BaseAddr);
    SetRegANDOR(Part1Port,0x02,0xFF,0x40); /*et Part1Port ,index 2, D6=1,  */
  }
}

static void GetCRT2Data(ULong ROMAddr,UShort ModeNo)
{
  if(IF_DEF_LVDS==0){ /*301  */
    GetCRT2Data301(ROMAddr,ModeNo);
    return;
  }else{ /*LVDS */
    GetCRT2DataLVDS(ROMAddr,ModeNo);
    return;
  }
}

static void GetCRT2DataLVDS(ULong ROMAddr,UShort ModeNo)
{
   UShort tempax,tempbx,OldREFIndex;  

   OldREFIndex=(UShort)REFIndex;         /*push di  */
   GetResInfo(ROMAddr,ModeNo);
   GetCRT2Ptr(ROMAddr,ModeNo);

   tempax=*((UShort *)(ROMAddr+REFIndex));
   tempax=tempax&0x0FFF;
   VGAHT=tempax;

   tempax=*((UShort *)(ROMAddr+REFIndex+1));
   tempax=tempax>>4;
   tempax=tempax&0x07FF;
   VGAVT=tempax;
/*   VGAVT=518;   */

   tempax=*((UShort *)(ROMAddr+REFIndex+3));
   tempax=tempax&0x0FFF;
   tempbx=*((UShort *)(ROMAddr+REFIndex+4));
   tempbx=tempbx>>4;
   tempbx=tempbx&0x07FF;

   HT=tempax;
   VT=tempbx;

   if(IF_DEF_TRUMPION==0){
     if(VBInfo&SetCRT2ToLCD){
       if(!(LCDInfo&LCDNonExpanding)){
         if(LCDResInfo==Panel800x600){
           tempax=800;
           tempbx=600;
         }else if(LCDResInfo==Panel1024x768){ 
           tempax=1024;
           tempbx=768;
         }else{
           tempax=1280;
           tempbx=1024;
         } 
         HDE=tempax;
         VDE=tempbx;
       }
     }
   }
   REFIndex=OldREFIndex;         /*pop di  */
   return;
}

static void GetCRT2Data301(ULong ROMAddr,UShort ModeNo)
{
  UShort tempax,tempbx,modeflag1,OldREFIndex;
  UShort tempal,tempah,tempbl,resinfo;

  OldREFIndex=REFIndex;         /* push di */
  RVBHRS=50;NewFlickerMode=0;RY1COE=0;
  RY2COE=0;RY3COE=0;RY4COE=0;

  GetResInfo(ROMAddr,ModeNo);
  if(VBInfo&SetCRT2ToRAMDAC){
    GetRAMDAC2DATA(ROMAddr,ModeNo);
    REFIndex=OldREFIndex;         /* pop di */
    return;
  }
  GetCRT2Ptr(ROMAddr,ModeNo);

  tempal=*((UChar *)(ROMAddr+REFIndex));
  tempah=*((UChar *)(ROMAddr+REFIndex+4));
  tempax=tempal|(((tempah<<8)>>7)&0xFF00);
  RVBHCMAX=tempax;

  tempal=*((UChar *)(ROMAddr+REFIndex+1));
  RVBHCFACT=tempal;

  tempax=*((UShort *)(ROMAddr+REFIndex+2));
  VGAHT=(tempax&0x0FFF);

  tempax=*((UShort *)(ROMAddr+REFIndex+3));
  VGAVT=((tempax>>4)&0x07FF);

  tempax=*((UShort *)(ROMAddr+REFIndex+5));
  tempax=(tempax&0x0FFF);
  tempbx=*((UShort *)(ROMAddr+REFIndex+6));
  tempbx=((tempbx>>4)&0x07FF);
  tempbl=tempbx&0x00FF;

  if(VBInfo&SetCRT2ToTV){
    tempax=*((UShort *)(ROMAddr+REFIndex+5));
    tempax=(tempax&0x0FFF);
    HDE=tempax;
    tempax=*((UShort *)(ROMAddr+REFIndex+6));
    tempax=((tempax>>4)&0x07FF);
    VDE=tempax;
    tempax=*((UShort *)(ROMAddr+REFIndex+8));
    tempbl=(tempax>>8);
    tempax=tempax&0x0FFF;
    modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
    if(modeflag1&HalfDCLK){
      tempax=*((UShort *)(ROMAddr+REFIndex+10));
    }
    RVBHRS=tempax;
/*  ynlai  begin */
    tempbl=tempbl&0x80;
    if(IF_DEF_HiVision==1) {
       resinfo=*((UChar *)(ROMAddr+ModeIDOffset+0x09)); /* si+Ext_ResInfo */
       if(resinfo==8) tempbl=0x40;
       else if(resinfo==9) tempbl=0x40;
       else if(resinfo==10) tempbl=0x40;
    }
/*  ynlai  end */
    NewFlickerMode=tempbl;

/* ynlai begin */
    if(IF_DEF_HiVision==1) {
      if(VGAVDE==350) SetFlag=SetFlag|TVSimuMode;
      tempax=ExtHiTVHT;
      tempbx=ExtHiTVVT;
      if(VBInfo&SetInSlaveMode) {
        if(SetFlag&TVSimuMode) {
          tempax=StHiTVHT;
          tempbx=StHiTVVT;
          modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
          if(!(modeflag1&Charx8Dot)){
             tempax=StHiTextTVHT;
             tempbx=StHiTextTVVT;
          }
        }
      }
    }
    else {
      tempax=*((UShort *)(ROMAddr+REFIndex+12));
      RY1COE=(tempax&0x00FF);
      RY2COE=((tempax&0xFF00)>>8);
      tempax=*((UShort *)(ROMAddr+REFIndex+14));
      RY3COE=(tempax&0x00FF);
      RY4COE=((tempax&0xFF00)>>8);
      if(!(VBInfo&SetPALTV)){
        tempax=NTSCHT;
        tempbx=NTSCVT;
      }else{
        tempax=PALHT;
        tempbx=PALVT;
      }
    }
/* ynlai end */
  }
  HT=tempax;
  VT=tempbx;
  if(!(VBInfo&SetCRT2ToLCD)){
    REFIndex=OldREFIndex;         /* pop di */
    return;
  }

  tempax=1024;
  if(VGAVDE==350){      /* cx->VGAVDE */
    tempbx=560;
  }else if(VGAVDE==400){
    tempbx=640;
  }else{
    tempbx=768;
  }

  if(LCDResInfo==Panel1280x1024){
    tempax=1280;
    if(VGAVDE==360){
      tempbx=768;
    }else if(VGAVDE==375){
      tempbx=800;
    }else if(VGAVDE==405){
      tempbx=864;
    }else{
      tempbx=1024;
    }
  }

  HDE=tempax;
  VDE=tempbx;
  REFIndex=OldREFIndex;         /* pop di */
  return;
}

static void GetResInfo(ULong ROMAddr,UShort ModeNo)
{
  UShort temp,xres,yres,modeflag1;
  if(ModeNo<=0x13){
    temp=(UShort)*((UChar *)(ROMAddr+ModeIDOffset+0x05));   /* si+St_ResInfo */
    xres=StResInfo[temp][0];
    yres=StResInfo[temp][1];
  }else{
    temp=(UShort)*((UChar *)(ROMAddr+ModeIDOffset+0x09));   /* si+Ext_ResInfo */
    xres=ModeResInfo[temp][0];  /* xres->ax */
    yres=ModeResInfo[temp][1];  /* yres->bx */
    modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));    /* si+St_ModeFlag */
    if(modeflag1&HalfDCLK){ xres=xres*2;}
    if(modeflag1&DoubleScanMode){yres=yres*2;}
  }
  if(!(LCDResInfo==Panel1024x768)){
    if(yres==400) yres=405;
    if(yres==350) yres=360;
    if(SetFlag&LCDVESATiming){
      if(yres==360) yres=375;
    }
  }
  if(IF_DEF_LVDS==1) {
    if(xres==720) xres=640;
  }
  VGAHDE=xres;
  HDE=xres;
  VGAVDE=yres;
  VDE=yres;
}

static void GetLVDSDesData(ULong ROMAddr,UShort ModeNo)
{
  UShort old_REFIndex,tempax;

  old_REFIndex=(UShort)REFIndex; /*push di  */
  REFIndex=GetLVDSDesPtr(ROMAddr,ModeNo);

  tempax=*((UShort *)(ROMAddr+REFIndex));
  tempax=tempax&0x0FFF;
  LCDHDES=tempax;

  if(LCDInfo&LCDNonExpanding){ /*hw walk-a-round  */
    if(LCDResInfo>=Panel1024x768){
      if(ModeNo<=0x13){
        LCDHDES=320;
      }
    }
  }

  tempax=*((UShort *)(ROMAddr+REFIndex+1));
  tempax=tempax>>4;
  tempax=tempax&0x07FF;
  LCDVDES=tempax;

  REFIndex=old_REFIndex; /*pop di  */
  return;  
}


static void GetRAMDAC2DATA(ULong ROMAddr,UShort ModeNo)
{
  UShort tempax,tempbx,tempbh,modeflag1,t1=0,t2;
  RVBHCMAX=1;RVBHCFACT=1;
  if(ModeNo<=0x13){
    tempax=*((UChar *)(ROMAddr+REFIndex+10));
    tempbx=*((UShort *)(ROMAddr+REFIndex+16));
  }else{
    t1=*((UChar *)(ROMAddr+REFIndex+0x2));      /* Ext_CRT1CRTC=2 */
    t1=t1*CRT1Len;
    REFIndex=*((UShort *)(ROMAddr+0x204));      /* Get CRT1Table */
    REFIndex=REFIndex+t1;
    t1=*((UChar *)(ROMAddr+REFIndex+0));
    t2=*((UChar *)(ROMAddr+REFIndex+14));
    tempax=(t1&0xFF)|((t2&0x03)<<8);
    tempbx=*((UShort *)(ROMAddr+REFIndex+6));
    t1=*((UChar *)(ROMAddr+REFIndex+13));
    t1=(t1&0x01)<<2;
  }

  tempbh=tempbx>>8;
  tempbh=((tempbh&0x20)>>4)|(tempbh&0x01);
  tempbh=tempbh|t1;
  tempbx=(tempbx&0xFF)|(tempbh<<8);
  tempax=tempax+5;
  modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
  if(modeflag1&Charx8Dot){
    tempax=tempax*8;
  }else{
    tempax=tempax*9;
  }

  VGAHT=tempax;
  HT=tempax;
  tempbx++;
  VGAVT=tempbx;
  VT=tempbx;

}

static void GetCRT2Ptr(ULong ROMAddr,UShort ModeNo)
{
  UShort tempcl,tempbx,tempal,tempax,CRT2PtrData=0;
  UShort Flag;

  if(IF_DEF_LVDS==0) {
    if(VBInfo&SetCRT2ToLCD){              /* LCD */
      tempbx=LCDResInfo;
      tempcl=LCDDataLen;
      tempbx=tempbx-Panel1024x768;
      if(!(SetFlag&LCDVESATiming)) tempbx+=5;  
    }
/* ynlai begin */
    else {
      if(IF_DEF_HiVision==1) {
        if(VGAVDE>480) SetFlag=SetFlag&(!TVSimuMode);
        tempcl=HiTVDataLen;
        tempbx=2;
        if(VBInfo&SetInSlaveMode) {
           if(!(SetFlag&TVSimuMode)) tempbx=10;
        }
      }
      else {
        if(VBInfo&SetPALTV){
          tempcl=TVDataLen;
          tempbx=3;
        }
        else{
          tempbx=4;
          tempcl=TVDataLen;
        }
      }
    }
/* ynlai end */
    if(SetFlag&TVSimuMode){
      tempbx=tempbx+4;
    }
    if(ModeNo<=0x13){
      tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));   /* si+St_CRT2CRTC */
    }else{
      tempal=*((UChar *)(ROMAddr+REFIndex+4));    /* di+Ext_CRT2CRTC */
    }
    tempal=tempal&0x1F;

    tempax=tempal*tempcl;
    REFIndex=*((UShort *)(ROMAddr + 0x20E + tempbx*2));
    REFIndex+=tempax;
  }
  else {   /* LVDS */
    Flag=1;
    tempbx=0;
    if(IF_DEF_CH7005==1) {
      if(!(VBInfo&SetCRT2ToLCD)) {
        Flag=0;
        tempbx=7;
        if(VBInfo&SetPALTV) tempbx=tempbx+2;
        if(VBInfo&SetCHTVOverScan) tempbx=tempbx+1;
      } 
    }
    tempcl=LVDSDataLen;
    if(Flag==1) {
      tempbx=LCDResInfo-Panel800x600;
      if(LCDInfo&LCDNonExpanding){
        tempbx=tempbx+3;
      }
    } 
    if(ModeNo<=0x13) tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));      /* si+St_CRT2CRTC */
    else tempal=*((UChar *)(ROMAddr+REFIndex+0x04));      /* di+Ext_CRT2CRTC */
    tempal=tempal&0x1F;
    tempax=tempal*tempcl;
    CRT2PtrData=*((UShort *)(ROMAddr+ADR_CRT2PtrData)); /*ADR_CRT2PtrData is defined in init.def  */
    REFIndex=*((UShort *)(ROMAddr+CRT2PtrData+tempbx*2));
    REFIndex+=tempax;
  }
}

void UnLockCRT2(UShort BaseAddr)
{
  UChar temp3;
  UShort  Part1Port;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  temp3=(UChar)GetReg1(Part1Port,0x24);
  temp3=temp3|0x01;
  SetReg1(Part1Port,0x24,(UShort)temp3);
}

static void SetCRT2ModeRegs(UShort BaseAddr,UShort ModeNo)
{
  UShort i,j;
  UShort tempcl,tempah,temp3;
  UShort  Part4Port;
  UShort  Part1Port;
  Part4Port=BaseAddr+IND_SIS_CRT2_PORT_14;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  for(i=0,j=4;i<3;i++,j++){
     SetReg1(Part1Port,j,0);
  }

  tempcl=ModeType;
  if(ModeNo>0x13){
    tempcl=tempcl-ModeVGA;
    if((tempcl>0)||(tempcl==0)){
      tempah=((0x010>>tempcl)|0x080);
    }
  }else{
    tempah=0x080;
  }

  if(VBInfo&SetInSlaveMode){
    tempah=(tempah^0x0A0);
  }
  if(VBInfo&CRT2DisplayFlag){
    tempah=0;
  }
  SetReg1(Part1Port,0,tempah);


  if(IF_DEF_LVDS==0) {
    tempah=0x01;
    if(!(VBInfo&SetInSlaveMode)){
      tempah=(tempah|0x02);
    }
    if(!(VBInfo&SetCRT2ToRAMDAC)){
      tempah=(tempah^0x05);
      if(!(VBInfo&SetCRT2ToLCD)){
        tempah=(tempah^0x01);
      }
    }
    tempah=(tempah<<5)&0xFF;
    if(VBInfo&CRT2DisplayFlag){
      tempah=0;
    }
    SetReg1(Part1Port,0x01,tempah);

    tempah=tempah>>5;
    if((ModeType==ModeVGA)&&(!(VBInfo&SetInSlaveMode))){
      tempah=tempah|0x010;
    }
    if(LCDResInfo!=Panel1024x768){
      tempah=tempah|0x080;
    }
    if(VBInfo&SetCRT2ToTV){
      if(VBInfo&SetInSlaveMode){
        tempah=tempah|0x020;
      }
    }

    temp3=(UChar)GetReg1(Part4Port,0x0D);
    temp3=temp3&(~0x0BF);
    temp3=temp3|tempah;
    SetReg1(Part4Port,0x0D,(UShort)temp3);

/* ynlai begin */
    tempah=0;
    if(VBInfo&SetCRT2ToTV) {
      if(VBInfo&SetInSlaveMode) {
        if(!(SetFlag&TVSimuMode)) {
          if(IF_DEF_HiVision==0) {
            SetFlag=SetFlag|RPLLDIV2XO;
            tempah=tempah|0x40;
          }
        }
      }
      else {
        SetFlag=SetFlag|RPLLDIV2XO;
        tempah=tempah|0x40;
      }
    }
    if(LCDResInfo==Panel1280x1024) tempah=tempah|0x80;
    if(LCDResInfo==Panel1280x960) tempah=tempah|0x80;
    SetReg1(Part4Port,0x0C,(UShort)temp3);
/* ynlai end */
  }
  else {
    tempah=0;
    if(!(VBInfo&SetInSlaveMode)){
      tempah=tempah|0x02;
    }
    tempah=(tempah<<5)&0x0FF;
    if(VBInfo&CRT2DisplayFlag){
      tempah=0;
    }
    SetReg1(Part1Port,0x01,tempah);
  }
}

static void SetGroup1(UShort BaseAddr,ULong ROMAddr,UShort ModeNo,
               ScrnInfoPtr pScrn)
{
  if(IF_DEF_LVDS==0){  /*301  */
    SetGroup1_301(BaseAddr,ROMAddr,ModeNo,pScrn);
  }else{  /*LVDS  */
    SetGroup1_LVDS(BaseAddr,ROMAddr,ModeNo,pScrn);
  }
}

static void SetGroup1_LVDS(UShort  BaseAddr,ULong ROMAddr,UShort ModeNo,
                    ScrnInfoPtr pScrn)
{
  UShort temp1,temp2,tempcl,tempch,tempbh,tempal,tempah,tempax,tempbx;
  UShort tempcx,OldREFIndex,lcdhdee;
  UShort Part1Port;
  UShort temppush1,temppush2;
  unsigned long int tempeax,tempebx,tempecx,templong;

  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  OldREFIndex=(UShort)REFIndex;         /*push di  */

  SetCRT2Offset(Part1Port,ROMAddr);
  SetCRT2FIFO(Part1Port,ROMAddr,ModeNo,pScrn);
  SetCRT2Sync(BaseAddr,ROMAddr,ModeNo);

  temp1=(VGAHT-1)&0x0FF;            /*BTVGA2HT 0x08,0x09  */
  SetReg1(Part1Port,0x08,temp1);
  temp1=(((VGAHT-1)&0xFF00)>>8)<<4;
  SetRegANDOR(Part1Port,0x09,~0x0F0,temp1);
 
  
  temp1=(VGAHDE+12)&0x0FF;              /*BTVGA2HDEE 0x0A,0x0C  */
  SetReg1(Part1Port,0x0A,temp1);
  /*temp1=((VGAHDE+12)&0xFF00)>>8;    Wrong              */
  /*SetReg1(Part1Port,0x0C,temp1);  */
  
  temp1=VGAHDE+12;      /*bx  BTVGA@HRS 0x0B,0x0C  */
  temp2=(VGAHT-VGAHDE)>>2;      /* */
  temp1=temp1+temp2;
  temp2=(temp2<<1)+temp1;
  tempcl=temp2&0x0FF;

  SetReg1(Part1Port,0x0B,(UShort)(temp1&0x0FF));
  tempah=(temp1&0xFF00)>>8;
  tempbh=((((VGAHDE+12)&0xFF00)>>8)<<4)&0x0FF;
  tempah=tempah|tempbh;
  SetReg1(Part1Port,0x0C,tempah);
  SetReg1(Part1Port,0x0D,tempcl);       /*BTVGA2HRE 0x0D  */
  tempcx=(VGAVT-1);
  tempah=tempcx&0x0FF;
  if(IF_DEF_CH7005==1) {
    if(VBInfo&0x0C)  tempah=tempah-1;
  } 
  SetReg1(Part1Port,0x0E,tempah);        /*BTVGA2TV 0x0E,0x12 */
  tempbx=VGAVDE-1;
  tempah=tempbx&0x0FF;
  if(IF_DEF_CH7005==1) {
    if(VBInfo&0x0C)  tempah=tempah-1;
  } 
  SetReg1(Part1Port,0x0F,tempah);       /*BTVGA2VDEE 0x0F,0x12  */
  tempah=((tempbx&0xFF00)<<3)>>8;
  tempah=tempah|((tempcx&0xFF00)>>8);
  SetReg1(Part1Port,0x12,tempah);
  
  tempbx=(VGAVT+VGAVDE)>>1;             /*BTVGA2VRS     0x10,0x11 */
  tempcx=((VGAVT-VGAVDE)>>4)+tempbx+1;  /*BTVGA2VRE     0x11  */

  tempah=tempbx&0x0FF;
  SetReg1(Part1Port,0x10,tempah);
  tempbh=(tempbx&0xFF00)>>8;
  tempah=((tempbh<<4)&0x0FF)|(tempcx&0x0F);
  SetReg1(Part1Port,0x11,tempah);

  SetRegANDOR(Part1Port,0x13,~0x03C,tempah);

  /*lines below are newly added for LVDS  */
  tempax=LCDHDES;
  tempbx=HDE;
  tempcx=HT;
  tempcx=tempcx-tempbx; /*HT-HDE  */
  /*push ax       lcdhdes  */
  tempax=tempax+tempbx; /*lcdhdee  */
  tempbx=HT;
  if(tempax>=tempbx){
    tempax=tempax-tempbx;
  }
  /*push ax   lcdhdee  */
  lcdhdee=tempax;
  tempcx=tempcx>>2;     /*temp  */
  tempcx=tempcx+tempax; /*lcdhrs  */
  if(tempcx>=tempbx){
    tempcx=tempcx-tempbx;
  }
  /* v ah,cl  */
  tempax=tempcx;
  tempax=tempax>>3; /*BPLHRS */
  tempah=tempax&0x0FF;
  SetReg1(Part1Port,0x14,tempah); /*Part1_14h  */
  tempah=tempah+2;
  tempah=tempah+0x01F;
  tempcl=tempcx&0x0FF;
  tempcl=tempcl&0x07;
  tempcl=(tempcl<<5)&0xFF; /* PHLHSKEW */
  tempah=tempah|tempcl;
  SetReg1(Part1Port,0x15,tempah); /*Part1_15h  */
  tempbx=lcdhdee;       /*lcdhdee  */
  tempcx=LCDHDES;       /*lcdhdes  */
  tempah=(tempcx&0xFF);
  tempah=tempah&0x07;   /*BPLHDESKEW  */
  SetReg1(Part1Port,0x1A,tempah); /*Part1_1Ah  */
  tempcx=tempcx>>3;     /*BPLHDES */
  tempah=(tempcx&0xFF);
  SetReg1(Part1Port,0x16,tempah); /*Part1_16h  */
  tempbx=tempbx>>3;     /*BPLHDEE  */
  tempah=tempbx&0xFF;
  SetReg1(Part1Port,0x17,tempah); /*Part1_17h  */

  tempcx=VGAVT;
  tempbx=VGAVDE;
  tempcx=tempcx-tempbx;         /* GAVT-VGAVDE  */
  tempbx=LCDVDES;               /*VGAVDES  */
  temppush1=tempbx;             /*push bx temppush1 */
  if(IF_DEF_TRUMPION==0){
    if(IF_DEF_CH7005==1) tempax=VGAVDE;
    if(VBInfo&SetCRT2ToLCD) {
      if(LCDResInfo==Panel800x600) tempax=600;
      else tempax=768;  
    }
  }
  else tempax=VGAVDE;
  tempbx=tempbx+tempax;
  tempax=VT;            /*VT  */
  if(tempbx>=VT){
    tempbx=tempbx-tempax;
  }
  temppush2=tempbx;     /*push bx  temppush2  */
  tempcx=tempcx>>1;
  tempbx=tempbx+tempcx;
  tempbx++;     /*BPLVRS  */
  if(tempbx>=tempax){
    tempbx=tempbx-tempax;
  }
  tempah=tempbx&0xFF;
  SetReg1(Part1Port,0x18,tempah); /*Part1_18h  */
  tempcx=tempcx>>3;
  tempcx=tempcx+tempbx;
  tempcx++;             /*BPLVRE  */
  tempah=tempcx&0xFF;
  tempah=tempah&0x0F;
  tempah=tempah|0x030;
  SetRegANDOR(Part1Port,0x19,~0x03F,tempah); /*Part1_19h  */
  tempbh=(tempbx&0xFF00)>>8;
  tempbh=tempbh&0x07;
  tempah=tempbh;
  tempah=(tempah<<3)&0xFF;      /*BPLDESKEW =0 */
  /*movzx  */
  tempbx=VGAVDE;
  if(tempbx!=VDE){
    tempah=tempah|0x40;
  }
  SetRegANDOR(Part1Port,0x1A,0x07,tempah); /*Part1_1Ah */
  tempecx=VGAVT;
  tempebx=VDE;
  tempeax=VGAVDE;
  tempecx=tempecx-tempeax;      /*VGAVT-VGAVDE  */
  tempeax=tempeax*64;
  templong=tempeax/tempebx;
  if(templong*tempebx<tempeax){
    templong++;
  }
  tempebx=templong;     /*BPLVCFACT  */
  if(SetFlag&EnableLVDSDDA){
    tempebx=tempebx&0x03F;
  }
  tempah=(UShort)(tempebx&0x0FF);
  SetReg1(Part1Port,0x1E,tempah); /*Part1_1Eh */
  tempbx=temppush2;     /* p bx temppush2 BPLVDEE  */
  tempcx=temppush1;     /*pop cx temppush1 NPLVDES */
  tempbh=(tempbx&0xFF00)>>8;
  tempah=tempah&0x07;
  tempah=tempbh;
  tempah=tempah<<3;
  tempch=(tempcx&0xFF00)>>8;
  tempch=tempch&0x07;
  tempah=tempah|tempch;
  SetReg1(Part1Port,0x1D,tempah); /*Part1_1Dh */
  tempah=tempbx&0xFF;
  SetReg1(Part1Port,0x1C,tempah); /*Part1_1Ch  */
  tempah=tempcx&0xFF;
  SetReg1(Part1Port,0x1B,tempah); /*Part1_1Bh  */
  
  tempecx=VGAHDE;
  tempebx=HDE;
  tempeax=tempecx;
  tempeax=tempeax<<6;
  tempeax=tempeax<<10;
  tempeax=tempeax/tempebx;
  if(tempebx==tempecx){
    tempeax=65535;
  }
  tempecx=tempeax;
  tempeax=VGAHT;
  tempeax=tempeax<<6;
  tempeax=tempeax<<10;
  tempeax=tempeax/tempecx;
  tempecx=tempecx<<16;
  tempeax=tempeax-1;
  tempax=(UShort)(tempeax&0x00FFFF);
  tempcx=tempax;
  tempah=tempcx&0x0FF;
  SetReg1(Part1Port,0x1F,tempah); /*Part1_1Fh  */
  tempbx=VDE;
  tempbx--;             /*BENPLACCEND */
  if(SetFlag&EnableLVDSDDA){
    tempbx=1;
  }
  tempah=(tempbx&0xFF00)>>8;
  tempah=(tempah<<3)&0xFF;
  tempch=(tempcx&0xFF00)>>8;
  tempch=tempch&0x07;
  tempah=tempah|tempch;
  SetReg1(Part1Port,0x20,tempah); /*Part1_20h */
  tempah=tempbx&0xFF;
  SetReg1(Part1Port,0x21,tempah); /*Part1_21h */
  tempecx=tempecx>>16;          /*BPLHCFACT  */
  temp1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag  */
  if(temp1&HalfDCLK){
    tempecx=tempecx>>1;
  }
  tempcx=(UShort)(tempecx&0x0FFFF);
  tempah=(tempcx&0xFF00)>>8;
  SetReg1(Part1Port,0x22,tempah); /*Part1_22h */
  tempah=tempcx&0x0FF;
  SetReg1(Part1Port,0x23,tempah); /*Part1_23h */
  if(IF_DEF_TRUMPION==1){
    tempal=(UShort)*((UChar *)(ROMAddr+ModeIDOffset+0x05));   /* si+St_ResInfo */
    if(ModeNo>0x13){
      SetFlag=SetFlag|ProgrammingCRT2;
      GetRatePtrCRT2(ROMAddr,ModeNo);
      tempal=*((UChar *)(ROMAddr+REFIndex+0x04));      /* di+Ext_CRT2CRTC */
      tempal=tempal&0x1F;
    }
    tempah=0x80;
    tempal=tempal*tempah;
    REFIndex= offset_Zurac; /*offset Zurac need added in rompost.asm  */
    REFIndex=REFIndex+tempal;
    SetTPData();   /*this function not implemented yet  */
  }
 
  REFIndex=OldREFIndex;         /*pop di  */

  return;
}

static void SetTPData(void)
{
  return;
}


static void SetGroup1_301(UShort BaseAddr,ULong ROMAddr,UShort ModeNo,ScrnInfoPtr pScrn)
{
  SISPtr  pSiS = SISPTR(pScrn);
  UShort  temp1,temp2,tempcl,tempch,tempbl,tempbh,tempal,tempah,tempax,tempbx;
  UShort  tempcx,OldREFIndex;
  UShort  Part1Port,resinfo,modeflag;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  OldREFIndex=REFIndex;         /* push di */

  SetCRT2Offset(Part1Port,ROMAddr);
  SetCRT2FIFO(Part1Port,ROMAddr,ModeNo,pScrn);
  SetCRT2Sync(BaseAddr,ROMAddr,ModeNo);

  GetCRT1Ptr(ROMAddr);

  temp1=(VGAHT-1)&0x0FF;            /* BTVGA2HT 0x08,0x09 */
  SetReg1(Part1Port,0x08,temp1);
  temp1=(((VGAHT-1)&0xFF00)>>8)<<4;
  SetRegANDOR(Part1Port,0x09,~0x0F0,temp1);

  temp1=(VGAHDE+12)&0x0FF;              /* BTVGA2HDEE 0x0A,0x0C */
  SetReg1(Part1Port,0x0A,temp1);

  temp1=VGAHDE+12;      /* bx  BTVGA@HRS 0x0B,0x0C */
  temp2=(VGAHT-VGAHDE)>>2;      /* cx */
  temp1=temp1+temp2;
  temp2=(temp2<<1)+temp1;
  tempcl=temp2&0x0FF;
  if(VBInfo&SetCRT2ToRAMDAC){
    tempbl=*((UChar *)(ROMAddr+REFIndex+4));    /* di+4      */
    tempbh=*((UChar *)(ROMAddr+REFIndex+14));   /* di+14     */
    temp1=((tempbh>>6)<<8)|tempbl;              /* temp1->bx */
    temp1=(temp1-1)<<3;
    tempcl=*((UChar *)(ROMAddr+REFIndex+5));    /* di+5  */
    tempch=*((UChar *)(ROMAddr+REFIndex+15));   /* di+15 */
    tempcl=tempcl&0x01F;
    tempch=(tempch&0x04)<<(6-2);
    tempcl=((tempcl|tempch)-1)<<3;
  }
  SetReg1(Part1Port,0x0B,(UShort)(temp1&0x0FF));
  tempah=(temp1&0xFF00)>>8;
  tempbh=((((VGAHDE+12)&0xFF00)>>8)<<4)&0x0FF;
  tempah=tempah|tempbh;
  SetReg1(Part1Port,0x0C,tempah);
  SetReg1(Part1Port,0x0D,tempcl);       /* BTVGA2HRE 0x0D */
  tempcx=(VGAVT-1);
  tempah=tempcx&0x0FF;
  SetReg1(Part1Port,0x0E,tempah);       /* BTVGA2TV 0x0E,0x12 */
  tempbx=VGAVDE-1;
  tempah=tempbx&0x0FF;
  SetReg1(Part1Port,0x0F,tempah);       /* BTVGA2VDEE 0x0F,0x12 */
  tempah=((tempbx&0xFF00)<<3)>>8;
  tempah=tempah|((tempcx&0xFF00)>>8);
  SetReg1(Part1Port,0x12,tempah);

  tempbx=(VGAVT+VGAVDE)>>1;             /* BTVGA2VRS     0x10,0x11 */
  tempcx=((VGAVT-VGAVDE)>>4)+tempbx+1;  /* BTVGA2VRE     0x11      */
  if(VBInfo&SetCRT2ToRAMDAC){
    tempbx=*((UChar *)(ROMAddr+REFIndex+8));   /* di+8 */
    temp1=*((UChar *)(ROMAddr+REFIndex+7));    /* di+7 */
    if(temp1&0x04){
      tempbx=tempbx|0x0100;
    }
    if(temp1&0x080){
      tempbx=tempbx|0x0200;
    }
    temp1=*((UChar *)(ROMAddr+REFIndex+13));    /* di+13 */
    if(temp1&0x08){
      tempbx=tempbx|0x0400;
    }
    tempcl= *((UChar *)(ROMAddr+REFIndex+9));   /* di+9 */
    tempcx=(tempcx&0xFF00)|(tempcl&0x00FF);
  }
  tempah=tempbx&0x0FF;
  SetReg1(Part1Port,0x10,tempah);
  tempbh=(tempbx&0xFF00)>>8;
  tempah=((tempbh<<4)&0x0FF)|(tempcx&0x0F);
  SetReg1(Part1Port,0x11,tempah);

  if( pSiS->Chipset == PCI_CHIP_SIS300 ){
    tempah=0x10;
    if((LCDResInfo!=Panel1024x768)&&(LCDResInfo==Panel1280x1024)) tempah=0x20;
  }
  else tempah=0x20;
  if(VBInfo&SetCRT2ToTV) tempah=0x08;
/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    if(VBInfo&SetInSlaveMode) tempah=0x2c;
    else tempah=0x20;
  }
/* ynlai end */
  SetRegANDOR(Part1Port,0x13,~0x03C,tempah);

  if(!(VBInfo&SetInSlaveMode)){
    REFIndex=OldREFIndex;
    return;
  }
  if(VBInfo&SetCRT2ToTV){
    tempax=0xFFFF;
  }else{
    tempax=GetVGAHT2();
  }
  tempcl=0x08;                  /* Reg 0x03 Horozontal Total */
  temp1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
  if(!(temp1&Charx8Dot)){                           /* temp1->St_ModeFlag */
    tempcl=0x09;
  }
  if(tempax>=VGAHT){
    tempax=VGAHT;
  }
  if(temp1&HalfDCLK){
    tempax=tempax>>1;
  }
  tempax=(tempax/tempcl)-5;
  tempbl=tempax;
  tempah=0xFF;          /* set MAX HT */
  SetReg1(Part1Port,0x03,tempah);

  tempax=VGAHDE;                /* 0x04 Horizontal Display End */
  if(temp1&HalfDCLK){
    tempax=tempax>>1;
  }
  tempax=(tempax/tempcl)-1;
  tempbh=tempax;
  SetReg1(Part1Port,0x04,tempax);

  tempah=tempbh;
  if(VBInfo&SetCRT2ToTV){
    tempah=tempah+2;
  }
/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    resinfo=*(UShort *)(ROMAddr+ModeIDOffset+0x09); /* si+Ext_ResInfo */
    if(resinfo==7) tempah=tempah-2;
  }
/* ynlai end */
  SetReg1(Part1Port,0x05,tempah); /* 0x05 Horizontal Display Start */
  SetReg1(Part1Port,0x06,0x03);   /* 0x06 Horizontal Blank end     */
                                  /* 0x07 horizontal Retrace Start */
/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    tempah=tempbl-1;
    modeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
    if(!(modeflag&HalfDCLK)) {
      tempah=tempah-6;
      if(SetFlag&TVSimuMode) {
        tempah=tempah-4;
        if(ModeNo>0x13) tempah=tempah-10;
      }
    }
  }
/* ynlai end */
  else {
    tempcx=(tempbl+tempbh)>>1;
    tempah=(tempcx&0xFF)+2;

    if(VBInfo&SetCRT2ToTV){
       tempah=tempah-1;
       if(!(temp1&HalfDCLK)){
          if((temp1&Charx8Dot)){
            tempah=tempah+4;
            if(VGAHDE>=800){
              tempah=tempah-6;
            }
          }
        }
    }else{
       if(!(temp1&HalfDCLK)){
         tempah=tempah-4;
         if(VGAHDE>=800){
           tempah=tempah-7;
           if(ModeType==ModeEGA){
             if(VGAVDE==1024){
               tempah=tempah+15;
               if(LCDResInfo!=Panel1280x1024){
                  tempah=tempah+7;
               }
             }
           }
           if(VGAHDE>=1280){
             tempah=tempah+28;
           }
         }
       }
    }
  }
  SetReg1(Part1Port,0x07,tempah); /* 0x07 Horizontal Retrace Start */

  SetReg1(Part1Port,0x08,0);      /* 0x08 Horizontal Retrace End   */
  SetReg1(Part1Port,0x18,0x03);   /* 0x18 SR08                     */
  SetReg1(Part1Port,0x19,0);      /* 0x19 SR0C                     */
  SetReg1(Part1Port,0x09,0xFF);   /* 0x09 Set Max VT               */

  tempcx=0x121;
  tempcl=0x21;
  tempch=0x01;
  tempbx=VGAVDE;                /* 0x0E Virtical Display End */
  if(tempbx==360) tempbx=350;
  if(tempbx==375) tempbx=350;
  if(tempbx==405) tempbx=400;
  tempbx--;
  tempah=tempbx&0x0FF;
  SetReg1(Part1Port,0x0E,tempah);
  SetReg1(Part1Port,0x10,tempah); /* 0x10 vertical Blank Start */
  tempbh=(tempbx&0xFF00)>>8;
  if(tempbh&0x01){
    tempcl=tempcl|0x0A;
  }
  tempah=0;tempal=0x0B;
  if(temp1&DoubleScanMode){
    tempah=tempah|0x080;
  }
  if(tempbh&0x02){
    tempcl=tempcl|0x040;
    tempah=tempah|0x020;
  }
  SetReg1(Part1Port,0x0B,tempah);
  if(tempbh&0x04){
    tempch=tempch|0x06;
  }

  SetReg1(Part1Port,0x11,0);  /* 0x11 Vertival Blank End */

  tempax=VGAVT-tempbx;          /* 0x0C Vertical Retrace Start */
  tempax=tempax>>2;
  temp2=tempax;         /* push ax */
  tempax=tempax<<1;
  tempbx=tempax+tempbx;
/*  ynlai begin */
/*  ynlai end   */
  if((SetFlag&TVSimuMode)&&(VBInfo&SetPALTV)&&(VGAHDE==800)){
    tempbx=tempbx+40;
  }
  tempah=(tempbx&0x0FF);
  SetReg1(Part1Port,0x0C,tempah);
  tempbh=(tempbx&0xFF00)>>8;
  if(tempbh&0x01){
    tempcl=tempcl|0x04;
  }
  if(tempbh&0x02){
    tempcl=tempcl|0x080;
  }
  if(tempbh&0x04){
    tempch=tempch|0x08;
  }

  tempax=temp2;         /* pop ax */
  tempax=(tempax>>2)+1;
  tempbx=tempbx+tempax;
  tempah=(tempbx&0x0FF)&0x0F;
  SetReg1(Part1Port,0x0D,tempah);  /* 0x0D vertical Retrace End */
  tempbl=tempbx&0x0FF;
  if(tempbl&0x10){
    tempch=tempch|0x020;
  }

  tempah=tempcl;
  SetReg1(Part1Port,0x0A,tempah); /* 0x0A CR07 */
  tempah=tempch;
  SetReg1(Part1Port,0x17,tempah); /* 0x17 SR0A */
  tempax=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
  tempah=(tempax&0xFF00)>>8;
  tempah=(tempah>>1)&0x09;
  SetReg1(Part1Port,0x16,tempah); /* 0x16 SR01 */
  SetReg1(Part1Port,0x0F,0);      /* 0x0F CR14 */
  SetReg1(Part1Port,0x12,0);      /* 0x12 CR17 */
  SetReg1(Part1Port,0x1A,0);      /* 0x1A SR0E */

  REFIndex=OldREFIndex;           /* pop di */
}

static void SetCRT2Offset(UShort Part1Port,ULong ROMAddr)
{
  UShort offset;
  if(VBInfo&SetInSlaveMode){
    return;
  }
  offset=GetOffset(ROMAddr);
  SetReg1(Part1Port,0x07,(UShort)(offset&0xFF));
  SetReg1(Part1Port,0x09,(UShort)((offset&0xFF00)>>8));
  SetReg1(Part1Port,0x03,(UShort)(((offset>>3)&0xFF)+1));
}

static UShort GetOffset(ULong ROMAddr)
{
  UShort tempal,temp1,colordepth;
  tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x03));    /* si+Ext_ModeInfo */
  tempal=(tempal>>4)&0xFF;
  ScreenOffset=*((UShort *)(ROMAddr+0x206));         /* Get ScreeOffset table */
  tempal=*((UChar *)(ROMAddr+ScreenOffset+tempal));  /* get ScreenOffset */
  tempal=tempal&0xFF;
  temp1=*((UChar *)(ROMAddr+REFIndex));              /* di+Ext_InfoFlag */
  if(temp1&InterlaceMode){
    tempal=tempal<<1;
  }
  colordepth=GetColorDepth(ROMAddr);
  return(tempal*colordepth);
}

static UShort GetColorDepth(ULong ROMAddr)
{
  UShort ColorDepth[6]={1,2,4,4,6,8};
  UShort temp;
  int temp1;
  temp=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
  temp1=(temp&ModeInfoFlag)-ModeEGA;
  if(temp1<0) temp1=0;
  return(ColorDepth[temp1]);
}

static void SetCRT2FIFO(UShort  Part1Port,ULong ROMAddr,UShort ModeNo,ScrnInfoPtr pScrn)
{
  SISPtr  pSiS = SISPTR(pScrn);
  UShort temp,temp1,temp2,temp3,flag;
  UShort vclk2ptr,latencyindex;
  UShort oldREFIndex,CRT1ModeNo,oldModeIDOffset;
  long int longtemp;

  static UShort LatencyFactor[48]={ 88, 80, 78, 72, 70, 00,        /* 64  bit    BQ=2 */
                           00, 79, 77, 71, 69, 49,          /* 64  bit    BQ=1 */
                           88, 80, 78, 72, 70, 00,          /* 128 bit    BQ=2 */
                           00, 72, 70, 64, 62, 44,          /* 128 bit    BQ=1 */
                           73, 65, 63, 57, 55, 00,          /* 64  bit    BQ=2 */
                           00, 64, 62, 56, 54, 34,          /* 64  bit    BQ=1 */
                           78, 70, 68, 62, 60, 00,          /* 128 bit    BQ=2 */
                           00, 62, 60, 54, 52, 34};         /* 128 bit    BQ=1 */

  oldREFIndex=REFIndex;         /* push REFIndex(CRT2 now) */
  oldModeIDOffset=ModeIDOffset; /* push ModeIDOffset       */

  CRT1ModeNo=(UChar)GetReg1(P3d4,0x34); /* get CRT1 ModeNo  */
  SearchModeID(ROMAddr,CRT1ModeNo);     /* Get ModeID Table */

  GetRatePtr(ROMAddr,CRT1ModeNo); /* Set REFIndex-> for crt1 refreshrate */
  temp1=GetVCLK(ROMAddr,CRT1ModeNo);
  temp2=GetColorTh(ROMAddr);
  temp3=GetMCLK(ROMAddr);
  temp=((UShort)(temp1*temp2)/temp3);   /* temp->bx */
  temp1=(UChar)GetReg1(P3c4,0x14);      /* SR_14    */
  temp1=temp1>>6;
  temp1=temp1<<1;
  if(temp1==0) temp1=1;
  temp1=temp1<<2;               /* temp1->ax */

  longtemp=temp1-temp;

  temp2=(UShort)((28*16)/(int)longtemp);   /* temp2->cx */
  if(!((temp2*(int)longtemp)==(28*16))) temp2++;

  if( pSiS->Chipset == PCI_CHIP_SIS300 ){
    temp1=CalcDelayVB();
  }else{ /* for Trojan and Spartan */
    flag=(UChar)GetReg1(P3c4,0x14);   /* SR_14 */
    if(flag&0x80){
      latencyindex=12;  /* 128 bit */
    }else{
      latencyindex=0;   /* 64 bit */
    }
    flag=GetQueueConfig();
    if(!(flag&0x01)){
      latencyindex+=24; /* GUI timing =0 */
    }
    if(flag&0x10){
      latencyindex+=6;  /* BQ =2 */
    }
    latencyindex=latencyindex + (flag>>5);
    temp1= LatencyFactor[latencyindex];
    temp1=temp1+15;
    flag=(UChar)GetReg1(P3c4,0x14);   /* SR_14 */
    if(!(flag&0x80)){
      temp1=temp1+5;    /* 64 bit */
    }
  }

  temp2=temp2+temp1;
  REFIndex=oldREFIndex;         /* pop REFIndex(CRT2) */
  ModeIDOffset=oldModeIDOffset; /* pop ModeIDOffset */

  vclk2ptr=GetVCLK2Ptr(ROMAddr,ModeNo);
  temp1=*((UShort *)(ROMAddr+vclk2ptr+(VCLKLen-2)));
  temp3=GetColorTh(ROMAddr);
  longtemp=temp1*temp2*temp3;
  temp3=GetMCLK(ROMAddr);
  temp3=temp3<<4;
  temp2=(int)(longtemp/temp3);
  if((long int)temp2*(long int)temp3<(long int)longtemp)
      temp2++;                /* temp2->cx */

  temp1=(UChar)GetReg1(Part1Port,0x01);         /* part1port index 01 */
  temp1=(temp1&(~0x1F))|0x16;
  SetReg1(Part1Port,0x01,temp1);

/* ynlai begin */
  if(IF_DEF_HiVision==1) { if(temp2<=10) temp2=10; }
  else { if(temp2<=6) temp2=6; }
/* ynlai end */
  if(temp2>0x14) temp2=0x14;
  temp1=(UChar)GetReg1(Part1Port,0x02);         /* part1port index 02 */
  temp1=(temp1&(~0x1F))|temp2;
  SetReg1(Part1Port,0x02,temp1);
}

static UShort GetVCLK(ULong ROMAddr,UShort ModeNo)
{
  UShort tempptr;
  UShort temp1;
  tempptr=GetVCLKPtr(ROMAddr,ModeNo);
  temp1=*((UShort *)(ROMAddr+tempptr+(VCLKLen-2)));
  return temp1;
}

static UShort GetQueueConfig(void)
{
  UShort tempal,tempbl;
  ULong tempeax;

  SetReg4(0xcf8,0x80000050);
  tempeax=GetReg3(0xcfc);
  tempeax=(tempeax>>24)&0x0f;
  tempbl=(UShort)tempeax;
  tempbl=tempbl<<4;

  SetReg4(0xcf8,0x800000A0);
  tempeax=GetReg3(0xcfc);
  tempeax=(tempeax>>24)&0x0f;
  tempal=(UShort)tempeax;
  tempbl=tempbl|tempal;

  return(tempbl);
}

static UShort GetVCLKPtr(ULong ROMAddr,UShort ModeNo)
{
  UShort tempal=0;
  if(IF_DEF_LVDS==0) {
    tempal=(UChar)GetReg2((UShort)(P3ca+0x02)); /*  Port 3cch */
    tempal=((tempal>>2)&0x03);
    if(ModeNo>0x13){
      tempal=*((UChar *)(ROMAddr+REFIndex+0x03));    /* di+Ext_CRTVCLK */
    }
    VCLKLen=GetVCLKLen(ROMAddr);
    tempal=tempal*VCLKLen;
    tempal=tempal+(*((UShort *)(ROMAddr+0x208)));  /* VCLKData */
    return ((UShort)tempal);
  } else {
    if(LCDResInfo==Panel800x600) {
        tempal=VCLK40;
    } else if(LCDResInfo==Panel1024x768) {
                tempal=VCLK65;
           }
   VCLKLen=GetVCLKLen(ROMAddr);
   tempal=tempal*VCLKLen;
   tempal=tempal+(*((UShort *)(ROMAddr+0x208)));
   return((UShort)tempal);
  }
}

static UShort GetColorTh(ULong ROMAddr)
{
  UShort temp;
  temp=GetColorDepth(ROMAddr);
  temp=temp>>1;
  if(temp==0) temp++;
  return temp;
}

static UShort GetMCLK(ULong ROMAddr)
{
  UShort tempmclkptr;
  UShort tempmclk;
  tempmclkptr=GetMCLKPtr(ROMAddr);
  tempmclk=*((UShort *)(ROMAddr+tempmclkptr+0x03));        /* di+3 */
  return tempmclk;
}

static UShort GetMCLKPtr(ULong ROMAddr)
{
  UShort tempdi;
  UShort tempdramtype,tempax;

  tempdi=*((UShort *)(ROMAddr+0x20C));        /* MCLKData */
  tempdramtype=GetDRAMType(ROMAddr);
  tempax=5*tempdramtype;
  tempdi=tempdi+tempax;
  return (tempdi);
}

static UShort GetDRAMType(ULong ROMAddr)
{
 UShort tsoftsetting,temp3;
 tsoftsetting=*((UChar *)(ROMAddr+0x52));
 if(!(tsoftsetting&SoftDramType)){
   temp3=(UChar)GetReg1(P3c4,0x3A);
   tsoftsetting=temp3;
 }
 tsoftsetting=tsoftsetting&0x07;
 return(tsoftsetting);
}

static UShort CalcDelayVB(void)
{
  UShort tempal,tempah,temp1,tempbx;
  UShort ThTiming[8]={1,2,2,3,0,1,1,2};
  UShort ThLowB[24]={81,4,72,6,88,8,120,12,
                  55,4,54,6,66,8,90,12,
                  42,4,45,6,55,8,75,12};

  tempah=(UChar)GetReg1(P3c4,0x18);     /* SR_18 */
  tempah=tempah&0x62;
  tempah=tempah>>1;
  tempal=tempah;
  tempah=tempah>>3;
  tempal=tempal|tempah;
  tempal=tempal&0x07;

  temp1=ThTiming[tempal];       /* temp1->cl */

  tempbx=(UChar)GetReg1(P3c4,0x16);     /* SR_16 */
  tempbx=tempbx>>6;
  tempah=(UChar)GetReg1(P3c4,0x14);     /* SR_14 */
  tempah=((tempah>>4)&0x0C);
  tempbx=((tempbx|tempah)<<1);

  tempal=ThLowB[tempbx+1]*temp1;
  tempbx=ThLowB[tempbx];
  tempbx=tempal+tempbx;

  return(tempbx);
}

static UShort GetVCLK2Ptr(ULong ROMAddr,UShort ModeNo)
{
  UShort tempal,tempbx,temp;
  UShort LCDXlat1VCLK[4]={VCLK65,VCLK65,VCLK65,VCLK65};
  UShort LCDXlat2VCLK[4]={VCLK108_2,VCLK108_2,VCLK108_2,VCLK108_2};
  UShort LVDSXlat1VCLK[4]={VCLK40,VCLK40,VCLK40,VCLK40};
  UShort LVDSXlat2VCLK[4]={VCLK65,VCLK65,VCLK65,VCLK65};
  UShort LVDSXlat3VCLK[4]={VCLK65,VCLK65,VCLK65,VCLK65};

  if(IF_DEF_LVDS==0) {
    if(ModeNo<=0x13){
      tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));  /* si+St_CRT2CRTC */
    }else{
      tempal=*((UChar *)(ROMAddr+REFIndex+0x04));      /* di+Ext_CRT2CRTC */
    }
    tempal=tempal>>6;
    if(LCDResInfo!=Panel1024x768){
      tempal=LCDXlat2VCLK[tempal];
    }else{
      tempal=LCDXlat1VCLK[tempal];
    }

    if(VBInfo&SetCRT2ToLCD){
      tempal=tempal;
    }
/* ynlai begin */
    else {        /* for TV */
      if(VBInfo&SetCRT2ToTV) {
        if(IF_DEF_HiVision==1) {
          if(SetFlag&RPLLDIV2XO) tempal=HiTVVCLKDIV2;
          else tempal=HiTVVCLK;
          if(SetFlag&TVSimuMode){
            temp=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
            if(temp&Charx8Dot) tempal=HiTVSimuVCLK;
            else tempal=HiTVTextVCLK;
          }
        }
        else {
          if(VBInfo&SetCRT2ToTV){
            if(SetFlag&RPLLDIV2XO) tempal=TVVCLKDIV2;
            else tempal=TVVCLK;
          }
          else {
            tempal=(UChar)GetReg2((UShort)(P3ca+0x02));      /*  Port 3cch */
            tempal=((tempal>>2)&0x03);
            if(ModeNo>0x13) tempal=*((UChar *)(ROMAddr+REFIndex+0x03));    /* di+Ext_CRTVCLK */
          }
        }
      }
    }
  } 
/* ynlai end */
  else {       /*   LVDS  */
    if(ModeNo<=0x13) tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));
    else tempal=*((UChar *)(ROMAddr+REFIndex+0x04));
    if(IF_DEF_CH7005==1) {
      if(!(VBInfo&SetCRT2ToLCD)) {
        tempal=tempal&0x1f;
        tempbx=0; 
        if(VBInfo&SetPALTV) tempbx=tempbx+2;
        if(VBInfo&SetCHTVOverScan) tempbx=tempbx+1;
        tempbx=tempbx<<1;
        temp=(*((UShort *)(ROMAddr+ADR_CHTVVCLKPtr)));
        tempbx=(*((UShort *)(ROMAddr+temp+tempbx)));
        tempal=(*((UShort *)(ROMAddr+tempbx+tempal)));
        tempal=tempal&0x00FF;
      }
    }
    else {
      tempal=tempal>>6;
      if(LCDResInfo==Panel800x600) tempal=LVDSXlat1VCLK[tempal];
      else if(LCDResInfo==Panel1024x768) tempal=LVDSXlat2VCLK[tempal];
      else tempal=LVDSXlat3VCLK[tempal];
    }
  }
  VCLKLen=GetVCLKLen(ROMAddr);
  tempal=tempal*VCLKLen;
  tempal=tempal+(*((UShort *)(ROMAddr+0x208)));      /* VCLKData */
  return ((UShort)tempal);
}

static UShort GetVCLKLen(ULong ROMAddr)
{
  UShort VCLKDataStart,vclklabel,temp;
  VCLKDataStart=*((UShort *)(ROMAddr+0x208));
  for(temp=0;;temp++){
     vclklabel=*((UShort *)(ROMAddr+VCLKDataStart+temp));
     if(vclklabel==VCLKStartFreq){
       temp=temp+2;
       return(temp);
     }
  }
  return(0);
}


static void SetCRT2Sync(UShort BaseAddr,ULong ROMAddr,UShort ModeNo)
{
  UShort temp1,tempah=0;
  UShort temp;
  UShort  Part1Port;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  if(IF_DEF_LVDS==1){
    if(VBInfo&SetCRT2ToLCD){
      tempah=LCDInfo;
      if(!(tempah&LCDSync)){
        temp=*((UShort *)(ROMAddr+REFIndex));    /*di+Ext_InfoFlag */
        tempah=(temp>>8)&0x0C0;
      }else{
        tempah=tempah&0x0C0;
      }
    }
  }
  else {
  temp=*((UShort *)(ROMAddr+REFIndex));    /* di+Ext_InfoFlag */
  tempah=(temp>>8)&0x0C0;
  }
  temp1=(UChar)GetReg1(Part1Port,0x19);    /* part1port index 02 */
  temp1=(temp1&(~0x0C0))|tempah;
  SetReg1(Part1Port,0x19,temp1);
}

static void GetCRT1Ptr(ULong ROMAddr)
{
  UShort temprefcrt1;
  UShort temp;
  temp=*((UChar *)(ROMAddr+REFIndex+0x02));    /* di+Ext_CRT1CRTC */
  temp=temp*CRT1Len;
  temprefcrt1=*((UShort *)(ROMAddr+0x204));    /* Get CRT1Table */
  REFIndex=temprefcrt1+temp;         /* di->CRT1Table+Ext_CRT1CRTC*CRT1Len */
}

static void SetRegANDOR(UShort Port,UShort Index,UShort DataAND,UShort DataOR)
{
  UShort temp1;
  temp1=GetReg1(Port,Index);     /* part1port index 02 */
  temp1=(temp1&(DataAND))|DataOR;
  SetReg1(Port,Index,temp1);
}

static UShort GetVGAHT2(void)
{
  long int temp1,temp2;
  temp1=(VGAVT-VGAVDE)*RVBHCMAX;
  temp1=temp1&0x0FFFF;
  temp2=(VT-VDE)*RVBHCFACT;
  temp2=temp2&0x0FFFF;
  temp2=temp2*HT;
  temp2=temp2/temp1;
  return((UShort)temp2);
}

static void SetGroup2(UShort  BaseAddr,ULong ROMAddr, UShort ModeNo)
{
  UShort tempah,tempbl,tempbh,tempcl,i,j,tempcx,pushcx,tempbx,tempax;
  UShort tempmodeflag,tempflowflag;
  UChar *temp1;
  UShort *temp2;
  UShort pushbx;
  UShort  Part2Port;
  UShort  modeflag;
  long int longtemp;

  Part2Port=BaseAddr+IND_SIS_CRT2_PORT_10;
  tempcx=VBInfo;
  tempah=VBInfo&0x0FF;
  tempbl=VBInfo&0x0FF;
  tempbh=VBInfo&0x0FF;
  tempbx=(tempbl&0xFF)|(tempbh<<8);
  tempbl=tempbl&0x10;
  tempbh=(tempbh&0x04)<<1;
  tempah=(tempah&0x08)>>1;
  tempah=tempah|tempbh;
  tempbl=tempbl>>3;
  tempah=tempah|tempbl;
  tempah=tempah^0x0C;

  if(IF_DEF_HiVision==1) {
    temp1=(UChar *)(ROMAddr+0x0F1);     /* PALPhase */
    tempah=tempah^0x01;
    if(VBInfo&SetInSlaveMode) {
      temp2=HiTVSt2Timing;
      if(SetFlag&TVSimuMode) {
        modeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   
        if(modeflag&Charx8Dot) temp2=HiTVSt1Timing;
        else temp2=HiTVTextTiming;
      } 
    }
    else  temp2=HiTVExtTiming;
  }
  else {
    if(VBInfo&SetPALTV){
      temp1=(UChar *)(ROMAddr+0x0F1);     /* PALPhase */
      temp2=PALTiming;
    }else{
      tempah=tempah|0x10;
      temp1=(UChar *)(ROMAddr+0x0ED);     /* NTSCPhase */
      temp2=NTSCTiming;
    }
  }
  SetReg1(Part2Port,0x0,tempah);
  for(i=0x31;i<=0x34;i++,temp1++){
     SetReg1(Part2Port,i,*(UChar *)temp1);
  }
  for(i=0x01,j=0;i<=0x2D;i++,j++){
     SetReg1(Part2Port,i,temp2[j]);
  }
  for(i=0x39;i<=0x45;i++,j++){
     SetReg1(Part2Port,i,temp2[j]);   /* di->temp2[j] */
  }

  tempah=GetReg1(Part2Port,0x0A);
  tempah=tempah|NewFlickerMode;
  SetReg1(Part2Port,0x0A,tempah);

  SetReg1(Part2Port,0x35,RY1COE);
  SetReg1(Part2Port,0x36,RY2COE);
  SetReg1(Part2Port,0x37,RY3COE);
  SetReg1(Part2Port,0x38,RY4COE);

/* ynlai begin */
  if(IF_DEF_HiVision==1) tempax=950;
  else {
    if(VBInfo&SetPALTV) tempax=520;
    else tempax=440;
  }
  if(VDE<=tempax) {
    tempax=tempax-VDE;
    tempax=tempax>>2;
    tempah=(tempax&0xFF00)>>8;
    tempah=tempah+temp2[0];
    SetReg1(Part2Port,0x01,tempah);
    tempah=tempax&0x00FF;
    tempah=tempah+temp2[1];
    SetReg1(Part2Port,0x02,tempah);
  }
/* begin end */

  tempcx=HT-1;
  tempah=tempcx&0xFF;
  SetReg1(Part2Port,0x1B,tempah);
  tempah=(tempcx&0xFF00)>>8;
  SetRegANDOR(Part2Port,0x1D,~0x0F,(UChar)tempah);

  tempcx=HT>>1;
  pushcx=tempcx; /* push cx */

  tempcx=tempcx+7;
/* ynlai begin */
  if(IF_DEF_HiVision==1) tempcx=tempcx-4;
/* ynlai end   */
  tempah=(tempcx&0xFF);
  tempah=(tempah<<4)&0xFF;
  SetRegANDOR(Part2Port,0x22,~0x0F0,tempah);


  tempbx=temp2[j];
  tempbx=tempbx+tempcx;
  tempah=tempbx&0xFF;
  SetReg1(Part2Port,0x24,tempah);
  tempah=(tempbx&0xFF00)>>8;
  tempah=(tempah<<4)&0xFF;
  SetRegANDOR(Part2Port,0x25,~0x0F0,tempah);

  tempbx=tempbx+8;
/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    tempbx=tempbx-4;
    tempcx=tempbx;
  }
/* ynlai end */
  tempah=((tempbx&0xFF)<<4)&0xFF;
  SetRegANDOR(Part2Port,0x29,~0x0F0,tempah);

  tempcx=tempcx+temp2[++j];
  tempah=tempcx&0xFF;
  SetReg1(Part2Port,0x27,tempah);
  tempah=(((tempcx&0xFF00)>>8)<<4)&0xFF;
  SetRegANDOR(Part2Port,0x28,~0x0F0,tempah);

  tempcx=tempcx+8;
/* ynlai begin */
  if(IF_DEF_HiVision==1) tempcx=tempcx-4;
/* ynlai end   */
  tempah=tempcx&0xFF;
  tempah=(tempah<<4)&0xFF;
  SetRegANDOR(Part2Port,0x2A,~0x0F0,tempah);

  tempcx=pushcx;  /* pop cx */
  tempcx=tempcx-temp2[++j];
  tempah=tempcx&0xFF;
  tempah=(tempah<<4)&0xFF;
  SetRegANDOR(Part2Port,0x2D,~0x0F0,tempah);

  tempcx=tempcx-11;
  if(!(VBInfo&SetCRT2ToTV)){
    tempax=GetVGAHT2();
    tempcx=tempax-1;
  }
  tempah=tempcx&0xFF;
  SetReg1(Part2Port,0x2E,tempah);

  tempbx=VDE;
  if(VGAVDE==360){
    tempbx=746;
  }
  if(VGAVDE==375){
    tempbx=746;
  }
  if(VGAVDE==405){
    tempbx=853;
  }
  /* assuming <<ifndef>> HivisionTV */
  if((VBInfo&SetCRT2ToTV)){
    tempbx=tempbx>>1;
  }

  tempbx=tempbx-2;
  tempah=tempbx&0xFF;
/* ynlai begin */
  if(IF_DEF_HiVision==1)
    if(VBInfo&SetInSlaveMode)
      if(ModeNo==0x2f) tempah=tempah+1;
/* ynlai end   */
  SetReg1(Part2Port,0x2F,tempah);

  tempah=(tempcx&0xFF00)>>8;
  tempbh=(tempbx&0xFF00)>>8;
  tempbh=(tempbh<<6)&0xFF;
  tempah=tempah|tempbh;
  /* assuming <<ifndef>> hivisiontv */
/* ynlai begin */
  if(IF_DEF_HiVision==0) {
    tempah=tempah|0x10;
    if(!(VBInfo&SetCRT2ToSVIDEO)){
      tempah=tempah|0x20;
    }
  }
/* ynlai end   */
  SetReg1(Part2Port,0x30,tempah);

  tempbh=0;
  tempbx=tempbx&0xFF;

  tempmodeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
  tempflowflag=0;
  if(!(tempmodeflag&HalfDCLK)){
    tempcx=VGAHDE;
    if(tempcx>=HDE){
      tempbh=tempbh|0x20;
      tempbx=(tempbh<<8)|(tempbx&0xFF);
      tempah=0;
    }
  }
  tempcx=0x0101;
/* ynlai begin */
  if(IF_DEF_HiVision==1) {
     if(VGAHDE>=1024) {
       tempcx=0x1920;
       if(VGAHDE>=1280) tempcx=0x1420;
     }
  }
/* ynlai end   */
  if(!(tempbh&0x20)){
    if(tempmodeflag&HalfDCLK){
      tempcl=((tempcx&0xFF)<<1)&0xFF;
      tempcx=(tempcx&0xFF00)|tempcl;
    }
    pushbx=tempbx;
    tempax=VGAHDE;
    tempbx=(tempcx&0xFF00)>>8;
    longtemp=tempax*tempbx;
    tempcx=tempcx&0xFF;
    longtemp=longtemp/tempcx;
    longtemp=longtemp*8*1024;
    tempax=(longtemp)/HDE;
    if(tempax*HDE<longtemp){
      tempax=tempax+1;
    }else{
      tempax=tempax;
    }
    tempbx=pushbx;
    tempah=((tempax&0xFF00)>>8)&0x01F;
    tempbh=tempbh|tempah;
    tempah=tempax&0xFF;
  }

  SetReg1(Part2Port,0x44,tempah);
  tempah=tempbh;
  SetRegANDOR(Part2Port,0x45,~0x03F,tempah);

  if(IF_DEF_HiVision==1) {
    if(!(VBInfo&SetInSlaveMode)) {
      SetReg1(Part2Port,0x0B,0x00);
    }
  }

  if(VBInfo&SetCRT2ToTV){
    return;
  }
  tempah=0x01;
  if(LCDResInfo==Panel1280x1024){
    if(ModeType==ModeEGA){
      if(VGAHDE>=1024){
        tempah=0x02;
      }
    }
  }
  SetReg1(Part2Port,0x0B,tempah);

  tempbx=HDE-1;         /* RHACTE=HDE-1 */
  tempah=tempbx&0xFF;
  SetReg1(Part2Port,0x2C,tempah);
  tempah=(tempbx&0xFF00)>>8;
  tempah=(tempah<<4)&0xFF;
  SetRegANDOR(Part2Port,0x2B,~0x0F0,tempah);

  tempbx=VDE-1;         /* RTVACTEO=(VDE-1)&0xFF */
  tempah=tempbx&0xFF;
  SetReg1(Part2Port,0x03,tempah);
  tempah=((tempbx&0xFF00)>>8)&0x07;
  SetRegANDOR(Part2Port,0x0C,~0x07,tempah);

  tempcx=VT-1;
  tempah=tempcx&0xFF;   /* RVTVT=VT-1 */
  SetReg1(Part2Port,0x19,tempah);
  tempah=(tempcx&0xFF00)>>8;
  tempah=(tempah<<5)&0xFF;
  if(LCDInfo&LCDRGB18Bit){
    tempah=tempah|0x10;
  }
  SetReg1(Part2Port,0x1A,tempah);

  tempcx++;
  if(LCDResInfo==Panel1024x768){
    tempbx=768;
  }else{
    tempbx=1024;
  }

  if(tempbx==VDE){
    tempax=1;
  }else{
    tempax=tempbx;
    tempax=(tempax-VDE)>>1;
  }
  tempcx=tempcx-tempax; /* lcdvdes */
  tempbx=tempbx-tempax; /* lcdvdee */

  tempah=tempcx&0xFF;   /* RVEQ1EQ=lcdvdes */
  SetReg1(Part2Port,0x05,tempah);
  tempah=tempbx&0xFF;   /* RVEQ2EQ=lcdvdee */
  SetReg1(Part2Port,0x06,tempah);

  tempah=(tempbx&0xFF00)>>8;
  tempah=(tempah<<3)&0xFF;
  tempah=tempah|((tempcx&0xFF00)>>8);
  SetReg1(Part2Port,0x02,tempah);

  tempcx=(VT-VDE)>>4;   /* (VT-VDE)>>4 */
  tempbx=(VT+VDE)>>1;
  tempah=tempbx&0xFF;   /* RTVACTEE=lcdvrs */
  SetReg1(Part2Port,0x04,tempah);

 tempah=(tempbx&0xFF00)>>8;
 tempah=(tempah<<4)&0xFF;
 tempbx=tempbx+tempcx+1;
 tempbl=(tempbx&0x0F);
 tempah=tempah|tempbl;  /* RTVACTSO=lcdvrs&0x700>>4+lcdvre */
 SetReg1(Part2Port,0x01,tempah);

 tempah=GetReg1(Part2Port,0x09);
 tempah=tempah&0xF0;
 SetReg1(Part2Port,0x09,tempah);

 tempah=GetReg1(Part2Port,0x0A);
 tempah=tempah&0xF0;
 SetReg1(Part2Port,0x0A,tempah);

 tempcx=(HT-HDE)>>2;    /* (HT-HDE)>>2     */
 tempbx=(HDE+7);        /* lcdhdee         */
 tempah=tempbx&0xFF;    /* RHEQPLE=lcdhdee */
 SetReg1(Part2Port,0x23,tempah);
 tempah=(tempbx&0xFF00)>>8;
 SetRegANDOR(Part2Port,0x25,~0x0F,tempah);

 SetReg1(Part2Port,0x1F,0x07);  /* RHBLKE=lcdhdes */
 tempah=GetReg1(Part2Port,0x20);
 tempah=tempah&0x0F;
 SetReg1(Part2Port,0x20,tempah);

 tempbx=tempbx+tempcx;
 tempah=tempbx&0xFF;            /* RHBURSTS=lcdhrs */
 SetReg1(Part2Port,0x1C,tempah);
 tempah=(tempbx&0xFF00)>>8;
 tempah=(tempah<<4)&0xFF;
 SetRegANDOR(Part2Port,0x1D,~0x0F0,tempah);

 tempbx=tempbx+tempcx;
 tempah=tempbx&0xFF;            /* RHSYEXP2S=lcdhre */
 SetReg1(Part2Port,0x21,tempah);

 tempah=GetReg1(Part2Port,0x17);
 tempah=tempah&0xFB;
 SetReg1(Part2Port,0x17,tempah);

 tempah=GetReg1(Part2Port,0x18);
 tempah=tempah&0xDF;
 SetReg1(Part2Port,0x18,tempah);
 return;
}

static void SetGroup3(UShort  BaseAddr,ULong ROMAddr)
{
  UShort i;
  UShort *tempdi;
  UShort  Part3Port;
  UShort  modeflag;
  Part3Port=BaseAddr+IND_SIS_CRT2_PORT_12;
/* ynlai begin */
  SetReg1(Part3Port,0x00,0x00);
  if(VBInfo&SetPALTV){
    SetReg1(Part3Port,0x13,0xFA);
    SetReg1(Part3Port,0x14,0xC8);
  }
  else {
    SetReg1(Part3Port,0x13,0xF6);
    SetReg1(Part3Port,0x14,0xBF);
  }
  if(IF_DEF_HiVision==1) {
    tempdi=HiTVGroup3Data;
    if(SetFlag&TVSimuMode) {
      tempdi=HiTVGroup3Simu;
      modeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   
      if(!(modeflag&Charx8Dot)) {
        tempdi=HiTVGroup3Text;
      }
    }
    for(i=0;i<=0x3E;i++){
       SetReg1(Part3Port,i,tempdi[i]);
    }
  }
/* ynlai end   */
  return;
}

static void SetGroup4(UShort  BaseAddr,ULong ROMAddr,UShort ModeNo)
{
  UShort  Part4Port;
  UShort tempax,tempah,tempcx,tempbx,tempbh,tempch,tempmodeflag;
  long int tempebx,tempeax,templong;
  Part4Port=BaseAddr+IND_SIS_CRT2_PORT_14;

  tempax=0x0c;
  if(VBInfo&SetCRT2ToTV){
    if(VBInfo&SetInSlaveMode){
      if(!(SetFlag&TVSimuMode)){
        SetFlag=SetFlag|RPLLDIV2XO;
        tempax=tempax|0x04000;
      }
    }else{
      SetFlag=SetFlag|RPLLDIV2XO;
      tempax=tempax|0x04000;
    }
  }

  if(LCDResInfo!=Panel1024x768){
    tempax=tempax|0x08000;
  }
  tempah=(tempax&0xFF00)>>8;
  SetReg1(Part4Port,0x0C,tempah);

  tempah=RVBHCFACT;
  SetReg1(Part4Port,0x13,tempah);

  tempbx=RVBHCMAX;
  tempah=tempbx&0xFF;
  SetReg1(Part4Port,0x14,tempah);
  tempbh=(((tempbx&0xFF00)>>8)<<7)&0xFF;

  tempcx=VGAHT-1;
  tempah=tempcx&0xFF;
  SetReg1(Part4Port,0x16,tempah);
  tempch=(((tempcx&0xFF00)>>8)<<3)&0xFF;
  tempbh=tempbh|tempch;

  tempcx=VGAVT-1;
  if(!(VBInfo&SetCRT2ToTV)){
    tempcx=tempcx-5;
  }
  tempah=tempcx&0xFF;
  SetReg1(Part4Port,0x17,tempah);
  tempbh=tempbh|((tempcx&0xFF00)>>8);
  tempah=tempbh;
  SetReg1(Part4Port,0x15,tempah);

  tempcx=VBInfo;
  tempbx=VGAHDE;
  tempmodeflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
  if(tempmodeflag&HalfDCLK){
    tempbx=tempbx>>1;
  }

/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    tempah=0xA0;
    if(tempbx!=1024) {
       tempah=0xC0;
       if(tempbx!=1280) tempah=0;
    }
  }
  else {
    if(VBInfo&SetCRT2ToLCD){
      tempah=0;
      if(tempbx>800){
        tempah=0x60;
      }
    }else{
      tempah=0x080;
    }
  }
/* ynlai end   */
  if(LCDResInfo!=Panel1280x1024){
    tempah=tempah|0x0A;
  }

  SetRegANDOR(Part4Port,0x0E,~0xEF,tempah);

  tempebx=VDE;

/* ynlai begin */
  if(IF_DEF_HiVision==1) {
    if(!(tempah&0xE0)) tempbx=tempbx>>1;
  }
/* ynlai end   */

  tempcx=RVBHRS;
  tempah=tempcx&0xFF;
  SetReg1(Part4Port,0x18,tempah);

  tempeax=VGAVDE;
  tempcx=tempcx|0x04000;
  tempeax=tempeax-tempebx;
  if(tempeax<0){
    tempcx=tempcx^(0x04000);
    tempeax=VGAVDE;
  }

  templong=(tempeax*256*1024)/tempebx;
  if(tempeax*256*1024-templong*tempebx>0){
    tempebx=templong+1;
  }else{
    tempebx=templong;
  }


  tempah=tempebx&0xFF;
  SetReg1(Part4Port,0x1B,tempah);
  tempah=(tempebx&0xFF00)>>8;
  SetReg1(Part4Port,0x1A,tempah);
  tempebx=tempebx>>16;
  tempah=tempebx&0xFF;
  tempah=(tempah<<4)&0xFF;
  tempah=tempah|((tempcx&0xFF00)>>8);
  SetReg1(Part4Port,0x19,tempah);

  SetCRT2VCLK(BaseAddr,ROMAddr,ModeNo); 
}

static void SetCRT2VCLK(UShort BaseAddr,ULong ROMAddr,UShort ModeNo)
{
  UShort vclk2ptr;
  UShort tempah,temp1;
  UShort  Part4Port;

  Part4Port=BaseAddr+IND_SIS_CRT2_PORT_14;
  vclk2ptr=GetVCLK2Ptr(ROMAddr,ModeNo);
  SetReg1(Part4Port,0x0A,0x01);
  tempah=*((UChar *)(ROMAddr+vclk2ptr+0x01));   /* di+1 */
  SetReg1(Part4Port,0x0B,tempah);
  tempah=*((UChar *)(ROMAddr+vclk2ptr+0x00));   /* di */
  SetReg1(Part4Port,0x0A,tempah);
  SetReg1(Part4Port,0x12,0x00);
  tempah=0x08;
  if(VBInfo&SetCRT2ToRAMDAC){
    tempah=tempah|0x020;
  }
  temp1=GetReg1(Part4Port,0x12);
  tempah=tempah|temp1;
  SetReg1(Part4Port,0x12,tempah);
}

static void SetGroup5(UShort  BaseAddr,ULong ROMAddr)
{
  UShort  Part5Port;
  UShort Pindex,Pdata;
  Part5Port=BaseAddr+IND_SIS_CRT2_PORT_14+2;
  Pindex=Part5Port;
  Pdata=Part5Port+1;
  if(ModeType==ModeVGA){
    if(!(VBInfo&(SetInSlaveMode|LoadDACFlag|CRT2DisplayFlag))){
      EnableCRT2();
      LoadDAC2(ROMAddr,Part5Port);
    }
  }
  return;
}

static void EnableCRT2(void)
{
  UShort temp1;
  temp1=GetReg1(P3c4,0x1E);
  temp1=temp1|0x20;
  SetReg1(P3c4,0x1E,temp1);     /* SR 1E */
}

static void LoadDAC2(ULong ROMAddr,UShort Part5Port)
{
   UShort data,data2;
   UShort time,i,j,k;
   UShort m,n,o;
   UShort si,di,bx,dl;
   UShort al,ah,dh;
   UShort *table=0;
   UShort Pindex,Pdata;
   Pindex=Part5Port;
   Pdata=Part5Port+1;
   data=*((UShort *)(ROMAddr+ModeIDOffset+0x01));
   data=data&DACInfoFlag;
   time=64;
   if(data==0x00) table=MDA_DAC;
   if(data==0x08) table=CGA_DAC;
   if(data==0x10) table=EGA_DAC;
   if(data==0x18) {
     time=256;
     table=VGA_DAC;
   }
   if(time==256) j=16;
   else j=time;

   SetReg3(Pindex,0x00);

   for(i=0;i<j;i++) {
      data=table[i];
      for(k=0;k<3;k++) {
        data2=0;
        if(data&0x01) data2=0x2A;
        if(data&0x02) data2=data2+0x15;
        SetReg3(Pdata,data2);
        data=data>>2;
      }
   }

   if(time==256) {
      for(i=16;i<32;i++) {
         data=table[i];
         for(k=0;k<3;k++) SetReg3(Pdata,data);
      }
      si=32;
      for(m=0;m<9;m++) {
         di=si;
         bx=si+0x04;
         dl=0;
         for(n=0;n<3;n++) {
            for(o=0;o<5;o++) {
              dh=table[si];
              ah=table[di];
              al=table[bx];
              si++;
              WriteDAC2(Pdata,dl,ah,al,dh);
            }         /* for 5 */
            si=si-2;
            for(o=0;o<3;o++) {
              dh=table[bx];
              ah=table[di];
              al=table[si];
              si--;
              WriteDAC2(Pdata,dl,ah,al,dh);
            }         /* for 3 */
            dl++;
         }            /* for 3 */
         si=si+5;
      }               /* for 9 */
   }
}

static void WriteDAC2(UShort Pdata,UShort dl, UShort ah, UShort al, UShort dh)
{
  UShort temp;
  UShort bh,bl;

  bh=ah;
  bl=al;
  if(dl!=0) {
    temp=bh;
    bh=dh;
    dh=temp;
    if(dl==1) {
       temp=bl;
       bl=dh;
       dh=temp;
    }
    else {
       temp=bl;
       bl=bh;
       bh=temp;
    }
  }
  SetReg3(Pdata,(UShort)dh);
  SetReg3(Pdata,(UShort)bh);
  SetReg3(Pdata,(UShort)bl);
}

void LockCRT2(UShort BaseAddr)
{
  UShort  Part1Port;
  UShort  Part4Port;
  UShort temp1;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  Part4Port=BaseAddr+IND_SIS_CRT2_PORT_14;
  temp1=GetReg1(Part1Port,0x24);
  temp1=temp1&0xFE;
  SetReg1(Part1Port,0x24,temp1);
}

static void SetLockRegs(void)
{
  UShort temp1;

  if((VBInfo&SetInSlaveMode)&&(!(VBInfo&SetCRT2ToRAMDAC))){
    VBLongWait(); 
    temp1=GetReg1(P3c4,0x32);
    temp1=temp1|0x20;
    SetReg1(P3c4,0x32,temp1);
    VBLongWait(); 
  }
}

void EnableBridge(UShort BaseAddr)
{
  UShort part2_02,part2_05;
  UShort Part2Port;
  Part2Port=BaseAddr+IND_SIS_CRT2_PORT_10;

  if(IF_DEF_LVDS==0) {
    part2_02=(UChar)GetReg1(Part2Port,0x02);
    part2_05=(UChar)GetReg1(Part2Port,0x05);
    SetReg1(Part2Port,0x02,0x38);
    SetReg1(Part2Port,0x05,0xFF);
    LongWait();
    SetRegANDOR(Part2Port,0x00,~0x0E0,0x020);
 /*   WaitVBRetrace(BaseAddr);  */
    SetReg1(Part2Port,0x02,part2_02);
    SetReg1(Part2Port,0x05,part2_05);
  }
  else {
    EnableCRT2();
    UnLockCRT2(BaseAddr);
/*       SetRegANDOR(Part1Port,0x02,~0x040,0x0);   */    
  }
}

static void GetVBInfo(UShort BaseAddr,ULong ROMAddr)
{
  UShort flag1,tempbx,tempbl,tempbh,tempah,temp;

  SetFlag=0;
  tempbx=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
  tempbl=tempbx&ModeInfoFlag;
  ModeType=tempbl;
  tempbx=0;
  flag1=GetReg1(P3c4,0x38);     /* call BridgeisOn */
  if(IF_DEF_LVDS==0) {          /* for 301 */
   if(!(flag1&0x20)){
     VBInfo=CRT2DisplayFlag;
     return;
   }
  }
  tempbl=GetReg1(P3d4,0x30);
  tempbh=GetReg1(P3d4,0x31);

  tempah=((SetCHTVOverScan>>8)|(SetInSlaveMode>>8)|(DisableCRT2Display>>8));
  tempah=tempah^0xFF; 
  tempbh=tempbh&tempah;   
/*  ynlai begin */
  if(IF_DEF_LVDS==1){  /* for LVDS */
    if(IF_DEF_CH7005==1) temp=SetCRT2ToLCD|SetCRT2ToTV;
    else temp=SetCRT2ToLCD;
  }
  else {
    if(IF_DEF_HiVision==1) temp=0xFC;
    else temp=0x7C;
  }
  if(!(tempbl&temp)) {
    VBInfo=DisableCRT2Display;
    return;
  }
/*  ynlai end */
  if(IF_DEF_LVDS==0) {
    if(tempbl&SetCRT2ToRAMDAC){
      tempbl=tempbl&(SetCRT2ToRAMDAC|SwitchToCRT2|SetSimuScanMode);
    }else if(tempbl&SetCRT2ToLCD){
      tempbl=tempbl&(SetCRT2ToLCD|SwitchToCRT2|SetSimuScanMode);
    }else if(tempbl&SetCRT2ToSCART){
      tempbl=tempbl&(SetCRT2ToSCART|SwitchToCRT2|SetSimuScanMode);
      tempbh=tempbh|(SetPALTV>>8);
    }else if(tempbl&SetCRT2ToHiVisionTV){
      tempbl=tempbl&(SetCRT2ToHiVisionTV|SwitchToCRT2|SetSimuScanMode);
/* ynlai begin */
      tempbh=tempbh|(SetPALTV>>8);
/* ynlai end */
    }
  }
  else {
    if(IF_DEF_CH7005==1) {
      if(tempbl&SetCRT2ToTV) 
        tempbl=tempbl&(SetCRT2ToTV|SwitchToCRT2|SetSimuScanMode);
    } 
    if(tempbl&SetCRT2ToLCD) 
      tempbl=tempbl&(SetCRT2ToLCD|SwitchToCRT2|SetSimuScanMode);
  }
  tempah=GetReg1(P3d4,0x31);
  if(tempah&(CRT2DisplayFlag>>8)){
    if(!(tempbl&(SwitchToCRT2|SetSimuScanMode))){
      tempbx=SetSimuScanMode|CRT2DisplayFlag;
      tempbh=((tempbx&0xFF00)>>8);
      tempbl=tempbx&0xFF;
    }
  }
  if(!(tempbh&(DriverMode>>8))){
    tempbl=tempbl|SetSimuScanMode;
  }
  VBInfo=tempbl|(tempbh<<8);
  if(!(VBInfo&SetSimuScanMode)){
    if(!(VBInfo&SwitchToCRT2)){
      if(BridgeIsEnable(BaseAddr)){
        if(BridgeInSlave()){
          VBInfo=VBInfo|SetSimuScanMode;
        }
      }
    }
    else {
      flag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));      /* si+St_ModeFlag */
      if(!(flag1&CRT2Mode)) {
        VBInfo=VBInfo|SetSimuScanMode;
      }
    }
  }
  if(!(VBInfo&DisableCRT2Display)) {
    if(VBInfo&DriverMode) {
      if(VBInfo&SetSimuScanMode) {
        flag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
        if(!(flag1&CRT2Mode)) {
          VBInfo=VBInfo|SetInSlaveMode;
        }
      } 
    }
    else {
      VBInfo=VBInfo|SetSimuScanMode;
      if(IF_DEF_LVDS==0) {
        if(VBInfo&SetCRT2ToTV) {
          if(!(VBInfo&SetNotSimuMode))  SetFlag=SetFlag|TVSimuMode;
        }
      }
    }
  } 
  if(IF_DEF_CH7005==1) {
    tempah=GetReg1(P3d4,0x35);
    if(tempah&TVOverScan) VBInfo=VBInfo|SetCHTVOverScan; 
  }
}

static Bool BridgeIsEnable(UShort BaseAddr)
{
  UShort flag1;
  UShort  Part1Port;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;

  if(IF_DEF_LVDS==1){
    return 1;
  }
  flag1=GetReg1(P3c4,0x38);                   /* call BridgeisOn */
  if(!(flag1&0x20)){ return 0;}
  flag1=GetReg1(Part1Port,0x0);
  if(flag1&0x0a0){
    return 1;
  }else{
    return 0;
  }
}

static Bool BridgeInSlave(void)
{
  UShort flag1;
  flag1=GetReg1(P3d4,0x31);
  if(flag1&(SetInSlaveMode>>8)){
    return 1;
  }else{
    return 0;
  }
}

static Bool GetLCDResInfo(ULong ROMAddr,UShort P3d4)
{
  UShort tempah,tempbh,tempflag;        

  tempah=(UChar)GetReg1(P3d4,0x36);
  tempbh=tempah;
  tempah=tempah&0x0F;
/*  if(tempah!=0) tempah--; */
  if(tempah>Panel1280x1024) tempah=0;
  LCDResInfo=tempah;
  tempbh=tempbh>>4;
  LCDTypeInfo=tempbh;

  tempah=(UChar)GetReg1(P3d4,0x37);
  LCDInfo=tempah;

  if(IF_DEF_LVDS==1){
    tempflag=*((UShort *)(ROMAddr+ModeIDOffset+0x01));    /* si+St_ModeFlag  */
    if(tempflag&HalfDCLK){
      if(IF_DEF_TRUMPION==0){
        if(!(LCDInfo&LCDNonExpanding)){
          if(LCDResInfo==Panel1024x768){
            tempflag=*((UChar *)(ROMAddr+ModeIDOffset+0x09));/*si+Ext_ResInfo*/
            if(tempflag==4){ /*512x384  */
              SetFlag=SetFlag|EnableLVDSDDA;
            }
          }else{
            if(LCDResInfo==Panel800x600){
              tempflag=*((UChar*)(ROMAddr+ModeIDOffset+0x09));/*si+Ext_ResInfo*/
              if(tempflag==3){ /*400x300  */
                SetFlag=SetFlag|EnableLVDSDDA;
              } 
            }
          }
        }else{
          SetFlag=SetFlag|EnableLVDSDDA;
        }
      }else{
        SetFlag=SetFlag|EnableLVDSDDA;
      }
    }
  }

  if(!(VBInfo&SetCRT2ToLCD)){
    return 1;
  }
  if(!(VBInfo&(SetSimuScanMode|SwitchToCRT2))){
    return 1;
  }
  if(VBInfo&SetInSlaveMode){
    if(VBInfo&SetNotSimuMode){
      SetFlag=SetFlag|LCDVESATiming;
    }
  }else{
    SetFlag=SetFlag|LCDVESATiming;
  }
  return 1;
}

static void PresetScratchregister(UShort P3d4)
{
  SetReg1(P3d4,0x37,0x00);
}

static Bool GetLCDDDCInfo(ScrnInfoPtr pScrn)
{
  UShort tempah;
/*tempah=(HwDeviceExtension->usLCDType);// set in sisv.c                                                 */
  tempah=1;
  SetReg1(P3d4,0x36,tempah);  /* cr 36 0:no LCD 1:1024x768 2:1280x1024 */
  if(tempah>0) return 1;
  else return 0;
}

static void SetTVSystem(void)
{
  UShort tempah;
  tempah=GetReg1(P3c4,0x38);           /* SR 38                          */
  tempah=tempah&0x01;                  /* get SR 38 D0 TV Type Selection */
                                       /* 0:NTSC 1:PAL                   */
  SetRegANDOR(P3d4,0x31,~0x01,tempah); /* set CR 31 D0= SR 38 D0         */
  return;
}

static void LongWait(void)
{
  UShort i;
  for(i=0; i<0xFFFF; i++) {
     if(!(inSISREG(P3da) & 0x08))
       break;
  }
  for(i=0; i<0xFFFF; i++) {
     if((inSISREG(P3da) & 0x09) == 9)
       break;
  } 
}

static void VBLongWait(void)
{
  UShort regsr1f,tempah,temp;

  regsr1f=GetReg1(P3c4,0x1F);
  tempah=regsr1f&(~0xC0);
  SetReg1(P3c4,0x1F,tempah);

  for(temp=1;temp>0;){
     temp=GetReg2(P3da);
     temp=temp&0x08;
  }
  for(;temp==0;){
     temp=GetReg2(P3da);
     temp=temp&0x08;
  }

  SetReg1(P3c4,0x1F,regsr1f);
  return;
}

static Bool WaitVBRetrace(UShort  BaseAddr)
{
  UShort temp;
  UShort Part1Port;
  Part1Port=BaseAddr+IND_SIS_CRT2_PORT_04;
  temp=GetReg1(Part1Port,0x00);
  if(!(temp&0x80)){
    return 0;
  }

  for(temp=0;temp==0;){
     temp=GetReg1(Part1Port,0x25);
     temp=temp&0x01;
  }
  for(;temp>0;){
     temp=GetReg1(Part1Port,0x25);
     temp=temp&0x01;
  }
  return 1;
}


static void ModCRT1CRTC(ULong ROMAddr,UShort ModeNo)
{
  UShort OldREFIndex,temp,tempah,i,modeflag1;

  OldREFIndex=(UShort)REFIndex;
  temp=GetLVDSCRT1Ptr(ROMAddr,ModeNo);
  if(temp==0){
    REFIndex=OldREFIndex;
    return;
  }
  tempah=(UChar)GetReg1(P3d4,0x11);/*unlock cr0-7  */
  tempah=tempah&0x7F;
  SetReg1(P3d4,0x11,tempah);
  tempah=*((UChar *)(ROMAddr+REFIndex));
  SetReg1(P3d4,0x0,tempah);
  REFIndex++;
  for(i=0x02;i<=0x05;REFIndex++,i++){
    tempah=*((UChar *)(ROMAddr+REFIndex));
    SetReg1(P3d4,i,tempah); 
  }
  for(i=0x06;i<=0x07;REFIndex++,i++){
    tempah=*((UChar *)(ROMAddr+REFIndex));
    SetReg1(P3d4,i,tempah); 
  }
  for(i=0x10;i<=0x11;REFIndex++,i++){
    tempah=*((UChar *)(ROMAddr+REFIndex));
    SetReg1(P3d4,i,tempah); 
  }
  for(i=0x15;i<=0x16;REFIndex++,i++){
    tempah=*((UChar *)(ROMAddr+REFIndex));
    SetReg1(P3d4,i,tempah); 
  }

  for(i=0x0A;i<=0x0C;REFIndex++,i++){
    tempah=*((UChar *)(ROMAddr+REFIndex));
    SetReg1(P3c4,i,tempah); 
  }
  tempah=*((UChar *)(ROMAddr+REFIndex));
  tempah=tempah&0x0E0;
  SetReg1(P3c4,0x0E,tempah);

  tempah=*((UChar *)(ROMAddr+REFIndex));
  tempah=tempah&0x01;
  tempah=tempah<<5;
  modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag */
  if(modeflag1&DoubleScanMode){
    tempah=tempah|0x080;
  }
  SetRegANDOR(P3d4,0x09,~0x020,tempah);
  REFIndex=OldREFIndex;
  return; 
}

static void SetCRT2ECLK(ULong ROMAddr, UShort ModeNo)
{
  UShort OldREFIndex,tempah,tempal;
  UShort P3cc=P3c9+3;

  OldREFIndex=(UShort)REFIndex;
  if(IF_DEF_TRUMPION==0){  /*no trumpion  */
    tempal=GetReg2(P3cc);
    tempal=tempal&0x0C;
    REFIndex=GetVCLK2Ptr(ROMAddr,ModeNo);
  }else{  /*trumpion  */
    SetFlag=SetFlag&(~ProgrammingCRT2);
    tempal=*((UChar *)(ROMAddr+REFIndex+0x03));   /*&di+Ext_CRTVCLK  */
    tempal=tempal&0x03F;
    if(tempal==0x02){ /*31.5MHz  */
      REFIndex=REFIndex-Ext2StructSize;
    }
    REFIndex=GetVCLKPtr(ROMAddr,ModeNo);
    SetFlag=SetFlag|ProgrammingCRT2;
  }
  tempal=0x02B;
  if(!(VBInfo&SetInSlaveMode)){
    tempal=tempal+3; 
  }
  SetReg1(P3c4,0x05,0x86);
  tempah=*((UChar *)(ROMAddr+REFIndex));  
  SetReg1(P3c4,tempal,tempah);
  tempah=*((UChar *)(ROMAddr+REFIndex+1));
  tempal++; 
  SetReg1(P3c4,tempal,tempah);
  tempal++;
  SetReg1(P3c4,tempal,0x80);
  REFIndex=OldREFIndex;
  return;
}

static UShort GetLVDSDesPtr(ULong ROMAddr,UShort ModeNo)
{
  UShort tempcl,tempbx,tempal,tempptr,LVDSDesPtrData;
  UShort Flag;

  Flag=1;
  tempbx=0;
  if(IF_DEF_CH7005==1) {
    if(!(VBInfo&SetCRT2ToLCD)) {
      Flag=0;
      tempbx=32;
      if(VBInfo&SetPALTV) tempbx=tempbx+2;
      if(VBInfo&SetCHTVOverScan) tempbx=tempbx+1;
    }
  } 
  tempcl=LVDSDesDataLen;
  if(Flag) {
    tempbx=LCDTypeInfo;
    if(LCDInfo&LCDNonExpanding){
      tempbx=tempbx+16;
    }
  } 
  if(ModeNo<=0x13) tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));   /* si+St_CRT2CRTC  */
  else  tempal=*((UChar *)(ROMAddr+REFIndex+4));    /*di+Ext_CRT2CRTC  */
  tempal=tempal&0x1F;
  tempal=tempal*tempcl;
  tempbx=tempbx<<1;
  LVDSDesPtrData=*((UShort *)(ROMAddr+ADR_LVDSDesPtrData));
  tempptr=*((UShort *)(ROMAddr+LVDSDesPtrData+tempbx));
  tempptr=tempptr+tempal;
  return(tempptr);
  
}

static Bool GetLVDSCRT1Ptr(ULong ROMAddr,UShort ModeNo)
{
  UShort tempal,tempbx,modeflag1; 
  UShort LVDSCRT1DataPtr,Flag; 

  if(!(VBInfo&SetInSlaveMode)){
/*       return 0;  */
  }
  Flag=1;
  tempbx=0;
  if(IF_DEF_CH7005==1) {
    if(!(VBInfo&SetCRT2ToLCD)) {
      Flag=0;
      tempbx=12;
      if(VBInfo&SetPALTV) tempbx=tempbx+2;
      if(VBInfo&SetCHTVOverScan) tempbx=tempbx+1;       
    }
  }
  if(Flag) {
    tempbx=LCDResInfo;
    tempbx=tempbx-Panel800x600;
    if(LCDInfo&LCDNonExpanding) tempbx=tempbx+6;
    modeflag1=*((UShort *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag  */
    if(modeflag1&HalfDCLK) tempbx=tempbx+3;
  }
  if(ModeNo<=0x13) tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));   /* si+St_CRT2CRTC  */
  else tempal=*((UChar *)(ROMAddr+REFIndex+4));    /*di+Ext_CRT2CRTC  */
  tempal=tempal&0x3F;

  tempbx=tempbx<<1;
  LVDSCRT1DataPtr=*((UShort *)(ROMAddr+ADR_LVDSCRT1DataPtr));
  REFIndex=*((UShort *)(ROMAddr+LVDSCRT1DataPtr+tempbx));
  tempal=tempal*LVDSCRT1Len;
  REFIndex=REFIndex+tempal;
  return 1;

}

static void SetCHTVReg(ULong ROMAddr,UShort ModeNo) 
{
  UShort old_REFIndex,temp,tempbx,tempcl;

  old_REFIndex=(UShort)REFIndex;                /*push di  */
  GetCHTVRegPtr(ROMAddr,ModeNo);

  if(VBInfo&SetPALTV) {
    SetCH7005(0x4304); 
    SetCH7005(0x6909); 
  } 
  else {
    SetCH7005(0x0304); 
    SetCH7005(0x7109); 
  }

  temp=*((UShort *)(ROMAddr+REFIndex+0x00));
  tempbx=((temp&0x00FF)<<8)|0x00;
  SetCH7005(tempbx); 
  temp=*((UShort *)(ROMAddr+REFIndex+0x01));
  tempbx=((temp&0x00FF)<<8)|0x07;
  SetCH7005(tempbx);    
  temp=*((UShort *)(ROMAddr+REFIndex+0x02));
  tempbx=((temp&0x00FF)<<8)|0x08;
  SetCH7005(tempbx);    
  temp=*((UShort *)(ROMAddr+REFIndex+0x03));
  tempbx=((temp&0x00FF)<<8)|0x0A;
  SetCH7005(tempbx);    
  temp=*((UShort *)(ROMAddr+REFIndex+0x04));
  tempbx=((temp&0x00FF)<<8)|0x0B;
  SetCH7005(tempbx);    

  SetCH7005(0x2801);    
  SetCH7005(0x3103);    
  SetCH7005(0x003D);    
  SetCHTVRegANDOR(0x0010,0x1F); 
  SetCHTVRegANDOR(0x0211,0xF8);
  SetCHTVRegANDOR(0x001C,0xEF);
  
  if(!(VBInfo&SetPALTV)) {
    if(ModeNo<=0x13) tempcl=*((UChar *)(ROMAddr+ModeIDOffset+0x04));   /* si+St_CRT2CRTC */
    else tempcl=*((UChar *)(ROMAddr+REFIndex+4));    /* di+Ext_CRT2CRTC */
    tempcl=tempcl&0x3F;
    if(VBInfo&SetCHTVOverScan) {
      if(tempcl==0x04) {   /* 640x480   underscan */
        SetCHTVRegANDOR(0x0020,0xEF);   
        SetCHTVRegANDOR(0x0121,0xFE);   
      }
      else {
        if(tempcl==0x05) {    /* 800x600  underscan */    
          SetCHTVRegANDOR(0x0118,0xF0); 
          SetCHTVRegANDOR(0x0C19,0xF0); 
          SetCHTVRegANDOR(0x001A,0xF0); 
          SetCHTVRegANDOR(0x001B,0xF0); 
          SetCHTVRegANDOR(0x001C,0xF0); 
          SetCHTVRegANDOR(0x001D,0xF0); 
          SetCHTVRegANDOR(0x001E,0xF0); 
          SetCHTVRegANDOR(0x001F,0xF0); 
          SetCHTVRegANDOR(0x0120,0xEF); 
          SetCHTVRegANDOR(0x0021,0xFE); 
        }
      }
    }
    else {
      if(tempcl==0x04) {     /* 640x480   overscan  */
        SetCHTVRegANDOR(0x0020,0xEF);   
        SetCHTVRegANDOR(0x0121,0xFE);   
      }
      else {
        if(tempcl==0x05) {   /* 800x600   overscan */
          SetCHTVRegANDOR(0x0118,0xF0); 
          SetCHTVRegANDOR(0x0F19,0xF0); 
          SetCHTVRegANDOR(0x011A,0xF0); 
          SetCHTVRegANDOR(0x0C1B,0xF0); 
          SetCHTVRegANDOR(0x071C,0xF0); 
          SetCHTVRegANDOR(0x011D,0xF0); 
          SetCHTVRegANDOR(0x0C1E,0xF0); 
          SetCHTVRegANDOR(0x071F,0xF0); 
          SetCHTVRegANDOR(0x0120,0xEF); 
          SetCHTVRegANDOR(0x0021,0xFE); 
        }
      }
    }
  }

  REFIndex=old_REFIndex;                
}

static void SetCHTVRegANDOR(UShort tempax,UShort tempbh)
{
  UShort tempal,tempah,tempbl;

  tempal=tempax&0x00FF;
  tempah=(tempax>>8)&0x00FF;
  tempbl=GetCH7005(tempal); 
  tempbl=(((tempbl&tempbh)|tempah)<<8|tempal);  
  SetCH7005(tempbl);  
}

static void GetCHTVRegPtr(ULong ROMAddr,UShort ModeNo) 
{
  UShort tempbx,tempal,tempcl,CHTVRegDataPtr;

  if(VBInfo&SetCRT2ToTV) {
    tempbx=0;
    if(VBInfo&SetPALTV) tempbx=tempbx+2;
    if(VBInfo&SetCHTVOverScan) tempbx=tempbx+1;

    if(ModeNo<=0x13) tempal=*((UChar *)(ROMAddr+ModeIDOffset+0x04));   /* si+St_CRT2CRTC */
    else tempal=*((UChar *)(ROMAddr+REFIndex+4));    /* di+Ext_CRT2CRTC */
    tempal=tempal&0x3F;
    
    tempcl=CHTVRegDataLen;
    tempal=tempal*tempcl;
    tempbx=tempbx<<1;

    CHTVRegDataPtr=*((UShort *)(ROMAddr+ADR_CHTVRegDataPtr));
    REFIndex=*((UShort *)(ROMAddr+CHTVRegDataPtr+tempbx));
    REFIndex=REFIndex+tempal;
  }      
}

void SetCH7005(UShort tempbx)
{
  UShort tempah,temp;

  DDC_Port=0x3c4;
  DDC_Index=0x11;
  DDC_DataShift=0x00;
  DDC_DeviceAddr=0xEA;

  SetSwitchDDC2();  
  SetStart();
  tempah=DDC_DeviceAddr;
  temp=WriteDDC2Data(tempah); 
  tempah=tempbx&0x00FF; 
  temp=WriteDDC2Data(tempah);  
  tempah=(tempbx&0xFF00)>>8; 
  temp=WriteDDC2Data(tempah);  
  SetStop();   
}

UShort GetCH7005(UShort tempbx)
{
  UShort tempah;

  DDC_Port=0x3c4;
  DDC_Index=0x11;
  DDC_DataShift=0x00;
  DDC_DeviceAddr=0xEA;
  DDC_ReadAddr=tempbx;

  SetSwitchDDC2();  
  SetStart();
  tempah=DDC_DeviceAddr;
  WriteDDC2Data(tempah);
  tempah=DDC_ReadAddr;
  WriteDDC2Data(tempah);

  SetStart();
  tempah=DDC_DeviceAddr;
  tempah=tempah|0x01;
  if(WriteDDC2Data(tempah)) {
  }
  tempah=ReadDDC2Data(tempah);  
  SetStop();   
  return(tempah);
}

static void SetSwitchDDC2(void)
{
  UShort i;

  SetSCLKHigh();
  for(i=0;i<1000;i++) {
    GetReg1(DDC_Port,0x05);
  }
  SetSCLKLow();
  for(i=0;i<1000;i++) {
    GetReg1(DDC_Port,0x05);
  }
}


static void SetStart(void)
{
  SetSCLKLow();  
  SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x02);     /*  SetSDA(0x01); */
  SetSCLKHigh();    
  SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x00);      /* SetSDA(0x00); */
  SetSCLKHigh();    
}

static void SetStop(void)
{
  SetSCLKLow();  
  SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x00);    /*  SetSDA(0x00); */
  SetSCLKHigh();  
  SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x02);      /* SetSDA(0x01); */
  SetSCLKHigh();    
}

static UShort WriteDDC2Data(UShort tempax)
{ 
  UShort i,flag;

  flag=0x80;
  for(i=0;i<8;i++) {  
    SetSCLKLow();  
    if(tempax&flag) {
      SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x02);     
    }
    else {
      SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x00);     
    }
    SetSCLKHigh();    
    flag=flag>>1;
  }
  return(CheckACK());
}

static UShort ReadDDC2Data(UShort tempax)
{ 
  UShort i,temp,getdata;

  getdata=0;
  for(i=0;i<8;i++) {  
    getdata=getdata<<1;
    SetSCLKLow();
    SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x02);      
    SetSCLKHigh();
    temp=GetReg1(DDC_Port,DDC_Index);           
    if(temp&0x02) getdata=getdata|0x01;
  }  
  return(getdata);
}  

static void SetSCLKLow(void)
{
    SetRegANDOR(DDC_Port,DDC_Index,0xFE,0x00);      /* SetSCLKLow()  */
    DDC2Delay(); 
}


static void SetSCLKHigh(void)
{
  UShort temp;

  SetRegANDOR(DDC_Port,DDC_Index,0xFE,0x01);      /* SetSCLKLow()  */
  do {
    temp=GetReg1(DDC_Port,DDC_Index);
  } while(!(temp&0x01));
  DDC2Delay(); 
}  

static void DDC2Delay(void)
{
  UShort i;

   for(i=0;i<DDC2DelayTime;i++) {   
    GetReg1(P3c4,0x05);         
  }
}

static UShort CheckACK(void)
{
  UShort tempah;

  SetSCLKLow();
  SetRegANDOR(DDC_Port,DDC_Index,0xFD,0x02);  
  SetSCLKHigh();
  tempah=GetReg1(DDC_Port,DDC_Index);
  SetSCLKLow();
  if(tempah&0x01) return(1);
  else return(0);
}
