/*
 * Mode initializing code (CRT1 section) 
 * (Universal module for Linux kernel framebuffer and XFree86 4.x)
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License as published by
 * * the Free Software Foundation; either version 2 of the named License,
 * * or any later version.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) The name of the author may not be used to endorse or promote products
 * *    derived from this software without specific prior written permission.
 * *
 * * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: 	Thomas Winischhofer <thomas@winischhofer.net>
 *
 * Formerly based on non-functional code-fragements for 300 series by XGI, Inc.
 * Used by permission.
 *
 * TW says: This code looks awful, I know. But please don't do anything about
 * this otherwise debugging will be hell.
 * The code is extremely fragile as regards the different chipsets, different
 * video bridges and combinations thereof. If anything is changed, extreme
 * care has to be taken that that change doesn't break it for other chipsets,
 * bridges or combinations thereof.
 * All comments in this file are by me, regardless if they are marked TW or not.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "init.h"
#include "vgatypes.h"
#include "vb_def.h"
#include "vb_setmode.h"

/*********************************************/
/*            HELPER: Get ModeID             */
/*********************************************/
/* Jong 09/18/2007; patch to GIT */
/* VGAEngine is not used; FSTN is always FALSE */
USHORT
XGI_GetModeID(ULONG VBFlags, int HDisplay, int VDisplay,
              int Depth, int LCDwidth, int LCDheight)
{
   USHORT ModeIndex = 0;

   switch(HDisplay)
   {
     case 320:
       if(VDisplay == 200)
	   ModeIndex = ModeIndex_320x200[Depth];
       else if(VDisplay == 240)
	   {
	     ModeIndex = ModeIndex_320x240[Depth];
       }
       break;
     case 400:
          if(VDisplay == 300) ModeIndex = ModeIndex_400x300[Depth];
          break;
     case 512:
          if(VDisplay == 384) ModeIndex = ModeIndex_512x384[Depth];
          break;
     case 640:
          if(VDisplay == 480)      ModeIndex = ModeIndex_640x480[Depth];
	  else if(VDisplay == 400) ModeIndex = ModeIndex_640x400[Depth];
          break;
     case 720:
          if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 480)      ModeIndex = ModeIndex_720x480[Depth];
             else if(VDisplay == 576) ModeIndex = ModeIndex_720x576[Depth];
          }
          break;
     case 768:
          if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 576) ModeIndex = ModeIndex_768x576[Depth];
          }
	  break;
     case 800:
	  if(VDisplay == 600)    ModeIndex = ModeIndex_800x600[Depth];
	  else if(!(VBFlags & CRT1_LCDA)) {
	     if(VDisplay == 480) ModeIndex = ModeIndex_800x480[Depth];
	  }
          break;
     case 848:
          if(!(VBFlags & CRT1_LCDA)) {
	     if(VDisplay == 480) ModeIndex = ModeIndex_848x480[Depth];
	  }
	  break;
     case 856:
          if(!(VBFlags & CRT1_LCDA)) {
	     if(VDisplay == 480) ModeIndex = ModeIndex_856x480[Depth];
	  }
	  break;
     case 1024:
          if(VDisplay == 768) ModeIndex = ModeIndex_1024x768[Depth];
	  else if(!(VBFlags & CRT1_LCDA)) {
	     if(VDisplay == 576)    ModeIndex = ModeIndex_1024x576[Depth];
	  }
          break;
     case 1152:
          if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 864)    ModeIndex = ModeIndex_1152x864[Depth];
	  }
	  break;
     case 1280:
          if(VDisplay == 1024) ModeIndex = ModeIndex_1280x1024[Depth];
	  else if(VDisplay == 720) {
	     if((VBFlags & CRT1_LCDA) && (LCDwidth == 1280) && (LCDheight == 720)) {
	        ModeIndex = ModeIndex_1280x720[Depth];
	     } else if(!(VBFlags & CRT1_LCDA)) {
	        ModeIndex = ModeIndex_1280x720[Depth];
	     }
	  } else if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 960)      ModeIndex = ModeIndex_1280x960[Depth];
	     else if(VDisplay == 768) {
	           ModeIndex = ModeIndex_310_1280x768[Depth];
	     }
	  }
          break;
     case 1360:
          if(!(VBFlags & CRT1_LCDA)) {
	     if(VDisplay == 768)     ModeIndex = ModeIndex_1360x768[Depth];
	  }
          break;
     case 1400:
          break;
     case 1440: 
          /* if(VDisplay == 900) ModeIndex = ModeIndex_1440x900[Depth]; */
          break;
     case 1600:
          if(VDisplay == 1200) ModeIndex = ModeIndex_1600x1200[Depth];
          break;
     case 1680:
          break;
     case 1920:
          if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 1440) ModeIndex = ModeIndex_1920x1440[Depth];
	  }
          break;
     case 2048:
          if(!(VBFlags & CRT1_LCDA)) {
             if(VDisplay == 1536) {
	            ModeIndex = ModeIndex_310_2048x1536[Depth];
	     }
	  }
          break;
   }

   return(ModeIndex);
}

/*********************************************/
/*          HELPER: SetReg, GetReg           */
/*********************************************/

void
XGI_SetReg(XGIIOADDRESS port, USHORT index, USHORT data)
{
   outb(port,index);
   outb(port + 1,data);
}

void
XGI_SetRegByte(XGIIOADDRESS port, USHORT data)
{
   outb(port,data);
}

void
XGI_SetRegShort(XGIIOADDRESS port, USHORT data)
{
   outw(port,data);
}

void
XGI_SetRegLong(XGIIOADDRESS port, ULONG data)
{
    outl(port,data);
}

UCHAR
XGI_GetReg(XGIIOADDRESS port, USHORT index)
{
   outb(port,index);
   return inb(port + 1);
}

UCHAR
XGI_GetRegByte(XGIIOADDRESS port)
{
   return inb(port);
}

USHORT
XGI_GetRegShort(XGIIOADDRESS port)
{
   return inw(port);
}

ULONG
XGI_GetRegLong(XGIIOADDRESS port)
{
   return inl(port);
}

void
XGI_SetRegANDOR(XGIIOADDRESS Port,USHORT Index,USHORT DataAND,USHORT DataOR)
{
  USHORT temp;

  temp = XGI_GetReg(Port,Index);
  temp = (temp & (DataAND)) | DataOR;
  XGI_SetReg(Port,Index,temp);
}

void
XGI_SetRegAND(XGIIOADDRESS Port,USHORT Index,USHORT DataAND)
{
  USHORT temp;

  temp = XGI_GetReg(Port,Index);
  temp &= DataAND;
  XGI_SetReg(Port,Index,temp);
}

void
XGI_SetRegOR(XGIIOADDRESS Port,USHORT Index,USHORT DataOR)
{
  USHORT temp;

  temp = XGI_GetReg(Port,Index);
  temp |= DataOR;
  XGI_SetReg(Port,Index,temp);
}

/*********************************************/
/*      HELPER: DisplayOn, DisplayOff        */
/*********************************************/

void
XGI_New_DisplayOn(VB_DEVICE_INFO *XGI_Pr)
{
   XGI_SetRegAND(XGI_Pr->P3c4,0x01,0xDF);
}

void
XGI_New_DisplayOff(VB_DEVICE_INFO *XGI_Pr)
{
   XGI_SetRegOR(XGI_Pr->P3c4,0x01,0x20);
}

/*********************************************/
/*         HELPER: Init PCI & Engines        */
/*********************************************/

static void
XGIInitPCIetc(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo)
{
   CARD8  bForce=0x00; /* Jong 01/07/2008; force to disable 2D */

   switch(HwInfo->jChipType) {
   case XG40:
   case XG42:
   case XG20:
   case XG21:
      XGI_SetReg(XGI_Pr->P3c4,0x20,0xa1);
      /*  - Enable 2D (0x40)
       *  - Enable 3D (0x02)
       *  - Enable 3D vertex command fetch (0x10)
       *  - Enable 3D command parser (0x08)
       *  - Enable 3D G/L transformation engine (0x80)
       */
      XGI_SetRegOR(XGI_Pr->P3c4, 0x1E, 
		   SR1E_ENABLE_3D_TRANSFORM_ENGINE
		   | SR1E_ENABLE_2D
		   | SR1E_ENABLE_3D_AGP_VERTEX_FETCH
		   | SR1E_ENABLE_3D_COMMAND_PARSER
		   | SR1E_ENABLE_3D);

	  /* Jong 01/07/2008; support forcing to disable 2D engine */
	  if(HwInfo->jChipType == XG21)
	  {
  			inXGIIDXREG(XGI_Pr->P3c4, 0x3A, bForce) ;
			bForce &= 0x40;

			if(bForce != 0x00)
			  XGI_SetRegAND(XGI_Pr->P3c4,0x1E,0xBF);
	  }

	  break;
   }
}

/*********************************************/
/*             HELPER: GetVBType             */
/*********************************************/

void
XGI_New_GetVBType(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo)
{
  USHORT flag=0, rev=0, nolcd=0;

  XGI_Pr->VBType = 0;

  flag = XGI_GetReg(XGI_Pr->Part4Port,0x00);
PDEBUG(ErrorF("GetVBType: part4_0: %x \n",flag)); //yilin
  if(flag > 3) return;

  rev = XGI_GetReg(XGI_Pr->Part4Port,0x01);
PDEBUG(ErrorF("GetVBType: part4_1: %x \n",rev)); //yilin
  
  if(flag >= 2) {
     XGI_Pr->VBType = VB_XGI302B;
  } else if(flag == 1) {
     if(rev >= 0xC0) {
       	XGI_Pr->VBType = VB_XGI301C;
     } else if(rev >= 0xB0) {
       	XGI_Pr->VBType = VB_XGI301B;
	/* Check if 30xB DH version (no LCD support, use Panel Link instead) */
    	nolcd = XGI_GetReg(XGI_Pr->Part4Port,0x23);
        if(!(nolcd & 0x02)) XGI_Pr->VBType |= VB_NoLCD;
     } else {
        XGI_Pr->VBType = VB_XGI301;
     }
  }
  if(XGI_Pr->VBType & (VB_XGI301B | VB_XGI301C | VB_XGI302B)) {
     if(rev >= 0xE0) {
	flag = XGI_GetReg(XGI_Pr->Part4Port,0x39);
	if(flag == 0xff) XGI_Pr->VBType = VB_XGI302LV;
	else 	 	 XGI_Pr->VBType = VB_XGI302ELV;
     } else if(rev >= 0xD0) {
	XGI_Pr->VBType = VB_XGI301LV;
     }
  }
PDEBUG(ErrorF("GetVBType: XGI_Pr->VBType=%x \n",XGI_Pr->VBType)); //yilin
}

/*********************************************/
/*           HELPER: SearchModeID            */
/*********************************************/

BOOLEAN
XGI_SearchModeID(const XGI_StStruct *SModeIDTable, 
		 const XGI_ExtStruct *EModeIDTable,
		 unsigned char VGAINFO, USHORT *ModeNo, USHORT *ModeIdIndex)
{
    if (*ModeNo <= 0x13) {
	if ((*ModeNo) <= 0x05)
	    (*ModeNo) |= 0x01;

	for (*ModeIdIndex = 0; /* emtpy */; (*ModeIdIndex)++) {
	    if (SModeIDTable[*ModeIdIndex].St_ModeID == (*ModeNo))
		break;

	    if (SModeIDTable[*ModeIdIndex].St_ModeID == 0xFF)
		return FALSE;
	}

	if (*ModeNo == 0x07) {
	    if (VGAINFO & 0x10) 
		(*ModeIdIndex)++;   /* 400 lines */
	    /* else 350 lines */
	}

	if (*ModeNo <= 0x03) {
	    if (!(VGAINFO & 0x80))
		(*ModeIdIndex)++;

	    if (VGAINFO & 0x10)
		(*ModeIdIndex)++; /* 400 lines  */
	    /* else 350 lines  */
	}
	/* else 200 lines  */
    }
    else {

	for (*ModeIdIndex = 0; /* emtpy */; (*ModeIdIndex)++) {
	    if (EModeIDTable[*ModeIdIndex].Ext_ModeID == (*ModeNo))
		break;

	    if (EModeIDTable[*ModeIdIndex].Ext_ModeID == 0xFF)
		return FALSE;
	}
    }

    return TRUE;
}

/*********************************************/
/*            HELPER: GetModePtr             */
/*********************************************/

UCHAR
XGI_GetModePtr(const XGI_StStruct *SModeIDTable, unsigned ModeType, 
	       USHORT ModeNo, USHORT ModeIdIndex)
{
    return (ModeNo <= 0x13) 
	? SModeIDTable[ModeIdIndex].St_StTableIndex
	: ((ModeType <= 0x02) ? 0x1B /* 02 -> ModeEGA  */ : 0x0F);
}


/*********************************************/
/*           HELPER: LowModeTests            */
/*********************************************/

static BOOLEAN
XGI_DoLowModeTest(VB_DEVICE_INFO *XGI_Pr, USHORT ModeNo, PXGI_HW_DEVICE_INFO HwInfo)
{
    USHORT temp,temp1,temp2;

    if((ModeNo != 0x03) && (ModeNo != 0x10) && (ModeNo != 0x12))
       return(1);
    temp = XGI_GetReg(XGI_Pr->P3d4,0x11);
    XGI_SetRegOR(XGI_Pr->P3d4,0x11,0x80);
    temp1 = XGI_GetReg(XGI_Pr->P3d4,0x00);
    XGI_SetReg(XGI_Pr->P3d4,0x00,0x55);
    temp2 = XGI_GetReg(XGI_Pr->P3d4,0x00);
    XGI_SetReg(XGI_Pr->P3d4,0x00,temp1);
    XGI_SetReg(XGI_Pr->P3d4,0x11,temp);
    if (temp2 == 0x55)
	return(0);
    else 
	return(1);
}

static void
XGI_SetLowModeTest(VB_DEVICE_INFO *XGI_Pr, USHORT ModeNo, PXGI_HW_DEVICE_INFO HwInfo)
{
    if(XGI_DoLowModeTest(XGI_Pr, ModeNo, HwInfo)) {
       XGI_Pr->SetFlag |= LowModeTests;
    }
}

static void
XGI_HandleCRT1(VB_DEVICE_INFO *XGI_Pr)
{
    XGI_SetRegAND(XGI_Pr->P3d4, 0x53, 0xbf);
}

/*********************************************/
/*             HELPER: GetOffset             */
/*********************************************/

USHORT
XGI_New_GetOffset(VB_DEVICE_INFO *XGI_Pr,USHORT ModeNo,USHORT ModeIdIndex,
              USHORT RefreshRateTableIndex,PXGI_HW_DEVICE_INFO HwInfo)
{
  USHORT xres, temp, colordepth, infoflag;

  infoflag = XGI_Pr->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
  xres = XGI_Pr->RefIndex[RefreshRateTableIndex].XRes;

  colordepth = XGI_GetColorDepth(ModeNo, ModeIdIndex, XGI_Pr);

  temp = xres / 16;
  if(infoflag & InterlaceMode) temp <<= 1;
  temp *= colordepth;
  if(xres % 16) {
     colordepth >>= 1;
     temp += colordepth;
  }

  return(temp);
}

/*********************************************/
/*                 RESET VCLK                */
/*********************************************/

static void
XGI_ResetCRT1VCLK(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo)
{
    XGI_SetRegANDOR(XGI_Pr->P3c4,0x31,0xCF,0x20);
    XGI_SetReg(XGI_Pr->P3c4,0x2B,XGI_Pr->VCLKData[1].SR2B);
    XGI_SetReg(XGI_Pr->P3c4,0x2C,XGI_Pr->VCLKData[1].SR2C);
    XGI_SetReg(XGI_Pr->P3c4,0x2D,0x80);
    XGI_SetRegANDOR(XGI_Pr->P3c4,0x31,0xcf,0x10);
    XGI_SetReg(XGI_Pr->P3c4,0x2B,XGI_Pr->VCLKData[0].SR2B);
    XGI_SetReg(XGI_Pr->P3c4,0x2C,XGI_Pr->VCLKData[0].SR2C);
    XGI_SetReg(XGI_Pr->P3c4,0x2D,0x80);
}

/*********************************************/
/*                  CRTC/2                   */
/*********************************************/

static void
XGI_New_SetCRT1CRTC(VB_DEVICE_INFO *XGI_Pr, USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex,
		PXGI_HW_DEVICE_INFO HwInfo)
{
  UCHAR  index;
  USHORT temp,i,j,modeflag;

  XGI_SetRegAND(XGI_Pr->P3d4,0x11,0x7f);		/* unlock cr0-7 */

     if(ModeNo <= 0x13) {
        modeflag = XGI_Pr->SModeIDTable[ModeIdIndex].St_ModeFlag;
     } else {
        modeflag = XGI_Pr->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
     }

     index = XGI_Pr->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;

     for(i=0,j=0;i<=7;i++,j++) {
        XGI_SetReg(XGI_Pr->P3d4,j,XGI_Pr->XGINEWUB_CRT1Table[index].CR[i]);
     }
     for(j=0x10;i<=10;i++,j++) {
        XGI_SetReg(XGI_Pr->P3d4,j,XGI_Pr->XGINEWUB_CRT1Table[index].CR[i]);
     }
     for(j=0x15;i<=12;i++,j++) {
        XGI_SetReg(XGI_Pr->P3d4,j,XGI_Pr->XGINEWUB_CRT1Table[index].CR[i]);
     }
     for(j=0x0A;i<=15;i++,j++) {
        XGI_SetReg(XGI_Pr->P3c4,j,XGI_Pr->XGINEWUB_CRT1Table[index].CR[i]);
     }

     temp = XGI_Pr->XGINEWUB_CRT1Table[index].CR[16] & 0xE0;
     XGI_SetReg(XGI_Pr->P3c4,0x0E,temp);

     temp = ((XGI_Pr->XGINEWUB_CRT1Table[index].CR[16]) & 0x01) << 5;
     if(modeflag & DoubleScanMode)  temp |= 0x80;
     XGI_SetRegANDOR(XGI_Pr->P3d4,0x09,0x5F,temp);

  if(XGI_Pr->ModeType > ModeVGA) XGI_SetReg(XGI_Pr->P3d4,0x14,0x4F);
}

/*********************************************/
/*               OFFSET & PITCH              */
/*********************************************/
/*  (partly overruled by SetPitch() in XF86) */
/*********************************************/

static void
XGI_New_SetCRT1Offset(VB_DEVICE_INFO *XGI_Pr, USHORT ModeNo, USHORT ModeIdIndex,
                  USHORT RefreshRateTableIndex,
		  PXGI_HW_DEVICE_INFO HwInfo)
{
   USHORT temp, DisplayUnit, infoflag;

   infoflag = XGI_Pr->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;

   DisplayUnit = XGI_New_GetOffset(XGI_Pr,ModeNo,ModeIdIndex,
                     	       RefreshRateTableIndex,HwInfo);

   temp = (DisplayUnit >> 8) & 0x0f;
   XGI_SetRegANDOR(XGI_Pr->P3c4,0x0E,0xF0,temp);

   temp = DisplayUnit & 0xFF;
   XGI_SetReg(XGI_Pr->P3d4,0x13,temp);

   if(infoflag & InterlaceMode) DisplayUnit >>= 1;

   DisplayUnit <<= 5;
   temp = (DisplayUnit & 0xff00) >> 8;
   if(DisplayUnit & 0xff) temp++;
   temp++;
   XGI_SetReg(XGI_Pr->P3c4,0x10,temp);
}

/*********************************************/
/*                  VCLK                     */
/*********************************************/

static void
XGI_New_SetCRT1VCLK(VB_DEVICE_INFO *XGI_Pr, USHORT ModeNo, USHORT ModeIdIndex,
                PXGI_HW_DEVICE_INFO HwInfo, USHORT RefreshRateTableIndex)
{
  USHORT  index=0, clka, clkb;

     if((XGI_Pr->VBType & VB_XGI301BLV302BLV) && (XGI_Pr->VBInfo & SetCRT2ToLCDA)) {
        clka = XGI_Pr->VBVCLKData[index].Part4_A;
	clkb = XGI_Pr->VBVCLKData[index].Part4_B;
     } else {
        clka = XGI_Pr->VCLKData[index].SR2B;
	clkb = XGI_Pr->VCLKData[index].SR2C;
     }

    XGI_SetRegAND(XGI_Pr->P3c4,0x31,0xCF);
    XGI_SetReg(XGI_Pr->P3c4,0x2B,clka);
    XGI_SetReg(XGI_Pr->P3c4,0x2C,clkb);
    XGI_SetReg(XGI_Pr->P3c4,0x2D,0x01);
}

/*********************************************/
/*              MODE REGISTERS               */
/*********************************************/

static void
XGI_New_SetVCLKState(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
                 USHORT ModeNo, USHORT RefreshRateTableIndex,
                 USHORT ModeIdIndex)
{
  USHORT data=0, VCLK=0, index=0;

  if(ModeNo > 0x13) {
        VCLK = XGI_Pr->VCLKData[index].CLOCK;
  }

    if(VCLK >= 166)
	data |= 0x0c;

    XGI_SetRegANDOR(XGI_Pr->P3c4,0x32,0xf3,data);

    if(VCLK >= 166) {
	XGI_SetRegAND(XGI_Pr->P3c4,0x1f,0xe7);
    }

    /* DAC speed */
    XGI_SetRegANDOR(XGI_Pr->P3c4,0x07,0xE8,0x10);
}

static void
XGI_New_SetCRT1ModeRegs(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
                    USHORT ModeNo,USHORT ModeIdIndex,USHORT RefreshRateTableIndex)
{
  USHORT data,infoflag=0,modeflag;
  USHORT resindex = 0,xres;

     if(ModeNo > 0x13) {
    	modeflag = XGI_Pr->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    	infoflag = XGI_Pr->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
	xres = XGI_Pr->ModeResInfo[resindex].HTotal;
     } else {
    	modeflag = XGI_Pr->SModeIDTable[ModeIdIndex].St_ModeFlag;
	xres = XGI_Pr->StResInfo[resindex].HTotal;
     }

  /* Disable DPMS */
  XGI_SetRegAND(XGI_Pr->P3c4,0x1F,0x3F);

  data = 0;
  if(ModeNo > 0x13) {
     if(XGI_Pr->ModeType > 0x02) {
        data |= 0x02;
        data |= ((XGI_Pr->ModeType - ModeVGA) << 2);
     }
     if(infoflag & InterlaceMode) data |= 0x20;
  }
  XGI_SetRegANDOR(XGI_Pr->P3c4,0x06,0xC0,data);

    data = 0;
    if(infoflag & InterlaceMode) {
        if(xres <= 800)       data = 0x0020;
        else if(xres <= 1024) data = 0x0035;
        else                  data = 0x0048;
    }
    XGI_SetReg(XGI_Pr->P3d4,0x19,(data & 0xFF));
    XGI_SetRegANDOR(XGI_Pr->P3d4,0x1a,0xFC,(data >> 8));

  if(modeflag & HalfDCLK) {
     XGI_SetRegOR(XGI_Pr->P3c4,0x01,0x08);
  }

  data = 0;
  if(modeflag & LineCompareOff) data = 0x08;

     XGI_SetRegANDOR(XGI_Pr->P3c4,0x0F,0xB7,data);
     if(XGI_Pr->ModeType == ModeEGA) {
        if(ModeNo > 0x13) {
  	   XGI_SetRegOR(XGI_Pr->P3c4,0x0F,0x40);
        }
     }

    XGI_SetRegAND(XGI_Pr->P3c4,0x31,0xfb);

  data = 0x60;
  if(XGI_Pr->ModeType != ModeText) {
     data ^= 0x60;
     if(XGI_Pr->ModeType != ModeEGA) {
        data ^= 0xA0;
     }
  }
  XGI_SetRegANDOR(XGI_Pr->P3c4,0x21,0x1F,data);

  XGI_New_SetVCLKState(XGI_Pr, HwInfo, ModeNo, RefreshRateTableIndex, ModeIdIndex);
}

/*********************************************/
/*                 LOAD DAC                  */
/*********************************************/

extern const uint8_t XGI_MDA_DAC[];
extern const uint8_t XGI_CGA_DAC[];
extern const uint8_t XGI_EGA_DAC[];
extern const uint8_t XGI_VGA_DAC[];

void
XGI_New_LoadDAC(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
            USHORT ModeNo, USHORT ModeIdIndex)
{
   USHORT data,data2;
   USHORT time,i,j,k,m,n,o;
   USHORT si,di,bx,dl,al,ah,dh;
   USHORT shiftflag;
   XGIIOADDRESS DACAddr, DACData;
   const uint8_t *table = NULL;

   if(ModeNo <= 0x13) {
      data = XGI_Pr->SModeIDTable[ModeIdIndex].St_ModeFlag;
   } else {
      data = XGI_Pr->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
   }

   data &= DACInfoFlag;
   time = 64;
   if(data == 0x00) table = XGI_MDA_DAC;
   if(data == 0x08) table = XGI_CGA_DAC;
   if(data == 0x10) table = XGI_EGA_DAC;
   if(data == 0x18) {
      time = 256;
      table = XGI_VGA_DAC;
   }
   if(time == 256) j = 16;
   else            j = time;

   if( ( (XGI_Pr->VBInfo & SetCRT2ToLCD) &&        /* 301B-DH LCD */
         (XGI_Pr->VBType & VB_NoLCD) )        ||
       (XGI_Pr->VBInfo & SetCRT2ToLCDA)       ||   /* LCDA */
       (!(XGI_Pr->SetFlag & ProgrammingCRT2)) ) {  /* Programming CRT1 */
      DACAddr = XGI_Pr->P3c8;
      DACData = XGI_Pr->P3c9;
      shiftflag = 0;
      XGI_SetRegByte(XGI_Pr->P3c6,0xFF);
   } else {
      shiftflag = 1;
      DACAddr = XGI_Pr->Part5Port;
      DACData = XGI_Pr->Part5Port + 1;
   }

   XGI_SetRegByte(DACAddr,0x00);

   for(i=0; i<j; i++) {
      data = table[i];
      for(k=0; k<3; k++) {
	data2 = 0;
	if(data & 0x01) data2 = 0x2A;
	if(data & 0x02) data2 += 0x15;
	if(shiftflag) data2 <<= 2;
	XGI_SetRegByte(DACData, data2);
	data >>= 2;
      }
   }

   if(time == 256) {
      for(i = 16; i < 32; i++) {
   	 data = table[i];
	 if(shiftflag) data <<= 2;
	 for(k = 0; k < 3; k++) XGI_SetRegByte(DACData, data);
      }
      si = 32;
      for(m = 0; m < 9; m++) {
         di = si;
         bx = si + 4;
         dl = 0;
         for(n = 0; n < 3; n++) {
  	    for(o = 0; o < 5; o++) {
	       dh = table[si];
	       ah = table[di];
	       al = table[bx];
	       si++;
	       XGI_WriteDAC(DACData, shiftflag, dl, ah, al, dh);
	    }
	    si -= 2;
	    for(o = 0; o < 3; o++) {
	       dh = table[bx];
	       ah = table[di];
	       al = table[si];
	       si--;
	       XGI_WriteDAC(DACData, shiftflag, dl, ah, al, dh);
	    }
	    dl++;
	 }            /* for n < 3 */
	 si += 5;
      }               /* for m < 9 */
   }
}

/*********************************************/
/*         SET CRT1 REGISTER GROUP           */
/*********************************************/

static void
XGI_New_SetCRT1Group(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
                 USHORT ModeNo, USHORT ModeIdIndex)
{
    const USHORT StandTableIndex = XGI_GetModePtr(XGI_Pr->SModeIDTable,
                                                  XGI_Pr->ModeType,
                                                  ModeNo, ModeIdIndex);
    USHORT RefreshRateTableIndex = 0;


/*
  if(XGI_Pr->SetFlag & LowModeTests) {
     if(XGI_Pr->VBInfo & (SetSimuScanMode | SwitchToCRT2)) {
        XGI_New_DisableBridge(XGI_Pr, HwInfo);
     }
  }
*/
  XGI_SetSeqRegs(StandTableIndex, XGI_Pr);
  XGI_SetMiscRegs(StandTableIndex, XGI_Pr);
  XGI_SetCRTCRegs(StandTableIndex, XGI_Pr);
  XGI_SetATTRegs(ModeNo, StandTableIndex, ModeIdIndex, XGI_Pr);
  XGI_SetGRCRegs(StandTableIndex, XGI_Pr);
  XGI_ClearExt1Regs(ModeNo, XGI_Pr);
  XGI_ResetCRT1VCLK(XGI_Pr, HwInfo);

  XGI_Pr->SetFlag &= (~ProgrammingCRT2);

#ifdef LINUX_XF86
  xf86DrvMsgVerb(0, X_PROBED, 4, "(init: VBType=0x%04x, VBInfo=0x%04x)\n",
                    XGI_Pr->VBType, XGI_Pr->VBInfo);
#endif

  if(XGI_Pr->VBInfo & SetSimuScanMode) {
     if(XGI_Pr->VBInfo & SetInSlaveMode) {
        XGI_Pr->SetFlag |= ProgrammingCRT2;
     }
  }

  if(XGI_Pr->VBInfo & SetCRT2ToLCDA) {
     XGI_Pr->SetFlag |= ProgrammingCRT2;
  }

  if(!(XGI_Pr->VBInfo & SetCRT2ToLCDA)) {
     XGI_Pr->SetFlag &= ~ProgrammingCRT2;
  }

  if(RefreshRateTableIndex != 0xFFFF) {
     XGI_SetSync(RefreshRateTableIndex, XGI_Pr);
     XGI_New_SetCRT1CRTC(XGI_Pr, ModeNo, ModeIdIndex, RefreshRateTableIndex, HwInfo);
     XGI_New_SetCRT1Offset(XGI_Pr, ModeNo, ModeIdIndex, RefreshRateTableIndex, HwInfo);
     XGI_New_SetCRT1VCLK(XGI_Pr, ModeNo, ModeIdIndex, HwInfo, RefreshRateTableIndex);
  }

  XGI_New_SetCRT1ModeRegs(XGI_Pr, HwInfo, ModeNo, ModeIdIndex, RefreshRateTableIndex);

  XGI_New_LoadDAC(XGI_Pr, HwInfo, ModeNo, ModeIdIndex);
}


/*********************************************/
/*         XFree86: SET SCREEN PITCH         */
/*********************************************/

#ifdef LINUX_XF86
static void
XGI_SetPitchCRT1(VB_DEVICE_INFO *XGI_Pr, ScrnInfoPtr pScrn)
{
   XGIPtr pXGI = XGIPTR(pScrn);
   UShort HDisplay = pXGI->scrnPitch >> 3;

   XGI_SetReg(XGI_Pr->P3d4,0x13,(HDisplay & 0xFF));
   XGI_SetRegANDOR(XGI_Pr->P3c4,0x0E,0xF0,(HDisplay>>8));
}
#endif

/*********************************************/
/*          XFree86: XGIBIOSSetMode()        */
/*           for non-Dual-Head mode          */
/*********************************************/

#ifdef LINUX_XF86
BOOLEAN
XGIBIOSSetMode(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
	       ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    UShort  ModeNo=0;
    BOOLEAN SetModeRet = FALSE ;
    UShort  HDisplay = pXGI->scrnOffset >> 3 ;

#if XGI_USING_BIOS_SETMODE
    PDEBUG(ErrorF("XGI_USING_BIOS_SETMODE \n"));
    if ((pXGI->pVbe != NULL) && (pXGI->pVbe->pInt10 != NULL)) {
        xf86Int10InfoPtr pInt = pXGI->pVbe->pInt10;

        if (xf86LoadSubModule(pScrn, "int10")) {
            pInt->num = 0x10;
            pInt->ax = 0x80 | ModeNo;

            /* ah = 0, set mode */
            xf86ExecX86int10(pInt);
            SetModeRet = ((pInt->ax & 0x7f) == ModeNo);
        }
    }
    else
#endif
    {
        PDEBUG(ErrorF("XGI_USING_C_code_SETMODE \n"));
		/* Jong 08/21/2007; support external modeline in X configuration file */
		/* ------------------------------------------------------------------ */
		HwInfo->BPP = pScrn->bitsPerPixel;
		HwInfo->Frequency = mode->VRefresh;
		HwInfo->Horizontal_ACTIVE = mode->HDisplay;
		HwInfo->Vertical_ACTIVE = mode->VDisplay;
		HwInfo->Interlace=FALSE; 

		if( (mode->type == M_T_USERDEF) || ((mode->type & M_T_CLOCK_CRTC_C) == M_T_CLOCK_CRTC_C) ) /* custom mode */
		{
			xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3, "Setting a customer mode %dx%d\n", mode->HDisplay, mode->VDisplay);

			HwInfo->SpecifyTiming = TRUE;
			HwInfo->Horizontal_FP = mode->HSyncStart - mode->HDisplay; /* HSyncStart - HDisplay */
			HwInfo->Horizontal_BP = mode->HTotal - mode->HSyncEnd; /* HTotal - HSyncEnd */
			HwInfo->Horizontal_SYNC = mode->HSyncEnd - mode->HSyncStart; /* HSyncEnd - HSyncStart */
			HwInfo->Vertical_FP =  mode->VSyncStart - mode->VDisplay;
			HwInfo->Vertical_BP = mode->VTotal - mode->VSyncEnd;
			HwInfo->Vertical_SYNC = mode->VSyncEnd - mode->VSyncStart;
			HwInfo->DCLK = mode->Clock;
		}
		else
		{
			HwInfo->SpecifyTiming = FALSE;

			ModeNo = XGI_CalcModeIndex(pScrn, mode, pXGI->VBFlags);
			if(!ModeNo) return FALSE;

			xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3, "Setting a standard mode 0x%x\n", ModeNo);
		}
		/* ------------------------------------------------------------------ */

        SetModeRet = XGISetModeNew(HwInfo, XGI_Pr, ModeNo);
        PDEBUG(ErrorF("out_of_C_code_SETMODE \n"));
    }
    
    
    /* SetPitch: Adapt to virtual size & position */
    if ((ModeNo > 0x13) || (mode->type == M_T_USERDEF) || ((mode->type & M_T_CLOCK_CRTC_C) == M_T_CLOCK_CRTC_C)) {
        XGI_SetReg(XGI_Pr->Part1Port, 0x2f, 1);  //yilin for crt2pitch it shoude modify if not colone mode
        XGI_SetReg(XGI_Pr->Part1Port, 0x07, (HDisplay & 0xFF));
        XGI_SetRegANDOR(XGI_Pr->Part1Port, 0x09, 0xF0, (HDisplay>>8)); 

		/* Jong10052009; Set pitch with HDisplay = pXGI->scrnOffset >> 3 */
	    PDEBUG(ErrorF("scrnOffset is %d...\n", pXGI->scrnOffset));
        XGI_SetReg(XGI_Pr->P3d4,0x13,(HDisplay & 0xFF));
        XGI_SetRegANDOR(XGI_Pr->P3c4,0x0E,0xF0,(HDisplay>>8));
		/*
        XGI_SetReg(XGI_Pr->P3d4,0x13,(HDisplay & 0xFF));
        XGI_SetRegANDOR(XGI_Pr->P3c4,0x0E,0xF0,(HDisplay>>8)); */
    }

    return SetModeRet;
}

/*********************************************/
/*       XFree86: XGIBIOSSetModeCRT1()       */
/*           for Dual-Head modes             */
/*********************************************/

BOOLEAN
XGIBIOSSetModeCRT1(VB_DEVICE_INFO *XGI_Pr, PXGI_HW_DEVICE_INFO HwInfo,
		   ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    USHORT  ModeIdIndex, ModeNo=0;
    UCHAR backupreg=0;
    unsigned vga_info;
    XGIEntPtr pXGIEnt = ENTITY_PRIVATE(pXGI);
    UCHAR backupcr30, backupcr31, backupcr38, backupcr35, backupp40d=0;


    ModeNo = XGI_CalcModeIndex(pScrn, mode, pXGI->VBFlags);
    if(!ModeNo) return FALSE;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                   "Setting standard mode 0x%x on CRT1\n", ModeNo);

#if (defined(i386) || defined(__i386) || defined(__i386__) || defined(__AMD64__))
    vga_info = XGI_GetSetBIOSScratch(pScrn, 0x489, 0xff);
#else
    vga_info = 0x11;
#endif
   XGIInitPCIetc(XGI_Pr, HwInfo);

   XGI_SetReg(XGI_Pr->P3c4,0x05,0x86);

   if (!XGI_SearchModeID(XGI_Pr->SModeIDTable, XGI_Pr->EModeIDTable,
                         vga_info, &ModeNo, &ModeIdIndex)) {
       return FALSE;
   }

   /* Determine VBType */
   XGI_New_GetVBType(XGI_Pr, HwInfo);

    if (XGI_Pr->VBType & VB_XGI301BLV302BLV) {
	backupreg = XGI_GetReg(XGI_Pr->P3d4,0x38);
    }

   /* Get VB information (connectors, connected devices) */
   /* (We don't care if the current mode is a CRT2 mode) */
   XGI_SetLowModeTest(XGI_Pr, ModeNo, HwInfo);

   /* Set mode on CRT1 */
   XGI_New_SetCRT1Group(XGI_Pr, HwInfo, ModeNo, ModeIdIndex);
   /* SetPitch: Adapt to virtual size & position */
   XGI_SetPitchCRT1(XGI_Pr, pScrn);


   /* Reset CRT2 if changing mode on CRT1 */
   if(IS_DUAL_HEAD(pXGI)) {
      if(pXGIEnt->CRT2ModeNo != -1) {
         xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
				"(Re-)Setting mode for CRT2\n");
	 backupcr30 = XGI_GetReg(XGI_Pr->P3d4,0x30);
	 backupcr31 = XGI_GetReg(XGI_Pr->P3d4,0x31);
	 backupcr35 = XGI_GetReg(XGI_Pr->P3d4,0x35);
	 backupcr38 = XGI_GetReg(XGI_Pr->P3d4,0x38);
	 if(XGI_Pr->VBType & VB_XGIVB) {
	    /* Backup LUT-enable */
	    if(pXGIEnt->CRT2ModeSet) {
	       backupp40d = XGI_GetReg(XGI_Pr->Part4Port,0x0d) & 0x08;
	    }
	 }
	 if(XGI_Pr->VBInfo & SetCRT2ToLCDA) {
	    XGI_SetReg(XGI_Pr->P3d4,0x30,pXGIEnt->CRT2CR30);
	    XGI_SetReg(XGI_Pr->P3d4,0x31,pXGIEnt->CRT2CR31);
	    XGI_SetReg(XGI_Pr->P3d4,0x35,pXGIEnt->CRT2CR35);
	    XGI_SetReg(XGI_Pr->P3d4,0x38,pXGIEnt->CRT2CR38);
	 }
	
         XGI_SetReg(XGI_Pr->P3d4,0x30,backupcr30);
	 XGI_SetReg(XGI_Pr->P3d4,0x31,backupcr31);
	 XGI_SetReg(XGI_Pr->P3d4,0x35,backupcr35);
	 XGI_SetReg(XGI_Pr->P3d4,0x38,backupcr38);
	 if(XGI_Pr->VBType & VB_XGIVB) {
	    XGI_SetRegANDOR(XGI_Pr->Part4Port,0x0d, ~0x08, backupp40d);
	 }
      }
   }

   /* Warning: From here, the custom mode entries in XGI_Pr are
    * possibly overwritten
    */

   XGI_HandleCRT1(XGI_Pr);

   XGI_New_DisplayOn(XGI_Pr);
   XGI_SetRegByte(XGI_Pr->P3c6,0xFF);

   /* Backup/Set ModeNo in BIOS scratch area */
   XGI_GetSetModeID(pScrn,ModeNo);

   return TRUE;
}
#endif /* Linux_XF86 */
