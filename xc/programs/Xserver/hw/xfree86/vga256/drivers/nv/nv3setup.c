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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3setup.c,v 1.1.2.7 1998/11/18 16:38:46 hohndel Exp $ */

#include <stdlib.h>


#include "nvuser.h"
#include "nvreg.h"

typedef struct
{
    UINT32 id;
    UINT32 context;
}HashTableEntry;

typedef struct
{
    UINT32 context;  
    UINT32 dmaNotifyInst;  
    UINT32 memFormatInst;  
    UINT32 unknown;  
}ObjInstEntry;

typedef struct {
  char patchConfig;   
  char zwrite;        
  char chroma;        
  char plane;         
  char clip;          
  char colourFormat;  
  char alpha;          
}ObjectProperties;

typedef struct {
  int chid;                     
  int id;                       
  int device;                   
  int instance;                 
  ObjectProperties properties;  
}NVObject;

extern int ErrorF(const char *fmt,...);



static int graphicsEngineOk;


static void EnableOptimisations(void)
{
   
   

 nvPGRAPHPort[((4194432    )- (4194304) )/4] =(  (( 1    ) << (4))   |
                      (( 1    ) << (20))   |
                      (( 1    ) << (24))     )  ;
            
 nvPGRAPHPort[((4194436    )- (4194304) )/4] =(  (( 1    ) << (16))   |
                      (( 1    ) << (20))     )  ;

 nvPGRAPHPort[((4194440    )- (4194304) )/4] =(  (( 1    ) << (8))   |
                      (( 1    ) << (28))   |
                      (( 1    ) << (0))   |
                      (( 1    ) << (8))     )  ;

 nvPGRAPHPort[((4194444    )- (4194304) )/4] =(  (( 1    ) << (24))     )  ;
}

static void InitDMAInstance(void)
{
  nvPGRAPHPort[((4195968    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195972    )- (4194304) )/4] =(  0  )  ;
}

static void DisableFifo(void)
{
   
  nvPFIFOPort[((9472    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
  nvPFIFOPort[((12800    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
  nvPFIFOPort[((12864    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
  nvPFIFOPort[((12288    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
  nvPFIFOPort[((12352    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
}

static void EnableFifo(void)
{
     


    nvPTIMERPort[((37376    )- (36864) )/4] =(   8  )  ;
    nvPTIMERPort[((37392    )- (36864) )/4] =(   3  )  ;
     


    nvPFIFOPort[((12800    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
    nvPFIFOPort[((12864    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
    nvPFIFOPort[((9472    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
}







 















 
static void ClearOutFifoContext(void)
{
  int i;

  for(i= (4608/4) ;i< (512/4) ;i++) {
   nvPRAMINPort[ i ]=( 0 ) ;
  }
}  

int NV3KbRamUsedByHW(void)
{
   return (8192 );
}



static void ClearOutContext(void)
{
  int i;

   
  nvPGRAPHPort[((4194688    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4194708    )- (4194304) )/4] =(  0  )  ;

  ClearOutFifoContext();

   
  nvPFIFOPort[((12816    )- (8192) )/4] =(  0  )  ;nvPFIFOPort[((12912    )- (8192) )/4] =(  0  )  ;

   
  nvPFIFOPort[((9232    )- (8192) )/4] =(  0  )  ;
  nvPFIFOPort[((9248    )- (8192) )/4] =(  0  )  ;
  nvPFIFOPort[((9216    )- (8192) )/4] =(  0  )  ;

   
  for(i=0;i< 8 ;i++) {
    nvPFIFOPort[(((12928+( i )*16)    )- (8192) )/4] =(  0  )  ;
  }
  for(i=0;i< 8 ;i++) {
    nvPGRAPHPort[(((4194720+( i )*4)    )- (4194304) )/4] =(  0  )  ;
  }

}

static HashTableEntry   localHash[256 ][2 ];
static HashTableEntry *realHash= ((void *)0) ;






static void ClearOutHashTables(void)
{
  int i,j;


   

   
  for(i=0;i< 256 ;i++) {
    for(j=0;j< 2 ;j++) {
      localHash[i][j].id=0;
      localHash[i][j].context=0;
      nvPRAMINPort[ (((( i )* 2 )+( j ))*2)  ]=( 0 ) ;
      nvPRAMINPort[ (((( i )* 2 )+( j ))*2) +1 ]=( 0 ) ;
    }
  }
}

 




static void LoadChannelContext(int screenWidth,int screenHeight,int bpp)
{
  int i;
  UINT32 read;
  int pitch=((bpp+1)/8)*screenWidth;


   

  nvPFIFOPort[((12292    )- (8192) )/4] =(  0  )  ;
  nvPFIFOPort[((12804    )- (8192) )/4] =(  0  )  ;

   
  nvPFIFOPort[((12832    )- (8192) )/4] =(  0  )  ;
  nvPFIFOPort[((8704    )- (8192) )/4] =(  0  )  ;

  nvPFIFOPort[((12880    )- (8192) )/4] =(   0  )  ;
  read= nvPFIFOPort[((12880    )- (8192) )/4]   ;
  nvPFIFOPort[((12880    )- (8192) )/4] =(  read| (( 1    ) << (4))     )  ;
  read= nvPFIFOPort[((12880    )- (8192) )/4]   ;
   

   
  nvPGRAPHPort[((4194704    )- (4194304) )/4] =(  (( 2    ) << (0))    |
		           (( 0    ) << (8))    |
		           (( 0    ) << (16))    |
		           (( 0    ) << (24))    |
		           (( 1    ) << (28))     )  ;


   
  nvPGRAPHPort[((4196004    )- (4194304) )/4] =(  (( 1    ) << (0))     )  ;
 
     
  nvPGRAPHPort[((4195664    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195668    )- (4194304) )/4] =(  (((( 65535  )<<16))|( 65535  ))   )  ;

  nvPGRAPHPort[((4195672    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195676    )- (4194304) )/4] =(  (((( 65535  )<<16))|( 65535  ))   )  ;

   
  nvPGRAPHPort[((4195968    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195972    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195976    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195980    )- (4194304) )/4] =(  0  )  ;

   
   






  nvPGRAPHPort[((4195888    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195892    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195896    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195900    )- (4194304) )/4] =(  0  )  ;

   
  nvPGRAPHPort[((4195920    )- (4194304) )/4] =(  pitch  )  ;nvPGRAPHPort[((4195924    )- (4194304) )/4] =(  pitch  )  ;
  nvPGRAPHPort[((4195928    )- (4194304) )/4] =(  pitch  )  ;nvPGRAPHPort[((4195932    )- (4194304) )/4] =(  pitch  )  ;


  switch(bpp) { 
    case 8:
      nvPGRAPHPort[((4196008    )- (4194304) )/4] =(  (( 1    ) << (0))   |
                          (( 1    ) << (4))   |
                          (( 1    ) << (8))   |
                          (( 1    ) << (12))     )  ;
      break;
    case 15:
    case 16:
      nvPGRAPHPort[((4196008    )- (4194304) )/4] =(  (( 2    ) << (0))   |
                          (( 2    ) << (4))   |
                          (( 2    ) << (8))   |
                          (( 2    ) << (12))     )  ;
      break; 
    case 32:
      nvPGRAPHPort[((4196008    )- (4194304) )/4] =(  (( 3    ) << (0))   |
                          (( 3    ) << (4))   |
                          (( 3    ) << (8))   |
                          (( 3    ) << (12))     )  ;
      break;  
    default:
      break;
  }


  nvPGRAPHPort[((4196000    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195984    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195992    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195988    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195996    )- (4194304) )/4] =(  0  )  ;

  nvPGRAPHPort[((4195644    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195648    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195652    )- (4194304) )/4] =(  32767  )  ;
  nvPGRAPHPort[((4195656    )- (4194304) )/4] =(  32767  )  ;

   
  nvPGRAPHPort[((4195680    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195684    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195688    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195692    )- (4194304) )/4] =(  0  )  ;
 
  nvPGRAPHPort[((4195620    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195624    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195628    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195632    )- (4194304) )/4] =(  0  )  ;

  nvPGRAPHPort[((4195596    )- (4194304) )/4] =(  0  )  ;
   




  nvPGRAPHPort[((4195884    )- (4194304) )/4] =(  0  )  ;
    
  nvPGRAPHPort[((4195908    )- (4194304) )/4] =(  0  )  ;
 
   
  nvPGRAPHPort[((4195880    )- (4194304) )/4] =(  -1  )  ;
  nvPGRAPHPort[((4195904    )- (4194304) )/4] =(  0  )  ;

  for(i=0;i< 32 ;i++) {
    nvPGRAPHPort[(((4195328+( i )*4)    )- (4194304) )/4] =(  0  )  ;

  }
  for(i=0;i< 32 ;i++) {
    nvPGRAPHPort[(((4195456+( i )*4)    )- (4194304) )/4] =(  0  )  ;
  }

  nvPGRAPHPort[((4195636    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195640    )- (4194304) )/4] =(  0  )  ;

  nvPGRAPHPort[((4195604    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195608    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195612    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195616    )- (4194304) )/4] =(  0  )  ;
  nvPGRAPHPort[((4195584    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195588    )- (4194304) )/4] =(  0  )  ;

   
  nvPGRAPHPort[((4195840    )- (4194304) )/4] =(  0  )  ;nvPGRAPHPort[((4195844    )- (4194304) )/4] =(  255  )  ;
  nvPGRAPHPort[((4195848    )- (4194304) )/4] =(  1  )  ;nvPGRAPHPort[((4195852    )- (4194304) )/4] =(  255  )  ;
  nvPGRAPHPort[(((4195856+( 0 )*4)    )- (4194304) )/4] =(  -1  )  ;nvPGRAPHPort[(((4195856+( 1 )*4)    )- (4194304) )/4] =(  -1  )  ;
  nvPGRAPHPort[((4195864    )- (4194304) )/4] =(  0  )  ;

   
  nvPGRAPHPort[((4195876    )- (4194304) )/4] =(  204  )  ;

  nvPGRAPHPort[((4195592    )- (4194304) )/4] =(  0  )  ;

}

 







  
 
 


 


static NVObject ropObject;
static NVObject clipObject;
static NVObject patternObject;
static NVObject rectObject;
static NVObject blitObject;
static NVObject colourExpandObject;
static NVObject lineObject;
static NVObject linObject;
static NVObject glyphObject;


static void PlaceObjectInHashTable(NVObject *object)
{
  UINT32 hash;
  UINT32 context;
  int i;

   
   
  hash= (((( object->id )^(( object->id )>>8)^(( object->id )>>16)^(( object->id )>>24))&255)^(( object->chid )&127)) ;
  
  for(i=0;i< 2 ;i++) {
    if(localHash[hash][i].id==0) break;  
     
    if(localHash[hash][i].context==object->id) return;
  }
  if(i== 2 ) {
     


    ErrorF ("**** NO ROOM FOR OBJECT %08lx IN HASH TABLE ****\n",object->id);
    graphicsEngineOk=0;  
    return;
  }

  context=((object->device)<<16)| (( 1 ) << (23)) | (( object->instance ) << (0)) |
          (( object->chid ) << (24)) ;



   
  localHash[hash][i].id=object->id;
  localHash[hash][i].context=context;
  nvPRAMINPort[ (((( hash )* 2 )+( i ))*2)  ]=( object->id ) ;
  nvPRAMINPort[ (((( hash )* 2 )+( i ))*2) +1 ]=( context ) ;
}




static void PlaceObjectInInstTable(NVObject *object)
{
  ObjectProperties *p=&(object->properties);
  UINT32 context;

   
  context= (( p->patchConfig ) << (24)) | (( p->chroma ) << (13)) |
          (( p->plane ) << (14)) | (( p->clip ) << (15)) |
          (( p->colourFormat ) << (0)) | (( p->alpha ) << (3)) |(1<<20);





  nvPRAMINPort[ (object->instance<<2)+0 ]=( context ) ;
  nvPRAMINPort[ (object->instance<<2)+1 ]=( 0 ) ;
  nvPRAMINPort[ (object->instance<<2)+2 ]=( 0 ) ;
  nvPRAMINPort[ (object->instance<<2)+3 ]=( 0 ) ;
  
}


 


static int AllocateFreeInstance(void)
{
  static int freeInstance= ((5120/4) /16) ;

  return freeInstance++;
}

static int defaultColourFormat= 1 ;

static void InitObject(NVObject *o,int id,int device)
{
  ObjectProperties *p=&(o->properties);

  o->id=id;
  o->chid=0;
  o->instance=AllocateFreeInstance();
  o->device=device;

  p->patchConfig= 16 ;
  p->zwrite=0; 
  p->chroma=0; 
  p->plane=0;
  p->clip=1;
  p->colourFormat=defaultColourFormat;
  p->alpha=0;
}




static void SetUpObjects(int bpp)
{
  defaultColourFormat=(bpp==16) ? 0  : 1 ;

  InitObject(&ropObject,-1728053248 ,(((4325376) &8323072)>>16) );
  PlaceObjectInHashTable(&ropObject);
  PlaceObjectInInstTable(&ropObject);

  InitObject(&clipObject,-1728053247 ,(((4521984) &8323072)>>16) );
  PlaceObjectInHashTable(&clipObject);
  PlaceObjectInInstTable(&clipObject);

  InitObject(&rectObject,-2013265920 ,(((4653056) &8323072)>>16) );
  PlaceObjectInHashTable(&rectObject);
  PlaceObjectInInstTable(&rectObject);

  InitObject(&blitObject,-2013265919 ,(((5242880) &8323072)>>16) );
  PlaceObjectInHashTable(&blitObject);
  PlaceObjectInInstTable(&blitObject);

  InitObject(&colourExpandObject,-2013265918 ,
             (((5373952) &8323072)>>16) );
  colourExpandObject.properties.alpha=1;  
  PlaceObjectInHashTable(&colourExpandObject);
  PlaceObjectInInstTable(&colourExpandObject);

  InitObject(&glyphObject,-2013265915 ,(((4980736) &8323072)>>16) );
  glyphObject.properties.clip = 0;  
  PlaceObjectInHashTable(&glyphObject);
  PlaceObjectInInstTable(&glyphObject);

  InitObject(&lineObject,-2013265917 ,(((4784128) &8323072)>>16) );
  PlaceObjectInHashTable(&lineObject);
  PlaceObjectInInstTable(&lineObject);

  InitObject(&linObject,-2013265916 ,(((4849664) &8323072)>>16) );
  PlaceObjectInHashTable(&linObject);
  PlaceObjectInInstTable(&linObject);

}



static void ClearAndEnableInterrupts(void)
{

  nvPGRAPHPort[((4194560    )- (4194304) )/4] =(  (( 1    ) << (0))   |
                      (( 1    ) << (4))   |
                      (( 1    ) << (8))   |
                      (( 1    ) << (12))   |
                      (( 1    ) << (16))   |
                      (( 1    ) << (20))   |
                      (( 1    ) << (24))   |
                      (( 1    ) << (28))     )  ;
    
  nvPGRAPHPort[((4194624    )- (4194304) )/4] =(  -1  )  ;

  nvPGRAPHPort[((4194564    )- (4194304) )/4] =(  (( 1    ) << (0))   |
                      (( 1    ) << (4))   |
                      (( 1    ) << (12))   |
                      (( 1    ) << (16))     )  ;

   
  nvPGRAPHPort[((4194628    )- (4194304) )/4] =(  -1  )  ;

  nvPGRAPHPort[((4198656    )- (4194304) )/4] =(  -1  )  ;
  nvPGRAPHPort[((4198720    )- (4194304) )/4] =(  -1  )  ;

  
   
  nvPFIFOPort[((8448    )- (8192) )/4] =(   (( 1    ) << (0))   |
                      (( 1    ) << (4))   |
                      (( 1    ) << (8))   |
                      (( 1    ) << (12))   |
                      (( 1    ) << (16))     )  ;

  nvPFIFOPort[((8512    )- (8192) )/4] =(   (( 1    ) << (0))   |
                         (( 1    ) << (4))   |
                         (( 1    ) << (8))     )  ;

   
  nvPMCPort[((512    )- (0) )/4] =(  -1  )  ;

   



}

static void ResetEngine(void)
{
  nvPMCPort[((512    )- (0) )/4] =(  -65281  )  ;
  nvPMCPort[((512    )- (0) )/4] =(  -1  )  ;
   
  nvPGRAPHPort[((4194436    )- (4194304) )/4] =(  (( 1    ) << (0))     )  ;
  nvPGRAPHPort[((4194432    )- (4194304) )/4] =(  (( 1    ) << (0))     )  ;

}  

static void EnableChannel(void)
{

   
  nvPGRAPHPort[((4194704    )- (4194304) )/4] =(  (( 2    ) << (0))    |
		           (( 0    ) << (8))    |
		           (( 1    ) << (16))    |
		           (( 0    ) << (24))    |
		           (( 1    ) << (28))     )  ;
}  

 



static void InitInstanceMemory(void)
{
  int i;
   


  nvPFIFOPort[((8720    )- (8192) )/4] =(  ((   0   ) << (12))   | 
                    (( 0    ) << (16))     )  ;
  
   
  nvPFIFOPort[((8728    )- (8192) )/4] =(  ((   16   ) << (9))   |
                    (( 0    ) << (16))     )  ;
                      
   


  nvPFIFOPort[((8724    )- (8192) )/4] =(  ((   17   ) << (9))     )  ;
  
   

   
  for(i=0;i<(1024*1024)/4;i++) {
     nvPRAMINPort[ i ]=( 0 ) ;
  }
}

  


int NV3SetupGraphicsEngine(int screenWidth,int screenHeight,int bpp)
{
  graphicsEngineOk=1;

  DisableFifo();

  ResetEngine();

  EnableOptimisations();

  InitDMAInstance();

  ClearAndEnableInterrupts();

  InitInstanceMemory();

  ClearOutContext();

  ClearOutHashTables();

  LoadChannelContext(screenWidth,screenHeight,bpp);

  SetUpObjects(bpp);

  EnableChannel();

  EnableFifo();

  return graphicsEngineOk;

}


void NV3Sync(void)
{
  while(nvPGRAPHPort[((4196016    )- (4194304) )/4]   &1) ;
}

 











void NV3CheckForErrors(char *fileName,int lineNo) 
{
  UINT32 val= nvPMCPort[((256    )- (0) )/4]   ;
     
   
   
  if( val  &  (((unsigned)(1U << ((( 0)-( 0)+1)))-1)  << ( 0))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PAUDIO"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PFIFO"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 12)-( 12)+1)))-1)  << ( 12))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PGRAPH0"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 13)-( 13)+1)))-1)  << ( 13))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PGRAPH1"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 16)-( 16)+1)))-1)  << ( 16))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PVIDEO"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 20)-( 20)+1)))-1)  << ( 20))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PTIMER"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 24)-( 24)+1)))-1)  << ( 24))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PFB"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 28)-( 28)+1)))-1)  << ( 28))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_PBUS"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 31)-( 31)+1)))-1)  << ( 31))    ) { ErrorF ("<%s %d> ""PMC""_""INTR_0_SOFTWARE"" Set\n",fileName,lineNo);} ;

  val= nvPGRAPHPort[((4194560    )- (4194304) )/4]   ;
  if( val  &  (((unsigned)(1U << ((( 0)-( 0)+1)))-1)  << ( 0))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_RESERVED"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 4)-( 4)+1)))-1)  << ( 4))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_CONTEXT_SWITCH"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_VBLANK"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 12)-( 12)+1)))-1)  << ( 12))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_RANGE"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 16)-( 16)+1)))-1)  << ( 16))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_METHOD_COUNT"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 24)-( 24)+1)))-1)  << ( 24))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_COMPLEX_CLIP"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 28)-( 28)+1)))-1)  << ( 28))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_0_NOTIFY"" Set\n",fileName,lineNo);} ;

  val= nvPGRAPHPort[((4194564    )- (4194304) )/4]   ;
  if( val  &  (((unsigned)(1U << ((( 0)-( 0)+1)))-1)  << ( 0))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_1_METHOD"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 4)-( 4)+1)))-1)  << ( 4))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_1_DATA"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 12)-( 12)+1)))-1)  << ( 12))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_1_DOUBLE_NOTIFY"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 16)-( 16)+1)))-1)  << ( 16))    ) { ErrorF ("<%s %d> ""PGRAPH""_""INTR_1_CTXSW_NOTIFY"" Set\n",fileName,lineNo);} ;

  val= nvPFIFOPort[((8448    )- (8192) )/4]   ;
  if( val  &  (((unsigned)(1U << ((( 0)-( 0)+1)))-1)  << ( 0))    ) { ErrorF ("<%s %d> ""PFIFO""_""INTR_0_CACHE_ERROR"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 4)-( 4)+1)))-1)  << ( 4))    ) { ErrorF ("<%s %d> ""PFIFO""_""INTR_0_RUNOUT"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))    ) { ErrorF ("<%s %d> ""PFIFO""_""INTR_0_RUNOUT_OVERFLOW"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 12)-( 12)+1)))-1)  << ( 12))    ) { ErrorF ("<%s %d> ""PFIFO""_""INTR_0_DMA_PUSHER"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 16)-( 16)+1)))-1)  << ( 16))    ) { ErrorF ("<%s %d> ""PFIFO""_""INTR_0_DMA_PTE"" Set\n",fileName,lineNo);} ;


  val= nvPGRAPHPort[((4198656    )- (4194304) )/4]   ;
  if( val  &  (((unsigned)(1U << ((( 0)-( 0)+1)))-1)  << ( 0))    ) { ErrorF ("<%s %d> ""PGRAPH""_""DMA_INTR_0_INSTANCE"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 4)-( 4)+1)))-1)  << ( 4))    ) { ErrorF ("<%s %d> ""PGRAPH""_""DMA_INTR_0_PRESENT"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 8)-( 8)+1)))-1)  << ( 8))    ) { ErrorF ("<%s %d> ""PGRAPH""_""DMA_INTR_0_PROTECTION"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 12)-( 12)+1)))-1)  << ( 12))    ) { ErrorF ("<%s %d> ""PGRAPH""_""DMA_INTR_0_LINEAR"" Set\n",fileName,lineNo);} ;
  if( val  &  (((unsigned)(1U << ((( 16)-( 16)+1)))-1)  << ( 16))    ) { ErrorF ("<%s %d> ""PGRAPH""_""DMA_INTR_0_NOTIFY"" Set\n",fileName,lineNo);} ;


}
