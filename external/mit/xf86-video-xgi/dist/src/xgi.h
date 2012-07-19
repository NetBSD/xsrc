/*
 * Main global data and definitions
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:   Thomas Winischhofer <thomas@winischhofer.net>
 *            others (old code base)
 *
 */
#ifndef _XGI_H_
#define _XGI_H_

/***************
#define DEBUG 
#define DEBUG1
#define DEBUG2
#define DEBUG3
#define DEBUG4
#define DEBUG5
#define DEBUGI2C
*****************/

#ifndef XGI_VIDEO_HW /* avoid compile error in xgi_videohw.c; weird!  */
/* Jong 07/27/2009; use run-time debug instead except for HW acceleration routines */
extern Bool g_bRunTimeDebug;
#define RUNTIMEDEBUG(p)		if(g_bRunTimeDebug)p;

/* Jong@08052009 */
#ifdef  DEBUGI2C
#define PDEBUGI2C(p)       p
#else
#define PDEBUGI2C(p)
#endif

#ifdef  DEBUG
#define PDEBUG(p)       RUNTIMEDEBUG(p)
#else
#define PDEBUG(p)
#endif

#ifdef DEBUG1
#define PDEBUG1(p)		RUNTIMEDEBUG(p)
#else
#define PDEBUG1(p)
#endif

#ifdef DEBUG2
#define PDEBUG2(p)		RUNTIMEDEBUG(p)
#else
#define PDEBUG2(p)
#endif

#ifdef DEBUG3
#define PDEBUG3(p)		RUNTIMEDEBUG(p)
#else
#define PDEBUG3(p)
#endif

#ifdef DEBUG4
#define PDEBUG4(p)		RUNTIMEDEBUG(p)
#else
#define PDEBUG4(p)
#endif

#ifdef  DEBUG5
#define PDEBUG5(p)		RUNTIMEDEBUG(p)
#else
#define PDEBUG5(p)
#endif

#ifdef  CDEBUG
#define CPDEBUG(p)       p
#else
#define CPDEBUG(p)
#endif

#ifdef CDEBUG1
#define CPDEBUG1(p) p
#else
#define CPDEBUG1(p)
#endif

#ifdef CDEBUG2
#define CPDEBUG2(p) p
#else
#define CPDEBUG2(p)
#endif

#ifdef CDEBUG3
#define CPDEBUG3(p) p
#else
#define CPDEBUG3(p)
#endif

#ifdef CDEBUG4
#define CPDEBUG4(p) p
#else
#define CPDEBUG4(p)
#endif

#ifdef  CDEBUG5
#define CPDEBUG5(p)       p
#else
#define CPDEBUG5(p)
#endif

#ifdef  ACCELDEBUG
#define PACCELDEBUG(p)       p
#else
#define PACCELDEBUG(p)
#endif
#endif

/* Always unlock the registers (should be set!) */
#define UNLOCK_ALWAYS

#undef XGI_CP

#ifdef XSERVER_LIBPCIACCESS
#include <pciaccess.h>
#else
#include "xf86Pci.h"
#endif
#include "xf86Cursor.h"
#include "xf86xv.h"
#include "compiler.h"

#ifdef XORG_VERSION_CURRENT
#include "xorgVersion.h"

/* #if XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(7,0,0,0,0) */
#if ((XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(7,0,0,0,0)) || (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(2,0,0,0,0)) )
#define XGIISXORGPOST70
#endif

#if (XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0) )
#define XGI_USE_EXA
#endif

#endif

/* Jong 01/13/2009; support EXA */
#define XGI_USE_XAA 
/* #define XGI_USE_EXA */

#ifdef XGI_USE_XAA
#include "xaa.h"
#endif
#ifdef XGI_USE_EXA
#include "exa.h"
#endif

#include "vgaHW.h"
#include "vbe.h"

/*
#ifdef XORG_VERSION_CURRENT
#include "xorgVersion.h"
#endif */

/* #define XGIISXORGPOST70 */

#include "xgi_pci.h"
#include "osdef.h"
#include "vgatypes.h"
#include "vb_struct.h"

#ifdef XF86DRI
#define XGINEWDRI
#undef XGINEWDRI2

#ifdef XF86_VERSION_CURRENT
#if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(4,4,99,99,0)	/* Adapt this when the time has come */
#define XGINEWDRI2
#endif
#endif /* XF86_VERSION_CURRENT */

#include "xf86drm.h"
#include "sarea.h"
#define _XF86DRI_SERVER_

/* Jong@09032009 */
#ifdef XORG_VERSION_CURRENT
#if ( (XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)) || (XORG_VERSION_CURRENT <= XORG_VERSION_NUMERIC(2,0,0,0,0)) )
#include "X11/dri/xf86dri.h"
#else
#include "xf86dri.h"
#endif
#endif

#include "dri.h"
#include "GL/glxint.h"
#include "xgi_dri.h"
#endif

#ifdef XSERVER_LIBPCIACCESS
#define VENDOR_ID(p)      (p)->vendor_id
#define DEVICE_ID(p)      (p)->device_id
#define SUBSYS_ID(p)      (p)->subdevice_id
#define CHIP_REVISION(p)  (p)->revision
#else
#define VENDOR_ID(p)      (p)->vendor
#define DEVICE_ID(p)      (p)->chipType
#define SUBSYS_ID(p)      (p)->subsysCard
#define CHIP_REVISION(p)  (p)->chipRev
#endif

#if 1 
#define XGIDUALHEAD  		/* Include Dual Head code  */
#endif

#if 1 
#define XGIMERGED		/* Include Merged-FB mode */
#endif

#ifdef XGIMERGED
#if 1
#define XGIXINERAMA		/* Include Pseudo-Xinerama for MergedFB mode */
#define XGI_XINERAMA_MAJOR_VERSION  1
#define XGI_XINERAMA_MINOR_VERSION  1
#endif
#endif

#if 1
#define XGIGAMMA		/* Include code for gamma correction */
#endif

/* Jong 09/28/2007; disable this because it causes cursor drawing incorrectly */
#if 0				/* Include code for color hardware cursors */
#define XGI_ARGB_CURSOR
#endif

#if 0				/* Include YPbPr support on VB  */
#define ENABLE_YPBPR
#endif

#ifdef XGIMERGED
#ifdef XGIXINERAMA
#define EXTENSION_PROC_ARGS void *
#include "extnsionst.h"  	/* required */
#include <X11/extensions/panoramiXproto.h>  	/* required */
#endif
#endif

#if 1
#define XGIVRAMQ		/* Use VRAM queue mode on 315 series */
#endif

#undef XGI315DRI		/* define this if dri is adapted for 315/330 series */

#ifndef PCI_VENDOR_XGI
#define PCI_VENDOR_XGI 		    0x18CA
#endif
#ifndef PCI_CHIP_XGIXG40
#define PCI_CHIP_XGIXG40 		0x0040
#endif
#ifndef PCI_CHIP_XGIXG20
#define PCI_CHIP_XGIXG20 		0x0020
#endif

/* Jong 09/18/2007; patch to GIT */
#ifndef PCI_CHIP_XGIXG27
#define PCI_CHIP_XGIXG27 		0x0027
#endif

/* Jong 01/07/2008; support New XG21 */
#ifndef PCI_CHIP_XGIXG21
#define PCI_CHIP_XGIXG21 		0x0021
#endif

#define CONFIG_DRM_XGI

#define XGI_NAME                "XGI"
#define XGI_DRIVER_NAME         "xgi"
#define XGI_CURRENT_VERSION     ((PACKAGE_VERSION_MAJOR << 16) | \
                                 (PACKAGE_VERSION_MINOR << 8) | \
				 PACKAGE_VERSION_PATCHLEVEL)

/* pXGI->Flags (old series only) */
#define SYNCDRAM                0x00000001
#define RAMFLAG                 0x00000002
#define ESS137xPRESENT          0x00000004
#define SECRETFLAG              0x00000008
#define A6326REVAB              0x00000010
#define MMIOMODE                0x00010000
#define LFBQMODE                0x00020000
#define AGPQMODE                0x00040000
#define UMA                     0x80000000

#define BIOS_BASE               0xC0000
#define BIOS_SIZE               0x10000

#define SR_BUFFER_SIZE          5
#define CR_BUFFER_SIZE          5

#define XGI_VBFlagsVersion	1

/* VBFlags - if anything is changed here, increase VBFlagsVersion! */
#define CRT2_DEFAULT            0x00000001
#define CRT2_LCD                0x00000002  /* Never change the order of the CRT2_XXX entries */
#define CRT2_TV                 0x00000004
#define CRT2_VGA                0x00000008
#define TV_NTSC                 0x00000010
#define TV_PAL                  0x00000020
#define TV_HIVISION             0x00000040
#define TV_YPBPR                0x00000080
#define TV_AVIDEO               0x00000100
#define TV_SVIDEO               0x00000200
#define TV_SCART                0x00000400
#define VB_CONEXANT		0x00000800   /* 661 series only */
#define VB_TRUMPION		VB_CONEXANT  /* 300 series only */
#define TV_PALM                 0x00001000
#define TV_PALN                 0x00002000
#define TV_NTSCJ		0x00001000
#define VB_302ELV		0x00004000
#define TV_CHSCART              0x00008000
#define TV_CHYPBPR525I          0x00010000
#define CRT1_VGA		0x00000000
#define CRT1_LCDA		0x00020000
#define VGA2_CONNECTED          0x00040000
#define DISPTYPE_CRT1		0x00080000  	/* CRT1 connected and used */

#define SINGLE_MODE             0x20000000   	/* CRT1 or CRT2; determined by DISPTYPE_CRTx */
#define MIRROR_MODE		0x40000000   	/* CRT1 + CRT2 identical (mirror mode) */
#define DUALVIEW_MODE		0x80000000   	/* CRT1 + CRT2 independent (dual head mode) */

/* Aliases: */
#define CRT2_ENABLE		(CRT2_LCD | CRT2_TV | CRT2_VGA)
#define TV_STANDARD             (TV_NTSC | TV_PAL | TV_PALM | TV_PALN | TV_NTSCJ)
#define TV_INTERFACE            (TV_AVIDEO|TV_SVIDEO|TV_SCART|TV_HIVISION|TV_YPBPR)

/* Only if TV_YPBPR is set: */
#define TV_YPBPR525I		TV_NTSC
#define TV_YPBPR525P		TV_PAL
#define TV_YPBPR750P		TV_PALM
#define TV_YPBPR1080I	        TV_PALN
#define TV_YPBPRALL 		(TV_YPBPR525I | TV_YPBPR525P | TV_YPBPR750P | TV_YPBPR1080I)

#define TV_YPBPR43LB		TV_CHSCART
#define TV_YPBPR43		TV_CHYPBPR525I
#define TV_YPBPR169 		(TV_CHSCART | TV_CHYPBPR525I)
#define TV_YPBPRAR              (TV_CHSCART | TV_CHYPBPR525I)


#define DISPTYPE_DISP2		CRT2_ENABLE
#define DISPTYPE_DISP1		DISPTYPE_CRT1
#define VB_DISPMODE_SINGLE	SINGLE_MODE  	/* alias */
#define VB_DISPMODE_MIRROR	MIRROR_MODE  	/* alias */
#define VB_DISPMODE_DUAL	DUALVIEW_MODE 	/* alias */
#define DISPLAY_MODE            (SINGLE_MODE | MIRROR_MODE | DUALVIEW_MODE)

/* PresetMode argument */
#define XGI_MODE_SIMU 		0
#define XGI_MODE_CRT1 		1
#define XGI_MODE_CRT2 		2

/* pXGI->MiscFlags */
#define MISC_CRT1OVERLAY	0x00000001  /* Current display mode supports overlay */
#define MISC_PANELLINKSCALER    0x00000002  /* Panel link is currently scaling */
#define MISC_CRT1OVERLAYGAMMA	0x00000004  /* Current display mode supports overlay gamma corr on CRT1 */
#define MISC_TVNTSC1024		0x00000008  /* Current display mode is TV NTSC/PALM/YPBPR525I 1024x768  */


#define HW_DEVICE_EXTENSION	XGI_HW_DEVICE_INFO

#define BITMASK(h,l)             (((unsigned)(1U << ((h)-(l)+1))-1)<<(l))
#define GENMASK(mask)            BITMASK(1?mask,0?mask)

typedef unsigned long ULong;
typedef unsigned short UShort;
typedef unsigned char UChar;


/* ChipFlags */
/* Use only lower 16 bit for chip id! (xgictrl) */
#define XGICF_LARGEOVERLAY  0x00000001
#define XGICF_Is651         0x00000002
#define XGICF_IsM650        0x00000004
#define XGICF_IsM652        0x00000008
#define XGICF_IsM653        0x00000010
#define XGICF_Is652         0x00000020
#define XGICF_Is65x         (XGICF_Is651|XGICF_IsM650|XGICF_IsM652|XGICF_IsM653|XGICF_Is652)
#define XGICF_IsM661        0x00000100  /* M661FX */
#define XGICF_IsM741        0x00000200
#define XGICF_IsM760        0x00000400
#define XGICF_IsM661M       0x00000800  /* M661MX */
#define XGICF_IsM66x        (XGICF_IsM661 | XGICF_IsM741 | XGICF_IsM760 | XGICF_IsM661M)
#define XGICF_315Core       0x00010000  /* 3D: Real 315 */
#define XGICF_Real256ECore  0x00020000  /* 3D: Similar to 315 core, no T&L? (65x, 661, 740, 741) */
#define XGICF_XabreCore     0x00040000  /* 3D: Real Xabre */
#define XGICF_Ultra256Core  0x00080000  /* 3D: Similar to Xabre, no T&L?, no P:Shader? (660, 760) */
#define XGICF_UseLCDA       0x01000000
#define XGICF_760UMA        0x10000000  /* 760: UMA active */
#define XGICF_CRT2HWCKaputt 0x20000000  /* CRT2 Mono HWCursor engine buggy */
#define XGICF_Glamour3      0x40000000
#define XGICF_Integrated    0x80000000

/* Direct Xv-API */
#define XGI_SD_IS300SERIES     0x00000001
#define XGI_SD_IS315SERIES     0x00000002
#define XGI_SD_IS330SERIES     0x00000004
#define XGI_SD_SUPPORTPALMN    0x00000008   /* tv chip supports pal-m, pal-n */
#define XGI_SD_SUPPORT2OVL     0x00000010   /* set = 2 overlays, clear = support SWITCHCRT xv prop */
#define XGI_SD_SUPPORTTVPOS    0x00000020   /* supports changing tv position */
#define XGI_SD_ISDUALHEAD      0x00000040   /* Driver is in dual head mode */
#define XGI_SD_ISMERGEDFB      0x00000080   /* Driver is in merged fb mode */
#define XGI_SD_ISDHSECONDHEAD  0x00000100   /* Dual head: This is CRT1 (=second head) */
#define XGI_SD_ISDHXINERAMA    0x00000200   /* Dual head: We are running Xinerama */
#define XGI_SD_VBHASSCART      0x00000400   /* videobridge has SCART instead of VGA2 */
#define XGI_SD_ISDEPTH8        0x00000800   /* Depth is 8, no independent gamma correction */
#define XGI_SD_ENABLED         0x00002000   /* xgictrl is enabled (by option) */
#define XGI_SD_PSEUDOXINERAMA  0x00004000   /* pseudo xinerama is active */
#define XGI_SD_SUPPORTLCDA     0x00008000   /* Support LCD Channel A */
#define XGI_SD_SUPPORTNTSCJ    0x00010000   /* tv chip supports ntsc-j */
#define XGI_SD_ADDLSUPFLAG     0x00020000   /* 1 = the following flags are valid */
#define XGI_SD_SUPPORTVGA2     0x00040000   /* CRT2=VGA supported */
#define XGI_SD_SUPPORTSCART    0x00080000   /* CRT2=SCART supported */
#define XGI_SD_SUPPORTOVERSCAN 0x00100000   /* Overscan flag supported */
#define XGI_SD_SUPPORTXVGAMMA1 0x00200000   /* Xv Gamma correction for CRT1 supported */
#define XGI_SD_SUPPORTTV       0x00400000   /* CRT2=TV supported */
#define XGI_SD_SUPPORTYPBPR    0x00800000   /* CRT2=YPbPr (525i, 525p, 750p, 1080i) is supported */
#define XGI_SD_SUPPORTHIVISION 0x01000000   /* CRT2=HiVision is supported */
#define XGI_SD_SUPPORTYPBPRAR  0x02000000   /* YPbPr aspect ratio is supported */
#define XGI_SD_SUPPORTSCALE    0x04000000   /* Scaling of LCD panel supported */
#define XGI_SD_SUPPORTCENTER   0x08000000   /* If scaling supported: Centering of screen [NOT] supported (TMDS only) */

#define XGI_DIRECTKEY         0x03145792

/* XGICtrl: Check mode for CRT2 */
#define XGI_CF2_LCD          0x01
#define XGI_CF2_TV           0x02
#define XGI_CF2_VGA2         0x04
#define XGI_CF2_TVPAL        0x08
#define XGI_CF2_TVNTSC       0x10  /* + NTSC-J */
#define XGI_CF2_TVPALM       0x20
#define XGI_CF2_TVPALN       0x40
#define XGI_CF2_CRT1LCDA     0x80
#define XGI_CF2_TYPEMASK     (XGI_CF2_LCD | XGI_CF2_TV | XGI_CF2_VGA2 | XGI_CF2_CRT1LCDA)
#define XGI_CF2_TVSPECIAL    (XGI_CF2_LCD | XGI_CF2_TV)
#define XGI_CF2_TVSPECMASK   (XGI_CF2_TVPAL | XGI_CF2_TVNTSC | XGI_CF2_TVPALM | XGI_CF2_TVPALN)
#define XGI_CF2_TVHIVISION   XGI_CF2_TVPAL
#define XGI_CF2_TVYPBPR525I  XGI_CF2_TVNTSC
#define XGI_CF2_TVYPBPR525P  (XGI_CF2_TVPAL | XGI_CF2_TVNTSC)
#define XGI_CF2_TVYPBPR750P  XGI_CF2_TVPALM
#define XGI_CF2_TVYPBPR1080I (XGI_CF2_TVPALM | XGI_CF2_TVPAL)

/* AGP stuff for DRI */
#define AGP_PAGE_SIZE 4096
#define AGP_PAGES     2048	 /* Default: 2048 pages @ 4096 = 8MB */
/* 300 */
#define AGP_CMDBUF_PAGES 256
#define AGP_CMDBUF_SIZE (AGP_PAGE_SIZE * AGP_CMDBUF_PAGES)
/* 315/330 */
#define AGP_VTXBUF_PAGES 512
#define AGP_VTXBUF_SIZE (AGP_PAGE_SIZE * AGP_VTXBUF_PAGES)

#define VOLARI_CQSIZE   (1024*1024)
#define VOLARI_CQSIZEXG20   (128*1024)
#define VOLARI_CURSOR_SHAPE_SIZE   (64*64*4)

/* For backup of register contents */
typedef struct {
    unsigned char xgiRegs3C4[0x50];
    unsigned char xgiRegs3D4[0x90];
    unsigned char xgiRegs3C2;
    unsigned char xgiCapt[0x60];
    unsigned char xgiVid[0x50];
    unsigned char VBPart1[0x50];
    unsigned char VBPart2[0x100];
    unsigned char VBPart3[0x50];
    unsigned char VBPart4[0x50];
    unsigned short ch70xx[64];
    unsigned long xgiMMIO85C0;
    unsigned char xgi6326tv[0x46];
    unsigned long xgiRegsPCI50, xgiRegsPCIA0;
} XGIRegRec, *XGIRegPtr;


/* XGIFBLayout is mainly there because of DGA. It holds the
 * current layout parameters needed for acceleration and other
 * stuff. When switching mode using DGA, these are set up
 * accordingly and not necessarily match pScrn's. Therefore,
 * driver modules should read these values instead of pScrn's.
 */
typedef struct {
    int                bitsPerPixel;   	/* = pScrn->bitsPerPixel */
    int                depth;		/* = pScrn->depth */
    int                displayWidth;	/* = pScrn->displayWidth */
    DisplayModePtr     mode;		/* = pScrn->currentMode */
} XGIFBLayout;

/* Dual head private entity structure */
typedef struct {
    ScrnInfoPtr         pScrn_1;
    ScrnInfoPtr         pScrn_2;
    unsigned char *     BIOS;
    VB_DEVICE_INFO     *XGI_Pr;
    int 		CRT2ModeNo;		/* Current display mode for CRT2 */
    Bool		CRT2ModeSet;		/* CRT2 mode has been set */
    unsigned char	CRT2CR30, CRT2CR31, CRT2CR35, CRT2CR38;
    int			refCount;
    
    /**
     * Number of entities
     *
     * \bug
     * This field is tested in one place, but it doesn't appear to ever be
     * set or modified.
     */
    int 		lastInstance;

    Bool		DisableDual;		/* Emergency flag */
    Bool		ErrorAfterFirst;	/* Emergency flag: Error after first init -> Abort second */
    int                 maxUsedClock;  		/* Max used pixelclock on master head */
    
    /**
     * Framebuffer addresses and sizes
     *
     * \bug
     * These 4 fields are set, but the stored values don't appear to be used.
     */
    unsigned long       masterFbAddress;
    unsigned long	masterFbSize;
    unsigned long       slaveFbAddress;
    unsigned long	slaveFbSize;

    unsigned char *     FbBase;         	/* VRAM linear address */
    unsigned char *     IOBase;         	/* MMIO linear address */

    /**
     * Map / unmap queue counter.
     *
     * \bug
     * These vales are tested, set to zero, or decremented.  However, I don't
     * see anywhere in the code where they are incremented.
     */
    unsigned short      MapCountIOBase;
    unsigned short      MapCountFbBase;

    Bool 		forceUnmapIOBase;	/* ignore counter and unmap */
    Bool		forceUnmapFbBase;	/* ignore counter and unmap */
#ifdef __alpha__
    unsigned char *     IOBaseDense;    	/* MMIO for Alpha platform */
    unsigned short      MapCountIOBaseDense;
    Bool		forceUnmapIOBaseDense;  /* ignore counter and unmap */
#endif
    BOOLEAN		CRT1gamma;
    
    /**
     * \bug This field is tested and set to \c NULL but never used.
     */
    unsigned char       *RenderAccelArray;
    unsigned char *	FbBase1;
    unsigned long	OnScreenSize1;

#ifdef XGI_CP
    XGI_CP_H_ENT
#endif
} XGIEntRec, *XGIEntPtr;

#define XGIPTR(p)       ((XGIPtr)((p)->driverPrivate))
#define XAAPTR(p)       ((XAAInfoRecPtr)(XGIPTR(p)->AccelInfoPtr))

#define ExtRegSize    0x40


/* Relative merge position */
typedef enum {
   xgiLeftOf,
   xgiRightOf,
   xgiAbove,
   xgiBelow,
   xgiClone
} XGIScrn2Rel;

typedef struct MonitorRange {
	float loH,hiH,loV,hiV ;
}MonitorRangeRec,*MonitorRangePtr ; 

typedef struct {
    ScrnInfoPtr         pScrn;		/* -------------- DON'T INSERT ANYTHING HERE --------------- */
#ifdef XSERVER_LIBPCIACCESS
    struct pci_device * PciInfo;
#else 
    pciVideoPtr         PciInfo;	/* -------- OTHERWISE xgi_dri.so MUST BE RECOMPILED -------- */
    PCITAG              PciTag;
#endif
    EntityInfoPtr       pEnt;
    int                 Chipset;
    int                 ChipRev;
    VB_DEVICE_INFO     *XGI_Pr;         /* For new mode switching code */
    unsigned long       FbAddress;      /* VRAM physical address (in DHM: for each Fb!) */
    unsigned long       realFbAddress;  /* For DHM/PCI mem mapping: store global FBAddress */
    unsigned char *     FbBase;         /* VRAM virtual linear address */
    CARD32              IOAddress;      /* MMIO physical address */
    unsigned char *     IOBase;         /* MMIO linear address */
    IOADDRESS           IODBase;        /* Base of PIO memory area */
#ifdef __alpha__
    unsigned char *     IOBaseDense;    /* MMIO for Alpha platform */
#endif
    XGIIOADDRESS        RelIO;          /* Relocated IO Ports baseaddress */
    unsigned char *     BIOS;
    int                 MemClock;
    int                 BusWidth;
    int                 MinClock;
    int                 MaxClock;
    int                 Flags;          /* HW config flags */
    long                FbMapSize;	/* Used for Mem Mapping - DON'T CHANGE THIS */
    long                availMem;       /* Really available Fb mem (minus TQ, HWCursor) */
    unsigned long	maxxfbmem;      /* limit fb memory X is to use to this (KB) */
    unsigned long       xgifbMem;       /* heapstart of xgifb (if running) */
    unsigned long	dhmOffset;	/* Offset to memory for each head (0 or ..) */
    DGAModePtr          DGAModes;
    int                 numDGAModes;
    Bool                DGAactive;
    Bool                NoAccel;
    Bool				useEXA;  /* Jong 01/13/2009; support EXA */
    Bool                NoXvideo;
    Bool                TurboQueue;
    int                 ForceCRT1Type;
    int                 ForceCRT2Type;
    int                 OptROMUsage;
    Bool                ValidWidth;
    unsigned char       myCR63;
    unsigned long   	VBFlags;		/* Video bridge configuration */
    unsigned long       VBFlags_backup;         /* Backup for SlaveMode-modes */
    
    short               scrnOffset;		/* Screen pitch (data) */
    short               scrnPitch;		/* Screen pitch (display; regarding interlace) */
    unsigned long       DstColor;
    int                 xcurrent;               /* for temp use in accel */
    int                 ycurrent;               /* for temp use in accel */
    int                 CommandReg;

    Bool                HWCursor;
    CARD16		        CursorSize;  		/* Size of HWCursor area (bytes) */
    xf86CursorInfoPtr   CursorInfoPtr;
    unsigned            CursorOffset;

    /**
     * \bug This field is set to \c FALSE but never used.
     */
    Bool                DoColorExpand;

    XGIRegRec           SavedReg;
    XGIRegRec           ModeReg;

#ifdef XGI_USE_XAA
    XAAInfoRecPtr       AccelInfoPtr;
#endif
#ifdef XGI_USE_EXA /* Jong 01/13/2009; support EXA */
    ExaDriverPtr		EXADriverPtr;
    int			fillPitch, fillBpp;
    CARD32		fillDstBase;
    int			copyBpp;
    int			copySPitch, copyDPitch;
    CARD32		copySrcBase, copyDstBase;
    int			copyXdir, copyYdir;
    ExaOffscreenArea*	exa_scratch;
    unsigned int 		exa_scratch_next;
#endif

    CloseScreenProcPtr  CloseScreen;
    Bool        	(*ModeInit)(ScrnInfoPtr pScrn, DisplayModePtr mode);
    void        	(*XGISave)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*XGISave2)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*XGISave3)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*XGIRestore)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*XGIRestore2)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*XGIRestore3)(ScrnInfoPtr pScrn, XGIRegPtr xgireg);
    void        	(*LoadCRT2Palette)(ScrnInfoPtr pScrn, int numColors,
                		int *indicies, LOCO *colors, VisualPtr pVisual);

    int       		cmdQueueLen;		/* Current cmdQueueLength (for 2D and 3D) */
    unsigned long	cmdQueueLenMax;
    unsigned long	cmdQueueLenMin;
    unsigned char	*cmdQueueBase;
    int			*cmdQueueLenPtr;	/* Ptr to variable holding the current queue length */
    unsigned int        cmdQueueOffset;
    unsigned int        cmdQueueSize;
    unsigned long       cmdQueueSizeMask;

    /**
     * \bug This field is set but never used.
     */
    unsigned int        agpWantedPages;

#ifdef XF86DRI
    unsigned long 	agpHandle;
    unsigned long	agpAddr;
    unsigned char 	*agpBase;
    unsigned int 	agpSize;
    unsigned long	agpVtxBufAddr;	/* 315 series */
    unsigned char       *agpVtxBufBase;
    unsigned int        agpVtxBufSize;
    unsigned int        agpVtxBufFree;
    xgiRegion 		agp;
    Bool 		irqEnabled;
    int 		irq;
#endif
    unsigned long	DRIheapstart, DRIheapend;

    void		(*RenderCallback)(ScrnInfoPtr);

    /**
     * \bug This field is tested and set to \c NULL but never used.
     */
    unsigned char       *RenderAccelArray;

    /**
     * \bug This field is to \c TRUE but never used.
     */
    Bool		doRender;

    int 		PerColorExpandBufferSize;
    int 		ColorExpandBufferNumber;
    unsigned char 	*ColorExpandBufferAddr[32];
    int 		ColorExpandBufferScreenOffset[32];

    /**
     * \bug This field is read but never initialized.
     */
    int 		ImageWriteBufferSize;

    unsigned char 	*ImageWriteBufferAddr;

    int 		Rotate;

    BOOLEAN		HaveCustomModes; /* Jong 07/27/2009; support customer modes */

    /* ShadowFB support */
    Bool 		ShadowFB;
    unsigned char 	*ShadowPtr;
    int  		ShadowPitch;

    /**
     * \bug This field is set but never used.
     */
    Bool		loadDRI;

#ifdef XF86DRI
    Bool 		directRenderingEnabled;
    DRIInfoPtr 		pDRIInfo;
    int 		drmSubFD;
    int 		numVisualConfigs;
    __GLXvisualConfig* 	pVisualConfigs;
    XGIConfigPrivPtr 	pVisualConfigsPriv;
#endif

    HW_DEVICE_EXTENSION xgi_HwDevExt;      /* For new mode switching code */
    XF86VideoAdaptorPtr adaptor;
    ScreenBlockHandlerProcPtr BlockHandler;

    /**
     * \bug This field is tested and used but never set.
     */
    void                (*VideoTimerCallback)(ScrnInfoPtr, Time);

    void		(*ResetXv)(ScrnInfoPtr);
    void		(*ResetXvGamma)(ScrnInfoPtr);

    OptionInfoPtr 	Options;

    /**
     * \bug This field is used but never initialized.
     */
    unsigned char 	LCDon;
    Bool 		Blank;
    int 		CRT1off;		/* 1=CRT1 off, 0=CRT1 on */
    CARD16 		LCDheight;		/* Vertical resolution of LCD panel */
    CARD16 		LCDwidth;		/* Horizontal resolution of LCD panel */
    vbeInfoPtr 		pVbe;			/* For VESA mode switching */
    UCHAR ScratchSet[16];
    MonitorRangeRec CRT1Range,CRT2Range;

#ifdef XGIDUALHEAD
    BOOL 		DualHeadMode;		/* TRUE if we use dual head mode */
    BOOL 		SecondHead;		/* TRUE is this is the second head */
    XGIEntPtr 		entityPrivate;		/* Ptr to private entity (see above) */
#endif
    XGIFBLayout         CurrentLayout;		/* Current framebuffer layout */
    BOOL		Primary;		/* Display adapter is primary */
    xf86Int10InfoPtr    pInt;			/* Our int10 */
    
    /**
     * Use our own default modes? 
     *
     * \bug This field is set but never used.
     */
    Bool		noInternalModes;

    int			ForceTVType, SenseYPbPr;
    int                 NonDefaultPAL, NonDefaultNTSC;
    unsigned long	ForceYPbPrType, ForceYPbPrAR;
    unsigned long       lockcalls;		/* Count unlock calls for debug */

    Atom                xvBrightness, xvContrast, xvColorKey, xvHue, xvSaturation;
    Atom                xvAutopaintColorKey, xvSetDefaults, xvSwitchCRT;
    Atom		xvDisableGfx, xvDisableGfxLR, xvTVXPosition, xvTVYPosition;
    Atom		xvDisableColorkey, xvUseChromakey, xvChromaMin, xvChromaMax;
    Atom		xvInsideChromakey, xvYUVChromakey;
    Atom		xvGammaRed, xvGammaGreen, xvGammaBlue;
#ifdef XGI_CP
    XGI_CP_H
#endif
    unsigned long       ChipFlags;
    unsigned long       XGI_SD_Flags;
    BOOLEAN		UseHWARGBCursor;
    BOOLEAN		HWARGBCursor;
    int                 vb;
    BOOLEAN		restorebyset;
    BOOLEAN		nocrt2ddcdetection;
    BOOLEAN		forcecrt2redetection;
    BOOLEAN		CRT1gamma, CRT1gammaGiven, CRT2gamma, XvGamma, XvGammaGiven;
    int			XvDefCon, XvDefBri, XvDefHue, XvDefSat;
    BOOLEAN		XvDefDisableGfx, XvDefDisableGfxLR;
    BOOLEAN		XvUseMemcpy;
    int			XvGammaRed, XvGammaGreen, XvGammaBlue;
    CARD8		XvGammaRampRed[256], XvGammaRampGreen[256], XvGammaRampBlue[256];
    BOOLEAN		disablecolorkeycurrent;
    CARD32		colorKey;
    CARD32		MiscFlags;

#ifdef XGI_USE_XAA
    FBLinearPtr		AccelLinearScratch;
#endif

    float		zClearVal;
    unsigned long	bClrColor, dwColor;
    int			AllowHotkey;
    BOOLEAN		enablexgictrl;
    short		Video_MaxWidth, Video_MaxHeight;
    short               scrnPitch2;
    int	                CurXPreset ;
    int                 CurYPreset ;
    unsigned long	mmioSize;
#ifdef XGIMERGED
    Bool		MergedFB, MergedFBAuto;
    XGIScrn2Rel		CRT2Position;
    char *		CRT2HSync;
    char *		CRT2VRefresh;
    char *		MetaModes;
    ScrnInfoPtr		CRT2pScrn;
    DisplayModePtr	CRT1Modes;
    DisplayModePtr	CRT1CurrentMode;
    int			CRT1frameX0;
    int			CRT1frameY0;
    int			CRT1frameX1;
    int			CRT1frameY1;
    Bool		CheckForCRT2;
    int			MergedFBXDPI, MergedFBYDPI;
#ifdef XGIXINERAMA
    Bool		UsexgiXinerama;
    Bool		CRT2IsScrn0;
    ExtensionEntry 	*XineramaExtEntry;
    int			xgiXineramaVX, xgiXineramaVY;
    Bool		AtLeastOneNonClone;
#endif
#endif

    /* Added for 3D */
    unsigned long        cmdQueue_shareWP_only2D;
    unsigned long        *pCQ_shareWritePort;
    void                (*SetThreshold)(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                        unsigned short *Low, unsigned short *High);

    XGI_DSReg           SRList[ExtRegSize] ;
    XGI_DSReg           CRList[ExtRegSize] ;
    
    /**
     * Total number of iterations to wait in \c Volari_Idle.
     */
    unsigned int        idle_wait_count;

//:::: for capture
    Bool		v4l_videoin;
    int			v4l_devnum;	/* v4l device number, 0,1,2....*/
//~::::    

	int			TargetRefreshRate; 
	Bool		IgnoreDDC; 

	Bool		Non_DDC_DefaultMode;
	int			Non_DDC_DefaultResolutionX ;
	int			Non_DDC_DefaultResolutionY ;
	int			Non_DDC_DefaultRefreshRate ;
} XGIRec, *XGIPtr;

#ifdef XGIDUALHEAD
# define IS_DUAL_HEAD(x)      ((x)->DualHeadMode)
# define IS_SECOND_HEAD(x)    ((x)->SecondHead)
# define ENTITY_PRIVATE(x)    ((x)->entityPrivate)
#else
# define IS_DUAL_HEAD(x)      FALSE
# define IS_SECOND_HEAD(x)    FALSE
# define ENTITY_PRIVATE(x)    NULL
#endif


#define SEQ_ADDRESS_PORT  0x0014
#define MISC_OUTPUT_REG_WRITE_PORT  0x0012
#define MISC_OUTPUT_REG_READ_PORT   0x001C
#define GRAPH_ADDRESS_PORT  0x001E
#define VIDEO_SUBSYSTEM_ENABLE_PORT 0x0013
#define CRTC_ADDRESS_PORT_COLOR  0x0024
#define PCI_COMMAND  0x04

#define SDMPTR(x) ((XGIMergedDisplayModePtr)(x->currentMode->Private))
#define CDMPTR    ((XGIMergedDisplayModePtr)(pXGI->CurrentLayout.mode->Private))

#define BOUND(test,low,hi) { \
    if(test < low) test = low; \
    if(test > hi) test = hi; }

#define REBOUND(low,hi,test) { \
    if(test < low) { \
        hi += test-low; \
        low = test; } \
    if(test > hi) { \
        low += test-hi; \
        hi = test; } }

typedef struct _MergedDisplayModeRec {
    DisplayModePtr CRT1;
    DisplayModePtr CRT2;
    XGIScrn2Rel    CRT2Position;
} XGIMergedDisplayModeRec, *XGIMergedDisplayModePtr;


typedef struct _region {
    int x0,x1,y0,y1;
} region;


extern void  xgiOptions(ScrnInfoPtr pScrn);
extern const OptionInfoRec * XGIAvailableOptions(int chipid, int busid);
extern void  XGISetup(ScrnInfoPtr pScrn);
extern void  XGIVGAPreInit(ScrnInfoPtr pScrn);
extern Bool  XGIAccelInit(ScreenPtr pScreen);
extern Bool  XGIHWCursorInit(ScreenPtr pScreen);
extern Bool  XGIDGAInit(ScreenPtr pScreen);
extern void  XGIInitVideo(ScreenPtr pScreen);

extern int   XGI_GetCHTVlumabandwidthcvbs(ScrnInfoPtr pScrn);

int    XG40Mclk(XGIPtr pXGI);

void XGINew_InitVBIOSData(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo) ;
int compute_vclk(int Clock, int *out_n, int *out_dn, int *out_div,
                 int *out_sbit, int *out_scale);

void XGI_WaitBeginRetrace(XGIIOADDRESS RelIO);
void XGI_WaitEndRetrace(XGIIOADDRESS RelIO);

/* 2005/11/21 added by jjtseng */
#define DelayS(sec) usleep((sec)*1000000)
#define DelayMS(millisec) usleep((millisec)*1000)
#define DelayUS(microsec) usleep((microsec))
/*~jjtseng 2005/11/21 */

Bool Volari_AccelInit(ScreenPtr pScreen) ;
/* void XGI_UnLockCRT2(PXGI_HW_DEVICE_INFO,USHORT BaseAddr); */
/* void XGI_LockCRT2(PXGI_HW_DEVICE_INFO,USHORT BaseAddr); */
/* void XGI_DisableBridge(PXGI_HW_DEVICE_INFO,USHORT BaseAddr); */
/* void XGI_EnableBridge(PXGI_HW_DEVICE_INFO,USHORT BaseAddr); */
#endif

extern USHORT XGI_GetModeID(ULONG VBFlags, int HDisplay, int VDisplay,
    int Depth, int LCDwith, int LCDheight);

extern BOOLEAN XGI_SearchModeID(const XGI_StStruct *SModeIDTable, 
    const XGI_ExtStruct *EModeIDTable, unsigned char VGAINFO,
    USHORT *ModeNo, USHORT *ModeIdIndex);

extern UCHAR XGI_GetModePtr(const XGI_StStruct *SModeIDTable,
    unsigned ModeType, USHORT ModeNo, USHORT ModeIdIndex);

extern void XGI_SetReg(XGIIOADDRESS port, USHORT index, USHORT data);
extern void XGI_SetRegByte(XGIIOADDRESS port, USHORT data);
extern void XGI_SetRegShort(XGIIOADDRESS port, USHORT data);
extern void XGI_SetRegLong(XGIIOADDRESS port, ULONG data);
extern UCHAR XGI_GetReg(XGIIOADDRESS port, USHORT index);
extern UCHAR XGI_GetRegByte(XGIIOADDRESS port);
extern USHORT XGI_GetRegShort(XGIIOADDRESS port);
extern ULONG XGI_GetRegLong(XGIIOADDRESS port);
extern void XGI_SetRegANDOR(XGIIOADDRESS Port, USHORT Index, USHORT DataAND,
			    USHORT DataOR);
extern void XGI_SetRegAND(XGIIOADDRESS Port, USHORT Index, USHORT DataAND);
extern void XGI_SetRegOR(XGIIOADDRESS Port, USHORT Index, USHORT DataOR);

#ifndef __NetBSD__
#define uint8_t	CARD8
#endif

extern void XGI_WriteDAC(XGIIOADDRESS dac_data, unsigned shift,
    unsigned ordering, uint8_t red, uint8_t green, uint8_t blue);

#ifdef DEBUG
void XGIDumpRegs(ScrnInfoPtr pScrn);

/**
 * Write value to the PC's 0x80 debug port.
 * 
 * \bug
 * I'm pretty sure the debug 0x80 only exists on PCs.  Should this routine
 * be a no-op on non-x86 and non-x86-64 architectures?
 */
#define Newdebugcode(dbg_code) outb(0x80, dbg_code)
#else
#define Newdebugcode(dbg_code)
#endif
