/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/vga/vgaPCI.h,v 3.22.2.38 1999/08/25 12:21:40 hohndel Exp $ */
/*
 * PCI Probe
 *
 * Copyright 1995  The XFree86 Project, Inc.
 *
 * A lot of this comes from Robin Cutshaw's scanpci
 *
 */
/* $XConsortium: vgaPCI.h /main/16 1996/10/25 21:22:32 kaleb $ */

#ifndef _VGA_PCI_H
#define _VGA_PCI_H

#include "xf86_PCI.h"

#define PCI_VENDOR_REAL3D	0x003D
#define PCI_VENDOR_NCR_1	0x1000
#define PCI_VENDOR_ATI		0x1002
#define PCI_VENDOR_AVANCE	0x1005
#define PCI_VENDOR_TSENG	0x100C
#define PCI_VENDOR_WEITEK	0x100E
#define PCI_VENDOR_DIGITAL	0x1011
#define PCI_VENDOR_CIRRUS	0x1013
#define PCI_VENDOR_NCR_2	0x101A
#define PCI_VENDOR_TRIDENT	0x1023
#define PCI_VENDOR_MATROX	0x102B
#define PCI_VENDOR_CHIPSTECH	0x102C
#define PCI_VENDOR_NEC		0x1033
#define PCI_VENDOR_SIS		0x1039
#define PCI_VENDOR_SGS		0x104A
#define PCI_VENDOR_NUMNINE	0x105D
#define PCI_VENDOR_UMC		0x1060
#define PCI_VENDOR_CYRIX	0x1078
#define PCI_VENDOR_NEOMAGIC	0x10C8
#define PCI_VENDOR_NVIDIA	0x10DE
#define PCI_VENDOR_INTERGRAPHICS	0x10ea
#define PCI_VENDOR_ALLIANCE	0x1142
#define PCI_VENDOR_3DFX		0x121a
#define PCI_VENDOR_NVIDIA_SGS	0x12d2
#define PCI_VENDOR_SIGMADESIGNS 0x1236
#define PCI_VENDOR_RENDITION	0x1163
#define PCI_VENDOR_3DLABS	0x3D3D
#define PCI_VENDOR_S3		0x5333
#define PCI_VENDOR_INTEL	0x8086
#define PCI_VENDOR_ARK		0xEDD8


/* Real 3D */
#define PCI_CHIP_I740_PCI	0x00D1

/* ATI */
#define PCI_CHIP_MACH32		0x4158
#define PCI_CHIP_RAGE128_RE	0x5245
#define PCI_CHIP_RAGE128_RF	0x5246
#define PCI_CHIP_RAGE128_RK	0x524B
#define PCI_CHIP_RAGE128_RL	0x524C
#define PCI_CHIP_MACH64CT	0x4354
#define PCI_CHIP_MACH64CX	0x4358
#define PCI_CHIP_MACH64ET	0x4554
#define PCI_CHIP_MACH64GB	0x4742
#define PCI_CHIP_MACH64GD	0x4744
#define PCI_CHIP_MACH64GI	0x4749
#define PCI_CHIP_MACH64GM	0x474D
#define PCI_CHIP_MACH64GN	0x474E
#define PCI_CHIP_MACH64GO	0x474F
#define PCI_CHIP_MACH64GP	0x4750
#define PCI_CHIP_MACH64GQ	0x4751
#define PCI_CHIP_MACH64GR	0x4752
#define PCI_CHIP_MACH64GS	0x4753
#define PCI_CHIP_MACH64GT	0x4754
#define PCI_CHIP_MACH64GU	0x4755
#define PCI_CHIP_MACH64GV	0x4756
#define PCI_CHIP_MACH64GW	0x4757
#define PCI_CHIP_MACH64GX	0x4758
#define PCI_CHIP_MACH64GZ	0x475A
#define PCI_CHIP_MACH64LB	0x4C42
#define PCI_CHIP_MACH64LD	0x4C44
#define PCI_CHIP_MACH64LG	0x4C47
#define PCI_CHIP_MACH64LI	0x4C49
#define PCI_CHIP_MACH64LP	0x4C50
#define PCI_CHIP_MACH64VT	0x5654
#define PCI_CHIP_MACH64VU	0x5655
#define PCI_CHIP_MACH64VV	0x5656

/* Avance Logic */
#define PCI_CHIP_ALG2301	0x2301

/* Tseng */
#define PCI_CHIP_ET4000_W32P_A	0x3202
#define PCI_CHIP_ET4000_W32P_B	0x3205
#define PCI_CHIP_ET4000_W32P_D	0x3206
#define PCI_CHIP_ET4000_W32P_C	0x3207
#define PCI_CHIP_ET6000		0x3208
#define PCI_CHIP_ET6300		0x4702

/* Weitek */
#define PCI_CHIP_P9000		0x9001
#define PCI_CHIP_P9100		0x9100

/* Cirrus Logic */
#define PCI_CHIP_GD7548		0x0038
#define PCI_CHIP_GD7555		0x0040
#define PCI_CHIP_GD7556		0x004C
#define PCI_CHIP_GD5430		0x00A0
#define PCI_CHIP_GD5434_4	0x00A4
#define PCI_CHIP_GD5434_8	0x00A8
#define PCI_CHIP_GD5436		0x00AC
#define PCI_CHIP_GD5446         0x00B8
#define PCI_CHIP_GD5480         0x00BC
#define PCI_CHIP_GD5462		0x00D0
#define PCI_CHIP_GD5464		0x00D4
#define PCI_CHIP_GD5464BD	0x00D5
#define PCI_CHIP_GD5465		0x00D6
#define PCI_CHIP_GD7541		0x1204
#define PCI_CHIP_GD7542		0x1200
#define PCI_CHIP_GD7543		0x1202
#define PCI_CHIP_GD7541         0x1204

/* Trident */
#define PCI_CHIP_8400		0x8400
#define PCI_CHIP_8420		0x8420
#define PCI_CHIP_8500		0x8500
#define PCI_CHIP_9320		0x9320
#define PCI_CHIP_9420		0x9420
#define PCI_CHIP_9440		0x9440
#define PCI_CHIP_9660		0x9660
#define PCI_CHIP_9388		0x9388
#define PCI_CHIP_9397		0x9397
#define PCI_CHIP_9520		0x9520
#define PCI_CHIP_9525		0x9525
#define PCI_CHIP_9750		0x9750
#define PCI_CHIP_9850		0x9850
#define PCI_CHIP_9880		0x9880
/* Bill Mair */
#define PCI_CHIP_9397_DVD	0x939A

/* Matrox */
#define PCI_CHIP_MGA2085	0x0518
#define PCI_CHIP_MGA2064	0x0519
#define PCI_CHIP_MGA1064	0x051a
#define PCI_CHIP_MGA2164	0x051b
#define PCI_CHIP_MGA2164_AGP	0x051f
#define PCI_CHIP_MGAG200_PCI	0x0520
#define PCI_CHIP_MGAG200	0x0521
#define PCI_CHIP_MGAG100_PCI	0x1000
#define PCI_CHIP_MGAG100	0x1001
#define PCI_CHIP_MGAG400	0x0525

/* Chips & Tech */
#define PCI_CHIP_65545		0x00D8
#define PCI_CHIP_65548		0x00DC
#define PCI_CHIP_65550		0x00E0
#define PCI_CHIP_65554		0x00E4
#define PCI_CHIP_65555		0x00E5
#define PCI_CHIP_68554		0x00F4
#define PCI_CHIP_69000		0x00C0

/* NEC */
#define PCI_CHIP_PC98CORE	0x0009

/* SiS */
#define PCI_CHIP_SG86C201	0x0001
#define PCI_CHIP_SG86C202	0x0002
#define PCI_CHIP_SG86C205	0x0205
#define PCI_CHIP_SG86C215	0x0215
#define PCI_CHIP_SG86C225	0x0225
#define PCI_CHIP_SIS5598	0x0200	
#define PCI_CHIP_SIS5597	0x0200
#define PCI_CHIP_SIS6326	0x6326
#define PCI_CHIP_SIS530		0x6306
#define PCI_CHIP_SIS620		0x6306

/* SGS */
#define PCI_CHIP_STG2000	0x0008
#define PCI_CHIP_STG1764	0x0009

/* Number Nine */
#define PCI_CHIP_I128		0x2309
#define PCI_CHIP_I128_2		0x2339

/* Cyrix */
#define PCI_CHIP_MEDIAGX	0x0001
#define PCI_CHIP_CYRIX_5520	0x0002
#define PCI_CHIP_CYRIX_5530	0x0104

/* NeoMagic */
#define PCI_CHIP_NM2070		0x0001
#define PCI_CHIP_NM2090		0x0002
#define PCI_CHIP_NM2093		0x0003
#define PCI_CHIP_NM2097		0x0083
#define PCI_CHIP_NM2160		0x0004
#define PCI_CHIP_NM2200		0x0005

/* NVIDIA */
#define PCI_CHIP_NV1		0x0008
#define PCI_CHIP_DAC64		0x0009
#define PCI_CHIP_TNT		0x0020
#define PCI_CHIP_TNT2		0x0028
#define PCI_CHIP_UTNT2		0x0029
#define PCI_CHIP_VTNT2		0x002C
#define PCI_CHIP_UVTNT2		0x002D
#define PCI_CHIP_ITNT2		0x00A0

/* NVIDIA & SGS */
#define PCI_CHIP_RIVA128        0x0018

/* Alliance Semiconductor */
#define PCI_CHIP_AP6410		0x3210
#define PCI_CHIP_AP6422		0x6422
#define PCI_CHIP_AT24		0x6424

/* 3Dfx */
#define PCI_CHIP_BANSHEE	0x0003
#define PCI_CHIP_VOODOO3	0x0005

/* Rendition */
#define PCI_CHIP_V1000		0x0001
#define PCI_CHIP_V2x00		0x2000

/* 3Dlabs */
#define PCI_CHIP_300SX		0x0001
#define PCI_CHIP_500TX		0x0002
#define PCI_CHIP_DELTA		0x0003
#define PCI_CHIP_PERMEDIA	0x0004

/* S3 */
#define PCI_CHIP_VIRGE		0x5631
#define PCI_CHIP_TRIO		0x8811
#define PCI_CHIP_AURORA64VP	0x8812
#define PCI_CHIP_TRIO64UVP	0x8814
#define PCI_CHIP_TRIO64V2_DXGX	0x8901
#define PCI_CHIP_PLATO_PX	0x8902
#define PCI_CHIP_VIRGE_VX	0x883D
#define PCI_CHIP_VIRGE_DXGX	0x8A01
#define PCI_CHIP_VIRGE_GX2	0x8A10
#define PCI_CHIP_VIRGE_MX	0x8C01
#define PCI_CHIP_VIRGE_MXP	0x8C03
#define PCI_CHIP_TRIO_3D	0x8904
#define PCI_CHIP_TRIO_3D_2X	0x8A13
#define PCI_CHIP_SAVAGE3D	0x8A20
#define PCI_CHIP_SAVAGE3D_M	0x8A21
#define PCI_CHIP_SAVAGE4	0x8A22
#define PCI_CHIP_868		0x8880
#define PCI_CHIP_928		0x88B0
#define PCI_CHIP_864_0		0x88C0
#define PCI_CHIP_864_1		0x88C1
#define PCI_CHIP_964_0		0x88D0
#define PCI_CHIP_964_1		0x88D1
#define PCI_CHIP_968		0x88F0


/* Intel */
#define PCI_CHIP_I740_AGP	0x7800

/* ARK Logic */
#define PCI_CHIP_1000PV		0xA091
#define PCI_CHIP_2000PV		0xA099
#define PCI_CHIP_2000MT		0xA0A1
#define PCI_CHIP_2000MI		0xA0A9

/* SIGMA DESIGNS */
#define PCI_CHIP_SD_REALMAGIG64GX	0x6401

/* Intergraphics */
#define PCI_CHIP_INTERG_1680	0x1680
#define PCI_CHIP_INTERG_1682	0x1682

/* Increase this as required */
#define MAX_DEV_PER_VENDOR 48

typedef struct vgaPCIInformation {
    int Vendor;
    int ChipType;
    int ChipRev;
    CARD32 MemBase;
    CARD32 MMIOBase;
    CARD32 IOBase;
    int Bus;
    int Card;
    int Func;
    pciConfigPtr ThisCard; /* This isn't valid after calling xf86cleanpci() */
    pciConfigPtr *AllCards; /* This isn't valid after calling xf86cleanpci() */
} vgaPCIInformation;

extern vgaPCIInformation *vgaPCIInfo;

typedef struct pciVendorDeviceInfo {
    unsigned short VendorID;
    char *VendorName;
    struct pciDevice {
	unsigned short DeviceID;
	char *DeviceName;
    } Device[MAX_DEV_PER_VENDOR];
} pciVendorDeviceInfo;

extern pciVendorDeviceInfo xf86PCIVendorInfo[];

extern vgaPCIInformation *vgaGetPCIInfo(
#if NeedFunctionPrototypes
    void
#endif
);
   
#ifdef INIT_PCI_VENDOR_INFO
pciVendorDeviceInfo xf86PCIVendorInfo[] = {
    {PCI_VENDOR_REAL3D,	"Real 3D", {
				{PCI_CHIP_I740_PCI,	"i740 (PCI)"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NCR_1,	"NCR",	{
				{0x0000,		NULL}}},
    {PCI_VENDOR_ATI,	"ATI",	{
				{PCI_CHIP_MACH32,	"Mach32"},
				{PCI_CHIP_RAGE128_RE,   "Rage128 RE"},
				{PCI_CHIP_RAGE128_RF,   "Rage128 RF"},
				{PCI_CHIP_RAGE128_RK,   "Rage128 RK"},
				{PCI_CHIP_RAGE128_RL,   "Rage128 RL"},
				{PCI_CHIP_MACH64GX,	"Mach64 GX"},
				{PCI_CHIP_MACH64CX,	"Mach64 CX"},
				{PCI_CHIP_MACH64CT,	"Mach64 CT"},
				{PCI_CHIP_MACH64ET,	"Mach64 ET"},
				{PCI_CHIP_MACH64VT,	"Mach64 VT"},
				{PCI_CHIP_MACH64VU,	"Mach64 VT3"},
				{PCI_CHIP_MACH64VV,	"Mach64 VT4"},
				{PCI_CHIP_MACH64GT,	"Mach64 GT"},
				{PCI_CHIP_MACH64GU,	"Mach64 GT-B"},
				{PCI_CHIP_MACH64GV,	"Mach64 GT IIc"},
				{PCI_CHIP_MACH64GW,	"Mach64 GT IIc"},
				{PCI_CHIP_MACH64GZ,	"Mach64 GT IIc"},
				{PCI_CHIP_MACH64GB,	"Mach64 GT Pro"},
				{PCI_CHIP_MACH64GD,	"Mach64 GT Pro"},
				{PCI_CHIP_MACH64GI,	"Mach64 GT Pro"},
				{PCI_CHIP_MACH64GP,	"Mach64 GT Pro"},
				{PCI_CHIP_MACH64GQ,	"Mach64 GT Pro"},
				{PCI_CHIP_MACH64LG,	"Mach64 LT"},
				{PCI_CHIP_MACH64LB,	"Mach64 LT Pro"},
				{PCI_CHIP_MACH64LD,	"Mach64 LT Pro"},
				{PCI_CHIP_MACH64LI,	"Mach64 LT Pro"},
				{PCI_CHIP_MACH64LP,	"Mach64 LT Pro"},
				{PCI_CHIP_MACH64GM,	"Mach64 XL or XC"},
				{PCI_CHIP_MACH64GN,	"Mach64 XL or XC"},
				{PCI_CHIP_MACH64GO,	"Mach64 XL or XC"},
				{PCI_CHIP_MACH64GR,	"Mach64 XL or XC"},
				{PCI_CHIP_MACH64GS,	"Mach64 XL or XC"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_AVANCE,	"Avance Logic",	{
				{PCI_CHIP_ALG2301,	"ALG2301"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_TSENG,	"Tseng Labs", {
				{PCI_CHIP_ET4000_W32P_A, "ET4000W32P revA"},
				{PCI_CHIP_ET4000_W32P_B, "ET4000W32P revB"},
				{PCI_CHIP_ET4000_W32P_C, "ET4000W32P revC"},
				{PCI_CHIP_ET4000_W32P_D, "ET4000W32P revD"},
				{PCI_CHIP_ET6000,	 "ET6000/6100"},
				{PCI_CHIP_ET6300,	 "ET6300"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_WEITEK,	"Weitek", {
				{PCI_CHIP_P9000,	"P9000"},
				{PCI_CHIP_P9100,	"P9100"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_DIGITAL, "Digital", {
				{0x0000,		NULL}}},
    {PCI_VENDOR_CIRRUS,	"Cirrus Logic", {
				{PCI_CHIP_GD5430,	"GD5430"},
				{PCI_CHIP_GD5434_4,	"GD5434"},
				{PCI_CHIP_GD5434_8,	"GD5434"},
				{PCI_CHIP_GD5436,	"GD5436"},
				{PCI_CHIP_GD5446,       "GD5446"},
				{PCI_CHIP_GD5480,       "GD5480"},
				{PCI_CHIP_GD5462,       "GD5462"},
				{PCI_CHIP_GD5464,       "GD5464"},
				{PCI_CHIP_GD5464BD,     "GD5464BD"},
				{PCI_CHIP_GD5465,       "GD5465"},
				{PCI_CHIP_GD7541,	"GD7541"},
				{PCI_CHIP_GD7542,	"GD7542"},
				{PCI_CHIP_GD7543,	"GD7543"},
				{PCI_CHIP_GD7548,	"GD7548"},
				{PCI_CHIP_GD7555,	"GD7555"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NCR_2,	"NCR",	{
				{0x0000,		NULL}}},
    {PCI_VENDOR_TRIDENT, "Trident", {
				{PCI_CHIP_9320,		"Cyber 9320"},
				{PCI_CHIP_9420,		"TGUI 9420"},
				{PCI_CHIP_9440,		"TGUI 9440"},
				{PCI_CHIP_9660,		"TGUI 96xx"},
				{PCI_CHIP_9388,		"Cyber 9388"},
				{PCI_CHIP_9397,		"Cyber 9397"},
				{PCI_CHIP_9397_DVD,	"Cyber 9397 DVD"},
				{PCI_CHIP_9520,		"Cyber 9520"},
				{PCI_CHIP_9525,		"Cyber 9525 DVD"},
				{PCI_CHIP_9750,		"3DImage975"},
				{PCI_CHIP_9850,		"3DImage985"},
				{PCI_CHIP_9880,		"Blade3D"},
				{PCI_CHIP_8400,		"CyberBlade/i7"},
				{PCI_CHIP_8420,		"CyberBlade/i7/DSTN"},
				{PCI_CHIP_8500,		"CyberBlade/i1"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_MATROX,	"Matrox", {
				{PCI_CHIP_MGA2085,	"MGA 2085PX"},
				{PCI_CHIP_MGA2064,	"MGA 2064W"},
				{PCI_CHIP_MGA1064,	"MGA 1064SG"},
				{PCI_CHIP_MGA2164,	"MGA 2164W"},
				{PCI_CHIP_MGA2164_AGP,	"MGA 2164W AGP"},
				{PCI_CHIP_MGAG100,	"MGA G100 AGP"},
				{PCI_CHIP_MGAG100_PCI,	"MGA G100 PCI"},
				{PCI_CHIP_MGAG200,	"MGA G200 AGP"},
				{PCI_CHIP_MGAG200_PCI,	"MGA G200 PCI"},
				{PCI_CHIP_MGAG400,	"MGA G400 AGP"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_CHIPSTECH, "C&T", {
				{PCI_CHIP_65545,	"65545"},
				{PCI_CHIP_65548,	"65548"},
				{PCI_CHIP_65550,	"65550"},
				{PCI_CHIP_65554,	"65554"},
				{PCI_CHIP_65555,	"65555"},
				{PCI_CHIP_68554,	"68554"},
				{PCI_CHIP_69000,	"69000"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NEC,	"NEC",	{
				{PCI_CHIP_PC98CORE,	"PC-98 PCI to Core Graph Bridge"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_SIS,	"SiS",	{
				{PCI_CHIP_SG86C201,	"SG86C201"},
				{PCI_CHIP_SG86C202,	"SG86C202"},
				{PCI_CHIP_SG86C205,	"SG86C205"},
				{PCI_CHIP_SG86C215,	"SG86C215"},
				{PCI_CHIP_SG86C225,	"SG86C225"},
				{PCI_CHIP_SIS5597,	"5597/98"},
				{PCI_CHIP_SIS6326,	"6326"},
				{PCI_CHIP_SIS530,	"530/620"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_SGS,	"SGS-Thomson",	{
				{PCI_CHIP_STG2000,	"STG2000"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NUMNINE, "Number Nine", {
				{PCI_CHIP_I128,		"Imagine 128"},
				{PCI_CHIP_I128_2,	"Imagine 128 II"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_UMC,	"UMC",	{
				{0x0000,		NULL}}},
    {PCI_VENDOR_NVIDIA,	"NVidia",	{
				{PCI_CHIP_NV1,		"NV1"},
				{PCI_CHIP_TNT,		"Riva TNT"},
				{PCI_CHIP_TNT2,		"Riva TNT2"},
				{PCI_CHIP_UTNT2,	"Riva Ultra TNT2"},
				{PCI_CHIP_VTNT2,	"Riva Vanta TNT2"},
				{PCI_CHIP_UVTNT2,	"Riva Ultra Vanta"},
				{PCI_CHIP_ITNT2,	"Riva Integrated"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NVIDIA_SGS,	"NVidia/SGS-Thomson",	{
				{PCI_CHIP_RIVA128,	"Riva128"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_CYRIX,	"Cyrix",{
				{PCI_CHIP_MEDIAGX,	"MediaGX CPU"},
				{PCI_CHIP_CYRIX_5520,	"5520 companion chip"},
				{PCI_CHIP_CYRIX_5530,	"5530 companion chip"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_NEOMAGIC, "NeoMagic",  {
    				{PCI_CHIP_NM2070,	"NM2070"},
				{PCI_CHIP_NM2090,	"NM2090"},
				{PCI_CHIP_NM2093,	"NM2093"},
				{PCI_CHIP_NM2097,	"NM2097"},
				{PCI_CHIP_NM2160,	"NM2160"},
				{PCI_CHIP_NM2200,	"NM2200"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_ALLIANCE, "Alliance Semiconductor", {
				{PCI_CHIP_AP6410,	"ProMotion 6410"},
				{PCI_CHIP_AP6422,	"ProMotion 6422"},
				{PCI_CHIP_AT24,		"ProMotion AT24"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_3DFX, "3Dfx", {
				{PCI_CHIP_BANSHEE,	"Banshee"},
				{PCI_CHIP_VOODOO3,	"Voodoo3"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_RENDITION, "Rendition", {
				{PCI_CHIP_V1000,	"Verite 1000"},
				{PCI_CHIP_V2x00,	"Verite 2x00"},
				{0x0000,		NULL}}},
     {PCI_VENDOR_3DLABS, "3Dlabs", {
				{PCI_CHIP_300SX,	"GLINT 300SX"},
				{PCI_CHIP_500TX,	"GLINT 500TX"},
				{PCI_CHIP_DELTA,	"GLINT Delta"},
				{PCI_CHIP_PERMEDIA,	"GLINT Permedia"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_S3,	"S3",	{
				{PCI_CHIP_VIRGE,	"ViRGE"},
				{PCI_CHIP_TRIO,		"Trio32/64"},
				{PCI_CHIP_AURORA64VP,	"Aurora64V+"},
				{PCI_CHIP_TRIO64UVP,	"Trio64UV+"},
				{PCI_CHIP_TRIO64V2_DXGX,"Trio64V2/DX or /GX"},
				{PCI_CHIP_PLATO_PX,	"PLATO/PX"},
				{PCI_CHIP_VIRGE_VX,	"ViRGE/VX"},
				{PCI_CHIP_VIRGE_DXGX,	"ViRGE/DX or /GX"},
				{PCI_CHIP_VIRGE_GX2,	"ViRGE/GX2"},
				{PCI_CHIP_VIRGE_MX,	"ViRGE/MX"},
				{PCI_CHIP_VIRGE_MXP,	"ViRGE/MX+"},
				{PCI_CHIP_TRIO_3D,	"Trio3D"},
				{PCI_CHIP_TRIO_3D_2X,	"Trio3D/2X"},
				{PCI_CHIP_SAVAGE3D,	"Savage3D"},
				{PCI_CHIP_SAVAGE3D_M,	"Savage3D (Macrovision Support)"},
				{PCI_CHIP_SAVAGE4,	"Savage4"},
				{PCI_CHIP_868,		"868"},
				{PCI_CHIP_928,		"928"},
				{PCI_CHIP_864_0,	"864"},
				{PCI_CHIP_864_1,	"864"},
				{PCI_CHIP_964_0,	"964"},
				{PCI_CHIP_964_1,	"964"},
				{PCI_CHIP_968,		"968"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_INTEL,	"Intel", {
				{PCI_CHIP_I740_AGP,	"i740 (AGP)"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_ARK,	"ARK Logic", {
				{PCI_CHIP_1000PV,	"1000PV"},
				{PCI_CHIP_2000PV,	"2000PV"},
				{PCI_CHIP_2000MT,	"2000MT"},
				{PCI_CHIP_2000MI,	"2000MI"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_SIGMADESIGNS,	"Sigma Designs", {
				{PCI_CHIP_SD_REALMAGIG64GX,	"REALmagic64/GX (SD 6425)"},
				{0x0000,		NULL}}},
    {PCI_VENDOR_INTERGRAPHICS,	"Intergraphics", {
				{PCI_CHIP_INTERG_1680, "IGA-1680"},
				{PCI_CHIP_INTERG_1682, "IGA-1682"},
				{0x0000,		NULL}}},
    {0x0000,		NULL,	{
				{0x0000,		NULL}}},
};
#endif

#endif /* VGA_PCI_H */
