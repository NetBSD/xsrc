 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv4driver.c,v 1.1.2.4 1998/11/18 16:38:48 hohndel Exp $ */
/*
 * Copyright 1996-1997  David J. McKay
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
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <math.h>
#include <stdlib.h>


#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "vgaPCI.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"


#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "nvcursor.h"
#include "nvreg.h"

#include "nvvga.h"

static unsigned ramType, crystalFreq;

void NV4EnterLeave(Bool enter)
{
    unsigned char temp;

    if (enter)
    {
        xf86EnableIOPorts(vga256InfoRec.scrnIndex);
        outb(vgaIOBase + 4, 17); temp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, temp & 127);
        outb(980,( 31  ));outb(981,  87  ) ;
    }
    else
    {
        outb(vgaIOBase + 4, 17); temp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, (temp & 127) | 128);
        outb(980,( 31  ));outb(981,  153  ) ;
        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

static void MapNV4Regs(void *regBase,void *frameBase)
{
    nvPRAMDACPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (6815744) , ((6819839) - (6815744) +1) ) ;
    nvPFBPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (1048576) , ((1052671) - (1048576) +1) ) ;
    nvPFIFOPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (8192) , ((16383) - (8192) +1) ) ;
    nvPGRAPHPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (4194304) , ((4202495) - (4194304) +1) ) ;
    nvPEXTDEVPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (1052672) , ((1056767) - (1052672) +1) ) ;
    nvPTIMERPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (36864) , ((40959) - (36864) +1) ) ;
    nvPMCPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (0) , ((4095) - (0) +1) ) ;
    nvCHAN0Port=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (8388608) , ((8454143) - (8388608) +1) ) ;
    nvPRAMINPort = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex,
                                             3 ,
                                             ((char *)regBase+ (7340032) +65536),
                                             67584);
}

static void NV4FlipFunctions(vgaVideoChipRec *nv);

int NV4Probe(vgaVideoChipRec *nv,void *base0,void *base1)
{
    int noaccel= (( &vga256InfoRec.options )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] & (1 << (( 60  )% (8 * sizeof(CARD32)) ))) ;

    MapNV4Regs(base0,base1);
    crystalFreq= (nvPEXTDEVPort[((1052672    )- (1052672) )/4]    & (( 1    ) << (6))   ) ? 14318 : 13500;
     
    switch (nvPFBPort[((1048576    )- (1048576) )/4]    & (((unsigned)(1U << ((( 1)-( 0)+1)))-1)  << ( 0))    )
    {
        case 0 :
            vga256InfoRec.videoRam = 1024 * 32;
            break;
        case 1 :
            vga256InfoRec.videoRam = 1024 * 4;
            break;
        case 2 :
            vga256InfoRec.videoRam = 1024 * 8;
            break;
        case 3 :
        default:
            vga256InfoRec.videoRam = 1024 * 16;
            break;
    }
    switch ((nvPFBPort[((1048576    )- (1048576) )/4]    & (((unsigned)(1U << ((( 4)-( 3)+1)))-1)  << ( 3))    ) >> 3)
    {
        case 3 :
            ramType = 1 ;
            break;
        default:
            ramType = 0 ;
            break;
    }
    nv->ChipLinearSize=vga256InfoRec.videoRam * 1024;
    vga256InfoRec.maxClock = 250000 ;
    nv->ChipLinearBase=(int)base1;
    nv->ChipHas32bpp= 1 ;
    xf86ClearIOPortList (vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex,Num_VGA_IOPorts,VGA_IOPorts);
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    vgaIOBase = (inb(972) & 1) ? 976 : 944;
    NV4EnterLeave(1 );
    if (vgaBitsPerPixel==16 && !noaccel)
    {
        ErrorF("%s %s: %s: Setting RGB weight to 555\n","(--)" ,
               vga256InfoRec.name,
               vga256InfoRec.chipset);
        xf86weight.green=xf86weight.blue=xf86weight.red=5;
    }
    ((  &(nv->ChipOptionFlags) )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] |= (1 << (( 60  )% (8 * sizeof(CARD32)) ))) ;
    ((  &(nv->ChipOptionFlags) )->flag_bits[( 62  )/ (8 * sizeof(CARD32)) ] |= (1 << (( 62  )% (8 * sizeof(CARD32)) ))) ;
    NV4FlipFunctions(nv);
    return (1);
}

static int NV4ClockSelect(float clockIn,float *clockOut,int *mOut,int *nOut,int *pOut)
{
    unsigned lowM,     highM;
    unsigned DeltaNew, DeltaOld;
    unsigned VClk;
    unsigned Freq;
    unsigned M;
    unsigned N;
    unsigned O;
    unsigned P;
    
    DeltaOld = -1;
    VClk     = (unsigned)clockIn;

    if (crystalFreq == 14318)
    {
        lowM  = 8;
        highM = 14;
    }
    else
    {
        lowM  = 7;
        highM = 13;
    }                      
    for (P = 0; P <= 4; P ++)
    {
        Freq = VClk << P;
        if ((Freq >= 128000) && (Freq <= 256000))
        {
            for (M = lowM; M <= highM; M++)
            {
                N    = (VClk * M / crystalFreq) << P;
                Freq = (crystalFreq * N / M) >> P;
                if (Freq > VClk)
                    DeltaNew = Freq - VClk;
                else
                    DeltaNew = VClk - Freq;
                if (DeltaNew < DeltaOld)
                {
                    *mOut     = M;
                    *nOut     = N;
                    *pOut     = P;
                    *clockOut = (float)VClk;
                    DeltaOld  = DeltaNew;
                }
            }
        }
    }
    return (DeltaOld != -1);
}

static int CalculateCRTC(DisplayModePtr mode)
{
    int bpp          = vgaBitsPerPixel/8;
    int horizDisplay = (mode->CrtcHDisplay/8) - 1;
    int horizStart   = (mode->CrtcHSyncStart/8) - 1;
    int horizEnd     = (mode->CrtcHSyncEnd/8) - 1;
    int horizTotal   = (mode->CrtcHTotal/8) - 1;
    int vertDisplay  = mode->CrtcVDisplay - 1;
    int vertStart    = mode->CrtcVSyncStart  - 1;
    int vertEnd      = mode->CrtcVSyncEnd - 1;
    int vertTotal    = mode->CrtcVTotal - 2;

    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[19]=((vga256InfoRec.displayWidth/8)*bpp)&255;
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.repaint0=
        (( (((unsigned)((  (vga256InfoRec.displayWidth/8)*bpp  ) & (((unsigned)(1U << ((( 10)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (5))  ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0]  = (( horizTotal - 4 )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[1]  = (( horizDisplay )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[2]  = (( horizDisplay )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[3]  = (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))   | (1<<( 7 )) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[4]  = (( horizStart )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[5]  = (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 5)-( 5)+1)))-1)  << ( 5))  )) >> (5) )  ) << (7))  |
                                                 (( (((unsigned)((  horizEnd  ) & (((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))  ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[6]  = (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 7)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))  ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[7]  = (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (0))  |
                                                 (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (1))  |
                                                 (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (2))  |
                                                 (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (3))  |
                                                 (1<<( 4 )) |
                                                 (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (5))  |
                                                 (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (6))  |
                                                 (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (7))  ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[9]  = (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (5))   | (1<<( 6 )) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[10]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[11]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[12]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[13]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[14]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[15]  = 0;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[16] = (( vertStart )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[17] = (( (((unsigned)((  vertEnd  ) & (((unsigned)(1U << ((( 3)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))   | (1<<( 5 )) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[18] = (( vertDisplay )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[21] = (( vertDisplay )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[22] = (( vertTotal + 1 )&255) ;
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.screenExtra = (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 6)-( 6)+1)))-1)  << ( 6))  )) >> (6) )  ) << (4))   |
                                                       (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (3))   |
                                                       (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (2))   |
                                                       (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (1))   |
                                                       (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (0))  ;
    if (mode->Flags & 32 ) ((vgaNVPtr)vgaNewVideoState)->std.CRTC[9] |= 128;
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.horizExtra = (horizTotal < 260 ? 0 : 1);
    return (1);
}


static void InitPalette(DisplayModePtr mode)
{
    int bpp=vgaBitsPerPixel/8;
    int i;

    if(! (!(vgaBitsPerPixel==8 || xf86weight.green==6)) ) return;
    for (i=0;i<256;i++)
    {
        ((vgaNVPtr)vgaNewVideoState)->std.DAC[i*3]=i>>2;
        ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+1]=i>>2;
        ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+2]=i>>2;
    }
}

static Bool NV4Init(DisplayModePtr mode)
{
    int m,n,p;
    float clockIn=(float)vga256InfoRec.clock[mode->Clock];
    float clockOut;
    int time,data;
    int i;
    int pixelDepth;

    if (!vgaHWInit (mode, sizeof (vgaNVRec)))
        return (0);
    ((vgaNVPtr)vgaNewVideoState)->vgaValid=1;
    InitPalette(mode);
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.config0 = 285217044;
    pixelDepth = (vgaBitsPerPixel+1)/8;
    if (pixelDepth > 3) pixelDepth = 3;
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.pixelFormat=pixelDepth;
    CalculateCRTC(mode);
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.repaint1 =
        ((   mode->CrtcHDisplay<1280   ) << (2))   ;
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.generalControl=
        (( 0    ) << (4))   |
        ((   xf86weight.green==6   ) << (12))   |
        (( 0    ) << (17))   |
        (( 0    ) << (20))   |
        (( 1    ) << (8))   ;
    if (!NV4ClockSelect(clockIn,&clockOut,&m,&n,&p))
    {
        ErrorF("%s %s: %s: Unable to set desired video clock\n",
               "(--)" , vga256InfoRec.name,vga256InfoRec.chipset);
        return (0 );
    }
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.vpllCoeff=
        ((   n   ) << (8))    |
        ((   m   ) << (0))    |
        ((   p   ) << (16))   ;
    nv4UpdateArbitrationSettings((unsigned int)clockOut, 
                                 pixelDepth*8, 
                                 crystalFreq,
                                &(((vgaNVPtr)vgaNewVideoState)->regs.nv4.fifo),
                                &(((vgaNVPtr)vgaNewVideoState)->regs.nv4.fifoControl));
    ((vgaNVPtr)vgaNewVideoState)->regs.nv4.coeffSelect = 268437248;
    return (1 );
}

static void NV4Restore(void *data)
{
    int i, j;
    vgaNVPtr restore=data;
    NV4Registers *nv4=&(restore->regs.nv4);

    vgaProtect(1 );
    outb(980,( 25  ));outb(981, nv4->repaint0 ) ;
    outb(980,( 26  ));outb(981, nv4->repaint1 ) ;
    outb(980,( 37  ));outb(981, nv4->screenExtra ) ;
    outb(980,( 40  ));outb(981, nv4->pixelFormat ) ;
    outb(980,( 45  ));outb(981, nv4->horizExtra ) ;
    outb(980,( 27  ));outb(981, nv4->fifoControl ) ;
    outb(980,( 32  ));outb(981, nv4->fifo ) ;
    nvPFBPort[((1049088    )- (1048576) )/4] =(  nv4->config0  )  ;
    nvPRAMDACPort[((6817032    )- (6815744) )/4] =(  nv4->vpllCoeff  )  ;
    nvPRAMDACPort[((6817036    )- (6815744) )/4] =(  nv4->coeffSelect  )  ;
    nvPRAMDACPort[((6817280    )- (6815744) )/4] =(  nv4->generalControl  )  ;
    vgaHWRestore((vgaHWPtr)restore);
    vgaProtect(0 );
}

static void *NV4Save(void *data)
{
    vgaNVPtr save= ((void *)0) ;

    save=(vgaNVPtr)vgaHWSave((vgaHWPtr)data,sizeof(vgaNVRec));
    save->regs.nv4.repaint0= (outb(980, 25  ),inb(981)) ;
    save->regs.nv4.repaint1= (outb(980, 26  ),inb(981)) ;
    save->regs.nv4.screenExtra= (outb(980, 37  ),inb(981)) ;
    save->regs.nv4.pixelFormat= (outb(980, 40  ),inb(981)) ;
    save->regs.nv4.horizExtra= (outb(980, 45  ),inb(981)) ;
    save->regs.nv4.fifoControl= (outb(980, 27  ),inb(981)) ;
    save->regs.nv4.fifo= (outb(980, 32  ),inb(981)) ;
    save->regs.nv4.config0= nvPFBPort[((1049088    )- (1048576) )/4]   ;
    save->regs.nv4.vpllCoeff= nvPRAMDACPort[((6817032    )- (6815744) )/4]   ;
    save->regs.nv4.coeffSelect= nvPRAMDACPort[((6817036    )- (6815744) )/4]   ;
    save->regs.nv4.generalControl= nvPRAMDACPort[((6817280    )- (6815744) )/4]   ;
    return (void*)save;
}

static void NV4Adjust(int x,int y)
{
    int bpp=vgaBitsPerPixel/8;
    int startAddr=(((y*vga256InfoRec.virtualX)+x)*bpp);
    int offset=startAddr>>2;
    int pan=(startAddr&3)*2;
    unsigned char byte;

    outb(980,( 31  ));outb(981,  87  ) ;
     
    outb(980,( 13 ));outb(981, (( offset )&255)  ) ;
    outb(980,( 12 ));outb(981, (( (((unsigned)((  offset  ) & (((unsigned)(1U << ((( 15)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (0))   ) ;
    byte= (outb(980, 25  ),inb(981))  & ~(((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))    ;
    outb(980,( 25  ));outb(981, (( (((unsigned)((  offset  ) & (((unsigned)(1U << ((( 20)-( 16)+1)))-1)  << ( 16))  )) >> (16) )  ) << (0))  |byte ) ;
     
    byte=inb(vgaIOBase+10);
    outb(960,19);
    outb(960,pan);
}

static int NV4ValidMode(DisplayModePtr mode,Bool verbose,int flag)
{
    unsigned bw, bwMax, bpp;

    bpp = (vgaBitsPerPixel + 1) / 8;
    bwMax = (ramType == 0 ) ? 1000000 : 800000;
    bw = mode->Clock * bpp;
    return (bw > bwMax ? 255  : 0 );
}

extern vgaHWCursorRec vgaHWCursor;

static void NV4FbInit(void)
{
    if (! ((  &vga256InfoRec.options )->flag_bits[( 62  )/ (8 * sizeof(CARD32)) ] & (1 << (( 62  )% (8 * sizeof(CARD32)) ))) )
    {
        vgaHWCursor.Initialized = 1 ;
        vgaHWCursor.Init = NV4CursorInit;
        vgaHWCursor.Restore = NV4RestoreCursor;
        vgaHWCursor.Warp = NV4WarpCursor;
        vgaHWCursor.QueryBestSize = NV4QueryBestSize;
        if (xf86Verbose)
        {
            ErrorF("%s %s: %s: Using hardware cursor\n","(--)" ,
                   vga256InfoRec.name,vga256InfoRec.chipset);
        }
    }
    if (! ((  &vga256InfoRec.options )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] & (1 << (( 60  )% (8 * sizeof(CARD32)) ))) )
    {
        NVAccelInit();
    }
}

static void NV4DisplayPowerManagementSet(int mode)
{
}

static Bool NV4ScreenInit(ScreenPtr pScreen,pointer pbits,
                          int xsize,int ysize,int dpix,int dpiy,int width)
{
    return (1 );
}

static void NV4SaveScreen(int on)
{
    vgaHWSaveScreen(on);
}

static void NV4GetMode(DisplayModePtr display)
{
    
}

static void NV4FlipFunctions(vgaVideoChipRec *nv)
{
    nv->ChipEnterLeave=NV4EnterLeave;
    nv->ChipInit=NV4Init;
    nv->ChipValidMode=NV4ValidMode;
    nv->ChipSave=NV4Save;
    nv->ChipRestore=NV4Restore;
    nv->ChipAdjust=NV4Adjust;
    nv->ChipSaveScreen=NV4SaveScreen;
    nv->ChipGetMode=(void (*)())NoopDDA;
    nv->ChipFbInit=NV4FbInit;
}
