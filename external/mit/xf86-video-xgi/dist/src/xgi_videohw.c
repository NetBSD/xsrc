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

/****************************************************************************
* raw register access : these routines directly interact with the xgi's
*                       control aperature.  must not be called until after
*                       the board's pci memory has been mapped.
****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define XGI_VIDEO_HW /* avoid compile error in xgi.h; weird!  */
#include "xgi.h"

#include "xgi_videohw.h"
#include "xgi_video.h"
#include "fourcc.h"

#define  CAPTURE_340A1
/*
static CARD32 _XGIRead(XGIPtr pXGI, CARD32 reg)
{
    return *(pXGI->IOBase + reg);
}

static void _XGIWrite(XGIPtr pXGI, CARD32 reg, CARD32 data)
{
    *(pXGI->IOBase + reg) = data;
}
*/
static CARD8 GetVideoReg(XGIPtr pXGI, CARD8 reg)
{
    outb (pXGI->RelIO + vi_index_offset, reg);
    return inb(pXGI->RelIO + vi_data_offset);
}

static void SetVideoReg(XGIPtr pXGI, CARD8 reg, CARD8 data)
{
    outb (pXGI->RelIO + vi_index_offset, reg);
    outb (pXGI->RelIO + vi_data_offset, data);
}

static void SetVideoRegMask(XGIPtr pXGI, CARD8 reg, CARD8 data, CARD8 mask)
{
    CARD8   old;

    outb (pXGI->RelIO + vi_index_offset, reg);
    old = inb(pXGI->RelIO + vi_data_offset);
    data = (data & mask) | (old & (~mask));
    outb (pXGI->RelIO + vi_data_offset, data);
}

static CARD8 GetSRReg(XGIPtr pXGI, CARD8 reg)
{
    outb (pXGI->RelIO + sr_index_offset, 0x05);
    if (inb (pXGI->RelIO + sr_data_offset) != 0xa1)
            outb (pXGI->RelIO + sr_data_offset, 0x86);
    outb (pXGI->RelIO + sr_index_offset, reg);
    return inb(pXGI->RelIO + sr_data_offset);
}

static void SetSRReg(XGIPtr pXGI, CARD8 reg, CARD8 data)
{
    outb (pXGI->RelIO + sr_index_offset, 0x05);
    if (inb (pXGI->RelIO + sr_data_offset) != 0xa1)
            outb (pXGI->RelIO + sr_data_offset, 0x86);
    outb (pXGI->RelIO + sr_index_offset, reg);
    outb (pXGI->RelIO + sr_data_offset, data);
}

void SetSRRegMask(XGIPtr pXGI, CARD8 reg, CARD8 data, CARD8 mask)
{
    CARD8   old;

    outb (pXGI->RelIO + sr_index_offset, 0x05);
    if (inb (pXGI->RelIO + sr_data_offset) != 0xa1)
            outb (pXGI->RelIO + sr_data_offset, 0x86);
    outb (pXGI->RelIO + sr_index_offset, reg);
    old = inb(pXGI->RelIO + sr_data_offset);
    data = (data & mask) | (old & (~mask));
    outb (pXGI->RelIO + sr_data_offset, data);
}
/*
static void SetVCReg(XGIPtr pXGI, CARD8 reg, CARD8 data)
{
    outb (pXGI->RelIO + vc_index_offset, reg);
    outb (pXGI->RelIO + vc_data_offset, data);
}
*/
static void SetVCRegMask(XGIPtr pXGI, CARD8 reg, CARD8 data, CARD8 mask)
{
    CARD8   old;

    outb (pXGI->RelIO + vc_index_offset, reg);
    old = inb(pXGI->RelIO + vc_data_offset);
    data = (data & mask) | (old & (~mask));
    outb (pXGI->RelIO + vc_data_offset, data);
}
/*
static CARD8 GetXGIReg(XGIPtr pXGI, CARD8 index_offset, CARD8 reg)
{
    outb (pXGI->RelIO + index_offset, reg);
    return inb(pXGI->RelIO + index_offset+1);
}

static void SetXGIReg(XGIPtr pXGI, CARD8 index_offset, CARD8 reg, CARD8 data)
{
    outb (pXGI->RelIO + index_offset, reg);
    outb (pXGI->RelIO + index_offset+1, data);
}
*/
static float tap_dda_func(float x)
{
    double pi = 3.14159265358979;
    float r = 0.5;
    float y;

    if (x == 0)
            y = 1.0;
    else if ((x == -1.0/(2.0*r)) || (x == 1.0/(2.0*r)))
            y = (float) (r/2.0 * sin(pi/(2.0*r)));
    else
            y = (float) (sin(pi*x)/(pi*x)*cos(r*pi*x)/(1-4*r*r*x*x));

    return y;
}

/* ----------------T05_EnableCapture()---------------- */
#ifdef CAPTURE_340A1
void SetEnableCaptureReg(XGIPtr pXGI, Bool bEnable, Bool bFlip)
{
   if (bEnable)
   {
      SetVCRegMask(pXGI, Index_VC_Ver_Down_Scale_Factor_Over, 0x00, 0x10);

      if (pXGI->Chipset == PCI_CHIP_XGIXG40)
         SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x20, 0x20);
      else
         SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x01, 0x01);
   }
   else
   {
      SetVCRegMask(pXGI, Index_VC_Ver_Down_Scale_Factor_Over, 0x10, 0x10);

      if (pXGI->Chipset == PCI_CHIP_XGIXG40)
         SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x00, 0x20);
      else
         SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x01);
   }


   if (pXGI->Chipset == PCI_CHIP_XGIXG40)
   {
      if(bFlip)
         SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x01, 0x01);
      else
         SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x01);
   }
}
#else
void T05_EnableCapture(XGIPtr pXGI, Bool bEnable)
{
   if (bEnable)
   {
      SetVCRegMask(pXGI, Index_VC_Ver_Down_Scale_Factor_Over, 0x00, 0x10);
      SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x01, 0x01);
   }
   else
   {
      SetVCRegMask(pXGI, Index_VC_Ver_Down_Scale_Factor_Over, 0x10, 0x10);
      SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x01);
   }
}
#endif

static void
XGIComputeXvGamma(XGIPtr pXGI)
{
    int num = 255, i;
    double red = 1.0 / (double)((double)pXGI->XvGammaRed / 1000);
    double green = 1.0 / (double)((double)pXGI->XvGammaGreen / 1000);
    double blue = 1.0 / (double)((double)pXGI->XvGammaBlue / 1000);
	
    for(i = 0; i <= num; i++) {
        pXGI->XvGammaRampRed[i] =
	    (red == 1.0) ? i : (CARD8)(pow((double)i / (double)num, red) * (double)num + 0.5);

		pXGI->XvGammaRampGreen[i] =
	    (green == 1.0) ? i : (CARD8)(pow((double)i / (double)num, green) * (double)num + 0.5);

		pXGI->XvGammaRampBlue[i] =
	    (blue == 1.0) ? i : (CARD8)(pow((double)i / (double)num, blue) * (double)num + 0.5);
    }
}

static void
XGISetXvGamma(XGIPtr pXGI)
{
    int i;
    unsigned char backup = GetSRReg(pXGI, Index_SR_Power_Management);
	/* SR1F[4:3]
	 * 10: disable gamma1, enable gamma0
	 */
    SetSRRegMask(pXGI, Index_SR_Power_Management, 0x08, 0x18);
    for(i = 0; i <= 255; i++) {
       MMIO_OUT32(pXGI->IOBase, REG_GAMMA_PALETTE,
       			(i << 24)     |
			(pXGI->XvGammaRampBlue[i] << 16) |
			(pXGI->XvGammaRampGreen[i] << 8) |
			pXGI->XvGammaRampRed[i]);
    }
    SetSRRegMask(pXGI, Index_SR_Power_Management, backup, 0xff);
}

void
XGIUpdateXvGamma(XGIPtr pXGI, XGIPortPrivPtr pPriv)
{
    unsigned char sr7 = GetSRReg(pXGI, Index_SR_RAMDAC_Ctrl);

    /* SR7 [2]: 24-bit palette RAM and Gamma correction enable */
    if(!(sr7 & 0x04)) return;   
    
    XGIComputeXvGamma(pXGI);
    XGISetXvGamma(pXGI);
}

/* ----------------SetDDAReg()------------------------------------ */
VOID SetDDAReg (XGIPtr pXGI, float scale)
{
    float WW, W[4], tempW[4];
    int i, j, w, WeightMat[16][4];
    int *wm1, *wm2, *wm3, *wm4, *temp1, *temp2, *temp3, *temp4;

    for (i=0; i<16; i++)
    {
            /* The order of weights are inversed for convolution */
            W[0] = tap_dda_func((float)((1.0+(i/16.0))/scale));
            W[1] = tap_dda_func((float)((0.0+(i/16.0))/scale));
            W[2] = tap_dda_func((float)((-1.0+(i/16.0))/scale));
            W[3] = tap_dda_func((float)((-2.0+(i/16.0))/scale));

            /* Normalize the weights */
            WW = W[0]+W[1]+W[2]+W[3];

            /* for rouding */
            for(j=0; j<4; j++)
                    tempW[j] = (float)((W[j]/WW*16)+0.5);

            WeightMat[i][0] = (int) tempW[0];
            WeightMat[i][1] = (int) tempW[1];
            WeightMat[i][2] = (int) tempW[2];
            WeightMat[i][3] = (int) tempW[3];

            /* check for display abnormal caused by rounding */
            w = WeightMat[i][0] + WeightMat[i][1] + WeightMat[i][2] + WeightMat[i][3];
            if( w != 16 )
            {
                    temp1 = ( WeightMat[i][0] > WeightMat[i][1] ) ? &WeightMat[i][0] : &WeightMat[i][1];
                    temp2 = ( WeightMat[i][0] > WeightMat[i][1] ) ? &WeightMat[i][1] : &WeightMat[i][0];
                    temp3 = ( WeightMat[i][2] > WeightMat[i][3] ) ? &WeightMat[i][2] : &WeightMat[i][3];
                    temp4 = ( WeightMat[i][2] > WeightMat[i][3] ) ? &WeightMat[i][3] : &WeightMat[i][2];
                    wm1 = ( *temp1 > *temp3) ? temp1 : temp3;
                    wm4 = ( *temp2 > *temp4) ? temp4 : temp2;
                    wm2 = ( wm1 == temp1 ) ? temp3 : temp1;
                    wm3 = ( wm4 == temp2 ) ? temp4 : temp2;

                switch(w)
                {
                    case 12:
                        WeightMat[i][0]++;
                        WeightMat[i][1]++;
                        WeightMat[i][2]++;
                        WeightMat[i][3]++;
                        break;

                    case 13:
                        (*wm1)++;
                        (*wm4)++;
                        if( *wm2 > *wm3 )
                            (*wm2)++;
                        else
                            (*wm3)++;
                        break;

                    case 14:
                        (*wm1)++;
                        (*wm4)++;
                        break;

                    case 15:
                        (*wm1)++;
                        break;

                    case 17:
                            (*wm4)--;
                            break;

                    case 18:
                        (*wm1)--;
                        (*wm4)--;
                        break;

                    case 19:
                        (*wm1)--;
                        (*wm4)--;
                        if( *wm2 > *wm3 )
                            (*wm3)--;
                        else
                            (*wm2)--;
                        break;
                    case 20:
                        WeightMat[i][0]--;
                        WeightMat[i][1]--;
                        WeightMat[i][2]--;
                        WeightMat[i][3]--;
                        break;
                    default:
                        /* ErrorF("Invalid WeightMat value!\n"); */
                        break;
                }
            }
    }

    /* set DDA registers */
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_A0, WeightMat[0][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_A1, WeightMat[0][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_A2, WeightMat[0][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_A3, WeightMat[0][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_B0, WeightMat[1][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_B1, WeightMat[1][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_B2, WeightMat[1][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_B3, WeightMat[1][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_C0, WeightMat[2][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_C1, WeightMat[2][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_C2, WeightMat[2][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_C3, WeightMat[2][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_D0, WeightMat[3][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_D1, WeightMat[3][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_D2, WeightMat[3][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_D3, WeightMat[3][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_E0, WeightMat[4][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_E1, WeightMat[4][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_E2, WeightMat[4][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_E3, WeightMat[4][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_F0, WeightMat[5][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_F1, WeightMat[5][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_F2, WeightMat[5][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_F3, WeightMat[5][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_G0, WeightMat[6][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_G1, WeightMat[6][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_G2, WeightMat[6][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_G3, WeightMat[6][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_H0, WeightMat[7][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_H1, WeightMat[7][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_H2, WeightMat[7][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_H3, WeightMat[7][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_I0, WeightMat[8][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_I1, WeightMat[8][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_I2, WeightMat[8][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_I3, WeightMat[8][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_J0, WeightMat[9][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_J1, WeightMat[9][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_J2, WeightMat[9][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_J3, WeightMat[9][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_K0, WeightMat[10][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_K1, WeightMat[10][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_K2, WeightMat[10][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_K3, WeightMat[10][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_L0, WeightMat[11][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_L1, WeightMat[11][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_L2, WeightMat[11][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_L3, WeightMat[11][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_M0, WeightMat[12][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_M1, WeightMat[12][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_M2, WeightMat[12][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_M3, WeightMat[12][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_N0, WeightMat[13][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_N1, WeightMat[13][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_N2, WeightMat[13][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_N3, WeightMat[13][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_O0, WeightMat[14][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_O1, WeightMat[14][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_O2, WeightMat[14][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_O3, WeightMat[14][3], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_P0, WeightMat[15][0], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_P1, WeightMat[15][1], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_P2, WeightMat[15][2], 0x3F);
    SetVideoRegMask(pXGI, Index_DDA_Weighting_Matrix_P3, WeightMat[15][3], 0x3F);
}

void
XGIResetVideo(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    XGIPortPrivPtr pPriv = GET_PORT_PRIVATE(pScrn);

    /* Reset Xv gamma correction */
    XGIUpdateXvGamma(pXGI, pPriv);

    if (GetSRReg (pXGI, 0x05) != 0xa1)
    {
            SetSRReg (pXGI, 0x05, 0x86);
            if (GetSRReg (pXGI, 0x05) != 0xa1)
	    {}	    
                    /* xf86DrvMsg(pScrn->scrnIndex, X_ERROR, */
                    /* "Standard password not initialize\n"); */
    }
    if (GetVideoReg (pXGI, Index_VI_Passwd) != 0xa1)
    {
            SetVideoReg (pXGI, Index_VI_Passwd, 0x86);
            if (GetVideoReg (pXGI, Index_VI_Passwd) != 0xa1)
	    {}
                    /* xf86DrvMsg(pScrn->scrnIndex, X_ERROR, */
                    /* "Video password not initialize\n"); */
    }

    /* Initial Overlay 1 */
    SetVideoRegMask(pXGI, Index_VI_Control_Misc2, 0x00, 0x81);
    SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x03);
    /* Turn on Bob mode and digital video data transform bit */
    SetVideoRegMask(pXGI, Index_VI_Control_Misc1, 0x82, 0x82);
    SetVideoRegMask(pXGI, Index_VI_Scale_Control, 0x60, 0x60);
    SetVideoRegMask(pXGI, Index_VI_Contrast_Enh_Ctrl, 0x04, 0x1F);

    SetVideoReg(pXGI, Index_VI_Disp_Y_Buf_Preset_Low,     0x00);
    SetVideoReg(pXGI, Index_VI_Disp_Y_Buf_Preset_Middle,  0x00);
    SetVideoReg(pXGI, Index_VI_Disp_UV_Buf_Preset_Low,    0x00);
    SetVideoReg(pXGI, Index_VI_Disp_UV_Buf_Preset_Middle, 0x00);
    SetVideoReg(pXGI, Index_VI_Disp_Y_UV_Buf_Preset_High, 0x00);
    SetVideoReg(pXGI, Index_VI_Play_Threshold_Low,        0x00);
    SetVideoRegMask(pXGI, Index_VI_Play_Threshold_Low_Ext, 0x00, 0x01);
    SetVideoReg(pXGI, Index_VI_Play_Threshold_High,       0x00);
    SetVideoRegMask(pXGI, Index_VI_Play_Threshold_High_Ext, 0x00, 0x01);

    /* Disable fast page flip */
    SetSRRegMask(pXGI, Index_SR_CRT_Misc_Ctrl,    0x00,  0x02);

    #ifdef CAPTURE_340A1
       SetEnableCaptureReg(pXGI, FALSE, FALSE);
    #else
       SetEnableCaptureReg(pXGI, FALSE);
    #endif

    /* Disable video processor */
    SetSRRegMask(pXGI, Index_Video_Process,        0x00, 0x02);

        /* Enable Horizontal 4-tap DDA mode */
        SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x40, 0x40);
        /* Disable Vertical 4-tap DDA mode -- not surport not */
        SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x00, 0x80);
        /* The DDA registers should set scale to 1 as default */
    SetDDAReg (pXGI, 1.0);

        /*for 341, Init VR 2F [D5] to be 1,to set software flip as default*/
        SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x20, 0x20);

   /*Disable Contrast enhancement*/
   SetVideoRegMask(pXGI, Index_VI_Key_Overlay_OP, 0x00, 0x10);

   /* Default Color */
   SetVideoReg(pXGI, Index_VI_Brightness, Default_Brightness);
   SetVideoRegMask(pXGI, Index_VI_Contrast_Enh_Ctrl, Default_Contrast, 0x07);
   SetVideoReg(pXGI, Index_VI_Saturation, Default_Saturation);
   SetVideoRegMask(pXGI, Index_VI_Hue, Default_Hue, 0x07);
}

void
SetMergeLineBufReg(XGIPtr pXGI, Bool enable)
{
  if (enable) {
          /* select video 1 and disable All line buffer Merge */
     SetVideoRegMask(pXGI, Index_VI_Control_Misc2, 0x00, 0x11);
          /* set Individual Line buffer Merge */
     SetVideoRegMask(pXGI, Index_VI_Control_Misc1, 0x04, 0x04);
  }
  else {
          /* select video 1 and disable All line buffer Merge */
          SetVideoRegMask(pXGI, Index_VI_Control_Misc2, 0x00, 0x11);
          /* disable Individual Line buffer Merge */
          SetVideoRegMask(pXGI, Index_VI_Control_Misc1, 0x00, 0x04);
  }
}

void
SetVideoFormatReg(XGIPtr pXGI, int format)
{
    CARD8 fmt;

    switch (format)
    {
       case PIXEL_FMT_YV12:
            fmt = 0x0c;
            break;

       case PIXEL_FMT_YUY2:
            fmt = 0x28;
            break;

       case PIXEL_FMT_UYVY:
            fmt = 0x08;
            break;
			
	   case PIXEL_FMT_YVYU:
	   		fmt = 0x38;
		    break;
			
	   case PIXEL_FMT_NV12:
	        fmt = 0x4c;
		    break;
			
	   case PIXEL_FMT_NV21:
	        fmt = 0x5c;
		    break;
			
	   case PIXEL_FMT_RGB5:   /* D[5:4] : 00 RGB555, 01 RGB 565 */
	        fmt = 0x00;
		    break;
			
	   case PIXEL_FMT_RGB6:
	        fmt = 0x10;
		    break;
					
       default:
            fmt = 0x00;
            break;
    }

    SetVideoRegMask(pXGI, Index_VI_Control_Misc0, fmt, 0x7c);
}

void
SetColorkeyReg(XGIPtr pXGI, CARD32 colorkey)
{
    CARD8 r, g, b;

    b = LOBYTE(LOWORD(colorkey));
    g = HIBYTE(LOWORD(colorkey));
    r = LOBYTE(HIWORD(colorkey));

   /* Activate the colorkey mode */
    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Blue_Min  , b);
    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Green_Min , g);
    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Red_Min   , r);

    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Blue_Max  , b);
    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Green_Max , g);
    SetVideoReg(pXGI, Index_VI_Overlay_ColorKey_Red_Max   , r);
}

void
SetVideoBrightnessReg(XGIPtr pXGI, INT32 value)
{
    CARD8   brightness;

    brightness = LOBYTE(value);

    SetVideoReg(pXGI, Index_VI_Brightness  ,brightness);
}

void
SetVideoContrastReg(XGIPtr pXGI, INT32 value)
{
    CARD8   contrast;

    contrast = (CARD8)(((value * 7) / 255) & 0x000F);

    SetVideoRegMask(pXGI, Index_VI_Contrast_Enh_Ctrl, contrast, 0x07);
}

void
SetVideoHueReg(XGIPtr pXGI, INT32 value)
{
    CARD8   hue;

    if ( value > 0 )
    {
       SetVideoRegMask(pXGI, Index_VI_Hue, 0x00, 0x08);
    }
    else
    {
       SetVideoRegMask(pXGI, Index_VI_Hue, 0x08, 0x08);
	   
       value = -value;
    }

    hue = (CARD8)(((value * 7) / 180) & 0x0007);


    SetVideoRegMask(pXGI, Index_VI_Hue, hue, 0x07);
}

void
SetVideoSaturationReg(XGIPtr pXGI, INT32 value)
{
    CARD8   saturation;

    if ( value > 0 )
    {
       SetVideoRegMask(pXGI, Index_VI_Saturation, 0x00, 0x08);
       SetVideoRegMask(pXGI, Index_VI_Saturation, 0x00, 0x80);
    }
    else
    {
       SetVideoRegMask(pXGI, Index_VI_Saturation, 0x08, 0x08);
       SetVideoRegMask(pXGI, Index_VI_Saturation, 0x80, 0x80);
	   
       value = -value;
    }

    saturation = (CARD8)(((value * 7) / 180) & 0x000F);

    SetVideoRegMask(pXGI, Index_VI_Saturation, saturation, 0x07);
    SetVideoRegMask(pXGI, Index_VI_Saturation, saturation << 4, 0x70);
}

void
SetOverlayReg(XGIPtr pXGI, XGIOverlayPtr pOverlay)
{

    ScrnInfoPtr pScrn = pXGI->pScrn;
	XGIPortPrivPtr pPriv = GET_PORT_PRIVATE(pScrn);
	
    CARD32 tmpYPitch;
    CARD16 top, left;
    CARD16 bottom, right;
    CARD32 PSY, PSU, PSV;
    CARD16 screenX = pScrn->currentMode->HDisplay;
    CARD16 screenY = pScrn->currentMode->VDisplay;

    top    = pOverlay->dstBox.y1;
    bottom = pOverlay->dstBox.y2;

    if (bottom > screenY)
        bottom = screenY;

    left  = pOverlay->dstBox.x1;
    right = pOverlay->dstBox.x2;

    if (right > screenX)
        right = screenX;

    SetVideoReg(pXGI, Index_VI_Win_Hor_Disp_Start_Low, LOBYTE(left));
    SetVideoReg(pXGI, Index_VI_Win_Hor_Disp_End_Low, LOBYTE(right));
    SetVideoReg(pXGI, Index_VI_Win_Hor_Over, (HIBYTE(right)<<4)|HIBYTE(left));

    SetVideoReg(pXGI, Index_VI_Win_Ver_Disp_Start_Low, LOBYTE(top));
    SetVideoReg(pXGI, Index_VI_Win_Ver_Disp_End_Low, LOBYTE(bottom));
    SetVideoReg(pXGI, Index_VI_Win_Ver_Over, (HIBYTE(bottom)<<4)|HIBYTE(top));

    /*  Set ContrastFactor */
    SetVideoRegMask(pXGI, Index_VI_Contrast_Enh_Ctrl, pOverlay->dwContrastFactor << 6, 0xc0);
    SetVideoReg(pXGI, Index_VI_Contrast_Factor, pOverlay->SamplePixel);

	/* SetVideoRegMask(pXGI, Index_VI_Control_Misc3, 0x00, 0x03); */

	/* Does need to merge line buffer*/
	SetMergeLineBufReg(pXGI, pOverlay->pitch > pPriv->linebufMergeLimit);

	/* set video format */
	SetVideoFormatReg(pXGI, pOverlay->pixelFormat);

	/* set line buffer size */
	SetVideoReg(pXGI, Index_VI_Line_Buffer_Size, LOBYTE(LOWORD(pOverlay->lineBufSize)));
	SetVideoReg(pXGI, Index_VI_Line_Buffer_Size_Ext, HIBYTE(LOWORD(pOverlay->lineBufSize)));

	/* set Key Overlay Operation Mode */
	SetVideoRegMask (pXGI, Index_VI_Key_Overlay_OP, pOverlay->keyOP, 0x0f);


    /* set scale factor */
    SetVideoReg (pXGI, Index_VI_Hor_Post_Up_Scale_Low, LOBYTE(pOverlay->HUSF));
    SetVideoReg (pXGI, Index_VI_Hor_Post_Up_Scale_High, HIBYTE(pOverlay->HUSF));

    SetVideoReg (pXGI, Index_VI_Ver_Up_Scale_Low, LOBYTE(pOverlay->VUSF));
    SetVideoReg (pXGI, Index_VI_Ver_Up_Scale_High, HIBYTE(pOverlay->VUSF));

    SetVideoRegMask (pXGI, Index_VI_Scale_Control, (pOverlay->IntBit << 3)|(pOverlay->wHPre), 0x7f);

    /* for 340 4-tap DDA */
    SetDDAReg(pXGI, pOverlay->f_scale);

    /* set frame or field mode */
    SetVideoRegMask(pXGI, Index_VI_Control_Misc1, pOverlay->bobEnable, 0x1a);

    /* set Y start address */
    PSY = pOverlay->PSY >>1;        /* in unit of word */

    SetVideoReg (pXGI, Index_VI_Disp_Y_Buf_Start_Low, LOBYTE(LOWORD(PSY)));
    SetVideoReg (pXGI, Index_VI_Disp_Y_Buf_Start_Middle, HIBYTE(LOWORD(PSY)));
    SetVideoReg (pXGI, Index_VI_Disp_Y_Buf_Start_High, LOBYTE(HIWORD(PSY)));
    SetVideoRegMask(pXGI, Index_VI_Disp_Y_Buf_EXT_High, HIBYTE(HIWORD(PSY)), 0x03);
    tmpYPitch = pOverlay->pitch >> 1 ; /* in unit of word */
	
    if ((pOverlay->pixelFormat == PIXEL_FMT_YV12) ||
		(pOverlay->pixelFormat == PIXEL_FMT_NV12) ||
		(pOverlay->pixelFormat == PIXEL_FMT_NV21))
    {
        /* set UV pitch */
		CARD32  uvpitch = tmpYPitch;
        BYTE    bYUV_Pitch_High;

		if(pOverlay->pixelFormat == PIXEL_FMT_YV12)
	    	uvpitch >>= 1;
/*
        bYUV_Pitch_High = (HIBYTE(LOWORD(uvpitch))<<4) & 0xf0 |
                        (HIBYTE(LOWORD(tmpYPitch))) & 0x0f;  */
        bYUV_Pitch_High = ((HIBYTE(LOWORD(uvpitch))<<4) & 0xf0) | ((HIBYTE(LOWORD(tmpYPitch))) & 0x0f);  
        SetVideoReg    (pXGI, Index_VI_Disp_UV_Buf_Pitch_Low,         LOBYTE(LOWORD(uvpitch)));
        SetVideoReg    (pXGI, Index_VI_Disp_Y_UV_Buf_Pitch_High, bYUV_Pitch_High);
        SetVideoRegMask(pXGI, Index_VI_Disp_UV_Buf_Pitch_EXT_High,        uvpitch >> 12, 0x1f);
		
        /* set U/V start address */
        PSU = pOverlay->PSU >> 1; /* in unit of word; */
        PSV = pOverlay->PSV >> 1; /* in unit of word;; */

        SetVideoReg(pXGI, Index_VI_Disp_U_Buf_Start_Low, LOBYTE(LOWORD(PSU)));
        SetVideoReg(pXGI, Index_VI_Disp_U_Buf_Start_Middle, HIBYTE(LOWORD(PSU)));
        SetVideoReg(pXGI, Index_VI_Disp_U_Buf_Start_High, LOBYTE(HIWORD(PSU)));
        /* [26:24] save in the D[2:0] */
        SetVideoRegMask(pXGI, Index_VI_Disp_U_Buf_EXT_High, HIBYTE(HIWORD(PSU)), 0x03);

        SetVideoReg(pXGI, Index_VI_Disp_V_Buf_Start_Low, LOBYTE(LOWORD(PSV)));
        SetVideoReg(pXGI, Index_VI_Disp_V_Buf_Start_Middle, HIBYTE(LOWORD(PSV)));
        SetVideoReg(pXGI, Index_VI_Disp_V_Buf_Start_High, LOBYTE(HIWORD(PSV)));
        /* [26:24] save in the D[2:0] */
        SetVideoRegMask(pXGI, Index_VI_Disp_V_Buf_EXT_High, HIBYTE(HIWORD(PSV)), 0x03);
        tmpYPitch = pOverlay->pitch >> 1; /* in unit of word */
    }
    else
        SetVideoRegMask(pXGI, Index_VI_Disp_Y_UV_Buf_Pitch_High, HIBYTE(LOWORD(tmpYPitch)), 0x0f);

    /* set Y pitch */

    SetVideoReg(pXGI, Index_VI_Disp_Y_Buf_Pitch_Low, LOBYTE(LOWORD(tmpYPitch)));
    /* [16:12] save in the D[4:0] */
    SetVideoRegMask(pXGI, Index_VI_Disp_Y_Buf_Pitch_EXT_High, (tmpYPitch>>12)&0x1f , 0x1f);


     /* start address ready */
    SetVideoRegMask(pXGI, Index_VI_Control_Misc3, 0x03, 0x03);

    /* set contrast factor */
    /* SetVideoRegMask(pXGI, Index_VI_Contrast_Enh_Ctrl, pOverlay->contrastCtrl<<6, 0xc0); */
    /* SetVideoReg (pXGI, Index_VI_Contrast_Factor, pOverlay->contrastFactor); */
}

void
SetCloseOverlayReg(XGIPtr pXGI)
{
  SetVideoRegMask (pXGI, Index_VI_Control_Misc2, 0x00, 0x01);
  SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x02);
}

void
SetSelectOverlayReg(XGIPtr pXGI, CARD8 index)
{
  SetVideoRegMask(pXGI, Index_VI_Control_Misc2, index, 0x01);
}

void
SetEnableOverlayReg(XGIPtr pXGI, Bool bEnable)
{
        if (bEnable)
                SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x02, 0x02);
        else
                SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x02);
}

void
EnableCaptureAutoFlip(XGIPtr pXGI, Bool bEnable)
{
   if (bEnable)
      SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x01, 0x01);
   else
      SetVideoRegMask(pXGI, Index_VI_Control_Misc0, 0x00, 0x01);
}
