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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3driver.c,v 1.1.2.6 1998/11/18 16:38:45 hohndel Exp $ */
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

void NV3EnterLeave(Bool enter)
{
   unsigned char temp;

  if(enter) {
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    outb(vgaIOBase + 4, 17); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, temp & 127); 
    outb(964,( 6  ));outb(965, 87  ) ;
  }else {
    outb(vgaIOBase + 4, 17); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, (temp & 127) | 128);
    outb(964,( 6  ));outb(965, 153  ) ;
    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }
}








static void MapNV3Regs(void *regBase,void *frameBase)
{
  nvPRAMDACPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (6815744) , ((6819839) - (6815744) +1) ) ;
  nvPFBPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (1048576) , ((1052671) - (1048576) +1) ) ;
  nvPFIFOPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (8192) , ((16383) - (8192) +1) ) ;
  nvPGRAPHPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (4194304) , ((4202495) - (4194304) +1) ) ;
  nvPTIMERPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (36864) , ((40959) - (36864) +1) ) ;
  nvPEXTDEVPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (1052672) , ((1056767) - (1052672) +1) ) ;
  nvPMCPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (0) , ((4095) - (0) +1) ) ;
  nvCHAN0Port=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( regBase ))+ (8388608) , ((8454143) - (8388608) +1) ) ;
  nvPRAMINPort=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex, 3 , ((char*)( frameBase ))+ (12582912) , ((16777215) - (12582912) +1) ) ;
}



static void NV3FlipFunctions(vgaVideoChipRec *nv);

int NV3Probe(vgaVideoChipRec *nv,void *base0,void *base1)
{
  int boot0, chipRev, chipSubrev;
  int noaccel= (( &vga256InfoRec.options )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] & (1 << (( 60  )% (8 * sizeof(CARD32)) ))) ;

  MapNV3Regs(base0,base1);

   
  boot0      = nvPFBPort[((1048576    )- (1048576) )/4]   ;
  chipRev    = nvPMCPort[((0    )- (0) )/4]    & 240;
  chipSubrev = nvPMCPort[((0    )- (0) )/4]    & 15;
  crystalFreq= (nvPEXTDEVPort[((1052672    )- (1052672) )/4]    & (( 1    ) << (6))   ) ? 14318 : 13500;

  if (boot0 & (( 1    ) << (5))   )
  {
      if ((chipRev == 32 ) && (chipSubrev >= 2 ))
      {        
          ramType = 1 ;
          switch (boot0 & 3)
          {
              case 0 :
              case 3 :
                  vga256InfoRec.videoRam = 1024 * 8;
                  break;
              case 2 :
                  vga256InfoRec.videoRam = 1024 * 4;
                  break;
              case 1 :
              default:
                  vga256InfoRec.videoRam = 1024 * 2;
                  break;
          }
      }            
      else            
      {
          ramType = 0 ;
          vga256InfoRec.videoRam = 1024 * 8;
      }            
  }
  else
  {
      ramType = 0 ;
  
      switch (boot0 & 3)
      {
          case 0 :
              vga256InfoRec.videoRam = 1024 * 8;
              break;
          case 2 :
              vga256InfoRec.videoRam = 1024 * 4;
              break;
          case 1 :
          case 3 :
              vga256InfoRec.videoRam = 1024 * 2;
              break;
      }
  }        
  
   
  vga256InfoRec.maxClock = 230000 ;

  nv->ChipLinearSize=vga256InfoRec.videoRam*1024;
     
  nv->ChipLinearBase=(int)base1;  
  nv->ChipHas32bpp= 1 ;
   
  xf86ClearIOPortList (vga256InfoRec.scrnIndex);
  xf86AddIOPorts(vga256InfoRec.scrnIndex,Num_VGA_IOPorts,VGA_IOPorts);  
  xf86EnableIOPorts(vga256InfoRec.scrnIndex);
  vgaIOBase = (inb(972) & 1) ? 976 : 944;
  NV3EnterLeave(1 );

   
  if(vgaBitsPerPixel==16 && !noaccel) {
    ErrorF("%s %s: %s: Setting RGB weight to 555\n","(--)" , 
                                                    vga256InfoRec.name,
                                                    vga256InfoRec.chipset);
    xf86weight.green=xf86weight.blue=xf86weight.red=5;
  }

  ((  &(nv->ChipOptionFlags) )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] |= (1 << (( 60  )% (8 * sizeof(CARD32)) ))) ;
  ((  &(nv->ChipOptionFlags) )->flag_bits[( 62  )/ (8 * sizeof(CARD32)) ] |= (1 << (( 62  )% (8 * sizeof(CARD32)) ))) ;

  NV3FlipFunctions(nv);

  return 1;    
}

static int NV3ClockSelect(float clockIn,float *clockOut,int *mOut,int *nOut,int *pOut)
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
        highM = 13;
    }
    else
    {
         
        lowM  = 7;
        highM = 12;
    }                      
     


    for (P = 0; P <= 3; P ++)
    {
        Freq = VClk << P;
         


        if ((Freq >= 128000) && (Freq <= 230000))
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
  int bpp=vgaBitsPerPixel/8,
      horizDisplay = (mode->CrtcHDisplay/8) - 1,
      horizStart = (mode->CrtcHSyncStart/8) - 1,
      horizEnd = (mode->CrtcHSyncEnd/8) - 1,
      horizTotal = (mode->CrtcHTotal/8)	- 1,
      vertDisplay = mode->CrtcVDisplay - 1,
      vertStart = mode->CrtcVSyncStart	- 1,
      vertEnd = mode->CrtcVSyncEnd - 1,
      vertTotal = mode->CrtcVTotal - 2;
        
   
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[19]=((vga256InfoRec.displayWidth/8)*bpp)&255;
   
  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.repaint0=
    (( (((unsigned)((  (vga256InfoRec.displayWidth/8)*bpp  ) & (((unsigned)(1U << ((( 10)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (5))  ;

   

     
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0]= (( horizTotal - 4 )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[1]= (( horizDisplay )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[2]= (( horizDisplay )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[3]= (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))   | (1<<( 7 )) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[4]= (( horizStart )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[5]= (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 5)-( 5)+1)))-1)  << ( 5))  )) >> (5) )  ) << (7))  |
                     (( (((unsigned)((  horizEnd  ) & (((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))  ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[6]= (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 7)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))  ;

  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[7]= (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (0))  |
		     (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (1))  |
		     (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (2))  |
		     (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (3))  |
		     (1<<( 4 )) |
		     (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (5))  |
		     (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (6))  |
		     (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (7))  ;

  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[9]= (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 9)-( 9)+1)))-1)  << ( 9))  )) >> (9) )  ) << (5))   | (1<<( 6 )) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[16]= (( vertStart )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[17]= (( (((unsigned)((  vertEnd  ) & (((unsigned)(1U << ((( 3)-( 0)+1)))-1)  << ( 0))  )) >> (0) )  ) << (0))   | (1<<( 5 )) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[18]= (( vertDisplay )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[21]= (( vertDisplay )&255) ;
  ((vgaNVPtr)vgaNewVideoState)->std.CRTC[22]= (( vertTotal + 1 )&255) ;

  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.screenExtra= (( (((unsigned)((  horizTotal  ) & (((unsigned)(1U << ((( 6)-( 6)+1)))-1)  << ( 6))  )) >> (6) )  ) << (4))   |
                             (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (3))   |
                             (( (((unsigned)((  vertStart  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (2))   |
                             (( (((unsigned)((  vertDisplay  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (1))   |
                             (( (((unsigned)((  vertTotal  ) & (((unsigned)(1U << ((( 10)-( 10)+1)))-1)  << ( 10))  )) >> (10) )  ) << (0))  ;

  if(mode->Flags & 32 ) ((vgaNVPtr)vgaNewVideoState)->std.CRTC[9]|=128;
 
   


  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.horizExtra= (horizTotal < 260 ? 0 : 1);
  return 1;
}

 




static void InitPalette(DisplayModePtr mode)
{
  int bpp=vgaBitsPerPixel/8;
  int i;

  if(! (!(vgaBitsPerPixel==8 || xf86weight.green==6)) ) return;
   
   
  for(i=0;i<256;i++) {
    ((vgaNVPtr)vgaNewVideoState)->std.DAC[i*3]=i>>2;
    ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+1]=i>>2;
    ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+2]=i>>2;
  }
}

static Bool NV3Init(DisplayModePtr mode)
{
  int m,n,p;
  float clockIn=(float)vga256InfoRec.clock[mode->Clock];
  float clockOut;
  int time,data; 
  int i;
  int pixelDepth;
 
   
  if(!vgaHWInit (mode, sizeof (vgaNVRec))) {
    return 0;
  }
   
  ((vgaNVPtr)vgaNewVideoState)->vgaValid=1;
   
  if(!NV3ClockSelect(clockIn,&clockOut,&m,&n,&p)) {
    ErrorF("%s %s: %s: Unable to set desired video clock\n",
           "(--)" , vga256InfoRec.name,vga256InfoRec.chipset);
    return 0 ;  
  }
  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.vpllCoeff= ((   n   ) << (8))    | 
                          ((   m   ) << (0))    |
                          ((   p   ) << (16))   ;

  CalculateCRTC(mode);
  InitPalette(mode);
     
  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.repaint1=
    ((   mode->CrtcHDisplay<1280   ) << (2))    |
    (( 1    ) << (1))   ;
   



  pixelDepth=(vgaBitsPerPixel+1)/8;
  if(pixelDepth>3) pixelDepth=3;
  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.pixelFormat=pixelDepth;

  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.generalControl=
     (( 0    ) << (4))   |
     ((   xf86weight.green==6   ) << (12))   |
     (( 0    ) << (17))   |
     (( 0    ) << (20))   |  
     (( 1    ) << (8))   ;  
   


  nv3UpdateArbitrationSettings((unsigned int)clockOut,
                               pixelDepth*8,
                               crystalFreq,
                              &(((vgaNVPtr)vgaNewVideoState)->regs.nv3.fifo),
                              &(((vgaNVPtr)vgaNewVideoState)->regs.nv3.fifoControl));
   





  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.coeffSelect= (( 1    ) << (8))   
                                                    | (( 1    ) << (16))   
                                                    | (( 1    ) << (28))   ;
   


   



  ((vgaNVPtr)vgaNewVideoState)->regs.nv3.config0=
      ((   ((vga256InfoRec.displayWidth+31)/32)   ) << (0))   |
      ((   pixelDepth   ) << (8))   |
      (( 1    ) << (12))   ; 

  return 1 ;
}  

static void NV3Restore(void *data)
{
  vgaNVPtr restore=data;
  NV3Registers *nv3=&(restore->regs.nv3);

   


  vgaProtect(1 ); 
  vgaHWRestore((vgaHWPtr)restore);  
  outb(980,( 25   ));outb(981,  nv3->repaint0  )  ;
  outb(980,( 26   ));outb(981,  nv3->repaint1  )  ;
  outb(980,( 37   ));outb(981,  nv3->screenExtra  )  ;
  outb(980,( 40   ));outb(981,  nv3->pixelFormat  )  ;
  outb(980,( 45   ));outb(981,  nv3->horizExtra  )  ; 
  outb(980,( 27   ));outb(981,  nv3->fifoControl  )  ;
  outb(980,( 32   ));outb(981,  nv3->fifo  )  ;
  nvPFBPort[((1049088    )- (1048576) )/4] =(  nv3->config0  )  ;
  nvPRAMDACPort[((6817032    )- (6815744) )/4] =(  nv3->vpllCoeff  )  ;
  nvPRAMDACPort[((6817036    )- (6815744) )/4] =(  nv3->coeffSelect  )  ;
  nvPRAMDACPort[((6817280    )- (6815744) )/4] =(  nv3->generalControl  )  ;
  vgaProtect(0 );
}

static void *NV3Save(void *data)
{
  vgaNVPtr save= ((void *)0) ;

  save=(vgaNVPtr)vgaHWSave((vgaHWPtr)data,sizeof(vgaNVRec));  
  save->regs.nv3.repaint0= (outb(980, 25   ),inb(981))  ;
  save->regs.nv3.repaint1= (outb(980, 26   ),inb(981))  ;
  save->regs.nv3.screenExtra= (outb(980, 37   ),inb(981))  ;
  save->regs.nv3.pixelFormat= (outb(980, 40   ),inb(981))  ;
  save->regs.nv3.horizExtra= (outb(980, 45   ),inb(981))  ;
  save->regs.nv3.fifoControl= (outb(980, 27   ),inb(981))  ; 
  save->regs.nv3.fifo= (outb(980, 32   ),inb(981))  ;
  save->regs.nv3.config0= nvPFBPort[((1049088    )- (1048576) )/4]   ;
  save->regs.nv3.vpllCoeff= nvPRAMDACPort[((6817032    )- (6815744) )/4]   ;
  save->regs.nv3.coeffSelect= nvPRAMDACPort[((6817036    )- (6815744) )/4]   ;
  save->regs.nv3.generalControl= nvPRAMDACPort[((6817280    )- (6815744) )/4]   ;

  return (void*)save;
}

static void NV3Adjust(int x,int y) 
{
  int bpp=vgaBitsPerPixel/8;
  int startAddr=(((y*vga256InfoRec.virtualX)+x)*bpp);
  int offset=startAddr>>2;
  int pan=(startAddr&3)*2;
  unsigned char byte;
  
   
  outb(980,( 13 ));outb(981, (( offset )&255)  ) ;
  outb(980,( 12 ));outb(981, (( (((unsigned)((  offset  ) & (((unsigned)(1U << ((( 15)-( 8)+1)))-1)  << ( 8))  )) >> (8) )  ) << (0))   ) ;
  byte= (outb(980, 25   ),inb(981))   & ~(((unsigned)(1U << ((( 4)-( 0)+1)))-1)  << ( 0))    ;
  outb(980,( 25   ));outb(981,  (( (((unsigned)((  offset  ) & (((unsigned)(1U << ((( 20)-( 16)+1)))-1)  << ( 16))  )) >> (16) )  ) << (0))  |byte  )  ;
   
  byte=inb(vgaIOBase+10);
  outb(960,19);
  outb(960,pan);
}
 
 



static int NV3ValidMode(DisplayModePtr mode,Bool verbose,int flag)
{
    unsigned bw, bwMax, bpp;

    bpp = (vgaBitsPerPixel + 1) / 8;
     


    bwMax = (ramType == 0 ) ? 800000 : 700000;
     


    bw = mode->Clock * bpp;
    return (bw > bwMax ? 255  : 0 );
}


extern vgaHWCursorRec vgaHWCursor;

static void NV3FbInit(void)
{
   

  if(! ((  &vga256InfoRec.options )->flag_bits[( 62  )/ (8 * sizeof(CARD32)) ] & (1 << (( 62  )% (8 * sizeof(CARD32)) ))) ) {
     
    vgaHWCursor.Initialized = 1 ;
    vgaHWCursor.Init = NV3CursorInit;
    vgaHWCursor.Restore = NV3RestoreCursor;
    vgaHWCursor.Warp = NV3WarpCursor;
    vgaHWCursor.QueryBestSize = NV3QueryBestSize;
    if(xf86Verbose) {
      ErrorF("%s %s: %s: Using hardware cursor\n","(--)" , 
             vga256InfoRec.name,vga256InfoRec.chipset);
    }
  }

  if(! ((  &vga256InfoRec.options )->flag_bits[( 60  )/ (8 * sizeof(CARD32)) ] & (1 << (( 60  )% (8 * sizeof(CARD32)) ))) ) {
    NVAccelInit();
  }
  
}

static void NV3DisplayPowerManagementSet(int mode)
{
}

static Bool NV3ScreenInit(ScreenPtr pScreen,pointer pbits, 
                          int xsize,int ysize,int dpix,int dpiy,int width)
{
  return 1 ;
}

static void NV3SaveScreen(int on)
{
  vgaHWSaveScreen(on);
}

static void NV3GetMode(DisplayModePtr display)
{
}

 


static void NV3FlipFunctions(vgaVideoChipRec *nv)
{
  nv->ChipEnterLeave=NV3EnterLeave;
  nv->ChipInit=NV3Init;
  nv->ChipValidMode=NV3ValidMode;
  nv->ChipSave=NV3Save;
  nv->ChipRestore=NV3Restore;
  nv->ChipAdjust=NV3Adjust;
  nv->ChipSaveScreen=NV3SaveScreen;
  nv->ChipGetMode=(void (*)())NoopDDA;
  nv->ChipFbInit=NV3FbInit;
}
