/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Authors: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vesa/vesa.h,v 1.5 2000/12/01 19:56:01 paulo Exp $
 */

#ifndef _VESA_H_
#define _VESA_H_

/* All drivers should typically include these */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"

/* All drivers need this */
#include "xf86_ansic.h"

#include "compiler.h"

/* Drivers for PCI hardware need this */
#include "xf86PciInfo.h"

#include "vgaHW.h"

/* Drivers that need to access the PCI config space directly need this */
#include "xf86Pci.h"

/* VBE/DDC support */
#include "vbe.h"
#include "xf86DDC.h"

/* ShadowFB support */
#include "shadow.h"

/* Int 10 support */
#include "xf86int10.h"

/* bank switching */
#include "mibank.h"

/* Dga definitions */
#include "dgaproc.h"

#ifdef RENDER
#include "picturestr.h"
#endif

#include "xf86Resources.h"
#include "xf86RAC.h"

#include "xf1bpp.h"
#include "xf4bpp.h"
#include "fb.h"
#include "afb.h"

#define VESA_VERSION		4000
#define VESA_NAME		"VESA"
#define VESA_DRIVER_NAME	"vesa"
#define VESA_MAJOR_VERSION	1
#define VESA_MINOR_VERSION	0
#define VESA_PATCHLEVEL		0

typedef struct _VBEInfoBlock VBEInfoBlock;
typedef struct _ModeInfoBlock ModeInfoBlock;
typedef struct _CRTCInfoBlock CRTCInfoBlock;

typedef struct _VESARec
{
    xf86Int10InfoPtr pInt;
    EntityInfoPtr pEnt;
    CARD16 major, minor;
    VBEInfoBlock *vbeInfo;
    GDevPtr device;
    pciVideoPtr pciInfo;
    PCITAG pciTag;
    miBankInfoRec bank;
    int curBank, bankSwitchWindowB;
    CARD16 maxBytesPerScanline;
    int mapPhys, mapOff, mapSize;	/* video memory */
    void *base, *VGAbase;
    CARD8 *state, *pstate;	/* SVGA state */
    int statePage, stateSize, stateMode;
    int page;
    CARD8 *block;
    int pix24bpp;
    CARD32 *pal, *savedPal;
    CARD8 *fonts;
    xf86MonPtr monitor;
    Bool shadowFB, primary;
    CARD8 *shadowPtr;
    CARD32 windowAoffset;
    /* DGA info */
    DGAModePtr pDGAMode;
    int nDGAMode;
} VESARec, *VESAPtr;

#define FARP(p)		(((unsigned)(p & 0xffff0000) >> 12) | (p & 0xffff))

#ifndef __GNUC__
#define __attribute__(a)
#endif

typedef struct _ModeInfoData {
    int mode;
    ModeInfoBlock *data;
    CRTCInfoBlock *block;
} ModeInfoData;

/*
 * INT 0
 */
struct _VBEInfoBlock {
    /* VESA 1.2 fields */
    CARD8 VESASignature[4];		/* VESA */
    CARD16 VESAVersion;			/* Higher byte major, lower byte minor */
    /*CARD32*/char *OEMStringPtr;	/* Pointer to OEM string */
    CARD8 Capabilities[4];		/* Capabilities of the video environment */

    /*CARD32*/CARD16 *VideoModePtr;	/* pointer to supported Super VGA modes */

    CARD16 TotalMemory;			/* Number of 64kb memory blocks on board */
    /* if not VESA 2, 236 scratch bytes follow (256 bytes total size) */

    /* VESA 2 fields */
    CARD16 OemSoftwareRev;		/* VBE implementation Software revision */
    /*CARD32*/char *OemVendorNamePtr;	/* Pointer to Vendor Name String */
    /*CARD32*/char *OemProductNamePtr;	/* Pointer to Product Name String */
    /*CARD32*/char *OemProductRevPtr;	/* Pointer to Product Revision String */
    CARD8 Reserved[222];		/* Reserved for VBE implementation */
    CARD8 OemData[256];			/* Data Area for OEM Strings */
} __attribute__((packed));

/* Return Super VGA Information */
VBEInfoBlock *VESAGetVBEInfo(ScrnInfoPtr pScrn);
void VESAFreeVBEInfo(VBEInfoBlock *block);

/*
 * INT 1
 */
struct _ModeInfoBlock {
    CARD16 ModeAttributes;		/* mode attributes */
    CARD8 WinAAttributes;		/* window A attributes */
    CARD8 WinBAttributes;		/* window B attributes */
    CARD16 WinGranularity;		/* window granularity */
    CARD16 WinSize;			/* window size */
    CARD16 WinASegment;			/* window A start segment */
    CARD16 WinBSegment;			/* window B start segment */
    CARD32 WinFuncPtr;			/* real mode pointer to window function */
    CARD16 BytesPerScanline;		/* bytes per scanline */

    /* Mandatory information for VBE 1.2 and above */
    CARD16 XResolution;			/* horizontal resolution in pixels or characters */
    CARD16 YResolution;			/* vertical resolution in pixels or characters */
    CARD8 XCharSize;			/* character cell width in pixels */
    CARD8 YCharSize;			/* character cell height in pixels */
    CARD8 NumberOfPlanes;		/* number of memory planes */
    CARD8 BitsPerPixel;			/* bits per pixel */
    CARD8 NumberOfBanks;		/* number of banks */
    CARD8 MemoryModel;			/* memory model type */
    CARD8 BankSize;			/* bank size in KB */
    CARD8 NumberOfImages;		/* number of images */
    CARD8 Reserved;	/* 1 */		/* reserved for page function */

    /* Direct color fields (required for direct/6 and YUV/7 memory models) */
    CARD8 RedMaskSize;			/* size of direct color red mask in bits */
    CARD8 RedFieldPosition;		/* bit position of lsb of red mask */
    CARD8 GreenMaskSize;		/* size of direct color green mask in bits */
    CARD8 GreenFieldPosition;		/* bit position of lsb of green mask */
    CARD8 BlueMaskSize;			/* size of direct color blue mask in bits */
    CARD8 BlueFieldPosition;		/* bit position of lsb of blue mask */
    CARD8 RsvdMaskSize;			/* size of direct color reserved mask in bits */
    CARD8 RsvdFieldPosition;		/* bit position of lsb of reserved mask */
    CARD8 DirectColorModeInfo;		/* direct color mode attributes */

    /* Mandatory information for VBE 2.0 and above */
    CARD32 PhysBasePtr;			/* physical address for flat memory frame buffer */
    CARD32 Reserved32;	/* 0 */		/* Reserved - always set to 0 */
    CARD16 Reserved16;	/* 0 */		/* Reserved - always set to 0 */

    /* Mandatory information for VBE 3.0 and above */
    CARD16 LinBytesPerScanLine;		/* bytes per scan line for linear modes */
    CARD8 BnkNumberOfImagePages;	/* number of images for banked modes */
    CARD8 LinNumberOfImagePages;	/* number of images for linear modes */
    CARD8 LinRedMaskSize;		/* size of direct color red mask (linear modes) */
    CARD8 LinRedFieldPosition;		/* bit position of lsb of red mask (linear modes) */
    CARD8 LinGreenMaskSize;		/* size of direct color green mask (linear modes) */
    CARD8 LinGreenFieldPosition;	/* bit position of lsb of green mask (linear modes) */
    CARD8 LinBlueMaskSize;		/* size of direct color blue mask (linear modes) */
    CARD8 LinBlueFieldPosition;		/* bit position of lsb of blue mask (linear modes) */
    CARD8 LinRsvdMaskSize;		/* size of direct color reserved mask (linear modes) */
    CARD8 LinRsvdFieldPosition;		/* bit position of lsb of reserved mask (linear modes) */
    CARD32 MaxPixelClock;		/* maximum pixel clock (in Hz) for graphics mode */
    CARD8 Reserved2[189];		/* remainder of ModeInfoBlock */
} __attribute__((packed));

/* Return VBE Mode Information */
ModeInfoBlock *VESAGetModeInfo(ScrnInfoPtr pScrn, int mode);
void VESAFreeModeInfo(ModeInfoBlock *block);

/*
 * INT2
 */
#define CRTC_DBLSCAN	(1<<0)
#define CRTC_INTERLACE	(1<<1)
#define CRTC_NHSYNC	(1<<2)
#define CRTC_NVSYNC	(1<<3)

struct _CRTCInfoBlock {
    CARD16 HorizontalTotal;		/* Horizontal total in pixels */
    CARD16 HorizontalSyncStart;		/* Horizontal sync start in pixels */
    CARD16 HorizontalSyncEnd;		/* Horizontal sync end in pixels */
    CARD16 VerticalTotal;		/* Vertical total in lines */
    CARD16 VerticalSyncStart;		/* Vertical sync start in lines */
    CARD16 VerticalSyncEnd;		/* Vertical sync end in lines */
    CARD8 Flags;			/* Flags (Interlaced, Double Scan etc) */
    CARD32 PixelClock;			/* Pixel clock in units of Hz */
    CARD16 RefreshRate;			/* Refresh rate in units of 0.01 Hz */
    CARD8 Reserved[40];			/* remainder of ModeInfoBlock */
} __attribute__((packed));
/* CRTCInfoBlock is in the VESA 3.0 specs */

Bool VESASetVBEMode(ScrnInfoPtr pScrn, int mode, CRTCInfoBlock *crtc);

/*
 * INT 3
 */
Bool VESAGetVBEMode(ScrnInfoPtr pScrn, int *mode);

/*
 * INT 4
 */
/* Save/Restore Super VGA video state */
/* function values are (values stored in VESAPtr):
 *	0 := query & allocate amount of memory to save state
 *	1 := save state
 *	2 := restore state
 *
 *	function 0 called automatically if function 1 called without
 *	a previous call to function 0.
 */
#define MODE_QUERY	0
#define MODE_SAVE	1
#define MODE_RESTORE	2
Bool VESASaveRestore(ScrnInfoPtr pScrn, int function);

/*
 * INT 6
 */
#define	SCANWID_SET		0
#define SCANWID_GET		1
#define SCANWID_SET_BYTES	2
#define SCANWID_GET_MAX		3
#define VESASetLogicalScanline(pScrn, width)	\
	VESASetGetLogicalScanlineLength(pScrn, SCANWID_SET, \
					width, NULL, NULL, NULL)
#define VESASetLogicalScanlineBytes(pScrn, width)	\
	VESASetGetLogicalScanlineLength(pScrn, width, SCANWID_SET_BYTES, \
					NULL, NULL, NULL)
#define VESAGetLogicalScanline(pScrn, pixels, bytes, max)	\
	VESASetGetLogicalScanlineLength(pScrn, SCANWID_GET, NULL, \
					pixels, bytes, max)
#define VESAGetMaxLogicalScanline(pScrn, pixels, bytes, max)	\
	VESASetGetLogicalScanlineLength(pScrn, SCANWID_GET_MAX, \
					NULL, pixels, bytes, max)
Bool VESASetGetLogicalScanlineLength(ScrnInfoPtr pScrn, int command, int width,
				     int *pixels, int *bytes, int *max);

/*
 * INT 7
 */
/* 16 bit code */
Bool VESASetDisplayStart(ScrnInfoPtr pScrn, int x, int y, Bool wait_retrace);
Bool VESAGetDisplayStart(ScrnInfoPtr pScrn, int *x, int *y);

/*
 * INT 8
 */
/* if bits is 0, then it is a GET */
int VESASetGetDACPaletteFormat(ScrnInfoPtr pScrn, int bits);

/*
 * INT 9
 */
/*
 *  If getting a palette, the data argument is not used. It will return
 * the data.
 *  If setting a palette, it will return the pointer received on success,
 * NULL on failure.
 */
CARD32 *VESASetGetPaletteData(ScrnInfoPtr pScrn, Bool set, int first, int num,
			     CARD32 *data, Bool secondary, Bool wait_retrace);
#define VESAFreePaletteData(data)	xfree(data)

/*
 * INT A
 */
typedef struct _VESApmi {
    int seg_tbl;
    int tbl_off;
    int tbl_len;
} VESApmi;

VESApmi *VESAGetVBEpmi(ScrnInfoPtr pScrn);
#define VESAFreeVBEpmi(pmi)	xfree(pmi)

#endif /* _VESA_H_ */
