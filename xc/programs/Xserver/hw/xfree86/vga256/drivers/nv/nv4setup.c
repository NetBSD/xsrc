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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv4setup.c,v 1.1.2.3 1998/11/18 16:38:49 hohndel Exp $ */
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

typedef struct
{
    char patchConfig;   
    char zwrite;        
    char chroma;        
    char plane;         
    char clip;          
    char colourFormat;  
    char alpha;         
}ObjectProperties;

typedef struct
{
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
    nvPGRAPHPort[((4194432    )- (4194304) )/4] =(   305250305  )  ;
    nvPGRAPHPort[((4194436    )- (4194304) )/4] =(   1913721089  )  ;
    nvPGRAPHPort[((4194440    )- (4194304) )/4] =(   299233393  )  ;
    nvPGRAPHPort[((4194444    )- (4194304) )/4] =(   282394417  )  ;
}

static void InitDMAInstance(void)
{
    
}

static void DisableFifo(void)
{

    nvPFIFOPort[((9472    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
    nvPFIFOPort[((12800    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
    nvPFIFOPort[((12880    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
    nvPFIFOPort[((12288    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
    nvPFIFOPort[((12368    )- (8192) )/4] =(  (( 0    ) << (0))     )  ;
}

static void EnableFifo(void)
{
    nvPTIMERPort[((37376    )- (36864) )/4] =(   8  )  ;
    nvPTIMERPort[((37392    )- (36864) )/4] =(   3  )  ;
    nvPTIMERPort[((37120    )- (36864) )/4] =(   -1  )  ;
    nvPTIMERPort[((37184    )- (36864) )/4] =(   0  )  ;
    nvPFIFOPort[((12800    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
    nvPFIFOPort[((12880    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
    nvPFIFOPort[((9472    )- (8192) )/4] =(  (( 1    ) << (0))     )  ;
}

static void ClearOutFifoContext(void)
{
    int i;

    for (i= ((0 + (4096/4) ) + (512/4) ) ;i< (512/4) ;i++)
    {
        nvPRAMINPort[ i ]=( 0 ) ;
    }
}

int NV4KbRamUsedByHW(void)
{
    return (65536 );
}

static void ClearOutContext(void)
{
    int i;

    ClearOutFifoContext();
    nvPFIFOPort[((12304    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12400    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12816    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12912    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12864    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12868    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12840    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((9232    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((9248    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((9216    )- (8192) )/4] =(  0  )  ;
    for (i = 0; i < 8 ; i++)
    {
        nvPGRAPHPort[(((4194688+( i )*4)    )- (4194304) )/4] =(  0  )  ;
        nvPGRAPHPort[(((4194720+( i )*4)    )- (4194304) )/4] =(  0  )  ;
        nvPGRAPHPort[(((4194752+( i )*4)    )- (4194304) )/4] =(  0  )  ;
        nvPGRAPHPort[(((4194784+( i )*4)    )- (4194304) )/4] =(  0  )  ;
    }
}

static HashTableEntry  localHash[512 ];
static HashTableEntry *realHash= ((void *)0) ;

static void ClearOutHashTables(void)
{
    int i,j;

    for (j=0;j< 512 ;j++)
    {
        localHash[j].id=0;
        localHash[j].context=0;
        nvPRAMINPort[ (0 +( j )*2)  ]=( 0 ) ;
        nvPRAMINPort[ (0 +( j )*2) +1 ]=( 0 ) ;
    }
    nvPFIFOPort[((12888    )- (8192) )/4] =(  65535  )  ;
}

static void LoadChannelContext(int screenWidth,int screenHeight,int bpp)
{
    int i;
    UINT32 read;
    int pitch=((bpp+1)/8)*screenWidth;

    nvPFIFOPort[((12292    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12804    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((12832    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((9480    )- (8192) )/4] =(   0  )  ;
    nvPFIFOPort[((9476    )- (8192) )/4] =(  0  )  ;
    nvPFIFOPort[((9484    )- (8192) )/4] =(  65535  )  ;
    nvPFIFOPort[((12372    )- (8192) )/4] =(   1  )  ;
    nvPFIFOPort[((12884    )- (8192) )/4] =(   1  )  ;
    nvPGRAPHPort[((4194656    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4194660    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4194664    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4194668    )- (4194304) )/4] =(   0  )  ;
    for (i = 0; i < 8; i++)
    {
        nvPGRAPHPort[(((4194688+( i )*4)    )- (4194304) )/4] =(   0  )  ;
        nvPGRAPHPort[(((4194720+( i )*4)    )- (4194304) )/4] =(   0  )  ;
        nvPGRAPHPort[(((4194752+( i )*4)    )- (4194304) )/4] =(   0  )  ;
        nvPGRAPHPort[(((4194784+( i )*4)    )- (4194304) )/4] =(   0  )  ;
    }
    nvPGRAPHPort[((4194676    )- (4194304) )/4] =(      0  )  ;
    nvPGRAPHPort[((4198400    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4198404    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4198408    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4198412    )- (4194304) )/4] =(      0  )  ;
    nvPGRAPHPort[((4196192    )- (4194304) )/4] =(     0  )  ;
    for (i = 0; i < 6; i++)
    {
        nvPGRAPHPort[(((4195904+( i )*4)    )- (4194304) )/4] =(    0  )  ;
        nvPGRAPHPort[(((4195928+( i )*4)    )- (4194304) )/4] =(      0  )  ;
        nvPGRAPHPort[(((4195972+( i )*4)    )- (4194304) )/4] =(     16777215  )  ;
    }
    for (i = 0; i < 5; i++)
        nvPGRAPHPort[(((4195952+( i )*4)    )- (4194304) )/4] =(     pitch  )  ;
    nvPGRAPHPort[((4195856    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195860    )- (4194304) )/4] =(      0  )  ;
    nvPGRAPHPort[((4196108    )- (4194304) )/4] =(           0  )  ;
    nvPGRAPHPort[((4196112    )- (4194304) )/4] =(             -1  )  ;
    nvPGRAPHPort[((4195996    )- (4194304) )/4] =(         0  )  ;
    nvPGRAPHPort[((4196000    )- (4194304) )/4] =(         0  )  ;
    switch (bpp)
    {
        case 8:
            nvPGRAPHPort[((4196132    )- (4194304) )/4] =(  (( 1    ) << (0))   |
                                                                                   (( 1    ) << (4))   |
                                                                                   (( 1    ) << (8))   |
                                                                                   (( 1    ) << (12))   |
                                                                                   (( 1    ) << (16))   |
                                                                                   (( 1    ) << (20))     )  ;
            nvPGRAPHPort[((4195864    )- (4194304) )/4] =(  (( 0    ) << (0))   |
                                                                                   (( 1    ) << (4))   |
                                                                                   (( 1    ) << (12))     )  ;
            nvPGRAPHPort[((4196400    )- (4194304) )/4] =(  ((    1   ) << (0))    |
                                                                                   ((    1   ) << (8))    |
                                                                                   ((    1   ) << (16))    |
                                                                                   ((   1   ) << (24))     )  ;
            break;
        case 15:
            nvPGRAPHPort[((4196132    )- (4194304) )/4] =(  (( 2    ) << (0))   |
                                                                                   (( 2    ) << (4))   |
                                                                                   (( 2    ) << (8))   |
                                                                                   (( 2    ) << (12))   |
                                                                                   (( 2    ) << (16))   |
                                                                                   (( 2    ) << (20))     )  ;
            nvPGRAPHPort[((4195864    )- (4194304) )/4] =(  (( 1    ) << (0))   |
                                                                                   (( 7    ) << (4))   |
                                                                                   (( 2    ) << (12))     )  ;
            nvPGRAPHPort[((4196400    )- (4194304) )/4] =(  ((    9   ) << (0))    |
                                                                                   ((    9   ) << (8))    |
                                                                                   ((    9   ) << (16))    |
                                                                                   ((   9   ) << (24))     )  ;
            break;
        case 16:
            nvPGRAPHPort[((4196132    )- (4194304) )/4] =(  (( 5    ) << (0))   |
                                                                                   (( 5    ) << (4))   |
                                                                                   (( 5    ) << (8))   |
                                                                                   (( 5    ) << (12))   |
                                                                                   (( 5    ) << (16))   |
                                                                                   (( 5    ) << (20))     )  ;
            nvPGRAPHPort[((4195864    )- (4194304) )/4] =(  (( 2    ) << (0))   |
                                                                                   ((   12   ) << (4))   |
                                                                                   (( 5    ) << (12))     )  ;
            nvPGRAPHPort[((4196400    )- (4194304) )/4] =(  ((    12   ) << (0))    |
                                                                                   ((    12   ) << (8))    |
                                                                                   ((    12   ) << (16))    |
                                                                                   ((   12   ) << (24))     )  ;
            break;
        case 32:
            nvPGRAPHPort[((4196132    )- (4194304) )/4] =(  (( 7    ) << (0))   |
                                                                                   (( 7    ) << (4))   |
                                                                                   (( 7    ) << (8))   |
                                                                                   (( 7    ) << (12))   |
                                                                                   (( 7    ) << (16))   |
                                                                                   (( 7    ) << (20))     )  ;
            nvPGRAPHPort[((4195864    )- (4194304) )/4] =(  (( 5    ) << (0))   |
                                                                                   (( 14    ) << (4))   |
                                                                                   (( 7    ) << (12))     )  ;
            nvPGRAPHPort[((4196400    )- (4194304) )/4] =(  ((    7   ) << (0))    |
                                                                                   ((    7   ) << (8))    |
                                                                                   ((    7   ) << (16))    |
                                                                                   ((   7   ) << (24))     )  ;
            break;
    }
    nvPGRAPHPort[((4196116    )- (4194304) )/4] =(            0  )  ;
    nvPGRAPHPort[((4196352    )- (4194304) )/4] =(       -1  )  ;
    nvPGRAPHPort[((4196356    )- (4194304) )/4] =(       -1  )  ;
    nvPGRAPHPort[(((4196360+( 0 )*4)    )- (4194304) )/4] =(        0  )  ;
    nvPGRAPHPort[(((4196360+( 1 )*4)    )- (4194304) )/4] =(        0  )  ;
    nvPGRAPHPort[((4196368    )- (4194304) )/4] =(     0  )  ;
    nvPGRAPHPort[((4195840    )- (4194304) )/4] =(       -1  )  ;
    nvPGRAPHPort[((4195844    )- (4194304) )/4] =(              204  )  ;
    nvPGRAPHPort[((4196372    )- (4194304) )/4] =(            0  )  ;
    nvPGRAPHPort[((4195848    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4195852    )- (4194304) )/4] =(      0  )  ;
    nvPGRAPHPort[((4196376    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4196380    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4196384    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4196388    )- (4194304) )/4] =(             0  )  ;
    nvPGRAPHPort[((4195596    )- (4194304) )/4] =(      0  )  ;
    for (i = 0; i < 32; i++)
    {
        nvPGRAPHPort[(((4195328+( i )*4)    )- (4194304) )/4] =(   0  )  ;
        nvPGRAPHPort[(((4195456+( i )*4)    )- (4194304) )/4] =(   0  )  ;
    }        
    nvPGRAPHPort[((4195604    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195608    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195612    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195616    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195620    )- (4194304) )/4] =(           0  )  ;
    nvPGRAPHPort[((4195624    )- (4194304) )/4] =(           0  )  ;
    nvPGRAPHPort[((4195628    )- (4194304) )/4] =(           0  )  ;
    nvPGRAPHPort[((4195632    )- (4194304) )/4] =(           0  )  ;
    for (i = 0; i < 16; i++)
    {
        nvPGRAPHPort[(((4197632+( i )*4)    )- (4194304) )/4] =(      0  )  ;
        nvPGRAPHPort[(((4197696+( i )*4)    )- (4194304) )/4] =(      0  )  ;
        nvPGRAPHPort[(((4197760+( i )*4)    )- (4194304) )/4] =(      0  )  ;
    }
    nvPGRAPHPort[((4195708    )- (4194304) )/4] =(        0  )  ;
    nvPGRAPHPort[((4195712    )- (4194304) )/4] =(        0  )  ;
    nvPGRAPHPort[((4195716    )- (4194304) )/4] =(        0  )  ;
    nvPGRAPHPort[((4196196    )- (4194304) )/4] =(      0  )  ;
    nvPGRAPHPort[((4196200    )- (4194304) )/4] =(     0  )  ;
    nvPGRAPHPort[((4195600    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4195696    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4195700    )- (4194304) )/4] =(          0  )  ;
    nvPGRAPHPort[((4195584    )- (4194304) )/4] =(            0  )  ;
    nvPGRAPHPort[((4195588    )- (4194304) )/4] =(            0  )  ;
    nvPGRAPHPort[((4195644    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195648    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195652    )- (4194304) )/4] =(    262143  )  ;
    nvPGRAPHPort[((4195656    )- (4194304) )/4] =(    262143  )  ;
    nvPGRAPHPort[((4195680    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4195684    )- (4194304) )/4] =(   0  )  ;
    nvPGRAPHPort[((4195688    )- (4194304) )/4] =(   262143  )  ;
    nvPGRAPHPort[((4195692    )- (4194304) )/4] =(   262143  )  ;
    nvPGRAPHPort[((4195636    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195640    )- (4194304) )/4] =(    0  )  ;
    nvPGRAPHPort[((4195592    )- (4194304) )/4] =(            -1  )  ;
    nvPGRAPHPort[((4195704    )- (4194304) )/4] =(            -1  )  ;
    nvPGRAPHPort[((4194672    )- (4194304) )/4] =(   (( 0    ) << (0))   
                                                                            | (( 1    ) << (8))   
                                                                            | (( 1    ) << (16))   
                                                                            | (( 0    ) << (24))   
                                                                            | (( 1    ) << (28))     )  ;
    nvPGRAPHPort[((4196128    )- (4194304) )/4] =(  (( 1    ) << (0))     )  ;
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

static int PlaceObjectInHashTable(NVObject *object)
{
    UINT32 hash;
    UINT32 context;
    int i;

    hash = ((((unsigned)( object->id ))^(((unsigned)( object->id ))>>9)^(((unsigned)( object->id ))>>18)^(((unsigned)( object->id ))>>27))&511) ;
    if (localHash[hash].id != 0)
    {
        if (localHash[hash].id==object->id)
            return (1);
        else
        {
            ErrorF ("**** NO ROOM FOR OBJECT %08lx IN HASH TABLE ****\n",object->id);
            graphicsEngineOk=0;  
            return (0);
        }
    }
    context = (( 1 ) << (31)) 
              | (( 1 ) << (16)) 
              | (( (object->instance + 65536/16) ) << (0)) 
              | (( object->chid ) << (24)) ;
    localHash[hash].id      = object->id;
    localHash[hash].context = context;
    nvPRAMINPort[ (0 +( hash )*2)  ]=( object->id ) ;
    nvPRAMINPort[ (0 +( hash )*2) +1 ]=( context ) ;
    return (1);
}

static void PlaceObjectInInstTable(NVObject *object)
{
    ObjectProperties *p=&(object->properties);
    UINT32 context0, context1;

    context0 = (( object->device ) << (0)) 
               | (( p->chroma ) << (12)) 
               | (( p->clip ) << (13)) 
               | (( p->patchConfig ) << (15)) 
               | (( 1 ) << (24))    
               ; 
    context1 = (( 1 ) << (0))      
               | (( p->colourFormat ) << (8)) ;
    nvPRAMINPort[ (object->instance<<2)+0 ]=( context0 ) ;
    nvPRAMINPort[ (object->instance<<2)+1 ]=( context1 ) ;
    nvPRAMINPort[ (object->instance<<2)+2 ]=( 0 ) ;
    nvPRAMINPort[ (object->instance<<2)+3 ]=( 0 ) ;
}

static int AllocateFreeInstance(void)
{
    static int freeInstance= (((4096/4) + (512/4) + (512/4) ) /4) ;

    return (freeInstance++);
}

static int defaultColourFormat= 14 ;

static void InitObject(NVObject *o,int id,int device)
{
    ObjectProperties *p=&(o->properties);

    o->id=id;
    o->chid=0;
    o->instance=AllocateFreeInstance();
    o->device=device;
    p->patchConfig= 1 ;
    p->zwrite=0;
    p->chroma=0;
    p->plane=0;
    p->clip=1;
    p->colourFormat=defaultColourFormat;
    p->alpha=0;
}

static void SetUpObjects(int bpp)
{
    switch (bpp)
    {
        case 8:
            defaultColourFormat = 3 ;
            break;
        case 15:
            defaultColourFormat = 9 ;
            break;
        case 16:
            defaultColourFormat = 12 ;
            break;
        case 24:
            defaultColourFormat = 14 ;
            break;
    } 

    InitObject(&ropObject,-1728053248 ,(67  ) );
    PlaceObjectInHashTable(&ropObject);
    PlaceObjectInInstTable(&ropObject);

    InitObject(&clipObject,-1728053247 ,(25  ) );
    PlaceObjectInHashTable(&clipObject);
    PlaceObjectInInstTable(&clipObject);

    InitObject(&rectObject,-2013265920 ,(94  ) );
    PlaceObjectInHashTable(&rectObject);
    PlaceObjectInInstTable(&rectObject);

    InitObject(&blitObject,-2013265919 ,(95  ) );
    PlaceObjectInHashTable(&blitObject);
    PlaceObjectInInstTable(&blitObject);

    InitObject(&glyphObject,-2013265915 ,(75  ) );
    glyphObject.properties.clip = 0;  
    PlaceObjectInHashTable(&glyphObject);
    PlaceObjectInInstTable(&glyphObject);
}

static void ClearAndEnableInterrupts(void)
{
    nvPGRAPHPort[((4194560    )- (4194304) )/4] =(        -1  )  ;
    nvPGRAPHPort[((4194624    )- (4194304) )/4] =(     -1  )  ;
    nvPFIFOPort[((8448    )- (8192) )/4] =(       -1  )  ;
    nvPFIFOPort[((8512    )- (8192) )/4] =(    -1  )  ;
    nvPMCPort[((512    )- (0) )/4] =(         -1  )  ;
}

static void ResetEngine(void)
{
    nvPMCPort[((512    )- (0) )/4] =(  -65281  )  ;
    nvPMCPort[((512    )- (0) )/4] =(  -1  )  ;
}

static void EnableChannel(void)
{
    
}

static void InitInstanceMemory(void)
{
    int i;

    nvPFIFOPort[((8720    )- (8192) )/4] =(  (( 16    ) << (4))   |
                                                                          (( 0    ) << (16))   |
                                                                          (( 3    ) << (24))     )  ;
    nvPFIFOPort[((8724    )- (8192) )/4] =(  (( 136    ) << (1))     )  ;
    nvPFIFOPort[((8728    )- (8192) )/4] =(  (( 137    ) << (1))   |
                                                                          (( 0    ) << (16))     )  ;
    nvPFBPort[((1049088    )- (1048576) )/4] =(  4372  )  ;
    for (i = 0; i < (65536  + 2048 )/4; i++)
    {
        nvPRAMINPort[ i ]=( 0 ) ;
    }
}

int NV4SetupGraphicsEngine(int screenWidth,int screenHeight,int bpp)
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
    return (graphicsEngineOk);
}

void NV4Sync(void)
{
    while (nvPGRAPHPort[((4196096    )- (4194304) )/4]   &1) ;
}
