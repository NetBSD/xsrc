/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/initdef.h,v 1.4 2000/12/02 01:16:17 dawes Exp $ */

#define ULONG                   unsigned long
#define USHORT                  unsigned short
#define SHORT                   short
#define UCHAR                   unsigned char
#define VOID                    void
#define BOOLEAN                 Bool

#define CRT1Len                 17
#define LVDSCRT1Len             15
#define CHTVRegDataLen		5

#define ModeInfoFlag            0x07
#define IsTextMode              0x07
#define ModeText                0x00
#define ModeCGA                 0x01
#define ModeEGA                 0x02
#define ModeVGA                 0x03
#define Mode15Bpp               0x04
#define Mode16Bpp               0x05
#define Mode24Bpp               0x06
#define Mode32Bpp               0x07

#define DACInfoFlag             0x18
#define MemoryInfoFlag          0x1E0
#define MemorySizeShift         0x05

#define Charx8Dot               0x0200
#define LineCompareOff          0x0400
#define CRT2Mode	        0x0800
#define HalfDCLK                0x1000
#define NoSupportSimuTV         0x2000
#define DoubleScanMode          0x8000

#define SupportAllCRT2          0x0078
#define SupportTV	        0x0008
#define SupportHiVisionTV       0x0010
#define SupportLCD	        0x0020
#define SupportRAMDAC2          0x0040 	
#define NoSupportTV 	        0x0070
#define NoSupportHiVisionTV     0x0060
#define NoSupportLCD	        0x0058
#define SupportCHTV	        0x0800
#define InterlaceMode           0x0080
#define SyncPP		        0x0000
#define SyncPN	 	        0x4000
#define SyncNP		        0x8000
#define SyncNN  	        0xc000
#define ECLKindex0	        0x0000
#define ECLKindex1	        0x0100
#define ECLKindex2	        0x0200
#define ECLKindex3	        0x0300
#define ECLKindex4	        0x0400

#define SetSimuScanMode         0x0001
#define SwitchToCRT2		0x0002
#define SetCRT2ToTV             0x009C
#define SetCRT2ToAVIDEO		0x0004
#define SetCRT2ToSVIDEO		0x0008
#define SetCRT2ToSCART	 	0x0010
#define SetCRT2ToLCD            0x0020
#define SetCRT2ToRAMDAC         0x0040
#define SetCRT2ToHiVisionTV     0x0080
#define SetNTSCTV		0x0000
#define SetPALTV		0x0100
#define SetInSlaveMode		0x0200
#define	SetNotSimuMode		0x0400
#define SetDispDevSwitch	0x0800
#define LoadDACFlag		0x1000
#define DisableCRT2Display	0x2000
#define DriverMode		0x4000
#define HotKeySwitch		0x8000
#define SetCHTVOverScan		0x8000

#define TVOverScan		0x10
#define TVOverScanShift		4
#define ClearBufferFlag		0x20

#define	SetSCARTOutput		0x01
#define BoardTVType		0x02

#define ProgrammingCRT2         0x01
#define TVSimuMode		0x02
#define RPLLDIV2XO		0x04
#define LCDVESATiming		0x08
#define EnableLVDSDDA		0x10
#define SetDispDevSwitchFlag	0x20
#define CheckWinDos		0x40
#define SetJDOSMode		0x80

#define Panel800x600            0x01
#define Panel1024x768           0x02
#define Panel1280x1024          0x03
#define Panel1280x960           0x04
#define Panel640x480		0x05
#define LCDRGB18Bit		0x01
#define ExtChipType		0x0e
#define ExtChip301		0x02
#define ExtChipLVDS		0x04
#define ExtChipTrumpion		0x06
#define ExtChipCH7005		0x08
#define ExtChipMitacTV		0x0a
#define LCDNonExpanding		0x10
#define LCDNonExpandingShift	4
#define LCDSync			0x20
#define LCDSyncBit		0xe0
#define LCDSyncshift		6 

#define DDC2DelayTime		10	

#define CRT2DisplayFlag         0x2000
#define LCDDataLen              8
#define HiTVDataLen             12
#define TVDataLen               16
#define SetPALTV                0x0100
#define HalfDCLK                0x1000
#define NTSCHT                  1716
#define NTSCVT                  525
#define PALHT                   1728
#define PALVT                   625
#define StHiTVHT                892
#define StHiTVVT                1126
#define StHiTextTVHT            1000
#define StHiTextTVVT            1126
#define ExtHiTVHT               2100
#define ExtHiTVVT               1125

#define SelectCRT2Rate          0x4
#define VCLKStartFreq           25
#define SoftDramType            0x80
#define VCLK40                  0x04
#define VCLK65                  0x09
#define VCLK108_2               0x14
#define LCDRGB18Bit             0x01
#define LoadDACFlag             0x1000
#define AfterLockCRT2           0x4000
#define SetCRT2ToAVIDEO         0x0004
#define SetCRT2ToSCART          0x0010
#define Ext2StructSize          5
#define TVVCLKDIV2              0x021
#define TVVCLK                  0x022
#define HiTVVCLKDIV2            0x023
#define HiTVVCLK                0x024
#define HiTVSimuVCLK            0x025
#define HiTVTextVCLK            0x026
#define SwitchToCRT2            0x0002
#define LCDVESATiming           0x08
#define SetSCARTOutput          0x01
#define SCARTSense              0x04
#define Monitor1Sense           0x20
#define Monitor2Sense           0x10
#define SVIDEOSense             0x02
#define AVIDEOSense             0x01
#define LCDSense                0x08
#define BoardTVType             0x02
#define HotPlugFunction         0x08
#define StStructSize            0x06

#define IND_SIS_CRT2_PORT_04        0x04 - 0x030
#define IND_SIS_CRT2_PORT_10        0x10 - 0x30
#define IND_SIS_CRT2_PORT_12        0x12 - 0x30
#define IND_SIS_CRT2_PORT_14        0x14 - 0x30

#define IF_DEF_TRUMPION         0
#define LCDNonExpanding         0x10
#define ADR_CRT2PtrData         0x20E
#define offset_Zurac            0x210
#define ADR_LVDSDesPtrData      0x212
#define ADR_LVDSCRT1DataPtr     0x214
#define ADR_CHTVVCLKPtr         0x216
#define ADR_CHTVRegDataPtr      0x218

#define LVDSDataLen             6
#define EnableLVDSDDA           0x10
#define LVDSDesDataLen          3
#define ActiveNonExpanding      0x40
#define ActiveNonExpandingShift 6
#define ModeSwitchStatus        0x0F
#define SoftTVType              0x40

#define SelectCRT1Rate          0x4
#define SelectCRT2Rate          0x4
        
#define PanelType00             0x00    
#define PanelType01             0x08
#define PanelType02             0x10
#define PanelType03             0x18
#define PanelType04             0x20
#define PanelType05             0x28
#define PanelType06             0x30
#define PanelType07             0x38
#define PanelType08             0x40
#define PanelType09             0x48
#define PanelType0A             0x50
#define PanelType0B             0x58
#define PanelType0C             0x60
#define PanelType0D             0x68
#define PanelType0E             0x70
#define PanelType0F             0x78


