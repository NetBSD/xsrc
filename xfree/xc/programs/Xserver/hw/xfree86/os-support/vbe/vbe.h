/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/vbe/vbe.h,v 1.4 2000/04/20 21:28:50 tsi Exp $ */

#ifndef _VBE_H
#define _VBE_H
#include "xf86int10.h"
#include "xf86DDC.h"

typedef enum {
    DDC_UNCHECKED,
    DDC_NONE,
    DDC_1,
    DDC_2,
    DDC_1_2
}
ddc_lvl;

typedef struct {
    xf86Int10InfoPtr pInt10;
    int version;
    pointer memory;
    int real_mode_base;
    int num_pages;
    Bool init_int10;
    ddc_lvl ddc;
    Bool ddc_blank;
} vbeInfoRec, *vbeInfoPtr;

vbeInfoPtr VBEInit(xf86Int10InfoPtr pInt, int entityIndex);
void vbeFree(vbeInfoPtr pVbe);
xf86MonPtr vbeDoEDID(vbeInfoPtr pVbe, pointer pDDCModule);

#pragma pack(1)

typedef struct vbeControllerInfoBlock {
    CARD8 VbeSignature[4];
    CARD16 VbeVersion;
    CARD32 OemStringPtr;
    CARD8 Capabilities[4];
    CARD32 VideoModePtr;
    CARD16 TotalMem;
    CARD16 OemSoftwareRev;
    CARD32 OemVendorNamePtr;
    CARD32 OemProductNamePtr;
    CARD32 OemProductRevPtr;
    CARD8  Scratch[222];
    CARD8  OemData[256];
} vbeControllerInfoRec, *vbeControllerInfoPtr;

#pragma pack()
#endif
