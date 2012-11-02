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
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "osdef.h"

#ifdef LINUX_XF86
#include "xf86.h"
#include "xf86PciInfo.h"
#include "xgi.h"
#include "xgi_regs.h"
#endif

#ifdef LINUX_KERNEL
#include <linux/version.h>
#include <asm/io.h>
#include <linux/types.h>
#include "XGIfb.h"
#define MASK_OUTPUTSTATE_CRT2LCD 0x02
#endif

#include "vb_def.h"
#include "vgatypes.h"
#include "vb_struct.h"
#include "vb_setmode.h"
#include "vb_ext.h"
#include "vb_init.h"

static int XGINew_GetLCDDDCInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_BridgeIsEnable(PXGI_HW_DEVICE_INFO,
    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_Sense(USHORT tempbx, USHORT tempcx,
			    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_SenseHiTV(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				PVB_DEVICE_INFO pVBInfo);

#ifdef DEBUG
void XGIDumpSR(ScrnInfoPtr pScrn);
void XGIDumpCR(ScrnInfoPtr pScrn);
#endif

/**************************************************************
	Dynamic Sense
*************************************************************/


/**
 * Determine if the CRTC is a 301B.
 */
static int XGINew_Is301B(PVB_DEVICE_INFO pVBInfo)
{
    return !(XGI_GetReg((XGIIOADDRESS)pVBInfo->Part4Port, 0x01) > 0x0B0);
}

/* --------------------------------------------------------------------- */
/* Function : XGI_Is301C */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGI_Is301C( PVB_DEVICE_INFO pVBInfo )
{
    if ( ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x01 ) & 0xF0 ) == 0xC0 )
        return( 1 ) ;

    if ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x01 ) >= 0xD0 )
    {
        if ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x39 ) == 0xE0 )
            return( 1 ) ;
    }

    return( 0 ) ;
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_Sense */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGINew_Sense(USHORT tempbx, USHORT tempcx, PVB_DEVICE_INFO pVBInfo)
{
    unsigned temp;
    unsigned i;


    XGI_SetReg((XGIIOADDRESS)pVBInfo->Part4Port, 0x11, (tempbx & 0x0FF));

    XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part4Port, 0x10, ~0x1F, 
		       ((tempbx & 0xFF00) >> 8) | (tempcx & 0x00FF));

    for (i = 0; i < 10; i++)
        XGI_WaitEndRetrace(pVBInfo->RelIO);

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x03);
    temp = (temp ^ 0x0E) & ((tempcx & 0x7F00) >> 8);

    return (temp > 0);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetSenseStatus */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGI_GetSenseStatus( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax = 0 , tempbx , tempcx , temp ,
           P2reg0 = 0 , SenseModeNo = 0 , OutputSelect = pVBInfo->OutputSelect,
           ModeIdIndex , i ;
    pVBInfo->BaseAddr = ( ULONG )HwDeviceExtension->pjIOAddress ;

    {		/* for 301 */
        if ( pVBInfo->VBInfo & SetCRT2ToHiVisionTV )
        {	/* for HiVision */
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x38 ) ;
            temp = tempax & 0x01 ;
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x3A ) ;
            temp = temp | ( tempax & 0x02 ) ;
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0x32 , 0xA0 , temp ) ;
        }
        else
        {
            if ( XGI_BridgeIsOn( pVBInfo ) )
            {
                P2reg0 = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 ) ;
                if ( !XGINew_BridgeIsEnable( HwDeviceExtension, pVBInfo ) )
                {
		    USHORT temp_mode_no;

                    SenseModeNo = 0x2e ;
                    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x30 , 0x41 ) ; */
		    /* XGISetModeNew(HwDeviceExtension, pVBInfo, 0x2e); // ynlai InitMode */

		    temp_mode_no = SenseModeNo;
		    XGI_SearchModeID(pVBInfo->SModeIDTable,
				     pVBInfo->EModeIDTable, 0x11,
				     &temp_mode_no, &ModeIdIndex);

                    XGI_GetVBType( pVBInfo ) ;
                    pVBInfo->SetFlag = 0x00 ;
                    pVBInfo->ModeType = ModeVGA ;
                    pVBInfo->VBInfo = SetCRT2ToRAMDAC | LoadDACFlag | SetInSlaveMode ;
                    XGI_GetLCDInfo( 0x2e , ModeIdIndex, pVBInfo ) ;
                    XGI_GetTVInfo(  0x2e , ModeIdIndex, pVBInfo ) ;
                    XGI_EnableBridge( HwDeviceExtension, pVBInfo ) ;
                    XGI_SetCRT2Group301( SenseModeNo , HwDeviceExtension, pVBInfo ) ;
                    XGI_SetCRT2ModeRegs( 0x2e , HwDeviceExtension, pVBInfo ) ;
                    /* XGI_DisableBridge( HwDeviceExtension, pVBInfo ) ; */
                    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x01 , 0xDF , 0x20 ) ;	/* Display Off 0212 */
                    for( i = 0 ; i < 20 ; i++ )
                    {
                        XGI_WaitEndRetrace(pVBInfo->RelIO) ;
                    }
                }
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 , 0x1c ) ;
                tempax = 0 ;
                tempbx = *pVBInfo->pRGBSenseData ;

                if ( !( XGINew_Is301B( pVBInfo ) ) )
                {
                    tempbx = *pVBInfo->pRGBSenseData2 ;
                }

                tempcx = 0x0E08 ;
                if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        tempax |= Monitor2Sense;
                    }
                }

                if ( pVBInfo->VBType & VB_XGI301C)
                {
                    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x0d , 0x04 ) ;
                }

		if ( XGINew_SenseHiTV( HwDeviceExtension , pVBInfo) )		/* add by kuku for Multi-adapter sense HiTV */
		{
		    tempax |= HiTVSense ;
                    if ( ( pVBInfo->VBType & VB_XGI301C ) )
                    {
                	tempax ^= ( HiTVSense | YPbPrSense ) ;
                    }
                }

		if ( !( tempax & ( HiTVSense | YPbPrSense ) ) )		/* start */
                {

                tempbx = *pVBInfo->pYCSenseData ;

                if ( !( XGINew_Is301B(  pVBInfo ) ) )
                {
                    tempbx=*pVBInfo->pYCSenseData2;
                }

                tempcx = 0x0604 ;
                if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        tempax |= SVIDEOSense ;
                    }
                }

                if ( OutputSelect & BoardTVType )
                {
                    tempbx = *pVBInfo->pVideoSenseData ;

                    if ( !( XGINew_Is301B( pVBInfo ) ) )
                    {
                        tempbx = *pVBInfo->pVideoSenseData2 ;
                    }

                    tempcx = 0x0804 ;
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                            tempax |= AVIDEOSense;
                        }
                    }
                }
                else
                {
                    if ( !( tempax & SVIDEOSense ) )
                    {
                        tempbx = *pVBInfo->pVideoSenseData ;

                        if ( !( XGINew_Is301B( pVBInfo ) ) )
                        {
                            tempbx=*pVBInfo->pVideoSenseData2;
                        }

                        tempcx = 0x0804 ;
                        if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                            if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                                tempax |= AVIDEOSense;
                            }
                        }
                    }
                }
            }
    	    } /* end */
            if ( !( tempax & Monitor2Sense ) )
            {
                if (XGINew_GetLCDDDCInfo(HwDeviceExtension, pVBInfo)) {
                    tempax |= LCDSense;
                }
            }
            tempbx = 0 ;
            tempcx = 0 ;
            XGINew_Sense(tempbx, tempcx, pVBInfo);

            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0x32 , ~0xDF , tempax ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 , P2reg0 ) ;

            if ( !( P2reg0 & 0x20 ) )
            {
                pVBInfo->VBInfo = DisableCRT2Display ;
                /* XGI_SetCRT2Group301( SenseModeNo , HwDeviceExtension, pVBInfo ) ; */
            }
        }
    }
    XGI_DisableBridge( HwDeviceExtension, pVBInfo ) ;		/* shampoo 0226 */
}



/* --------------------------------------------------------------------- */
/* Function : XGINew_GetLCDDDCInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int
XGINew_GetLCDDDCInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
		     PVB_DEVICE_INFO pVBInfo)
{
    unsigned temp = HwDeviceExtension->ulCRT2LCDType;

    switch (HwDeviceExtension->ulCRT2LCDType) {
    /* add lcd sense */
    case LCD_UNKNOWN:
	return 0;

    case LCD_INVALID:
    case LCD_800x600:
    case LCD_1024x768:
    case LCD_1280x1024:
	break;

    case LCD_640x480:
    case LCD_1024x600:
    case LCD_1152x864:
    case LCD_1280x960:
    case LCD_1152x768:
	temp = 0;
	break;

    case LCD_1400x1050:
    case LCD_1280x768:
    case LCD_1600x1200:
	break;

    case LCD_1920x1440:
    case LCD_2048x1536:
	temp = 0;
	break;

    default:
	break;
    }

    XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->P3d4, 0x36, 0xF0, temp);
    return 1 ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_BridgeIsEnable */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGINew_BridgeIsEnable( PXGI_HW_DEVICE_INFO HwDeviceExtension ,PVB_DEVICE_INFO pVBInfo)
{
    USHORT flag ;

    if ( XGI_BridgeIsOn( pVBInfo ) == 0 )
    {
        flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x0 ) ;

        if ( flag & 0x050 )
        {
            return( 1 ) ;
        }
        else
        {
            return( 0 ) ;
        }

    }
    return( 0 ) ;
}

/* ------------------------------------------------------ */
/* Function : XGINew_SenseHiTV */
/* Input : */
/* Output : */
/* Description : */
/* ------------------------------------------------------ */
BOOLEAN XGINew_SenseHiTV( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo )
{
    USHORT tempbx , tempcx , temp , i , tempch;

#ifndef LINUX_XF86
    USHORT tempax ;
#endif
    tempbx = *pVBInfo->pYCSenseData2 ;

    tempcx = 0x0604 ;

    temp = tempbx & 0xFF ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
    temp = ( tempbx & 0xFF00 ) >> 8 ;
    temp |= ( tempcx & 0x00FF ) ;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

    for( i = 0 ; i < 10 ; i++ )
        XGI_WaitEndRetrace(pVBInfo->RelIO) ;

    tempch = ( tempcx & 0xFF00 ) >> 8;
    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
    temp = temp ^ ( 0x0E ) ;
    temp &= tempch ;

    if ( temp !=  tempch )
        return( 0 ) ;

    tempbx = *pVBInfo->pVideoSenseData2 ;

    tempcx = 0x0804 ;
    temp = tempbx & 0xFF ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
    temp = ( tempbx & 0xFF00 ) >> 8 ;
    temp |= ( tempcx & 0x00FF ) ;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

    for( i = 0 ; i < 10 ; i++ )
        XGI_WaitEndRetrace(pVBInfo->RelIO) ;

    tempch = ( tempcx & 0xFF00 ) >> 8;
    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
    temp = temp ^ ( 0x0E ) ;
    temp &= tempch ;

    if ( temp !=  tempch )
        return( 0 ) ;
    else
    {
      tempbx = 0x3FF ;
      tempcx = 0x0804 ;
      temp = tempbx & 0xFF ;
      XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
      temp = ( tempbx & 0xFF00 ) >> 8 ;
      temp |= ( tempcx & 0x00FF ) ;
      XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

      for( i = 0 ; i < 10 ; i++ )
          XGI_WaitEndRetrace(pVBInfo->RelIO) ;

      tempch = ( tempcx & 0xFF00 ) >> 8;
      temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
      temp = temp ^ ( 0x0E ) ;
      temp &= tempch ;

      if ( temp != tempch )
          return( 1 ) ;
      else
	  return( 0 ) ;
    }
}

void XGIPowerSaving(PVB_DEVICE_INFO pVBInfo, UCHAR PowerSavingStatus)
{
	ErrorF("XGIPowerSaving()...Begin\n");

	if(PowerSavingStatus & 0x01) /* turn off DAC1 */
	{
	   ErrorF("Turn off DAC1...\n");
       /* XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , ??? , ??? ) ; */			/* ??? */
       XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x07 , ~0x20, 0x20 ) ;	/* SR07[5] : Enable DAC1 power saving mode */
	}
	else
	{
	   ErrorF("Turn on DAC1...\n");
       /* XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , ??? , ??? ) ; */			
       XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x07 , ~0x20 ) ;			
	}

	if(PowerSavingStatus & 0x02) /* turn off DVO */
	{
	   ErrorF("Turn off DVO...\n");
       XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 ) ;			/* SR09[7] : Disable FP panel output */
       XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3d4 , 0xB4 , ~0x04 ) ;			/* CRB4[2] : FP power down */
	}
	else
	{
	   ErrorF("Turn on DVO...\n");
       XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 , 0x80 ) ;	
       XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0xB4 , ~0x04 , 0x04 ) ;	
	}

	if(PowerSavingStatus & 0x04) /* turn off DAC2 */
	{
	   ErrorF("Turn off DAC2...\n");
       XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x07 , ~0x08 ) ;			/* SR07[3] : Disable mirror DAC (DAC2) */
       XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x40 , ~0x20, 0x20 ) ;	/* SR40[5] : Enable DAC2 power saving mode */
	}
	else
	{
	   ErrorF("Turn on DAC2...\n");
       XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x07 , ~0x08 , 0x08 ) ;	
       XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x40 , ~0x20 ) ;			
	}

	ErrorF("XGIPowerSaving()...End\n");
}

extern UCHAR g_PowerSavingStatus;
extern void ResetVariableFor2DRegister();

/* --------------------------------------------------------------------- */
/* Function : XGISetDPMS */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
VOID XGISetDPMS(ScrnInfoPtr pScrn, PVB_DEVICE_INFO pVBInfo, PXGI_HW_DEVICE_INFO pXGIHWDE , ULONG VESA_POWER_STATE )
{
    USHORT ModeNo, ModeIdIndex ;
    UCHAR  temp ;
    /* VB_DEVICE_INFO VBINF; */
    /* PVB_DEVICE_INFO pVBInfo = pXGI->XGI_Pr */ /* &VBINF */;

	ErrorF("XGISetDPMS(VESA_POWER_STATE = 0x%lx)...\n", VESA_POWER_STATE);

    InitTo330Pointer( pXGIHWDE->jChipType,  pVBInfo ) ;
    ReadVBIOSTablData( pXGIHWDE->jChipType , pVBInfo) ;

    XGIInitMiscVBInfo(pXGIHWDE, pVBInfo);

	/* Jong@08212009; ignored at present */
	/*
    if ( pVBInfo->IF_DEF_CH7007 == 0 )
    {
        XGINew_SetModeScratch ( pXGIHWDE , pVBInfo ) ;
    } */

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x05 , 0x86 ) ;	/* 1.Openkey */
    XGI_UnLockCRT2( pXGIHWDE , pVBInfo) ;
    ModeNo = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x34 ) ;
    XGI_SearchModeID(pVBInfo->SModeIDTable, pVBInfo->EModeIDTable,  0x11, &ModeNo , &ModeIdIndex);
    /* XGI_SearchModeID( ModeNo , &ModeIdIndex, pVBInfo ) ; */

	/* Jong@08212009; ignored */
	/*
    if ( ( pXGIHWDE->ujVBChipID == VB_CHIP_301 ) || ( pXGIHWDE->ujVBChipID == VB_CHIP_302 ) || ( pVBInfo->IF_DEF_CH7007 == 1 ))
    {
        XGI_GetVBType( pVBInfo ) ;
        XGI_GetVBInfo( ModeNo , ModeIdIndex , pXGIHWDE, pVBInfo ) ;
        XGI_GetTVInfo( ModeNo , ModeIdIndex, pVBInfo ) ;
        XGI_GetLCDInfo( ModeNo , ModeIdIndex, pVBInfo ) ;
    } 

    if ( VESA_POWER_STATE == 0x00000400 )
      XGINew_SetReg1( pVBInfo->Part4Port , 0x31 , ( UCHAR )( XGINew_GetReg1( pVBInfo->Part4Port , 0x31 ) & 0xFE ) ) ;
    else
      XGINew_SetReg1( pVBInfo->Part4Port , 0x31 , ( UCHAR )( XGINew_GetReg1( pVBInfo->Part4Port , 0x31 ) | 0x01 ) ) ;
	*/

    temp = ( UCHAR )XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1f ) ;
    temp &= 0x3f ;
    switch ( VESA_POWER_STATE )
    {
        case 0x00000000 : /* on */
			/* Jong@08212009; not support */
			/*
            if ( ( pXGIHWDE->ujVBChipID == VB_CHIP_301 ) || ( pXGIHWDE->ujVBChipID == VB_CHIP_302 ) )
            {
                XGINew_SetReg1( pVBInfo->P3c4 , 0x1f , ( UCHAR )( temp | 0x00 ) ) ;
                XGI_EnableBridge( pXGIHWDE, pVBInfo ) ;
            }
            else */
            {
			   /* Handle LVDS */
               if ( pVBInfo->IF_DEF_LVDS == 1 )
			   {
				   if ( pXGIHWDE->jChipType == XG21 )
				   {
					   XGI_XG21BLSignalVDD( 0x01 , 0x01, pVBInfo ) ; /* LVDS VDD on */
					   XGI_XG21SetPanelDelay( 2,pVBInfo ) ;
				   }

				   if ( pXGIHWDE->jChipType == XG27 )
				   {
					   XGI_XG27BLSignalVDD( 0x01 , 0x01, pVBInfo ) ; /* LVDS VDD on */
					   XGI_XG21SetPanelDelay( 2,pVBInfo ) ;
				   }
			   }

               XGI_SetRegANDOR( (XGIIOADDRESS) pVBInfo->P3c4 , 0x1F , ~0xC0 , 0x00 ) ;	/* VESA DPMS status on */
               XGI_SetRegAND( (XGIIOADDRESS)pVBInfo->P3c4 , 0x01 , ~0x20 ) ;			/* enable memory retrieve for CRTC */
               
               if ( pXGIHWDE->jChipType == XG21 )
               {
                 temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x38 ) ;
                 if ( temp & 0xE0 )
                 {
                   XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 , 0x80 ) ;	/* DVO ON */
                   XGI_SetXG21FPBits( pVBInfo );
                   XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3d4 , 0x4A , ~0x20 ) ; 		/* Enable write GPIOF */
                   /*XGINew_SetRegANDOR( pVBInfo->P3d4 , 0x48 , ~0x20 , 0x20 ) ;*/ 		/* LCD Display ON */
                 }

                 XGI_XG21BLSignalVDD( 0x20 , 0x20, pVBInfo ) ; /* LVDS signal on */
                 XGI_DisplayOn( pXGIHWDE, pVBInfo );
               } 

               if ( pXGIHWDE->jChipType == XG27 )
               {
                 temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x38 ) ;
                 if ( temp & 0xE0 )
                 {
                   XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 , 0x80 ) ;	/* DVO ON */
                   XGI_SetXG27FPBits( pVBInfo );
                   XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3d4 , 0x4A , ~0x20 ) ; 			/* Enable write GPIOF */
                   /*XGINew_SetRegANDOR( pVBInfo->P3d4 , 0x48 , ~0x20 , 0x20 ) ;*/ 			/* LCD Display ON */
                 }
                 XGI_XG27BLSignalVDD( 0x20 , 0x20, pVBInfo ) ; /* LVDS signal on */

				 XGIPowerSaving(pVBInfo, g_PowerSavingStatus);

				 XGI_DisplayOn( pXGIHWDE, pVBInfo );
               } 
            }

			/* Jong@08252009; reset variables of register */
			ResetVariableFor2DRegister();

			/* Turn on 2D engine */
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x1E , ~0x40 , 0x40 ) ;	/* 2D ON */

            break ;
        case 0x00000100: /* standby */
			PDEBUG(ErrorF("Standby...dump regs...0\n"));
			PDEBUG(XGIDumpSR(pScrn));
			PDEBUG(XGIDumpCR(pScrn));

            if ( pXGIHWDE->jChipType >= XG21 )
            {
                XGI_DisplayOff( pXGIHWDE, pVBInfo );
            }
          
			PDEBUG(ErrorF("Standby...dump regs...1\n"));
			PDEBUG(XGIDumpSR(pScrn));
			PDEBUG(XGIDumpCR(pScrn));

            if ( pXGIHWDE->jChipType == XG21 )
            {
				XGIPowerSaving(pVBInfo, 0x03);	/* Turn off DAC1, DVO */
            }

            if ( pXGIHWDE->jChipType == XG27 )
            {
				XGIPowerSaving(pVBInfo, 0x07);	/* Turn off DAC1, DAC2, DVO */
            }

            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1f , ( UCHAR )( temp | 0x40 ) ) ;

			/* Turn off 2D engine */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x1E , ~0x40) ;	/* 2D OFF */

			PDEBUG(ErrorF("Standby...dump regs...2\n"));
			PDEBUG(XGIDumpSR(pScrn));
			PDEBUG(XGIDumpCR(pScrn));
            break ;
        case 0x00000200: /* suspend */
            if ( pXGIHWDE->jChipType == XG21 )
            {
                XGI_DisplayOff( pXGIHWDE, pVBInfo );
                XGI_XG21BLSignalVDD( 0x20 , 0x00, pVBInfo ) ; /* LVDS signal off */
				XGIPowerSaving(pVBInfo, 0x03);	/* Turn off DAC1, DVO */
            }

            if ( pXGIHWDE->jChipType == XG27 )
            {
                XGI_DisplayOff( pXGIHWDE, pVBInfo );
                XGI_XG27BLSignalVDD( 0x20 , 0x00, pVBInfo ) ; /* LVDS signal off */
				XGIPowerSaving(pVBInfo, 0x07);	/* Turn off DAC1, DAC2, DVO */
            }

            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1f , ( UCHAR )( temp | 0x80 ) ) ;

			/* Turn off 2D engine */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x1E , ~0x40) ;	/* 2D OFF */

            break ;
        case 0x00000400: /* off */
			/* Jong@08212009; not support */
			/*
            if ( (pXGIHWDE->ujVBChipID == VB_CHIP_301 ) || ( pXGIHWDE->ujVBChipID == VB_CHIP_302 ) )
            {
                XGINew_SetReg1( pVBInfo->P3c4 , 0x1f , ( UCHAR )( temp | 0xc0 ) ) ;
                XGI_DisableBridge( pXGIHWDE, pVBInfo ) ;
            }
            else */
            {
               if ( pXGIHWDE->jChipType == XG21 )
               {
                 XGI_DisplayOff( pXGIHWDE, pVBInfo );
                 
                 XGI_XG21BLSignalVDD( 0x20 , 0x00, pVBInfo ) ; /* LVDS signal off */

                 temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x38 ) ;

                 if ( temp & 0xE0 )
                 {
                   XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 ) ;		/* DVO Off */
                   XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3d4 , 0x4A , ~0x20 ) ; 		/* Enable write GPIOF */
                   /*XGINew_SetRegAND( pVBInfo->P3d4 , 0x48 , ~0x20 ) ;*/ 		/* LCD Display OFF */
                 }

				 XGIPowerSaving(pVBInfo, 0x03);	/* Turn off DAC1, DVO */

               }

               if ( pXGIHWDE->jChipType == XG27 )
               {
                 XGI_DisplayOff( pXGIHWDE, pVBInfo );
                 
                 XGI_XG27BLSignalVDD( 0x20 , 0x00, pVBInfo ) ; /* LVDS signal off */

                 temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x38 ) ;

                 if ( temp & 0xE0 )
                 {
                   XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x09 , ~0x80 ) ;		/* DVO Off */
                 }

				 XGIPowerSaving(pVBInfo, 0x07);	/* Turn off DAC1, DAC2, DVO */
               }

               XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x1F , ~0xC0 , 0xC0 ) ;
               XGI_SetRegOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x01 , 0x20 ) ;		/* CRT Off */
               
               if ( ( pXGIHWDE->jChipType == XG21 ) && ( pVBInfo->IF_DEF_LVDS == 1 ) )
               {
                 XGI_XG21SetPanelDelay( 4,pVBInfo ) ;
                 XGI_XG21BLSignalVDD( 0x01 , 0x00, pVBInfo ) ; /* LVDS VDD off */
                 XGI_XG21SetPanelDelay( 5,pVBInfo ) ;
               }

               if ( ( pXGIHWDE->jChipType == XG27 ) && ( pVBInfo->IF_DEF_LVDS == 1 ) )
               {
                 XGI_XG21SetPanelDelay( 4,pVBInfo ) ;
                 XGI_XG27BLSignalVDD( 0x01 , 0x00, pVBInfo ) ; /* LVDS VDD off */
                 XGI_XG21SetPanelDelay( 5,pVBInfo ) ;
               }
            }

			/* Turn off 2D engine */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4 , 0x1E , ~0x40) ;	/* 2D OFF */

            break ;

        default:
			ErrorF("XGISetDPMS()-invalid power status!\n");
            break ;
    }

    XGI_LockCRT2( pXGIHWDE , pVBInfo ) ;
}
