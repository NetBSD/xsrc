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
 
#ifndef  _VBEXT_
#define  _VBEXT_

#include "xf86.h" /* Jong@08252009; for ScrnInfoPtr */

/* Jong 10/04/2007; merge code */
struct DWORDREGS {
    ULONG    Eax, Ebx, Ecx, Edx, Esi, Edi, Ebp;
};

/* Jong 10/04/2007; merge code */
struct WORDREGS {
    USHORT    ax, hi_ax, bx, hi_bx, cx, hi_cx, dx, hi_dx, si, hi_si, di ,hi_di, bp, hi_bp;
};

/* Jong 10/04/2007; merge code */
struct BYTEREGS {
    UCHAR   al, ah, hi_al, hi_ah, bl, bh, hi_bl, hi_bh, cl, ch, hi_cl, hi_ch, dl, dh, hi_dl, hi_dh;
};

/* Jong 10/04/2007; merge code */
typedef union   _X86_REGS    {
    struct  DWORDREGS e;    
    struct  WORDREGS x;
    struct  BYTEREGS h;
} X86_REGS, *PX86_REGS;

extern void XGI_GetSenseStatus(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    PVB_DEVICE_INFO pVBInfo);

/* Jong 10/04/2007; merge code */
extern   void     XGIInitMiscVBInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo); /* Jong@08212009 */
extern   void     XGISetDPMS(ScrnInfoPtr pScrn, PVB_DEVICE_INFO pVBInfo, PXGI_HW_DEVICE_INFO pXGIHWDE , ULONG VESA_POWER_STATE ) ; /* Jong@08212009 */
extern   void     XGINew_SetModeScratch ( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo ) ;
extern   void 	  ReadVBIOSTablData( UCHAR ChipType , PVB_DEVICE_INFO pVBInfo);
extern   USHORT   XGINew_SenseLCD(PXGI_HW_DEVICE_INFO,PVB_DEVICE_INFO pVBInfo);

#endif
