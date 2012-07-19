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
 
#ifndef  _VBSETMODE_
#define  _VBSETMODE_

extern   void     InitTo330Pointer(UCHAR,PVB_DEVICE_INFO);
extern   void     XGI_UnLockCRT2(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_LockCRT2(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_LongWait( PVB_DEVICE_INFO );
extern   void     XGI_SetCRT2ModeRegs(USHORT ModeNo,PXGI_HW_DEVICE_INFO,  PVB_DEVICE_INFO  );
extern   void     XGI_DisableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void  	  XGI_EnableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_DisplayOff( PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_DisplayOn( PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_GetVBType(PVB_DEVICE_INFO);
extern   void     XGI_SenseCRT1(PVB_DEVICE_INFO );
extern   void     XGI_GetVGAType(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_GetVBInfo(USHORT ModeNo,USHORT ModeIdIndex,PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_GetTVInfo(USHORT ModeNo,USHORT ModeIdIndex, PVB_DEVICE_INFO );
extern   void     XGI_SetCRT1Offset(USHORT ModeNo,USHORT ModeIdIndex,USHORT RefreshRateTableIndex,PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );
extern   void     XGI_SetLCDAGroup(USHORT ModeNo,USHORT ModeIdIndex,PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO );

/* Jong 10/04/2007; merge code */
extern   USHORT   XGI_GetResInfo(USHORT ModeNo,USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo);

extern BOOLEAN XGISetModeNew(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    PVB_DEVICE_INFO pVBInfo, USHORT ModeNo);

extern   BOOLEAN  CheckDualChip(PVB_DEVICE_INFO );
extern   BOOLEAN  XGI_GetLCDInfo(USHORT ModeNo,USHORT ModeIdIndex,PVB_DEVICE_INFO );
extern   BOOLEAN  XGI_BridgeIsOn( PVB_DEVICE_INFO );
extern   BOOLEAN  XGI_SetCRT2Group301(USHORT ModeNo, PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO);
extern   USHORT   XGI_GetRatePtrCRT2( PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,USHORT ModeIdIndex, PVB_DEVICE_INFO );

extern USHORT XGI_GetColorDepth(USHORT ModeNo, USHORT ModeIdIndex,
    const VB_DEVICE_INFO *pVBInfo);

extern void XGI_SetSeqRegs(USHORT StandTableIndex,
    const VB_DEVICE_INFO *pVBInfo);

extern void XGI_SetMiscRegs(USHORT StandTableIndex,
    const VB_DEVICE_INFO *pVBInfo);

extern void XGI_SetCRTCRegs(unsigned StandTableIndex,
    const VB_DEVICE_INFO *pVBInfo);

extern void XGI_SetATTRegs(unsigned ModeNo, unsigned StandTableIndex,
    unsigned ModeIdIndex, const VB_DEVICE_INFO *pVBInfo);

extern void XGI_SetGRCRegs(unsigned StandTableIndex,
    const VB_DEVICE_INFO *pVBInfo);

extern void XGI_ClearExt1Regs(unsigned ModeNo, const VB_DEVICE_INFO *pVBInfo);

extern   void     XGI_SetXG21FPBits(PVB_DEVICE_INFO pVBInfo);
extern   void     XGI_SetXG27FPBits(PVB_DEVICE_INFO pVBInfo);
extern   void     XGI_XG21BLSignalVDD(USHORT tempbh,USHORT tempbl, PVB_DEVICE_INFO pVBInfo);
extern   void     XGI_XG27BLSignalVDD(USHORT tempbh,USHORT tempbl, PVB_DEVICE_INFO pVBInfo);
extern   void     XGI_XG21SetPanelDelay(USHORT tempbl, PVB_DEVICE_INFO pVBInfo);
extern   BOOLEAN  XGI_XG21CheckLVDSMode(USHORT ModeNo,USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo );
extern   void     XGI_SetXG21LVDSPara(USHORT ModeNo,USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo );
extern   USHORT XGI_GetLVDSOEMTableIndex(PVB_DEVICE_INFO pVBInfo);

extern void XGI_SetSync(unsigned RefreshRateTableIndex,
    const VB_DEVICE_INFO *pVBInfo);
#endif
