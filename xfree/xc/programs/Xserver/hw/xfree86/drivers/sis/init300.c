/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/init300.c,v 1.6 2000/12/02 15:30:50 tsi Exp $ */

#include "xf86.h"
#include "xf86PciInfo.h"

#include "sis.h"
#include "sis_regs.h"
#include "init300.h"

VOID SetReg1(USHORT, USHORT, USHORT);
VOID SetReg3(USHORT, USHORT);
VOID SetReg4(USHORT, ULONG);
USHORT GetReg1(USHORT, USHORT);
USHORT GetReg2(USHORT);
ULONG GetReg3(USHORT);
Bool SiSSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   SISPtr  pSiS = SISPTR(pScrn);
   ULONG   temp;
   USHORT  cr30flag,cr31flag;
   ULONG   ROMAddr  = (ULONG) SISPTR(pScrn)->BIOS;
   USHORT  BaseAddr = (USHORT) (SISPTR(pScrn)->RelIO +0x30);
   USHORT  ModeNo=0;
   USHORT  Rate;
   USHORT  tempal,i,Part1Port;

   P3c4=BaseAddr+0x14;
   P3d4=BaseAddr+0x24;
   P3c0=BaseAddr+0x10;
   P3ce=BaseAddr+0x1e;
   P3c2=BaseAddr+0x12;
   P3ca=BaseAddr+0x1a;
   P3c6=BaseAddr+0x16;
   P3c7=BaseAddr+0x17;
   P3c8=BaseAddr+0x18;
   P3c9=BaseAddr+0x19;
   P3da=BaseAddr+0x2A;

   ModeNo = CalcModeIndex(pScrn, mode);
   Rate = CalcRefreshRate(pScrn, mode);
   SetReg1(P3d4, 0x33, Rate);

   SetReg1(P3c4, 0x20, 0xa1);
   SetReg1(P3c4, 0x1E, 0x5A);

   if(pSiS->VBFlags & VB_LVDS)
      IF_DEF_LVDS = 1;
   else
      IF_DEF_LVDS = 0;
   if(pSiS->VBFlags & VB_CHRONTEL)
      IF_DEF_CH7005 = 1;
   else
      IF_DEF_CH7005 = 0;


/* ynlai begin */
   IF_DEF_HiVision=0;
/* ynlai end */

   PresetScratchregister(P3d4); /* add for CRT2 */
   /* replace GetSenseStatus,SetTVSystem,SetDisplayInfo */

   DisplayOff();
   SetReg1(P3c4,0x05,0x86);                     /* 1.Openkey */
   temp=SearchModeID(ROMAddr,ModeNo);           /* 2.Get ModeID Table */
   if(temp==0)  return(0);

   /*  SetTVSystem();   */                            /* add for CRT2 */
   /*GetLCDDDCInfo(pScrn);*/                        /* add for CRT2 */
   GetVBInfo(BaseAddr,ROMAddr);                 /* add for CRT2 */
   GetLCDResInfo(ROMAddr, P3d4);                /* add for CRT2 */

   temp=CheckMemorySize(ROMAddr);               /* 3.Check memory size */
   if(temp==0) return(0);
   cr30flag=(UCHAR)GetReg1(P3d4,0x30);
   if(((cr30flag&0x01)==1)||((cr30flag&0x02)==0)){
     /* if cr30 d[0]=1 or d[1]=0 set crt1 */
     SetReg1(P3d4,0x34,ModeNo);
     /* set CR34->CRT1 ModeNofor CRT2 FIFO */
     GetModePtr(ROMAddr,ModeNo);                  /* 4.GetModePtr */
     SetSeqRegs(ROMAddr);                         /* 5.SetSeqRegs */
     SetMiscRegs(ROMAddr);                        /* 6.SetMiscRegs */
     SetCRTCRegs(ROMAddr);                        /* 7.SetCRTCRegs */
     SetATTRegs(ROMAddr);                         /* 8.SetATTRegs */
     SetGRCRegs(ROMAddr);                         /* 9.SetGRCRegs */
     ClearExt1Regs();                             /* 10.Clear Ext1Regs */
     temp=GetRatePtr(ROMAddr,ModeNo);             /* 11.GetRatePtr */
     if(temp) {
       SetSync(ROMAddr);                          /* 12.SetSync */
       SetCRT1CRTC(ROMAddr);                      /* 13.SetCRT1CRTC */
       SetCRT1Offset(ROMAddr);                    /* 14.SetCRT1Offset */
       SetCRT1VCLK(ROMAddr);                      /* 15.SetCRT1VCLK */
       SetVCLKState(ROMAddr, ModeNo);
       if( (pSiS->Chipset == PCI_CHIP_SIS630) || (pSiS->Chipset == PCI_CHIP_SIS540) )
          SetCRT1FIFO2(ROMAddr);
       else
          SetCRT1FIFO(ROMAddr);
     }
     SetCRT1ModeRegs(ROMAddr, ModeNo);
     if( (pSiS->Chipset == PCI_CHIP_SIS630) || (pSiS->Chipset == PCI_CHIP_SIS540) )
         SetInterlace(ROMAddr,ModeNo);
     LoadDAC(ROMAddr);
   }
   cr31flag=(UCHAR)GetReg1(P3d4,0x31);
   if(((cr30flag&0x01)==1)||((cr30flag&0x03)==0x02)||
     (((cr30flag&0x03)==0x00)&&((cr31flag&0x20)==0x20))) {
     /* if CR30 d[0]=1 or d[1:0]=10, set CRT2 or cr30 cr31== 0x00 0x20 */
     SetCRT2Group(BaseAddr,ROMAddr,ModeNo, pScrn);   /*    add for CRT2   */
   }

/* ynlai begin test */
/* ynlai end test */

   SetPitch(pScrn, BaseAddr);                     /* 16.SetPitch */
   WaitVertical();
   DisplayOn();                                   /* 17.DisplayOn */
   return TRUE;
}

BOOLEAN SearchModeID(ULONG ROMAddr, USHORT ModeNo)
{
   UCHAR ModeID;
   USHORT  usIDLength;

   ModeIDOffset=*((USHORT *)(ROMAddr+0x20A));      /* Get EModeIDTable */
   ModeID=*((UCHAR *)(ROMAddr+ModeIDOffset));      /* Offset 0x20A  */
   usIDLength = GetModeIDLength(ROMAddr, ModeNo);
   while(ModeID!=0xff && ModeID!=ModeNo) {
/*    ModeIDOffset=ModeIDOffset+10;   */           /*StructSize  */
      ModeIDOffset=ModeIDOffset+usIDLength;
      ModeID=*((UCHAR *)(ROMAddr+ModeIDOffset));
   }
   if(ModeID==0xff) return(FALSE);
   else return(TRUE);
}

BOOLEAN CheckMemorySize(ULONG ROMAddr)
{
  USHORT memorysize;
  USHORT modeflag;
  USHORT temp;

  modeflag=*((USHORT *)(ROMAddr+ModeIDOffset+0x01));   /* si+St_ModeFlag   */
  ModeType=modeflag&ModeInfoFlag;                      /* Get mode type    */

  memorysize=modeflag&MemoryInfoFlag;
  memorysize=memorysize>MemorySizeShift;
  memorysize++;                                        /* Get memory size  */

  temp=GetReg1(P3c4,0x14);                             /* Get DRAM Size    */
  temp=temp&0x3F;
  temp++;

  if(temp<memorysize) return(FALSE);
  else return(TRUE);
}

VOID GetModePtr(ULONG ROMAddr, USHORT ModeNo)
{
   UCHAR index;

   StandTable=*((USHORT *)(ROMAddr+0x202));          /* Get First  0x202  */
                                                     /* StandTable Offset */
   if(ModeNo<=13) {
     index=*((UCHAR *)(ROMAddr+ModeIDOffset+0x03));  /* si+St_ModeFlag    */
   }
   else {
     if(ModeType <= 0x02) index=0x1B;                /* 02 -> ModeEGA     */
     else index=0x0F;
   }
   StandTable=StandTable+64*index;
}

VOID SetSeqRegs(ULONG ROMAddr)
{
   UCHAR SRdata;
   USHORT i;

   SetReg1(P3c4,0x00,0x03);                        /* Set SR0               */
   StandTable=StandTable+0x05;
   SRdata=*((UCHAR *)(ROMAddr+StandTable));        /* Get SR01 from file    */
   if(IF_DEF_LVDS==1){
     if(IF_DEF_CH7005==1) {
       if(VBInfo&SetCRT2ToTV) {
         if(VBInfo&SetInSlaveMode) {
           SRdata=SRdata|0x01;
         }      
       }
     }
     if(VBInfo&SetCRT2ToLCD){
       if(VBInfo&SetInSlaveMode){
         if(LCDInfo&LCDNonExpanding){
           SRdata=SRdata|0x01;
         }
       }
     }
   }
   SRdata=SRdata|0x20;
   SetReg1(P3c4,0x01,SRdata);                      /* Set SR1               */
   for(i=02;i<=04;i++) {
     StandTable++;
     SRdata=*((UCHAR *)(ROMAddr+StandTable));      /* Get SR2,3,4 from file */
     SetReg1(P3c4,i,SRdata);                       /* Set SR2 3 4           */
   }
}

VOID SetMiscRegs(ULONG ROMAddr)
{
   UCHAR Miscdata;

   StandTable++;
   Miscdata=*((UCHAR *)(ROMAddr+StandTable));      /* Get Misc from file  */
   SetReg3(P3c2,Miscdata);                         /* Set Misc(3c2)       */
}

VOID SetCRTCRegs(ULONG ROMAddr)
{
  UCHAR CRTCdata;
  USHORT i;

  CRTCdata=(UCHAR)GetReg1(P3d4,0x11);
  CRTCdata=CRTCdata&0x7f;
  SetReg1(P3d4,0x11,CRTCdata);                     /* Unlock CRTC        */

  for(i=0;i<=0x18;i++) {
     StandTable++;
     CRTCdata=*((UCHAR *)(ROMAddr+StandTable));    /* Get CRTC from file */
     SetReg1(P3d4,i,CRTCdata);                     /* Set CRTC(3d4)      */
  }
}

VOID SetATTRegs(ULONG ROMAddr)
{
   UCHAR ARdata;
   USHORT i;

   for(i=0;i<=0x13;i++) {
     StandTable++;
     ARdata=*((UCHAR *)(ROMAddr+StandTable));    /* Get AR for file  */
     if(IF_DEF_LVDS==1){  /*for LVDS*/
       if(IF_DEF_CH7005==1) {
         if(VBInfo&SetCRT2ToTV) {
           if(VBInfo&SetInSlaveMode) {
             if(i==0x13) ARdata=0;              
           }
         }
       }
       if(VBInfo&SetCRT2ToLCD){
         if(VBInfo&SetInSlaveMode){
           if(i==0x13) ARdata=0;
         }
       }
     }
     GetReg2(P3da);                              /* reset 3da        */
     SetReg3(P3c0,i);                            /* set index        */
     SetReg3(P3c0,ARdata);                       /* set data         */
   }

   GetReg2(P3da);                                /* reset 3da        */
   SetReg3(P3c0,0x14);                           /* set index        */
   SetReg3(P3c0,0x00);                           /* set data         */
   GetReg2(P3da);                                /* Enable Attribute */
   SetReg3(P3c0,0x20);
}

VOID SetGRCRegs(ULONG ROMAddr)
{
   UCHAR GRdata;
   USHORT i;

   for(i=0;i<=0x08;i++) {
     StandTable++;
     GRdata=*((UCHAR *)(ROMAddr+StandTable));    /* Get GR from file */
     SetReg1(P3ce,i,GRdata);                     /* Set GR(3ce)      */
   }
   if(ModeType>ModeVGA){
     GRdata=(UCHAR)GetReg1(P3ce,0x05);
     GRdata=GRdata&0xBF;
     SetReg1(P3ce,0x05,GRdata);
   }
}

VOID ClearExt1Regs()
{
  USHORT i;

  for(i=0x0A;i<=0x0E;i++) SetReg1(P3c4,i,0x00);      /* Clear SR0A-SR0E */
}


BOOLEAN GetRatePtr(ULONG ROMAddr, USHORT ModeNo)
{
  SHORT  index;
  USHORT temp;
  USHORT ulRefIndexLength;

  if(ModeNo<0x14) return(FALSE);                       /* Mode No <= 13h then return */

  index=GetReg1(P3d4,0x33);                            /* Get 3d4 CRTC33   */
  index=index&0x0F;                                    /* Frame rate index */
  if(index!=0) index--;
  REFIndex=*((USHORT *)(ROMAddr+ModeIDOffset+0x04));   /* si+Ext_point   */

  ulRefIndexLength = GetRefindexLength(ROMAddr, ModeNo);
  do {
    temp=*((USHORT *)(ROMAddr+REFIndex));              /* di => REFIndex */
    if(temp==0xFFFF) break;
    temp=temp&ModeInfoFlag;
    if(temp<ModeType) break;

    REFIndex=REFIndex+ulRefIndexLength;                /* rate size   */
    index--;
  } while(index>=0);

  REFIndex=REFIndex-ulRefIndexLength;                  /* rate size   */
  return(TRUE);
}

VOID SetSync(ULONG ROMAddr)
{
  USHORT sync;
  USHORT temp;

   sync=*((USHORT *)(ROMAddr+REFIndex));               /* di+0x00 */
   sync=sync&0xC0;
   temp=0x2F;
   temp=temp|sync;
   SetReg3(P3c2,temp);                                 /* Set Misc(3c2) */
}

VOID SetCRT1CRTC(ULONG ROMAddr)
{
  UCHAR  index;
  UCHAR  data;
  USHORT i;

  index=*((UCHAR *)(ROMAddr+REFIndex+0x02)) & 0x3F;   /* Get index */
  CRT1Table=*((USHORT *)(ROMAddr+0x204));             /* Get CRT1Table */
  CRT1Table=CRT1Table+index*CRT1Len;

  data=(UCHAR)GetReg1(P3d4,0x11);
  data=data&0x7F;
  SetReg1(P3d4,0x11,data);                            /* Unlock CRTC */

  CRT1Table--;
  for(i=0;i<=0x05;i++) {
    CRT1Table++;
    data=*((UCHAR *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x06;i<=0x07;i++) {
    CRT1Table++;
    data=*((UCHAR *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x10;i<=0x12;i++) {
    CRT1Table++;
    data=*((UCHAR *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x15;i<=0x16;i++) {
    CRT1Table++;
    data=*((UCHAR *)(ROMAddr+CRT1Table));
    SetReg1(P3d4,i,data);
  }
  for(i=0x0A;i<=0x0C;i++) {
    CRT1Table++;
    data=*((UCHAR *)(ROMAddr+CRT1Table));
    SetReg1(P3c4,i,data);
  }

  CRT1Table++;
   data=*((UCHAR *)(ROMAddr+CRT1Table));
  data=data&0xE0;
  SetReg1(P3c4,0x0E,data);

  data=(UCHAR)GetReg1(P3d4,0x09);
  data=data&0xDF;
  i=*((UCHAR *)(ROMAddr+CRT1Table));
  i=i&0x01;
  i=i<<5;
  data=data|i;
  i=*((USHORT *)(ROMAddr+ModeIDOffset+0x01));
  i=i&DoubleScanMode;
  if(i) data=data|0x80;
  SetReg1(P3d4,0x09,data);

  if(ModeType>0x03) SetReg1(P3d4,0x14,0x4F);
}

VOID SetCRT1Offset(ULONG ROMAddr)
{
   USHORT temp,ah,al;
   USHORT temp2,i;
   USHORT DisplayUnit;

   temp=*((UCHAR *)(ROMAddr+ModeIDOffset+0x03));         /* si+Ext_ModeInfo  */
   temp=temp>>4;                                         /* index            */
   ScreenOffset=*((USHORT *)(ROMAddr+0x206));            /* ScreenOffset     */
   temp=*((UCHAR *)(ROMAddr+ScreenOffset+temp));         /* data             */

   temp2=*((USHORT *)(ROMAddr+REFIndex+0x00));
   temp2=temp2&InterlaceMode;
   if(temp2) temp=temp<<1;
   temp2=ModeType-ModeEGA;
   switch (temp2) {
     case 0 : temp2=1; break;
     case 1 : temp2=2; break;
     case 2 : temp2=4; break;
     case 3 : temp2=4; break;
     case 4 : temp2=6; break;
     case 5 : temp2=8; break;
   }
   temp=temp*temp2;
   DisplayUnit=temp;

   temp2=temp;
   temp=temp>>8;                                         /* ah */
   temp=temp&0x0F;
   i=GetReg1(P3c4,0x0E);
   i=i&0xF0;
   i=i|temp;
   SetReg1(P3c4,0x0E,i);

   temp=(UCHAR)temp2;
   temp=temp&0xFF;                                        /* al */
   SetReg1(P3d4,0x13,temp);

   temp2=*((USHORT *)(ROMAddr+REFIndex+0x00));
   temp2=temp2&InterlaceMode;
   if(temp2) DisplayUnit>>=1;

   DisplayUnit=DisplayUnit<<5;
   ah=(DisplayUnit&0xff00)>>8;
   al=DisplayUnit&0x00ff;
   if(al==0) ah=ah+1;
   else ah=ah+2;
   SetReg1(P3c4,0x10,ah);
}

VOID SetCRT1VCLK(ULONG ROMAddr)
{
  USHORT i;
  UCHAR  index,data;

  index=*((UCHAR *)(ROMAddr+REFIndex+0x03)) & 0x3F; 
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((USHORT *)(ROMAddr+0x208));
  VCLKData=VCLKData+data;

  SetReg1(P3c4,0x31,0);
  for(i=0x2B;i<=0x2C;i++) {
     data=*((UCHAR *)(ROMAddr+VCLKData));
     SetReg1(P3c4,i,data);
     VCLKData++;
  }
  SetReg1(P3c4,0x2D,0x80);
}


VOID SetCRT1ModeRegs(ULONG ROMAddr, USHORT ModeNo)
{
  USHORT data,data2,data3;

  if(ModeNo>0x13)   data=*((USHORT *)(ROMAddr+REFIndex+0x00));
  else data=0;
  data2=0;
  if(ModeNo>0x13)
    if(ModeType>0x02) {
       data2=data2|0x02;
       data3=ModeType-ModeVGA;
       data3=data3<<2;
       data2=data2|data3;
    }

   data=data&InterlaceMode;
   if(data) data2=data2|0x20;
   SetReg1(P3c4,0x06,data2);

   data=GetReg1(P3c4,0x01);
   data=data&0xF7;
   data2=*((USHORT *)(ROMAddr+ModeIDOffset+0x01));
   data2=data2&HalfDCLK;
   if(data2) data=data|0x08;
   SetReg1(P3c4,0x01,data);

   data=GetReg1(P3c4,0x0F);
   data=data&0xF7;
   data2=*((USHORT *)(ROMAddr+ModeIDOffset+0x01));
   data2=data2&LineCompareOff;
   if(data2) data=data|0x08;
   SetReg1(P3c4,0x0F,data);

   data=GetReg1(P3c4,0x21);
   data=data&0x1F;
   if(ModeType==0x00) data=data|0x60;                /* Text Mode */
   else if(ModeType<=0x02) data=data|0x00;           /* EGA Mode  */
   else data=data|0xA0;                              /* VGA Mode  */
   SetReg1(P3c4,0x21,data);
}

VOID SetVCLKState(ULONG ROMAddr, USHORT ModeNo)
{
   USHORT data,data2;
   USHORT VCLK;
   UCHAR  index;

  index=*((UCHAR *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((USHORT *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((USHORT *)(ROMAddr+VCLKData));
  if(ModeNo<=0x13) VCLK=0;

   data=GetReg1(P3c4,0x07);
   data=data&0x7B;
   if(VCLK>=150) data=data|0x80;                     /* VCLK > 150 */
   SetReg1(P3c4,0x07,data);

   data=GetReg1(P3c4,0x32);
   data=data&0xD7;
   if(VCLK>=150) data=data|0x08;                     /* VCLK > 150 */
   SetReg1(P3c4,0x32,data);

   data2=0x03;
   if(VCLK>135) data2=0x02;
   if(VCLK>160) data2=0x01;
   if(VCLK>260) data2=0x00;
   data=GetReg1(P3c4,0x07);
   data=data&0xFC;
   data=data|data2;
   SetReg1(P3c4,0x07,data);
}

VOID LoadDAC(ULONG ROMAddr)
{
   USHORT data,data2;
   USHORT time,i,j,k;
   USHORT m,n,o;
   USHORT si,di,bx,dl;
   USHORT al,ah,dh;
   USHORT *table=0;

   data=*((USHORT *)(ROMAddr+ModeIDOffset+0x01));
   data=data&DACInfoFlag;
   time=64;
   if(data==0x00) table=MDA_DAC;
   if(data==0x08) table=CGA_DAC;
   if(data==0x10) table=EGA_DAC;
   if(data==0x18) {
     time=256;
     table=VGA_DAC;
   }
   if(time==256) j=16;
   else j=time;

   SetReg3(P3c6,0xFF);
   SetReg3(P3c8,0x00);

   for(i=0;i<j;i++) {
      data=table[i];
      for(k=0;k<3;k++) {
        data2=0;
        if(data&0x01) data2=0x2A;
        if(data&0x02) data2=data2+0x15;
        SetReg3(P3c9,data2);
        data=data>>2;
      }
   }

   if(time==256) {
      for(i=16;i<32;i++) {
         data=table[i];
         for(k=0;k<3;k++) SetReg3(P3c9,data);
      }
      si=32;
      for(m=0;m<9;m++) {
         di=si;
         bx=si+0x04;
         dl=0;
         for(n=0;n<3;n++) {
            for(o=0;o<5;o++) {
              dh=table[si];
              ah=table[di];
              al=table[bx];
              si++;
              WriteDAC(dl,ah,al,dh);
            }         /* for 5 */
            si=si-2;
            for(o=0;o<3;o++) {
              dh=table[bx];
              ah=table[di];
              al=table[si];
              si--;
              WriteDAC(dl,ah,al,dh);
            }         /* for 3 */
            dl++;
         }            /* for 3 */
         si=si+5;
      }               /* for 9 */
   }
}

VOID WriteDAC(USHORT dl, USHORT ah, USHORT al, USHORT dh)
{
  USHORT temp;
  USHORT bh,bl;

  bh=ah;
  bl=al;
  if(dl!=0) {
    temp=bh;
    bh=dh;
    dh=temp;
    if(dl==1) {
       temp=bl;
       bl=dh;
       dh=temp;
    }
    else {
       temp=bl;
       bl=bh;
       bh=temp;
    }
  }
  SetReg3(P3c9,(USHORT)dh);
  SetReg3(P3c9,(USHORT)bh);
  SetReg3(P3c9,(USHORT)bl);
}

VOID DisplayOn()
{
   USHORT data;

   data=GetReg1(P3c4,0x01);
   data=data&0xDF;
   SetReg1(P3c4,0x01,data);
}

VOID DisplayOff()
{
   USHORT data;

   data=GetReg1(P3c4,0x01);
   data=data|0x20;
   SetReg1(P3c4,0x01,data);
}

VOID SetReg1(USHORT port, USHORT index, USHORT  data)
{
    outb(port ,(UCHAR)(index & 0xff));
    port++;
    outb(port ,(UCHAR)(data  & 0xff));
}

VOID SetReg3(USHORT port, USHORT data)
{
    outb(port, (UCHAR)(data & 0xff));
}

USHORT GetReg1(USHORT port, USHORT index)
{
    UCHAR   data;

    outb(port, (UCHAR)(index & 0xff));
    port += 1;
    data = inb(port);
   return(data);
}

USHORT GetReg2(USHORT port)
{
    UCHAR   data;

    data = inb(port);

    return(data);
}

USHORT GetModeIDLength(ULONG ROMAddr, USHORT ModeNo)
{
   UCHAR  ModeID;
   USHORT modeidlength;
   USHORT usModeIDOffset;

   return(10);
   modeidlength=0;
   usModeIDOffset=*((USHORT *)(ROMAddr+0x20A));      /* Get EModeIDTable    */
   ModeID=*((UCHAR *)(ROMAddr+usModeIDOffset));      /* Offset 0x20A        */
   while(ModeID!=0x2E) {
      modeidlength++;
      usModeIDOffset=usModeIDOffset+1;               /* 10 <= ExtStructSize */
      ModeID=*((UCHAR *)(ROMAddr+usModeIDOffset));
   }
   return(modeidlength);
}

USHORT GetRefindexLength(ULONG ROMAddr, USHORT ModeNo)
{
   UCHAR ModeID;
   UCHAR temp;
   USHORT refindexlength;
   USHORT usModeIDOffset;
   USHORT usREFIndex;
   USHORT usIDLength;

   usModeIDOffset=*((USHORT *)(ROMAddr+0x20A));           /* Get EModeIDTable  */
   ModeID=*((UCHAR *)(ROMAddr+usModeIDOffset));           /* Offset 0x20A      */
   usIDLength = GetModeIDLength(ROMAddr, ModeNo);
   while(ModeID!=0x40) {
      usModeIDOffset=usModeIDOffset+usIDLength;           /*10 <= ExtStructSize */
      ModeID=*((UCHAR *)(ROMAddr+usModeIDOffset));
   }

   refindexlength=1;
   usREFIndex=*((USHORT *)(ROMAddr+usModeIDOffset+0x04)); /* si+Ext_point      */
   usREFIndex++;
   temp=*((UCHAR *)(ROMAddr+usREFIndex));                 /* di => REFIndex    */
   while(temp!=0xFF) {
      refindexlength++;
      usREFIndex++;
      temp=*((UCHAR *)(ROMAddr+usREFIndex));              /* di => REFIndex    */
   }
   return(refindexlength);
}

VOID SetInterlace(ULONG ROMAddr, USHORT ModeNo)
{
  ULONG Temp;
  USHORT data,Temp2;

  Temp = (ULONG)GetReg1(P3d4, 0x01);
  Temp++;
  Temp=Temp*8;

  if(Temp==1024) data=0x0035;
  else if(Temp==1280) data=0x0048;
  else data=0x0000;

  Temp2=*((USHORT *)(ROMAddr+REFIndex+0x00));
  Temp2 &= InterlaceMode;
  if(Temp2 == 0) data=0x0000;

  SetReg1(P3d4,0x19,data);

  Temp = (ULONG)GetReg1(P3d4, 0x1A);
  Temp2= (USHORT)(Temp & 0xFC);
  SetReg1(P3d4,0x1A,(USHORT)Temp);

  Temp = (ULONG)GetReg1(P3c4, 0x0f);
  Temp2= (USHORT)Temp & 0xBF;
  if(ModeNo==0x37) Temp2=Temp2|0x40;
  SetReg1(P3d4,0x1A,(USHORT)Temp2);
}

VOID SetCRT1FIFO(ULONG ROMAddr)
{
  USHORT  index,data,VCLK,data2,MCLKOffset,MCLK,colorth=0;
  USHORT  ah,bl,A,B;

  index=*((UCHAR *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((USHORT *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((USHORT *)(ROMAddr+VCLKData));           /* Get VCLK */

  MCLKOffset=*((USHORT *)(ROMAddr+0x20C));
  index=GetReg1(P3c4,0x3A);
  index=index&07;
  MCLKOffset=MCLKOffset+index*5;
  MCLK=*((UCHAR *)(ROMAddr+MCLKOffset+0x03));     /* Get MCLK */

  data2=ModeType-0x02;
    switch (data2) {
      case 0 : colorth=1; break;
      case 1 : colorth=2; break;
      case 2 : colorth=4; break;
      case 3 : colorth=4; break;
      case 4 : colorth=6; break;
      case 5 : colorth=8; break;
    }

  do{
/*==============*/
     B=(USHORT)(CalcDelay(ROMAddr,0)*VCLK*colorth);
     B=B/(16*MCLK);
     B++;

     A=(CalcDelay(ROMAddr,1)*VCLK*colorth);
     A=A/(16*MCLK);
     A++;

     if(A<4) A=0;
     else A=A-4;

     if(A>B)  bl=A;
     else bl=B;

     bl++;
     if(bl>0x13) {
        data=GetReg1(P3c4,0x16);
        data=data>>6;
        if(data!=0) {
          data--;
          data=data<<6;
          data2=GetReg1(P3c4,0x16);
          data2=(data2&0x3f)|data;
          SetReg1(P3c4,0x16,data2);
        }
        else bl=0x13;
     }
/*==============*/
  } while(bl>0x13);

  ah=bl;
  ah=ah<<4;
  ah=ah|0x0f;
  SetReg1(P3c4,0x08,ah);

  data=bl;
  data=data&0x10;
  data=data<<1;
  data2=GetReg1(P3c4,0x0F);
  data2=data2&0x9f;
  data2=data2|data;
  SetReg1(P3c4,0x0F,data2);

  data=bl+3;
  if(data>0x0f) data=0x0f;
  SetReg1(P3c4,0x3b,0x00);
  data2=GetReg1(P3c4,0x09);
  data2=data2&0xF0;
  data2=data2|data;
  SetReg1(P3c4,0x09,data2);
}

USHORT CalcDelay(ULONG ROMAddr,USHORT key)
{
  USHORT data,data2,temp0,temp1;
  UCHAR   ThLowA[]=   {61,3,52,5,68,7,100,11,
                     43,3,42,5,54,7, 78,11,
                     34,3,37,5,47,7, 67,11};
  UCHAR   ThLowB[]=   {81,4,72,6,88,8,120,12,
                     55,4,54,6,66,8, 90,12,
                     42,4,45,6,55,8, 75,12};
  UCHAR   ThTiming[]= {1,2,2,3,0,1,1,2};

  data=GetReg1(P3c4,0x16);
  data=data>>6;
  data2=GetReg1(P3c4,0x14);
  data2=(data2>>4)&0x0C;
  data=data|data2;
  data=data<1;
  if(key==0) {
    temp0=(USHORT)ThLowA[data];
    temp1=(USHORT)ThLowA[data+1];
  }
  else {
    temp0=(USHORT)ThLowB[data];
    temp1=(USHORT)ThLowB[data+1];
  }

  data2=0;
  data=GetReg1(P3c4,0x18);
  if(data&0x02) data2=data2|0x01;
  if(data&0x20) data2=data2|0x02;
  if(data&0x40) data2=data2|0x04;

  data=temp1*ThTiming[data2]+temp0;
  return(data);
}

VOID SetCRT1FIFO2(ULONG ROMAddr)
{
  USHORT  index,data,VCLK,data2,MCLKOffset,MCLK,colorth=0;
  USHORT  ah,bl,B;
  ULONG   eax;

  index=*((UCHAR *)(ROMAddr+REFIndex+0x03));
  CRT1VCLKLen=GetVCLKLen(ROMAddr);
  data=index*CRT1VCLKLen;
  VCLKData=*((USHORT *)(ROMAddr+0x208));
  VCLKData=VCLKData+data+(CRT1VCLKLen-2);
  VCLK=*((USHORT *)(ROMAddr+VCLKData));           /* Get VCLK */

  MCLKOffset=*((USHORT *)(ROMAddr+0x20C));
  index=GetReg1(P3c4,0x1A);
  index=index&07;
  MCLKOffset=MCLKOffset+index*5;
  MCLK=*((USHORT *)(ROMAddr+MCLKOffset+0x03));     /* Get MCLK */

  data2=ModeType-0x02;
    switch (data2) {
      case 0 : colorth=1; break;
      case 1 : colorth=1; break;
      case 2 : colorth=2; break;
      case 3 : colorth=2; break;
      case 4 : colorth=3; break;
      case 5 : colorth=4; break;
    }

  do{
/*==============*/
     B=(CalcDelay2(ROMAddr,0)*VCLK*colorth);
     if (B%(16*MCLK) == 0)
     {
       B=B/(16*MCLK);
       bl=B+1;
     }
     else
     {
       B=B/(16*MCLK);
       bl=B+2;
     }

     if(bl>0x13) {
        data=GetReg1(P3c4,0x15);
        data=data&0xf0;
        if(data!=0xb0) {
          data=data+0x20;
          if(data==0xa0) data=0x30;

          data2=GetReg1(P3c4,0x15);
          data2=(data2&0x0f)|data;
          SetReg1(P3c4,0x15,data2);
        }
        else bl=0x13;
     }
/*==============*/
  } while(bl>0x13);

  data2=GetReg1(P3c4,0x15);
  data2=(data2&0xf0)>>4;
  data2=data2<<24;

/* ========================*/
  SetReg4(0xcf8,0x80000050);
  eax=GetReg3(0xcfc);
  eax=eax&0x0f0ffffff;
  eax=eax|data2;
  SetReg4(0xcfc,eax);
/* ========================*/

  ah=bl;
  ah=ah<<4;
  ah=ah|0x0f;
  SetReg1(P3c4,0x08,ah);

  data=bl;
  data=data&0x10;
  data=data<<1;
  data2=GetReg1(P3c4,0x0F);
  data2=data2&0x9f;
  data2=data2|data;
  SetReg1(P3c4,0x0F,data2);

  data=bl+3;
  if(data>0x0f) data=0x0f;
  SetReg1(P3c4,0x3b,0x00);
  data2=GetReg1(P3c4,0x09);
  data2=data2&0xF0;
  data2=data2|data;
  SetReg1(P3c4,0x09,data2);
}

USHORT CalcDelay2(ULONG ROMAddr,USHORT key)
{
  USHORT data,index;
  UCHAR  LatencyFactor[]={88,80,78,72,70,00,
                          00,79,77,71,69,49,
                          88,80,78,72,70,00,
                          00,72,70,64,62,44};

  index=0;
  data=GetReg1(P3c4,0x14);
  if(data&0x80) index=index+12;

  data=GetReg1(P3c4,0x15);
  data=(data&0xf0)>>4;
  if(data&0x01) index=index+6;

  data=data>>1;
  index=index+data;
  data=LatencyFactor[index];

  return(data);
}

VOID SetReg4(USHORT port, ULONG data)
{
    outl(port, (ULONG)(data & 0xffffffff));
}

ULONG GetReg3(USHORT port)
{
    ULONG   data;
    
    data = inl(port);
    return(data);
}

VOID SetPitch(ScrnInfoPtr pScrn, USHORT BaseAddr)
{
    SISPtr pSiS = SISPTR(pScrn);
    ULONG  HDisplay;
    ULONG  temp;
    USHORT Port = BaseAddr + IND_SIS_CRT2_PORT_04;

    HDisplay = pSiS->scrnOffset / 8;
    SetReg1(P3d4, 0x13, HDisplay);
    temp = (GetReg1(P3c4, 0x0E) & 0xF0) | (HDisplay>>8);
    SetReg1(P3c4, 0x0E, temp);
    
    SetReg1(Port, 0x24, 1);
    SetReg1(Port, 0x07, HDisplay); 
    temp = (GetReg1(Port, 0x09) & 0xF0) | (HDisplay>>8);
    SetReg1(Port, 0x09, temp);


}
USHORT CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   USHORT i = (pScrn->bitsPerPixel+7)/8 - 1;
   USHORT ModeIndex = 0;
   switch(mode->HDisplay)
   {
     case 640:
          ModeIndex = ModeIndex_640x480[i];
          break;
     case 720:
          if(mode->VDisplay == 480)
            ModeIndex = ModeIndex_720x480[i];
          else  
            ModeIndex = ModeIndex_720x576[i];
          break;
     case 800:
          ModeIndex = ModeIndex_800x600[i];
          break;
     case 1024:
          ModeIndex = ModeIndex_1024x768[i];
          break;
     case 1280:
          ModeIndex = ModeIndex_1280x1024[i];
          break;
     case 1600:
          ModeIndex = ModeIndex_1600x1200[i];
          break;
   }

   return(ModeIndex);
}
USHORT CalcRefreshRate(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   SISPtr pSiS = SISPTR(pScrn);
   USHORT Index=0;
   USHORT i=0;
   USHORT Rate=1;
   USHORT temp = (int)(mode->VRefresh+0.5);
   
   switch(mode->HDisplay)
   {
     case 640:
          Index = 0;
          break;
     case 800:
          Index = 1;
          break;
     case 1024:
          Index = 2;
          Rate  = 2;
          break;
     case 1280:
          Index = 3;
          Rate  = 2;
          break;
     case 1600:
          Index = 4;
          break;
     case 720:
          if(mode->VDisplay == 480)
            Index = 5;
          else  
            Index = 6;
          break;
    
   }
   while(RefreshRate[Index][i] != 0)
   {
      if(temp == RefreshRate[Index][i])
      {  
         Rate=i+1;
         break;
      }
      else
         i++;
   } 
   if(pSiS->VBFlags & CRT2_VGA)
      Rate |= Rate << 4;
   return(Rate);
}

VOID WaitVertical(VOID)
{
  USHORT tempax,tempdx;

/*  tempdx=0x3da;
  do {
    tempax=GetReg2(tempdx);
  } while(!(tempax&01));

  do {
    tempax=GetReg2(tempdx);
  } while(!(tempax&01));
*/  

}
