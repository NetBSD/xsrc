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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nvsetup.c,v 1.1.2.3 1998/01/24 11:55:10 dawes Exp $ */

#include <stdlib.h>


#include "nvuser.h"
#include "nv1ref.h"
#include "nvreg.h"

typedef struct {
  UINT32 id;      /* 32 bit unique identifer for this object */
  UINT32 channel; /* What channel this object belongs to */
  UINT32 context; /* Holds configuration of pipeline. Written to CTX SWITCH */
}NVObject;



static int graphicsEngineOk;

#define WaitForIdle() while(PGRAPH_Read(STATUS))

static void EnableOptimisations(void)
{
  /* Now switch on all the optimisations that there are */
  PGRAPH_Write(DEBUG_0,PGRAPH_Def(DEBUG_0_EDGE_FILLING,ENABLED) |
                       PGRAPH_Def(DEBUG_0_WRITE_ONLY_ROPS,ENABLED) |     
                       PGRAPH_Def(DEBUG_0_NONBLOCK_BROAD,ENABLED) |
                       PGRAPH_Def(DEBUG_0_BLOCK_BROAD,ENABLED) |
                       PGRAPH_Def(DEBUG_0_BLOCK,ENABLED) |
                       PGRAPH_Def(DEBUG_0_BULK_READS,ENABLED));

  PGRAPH_Write(DEBUG_1,PGRAPH_Def(DEBUG_1_HIRES_TM,DISABLED) |
	               PGRAPH_Def(DEBUG_1_FAST_BUS,DISABLED) |
	               PGRAPH_Def(DEBUG_1_TM_QUAD_HANDOFF,ENABLED) |
	               PGRAPH_Def(DEBUG_1_FAST_RMW_BLITS,ENABLED) | 
	               PGRAPH_Def(DEBUG_1_PATT_BLOCK,ENABLED) |
	               PGRAPH_Def(DEBUG_1_TRI_OPTS,DISABLED) |
	               PGRAPH_Def(DEBUG_1_BI_RECTS,DISABLED) |
	               PGRAPH_Def(DEBUG_1_DMA_ACTIVITY,IGNORE) |
	               PGRAPH_Def(DEBUG_1_VOLATILE_RESET,NOT_LAST));

  PGRAPH_Write(DEBUG_2,PGRAPH_Def(DEBUG_2_VOLATILE_RESET,ENABLED) |
	               PGRAPH_Def(DEBUG_2_TM_FASTINPUT,ENABLED) |
	               PGRAPH_Def(DEBUG_2_BUSY_PATIENCE,ENABLED) |
	               PGRAPH_Def(DEBUG_2_TRAPEZOID_TEXEL,ENABLED) |
	               PGRAPH_Def(DEBUG_2_MONO_ABORT,DISABLED) | 
	               PGRAPH_Def(DEBUG_2_BETA_ABORT,ENABLED) |
	               PGRAPH_Def(DEBUG_2_ALPHA_ABORT,ENABLED) |
	               PGRAPH_Def(DEBUG_2_AVOID_RMW_BLEND,DISABLED));

  /* 
   * REV c parts have another debug register here, but ignore for the 
   * moment
   */
}

static void InitDMAInstance(void)
{
  PDMA_Write(GR_CHANNEL,PDMA_Def(GR_CHANNEL_ACCESS,DISABLED));
  PDMA_Write(GR_INSTANCE,0);
  PGRAPH_Write(DMA,0);
  PGRAPH_Write(NOTIFY,0);
  PDMA_Write(GR_CHANNEL,PDMA_Def(GR_CHANNEL_ACCESS,ENABLED));
}



static void DisableFifo(void)
{
  /* Disable CACHE1 first */
  PFIFO_Write(CACHES,PFIFO_Def(CACHES_REASSIGN,DISABLED));
  PFIFO_Write(CACHE1_PUSH0,PFIFO_Def(CACHE1_PUSH0_ACCESS,DISABLED));
  PFIFO_Write(CACHE1_PULL0,PFIFO_Def(CACHE1_PULL0_ACCESS,DISABLED));
  PFIFO_Write(CACHE0_PUSH0,PFIFO_Def(CACHE1_PUSH0_ACCESS,DISABLED));
  PFIFO_Write(CACHE0_PULL0,PFIFO_Def(CACHE1_PULL0_ACCESS,DISABLED));

}

static void EnableFifo(void)
{
  /* Enable CACHE1 first */
  PFIFO_Write(CACHE1_PUSH0,PFIFO_Def(CACHE1_PUSH0_ACCESS,ENABLED));
  PFIFO_Write(CACHE1_PULL0,PFIFO_Def(CACHE1_PULL0_ACCESS,ENABLED));
  PFIFO_Write(CACHES,PFIFO_Def(CACHES_REASSIGN,ENABLED));
}


#define PRIV_RAM_SIZE 0
#define NUM_FIFO_CONTEXT ((PRIV_RAM_SIZE)? 128 : 64)

/* Clear out channel 0 fifo context */
static void ClearOutFifoContext(void)
{
  int i;

  /* Set up size of PRAM */
  PRAM_Write(CONFIG_0,PRAM_Val(CONFIG_0_SIZE,PRIV_RAM_SIZE));

  for(i=0;i<NUM_FIFO_CONTEXT;i++) {
    nvPRAMFCPort[i]=0;
  }
}  

int NvKbRamUsedByHW(void)
{
  /* Audio scratch and password context are fixed at 4K, 
   * but hash table, runout and fifo vary with PRIV_RAM_SIZE
   */
   return ( (8<<PRIV_RAM_SIZE)+4 );
}



static void ClearOutContext(void)
{
  int i;

  /* Init context register */
  PGRAPH_Write(CTX_SWITCH,0);

  ClearOutFifoContext();

  /* Set PUT and GET pointers to address 0*/
  PFIFO_Write(CACHE1_PUT,0);PFIFO_Write(CACHE1_GET,0);

  /* Make sure there is no runout data */
  PFIFO_Write(RUNOUT_PUT,0);
  PFIFO_Write(RUNOUT_GET,0);

  /* Nobody is allowed to lie about how much space in the fifo */ 
  PFIFO_Write(CONFIG_0,PFIFO_Def(CONFIG_0_FREE_LIE,DISABLED));

  /* Clear out CACHED CONTEXT registers */
  for(i=0;i<NV_PFIFO_CACHE1_CTX__SIZE_1;i++) {
    PFIFO_Write(CACHE1_CTX(i),0);
  }
}

#define HASH_TABLE_DEPTH (2<<PRIV_RAM_SIZE)
#define HASH_TABLE_SIZE 256

typedef struct {
  UINT32 id;
  UINT32 context;
}HashTableEntry;

static HashTableEntry   localHash[HASH_TABLE_SIZE][HASH_TABLE_DEPTH];
static HashTableEntry *realHash=NULL;


#define HASH_FIFO(h,c) ((((h)^((h)>>8)^((h)>>16)^((h)>>24))&0xFF)^((c)&0x7F))

#define HASH_ENTRY(index,depth) (((index)*HASH_TABLE_DEPTH)+(depth))

static void ClearOutHashTables(void)
{
  int i,j;

  if(!realHash) realHash=(HashTableEntry *) nvPRAMHTPort;

  /* Clear out host copy of hash table */
  for(i=0;i<HASH_TABLE_SIZE;i++) {
    for(j=0;j<HASH_TABLE_DEPTH;j++) {
      localHash[i][j].id=0;
      localHash[i][j].context=0;
      (realHash+HASH_ENTRY(i,j))->id=0;
      (realHash+HASH_ENTRY(i,j))->context=0;
    }
  }
  /* Clear out hash virtual registers */
  for(i=0;i<NV_PRAM_HASH_VIRTUAL__SIZE_1;i++) {
    PRAM_Write(HASH_VIRTUAL(i),0);
  }
}

/* Defaults the context for the channel to be something sensible. 
 * This code is the basis of what needs to be context switched if this 
 * driver ever expands to cope with multiple channels at the same time.
 */

void LoadChannelContext(void)
{
  int i;
  UINT32 read;


  /* Force Cache 0 and Cache 1 to be set for channel 0 */

  PFIFO_Write(CACHE0_PUSH1,0);
  PFIFO_Write(CACHE1_PUSH1,0);

  PFIFO_Write(CACHE1_PULL1, 0);
  read=PFIFO_Read(CACHE1_PULL1);
  PFIFO_Write(CACHE1_PULL1,read|PFIFO_Def(CACHE1_PULL1_CTX,DIRTY));
  read=PFIFO_Read(CACHE1_PULL1);
  PFIFO_Write(CACHE1_PULL1,read|PFIFO_Def(CACHE1_PULL1_OBJECT,CHANGED));

  /* Set context control. Don't enable channel yet */
  PGRAPH_Write(CTX_CONTROL,PGRAPH_Def(CTX_CONTROL_MINIMUM_TIME,2MS) |
		           PGRAPH_Def(CTX_CONTROL_TIME,EXPIRED) |
		           PGRAPH_Def(CTX_CONTROL_CHID,INVALID) |
		           PGRAPH_Def(CTX_CONTROL_SWITCHING,IDLE) |
		           PGRAPH_Def(CTX_CONTROL_DEVICE,ENABLED));

  PGRAPH_Write(CANVAS_MIN,0);
  PGRAPH_Write(CANVAS_MAX,PACK_UINT16(MAX_UINT16,MAX_UINT16));

  PGRAPH_Write(CLIP_MISC,0);
  PGRAPH_Write(CLIP0_MIN,0);
  PGRAPH_Write(CLIP1_MIN,0);
  PGRAPH_Write(CLIP0_MAX,0);
  PGRAPH_Write(CLIP1_MAX,0);

  PGRAPH_Write(CANVAS_MISC,PGRAPH_Def(CANVAS_MISC_DAC_BYPASS,DISABLED)|
                           PGRAPH_Def(CANVAS_MISC_DITHER,ENABLED)|
                           PGRAPH_Def(CANVAS_MISC_REPLICATE,ENABLED));

  PGRAPH_Write(SOURCE_COLOR,0);
  PGRAPH_Write(MONO_COLOR0,0);
  PGRAPH_Write(MONO_COLOR1,0);

  PGRAPH_Write(ABS_UCLIP_XMIN,0);
  PGRAPH_Write(ABS_UCLIP_YMIN,0);
  PGRAPH_Write(ABS_UCLIP_XMAX,0);
  PGRAPH_Write(ABS_UCLIP_YMAX,0);

  /* Beta and Plane Mask */
  PGRAPH_Write(PLANE_MASK,0xffffffff);
  PGRAPH_Write(BETA,0);
  for(i=0;i<NV_PGRAPH_BETA_RAM__SIZE_1;i++) {
    PGRAPH_Write(BETA_RAM(i),0);
  }

  for(i=0;i<NV_PGRAPH_ABS_X_RAM__SIZE_1;i++) {
    PGRAPH_Write(ABS_X_RAM(i),0);

  }
  for(i=0;i<NV_PGRAPH_ABS_Y_RAM__SIZE_1;i++) {
    PGRAPH_Write(ABS_Y_RAM(i),0);
  }

  PGRAPH_Write(ABS_ICLIP_XMAX,0);PGRAPH_Write(ABS_ICLIP_YMAX,0);

  PGRAPH_Write(XY_LOGIC_MISC0,0);PGRAPH_Write(XY_LOGIC_MISC1,0);
  PGRAPH_Write(X_MISC,0);PGRAPH_Write(Y_MISC,0);

  PGRAPH_Write(SUBDIVIDE,0);PGRAPH_Write(EDGEFILL,0);

  /* Pattern registers . Initialise to something sensible */
  PGRAPH_Write(PATT_COLOR0_0,0);PGRAPH_Write(PATT_COLOR0_1,0xff);
  PGRAPH_Write(PATT_COLOR1_0,1);PGRAPH_Write(PATT_COLOR1_1,0xff);
  PGRAPH_Write(PATTERN(0),0xffffffff);PGRAPH_Write(PATTERN(1),0xffffffff);
  PGRAPH_Write(PATTERN_SHAPE,0);

  /* Set the ROP to be COPY (Uses Microshaft raster op codes) */
  PGRAPH_Write(ROP3,0xcc);

  PGRAPH_Write(EXCEPTIONS,0);
  PGRAPH_Write(BIT33,0);

}

static void EnableFlowThru(void)
{
  /* Disable the fifo and the DMA engine, but keep flowthu enabled. 
   * This state is needed to actually access many of the registers 
   * in the graphics engine that we are going to set up
   */
  PGRAPH_Write(MISC,PGRAPH_Def(MISC_FLOWTHRU_WRITE,ENABLED) |
                    PGRAPH_Def(MISC_FLOWTHRU,ENABLED) | 
		    PGRAPH_Def(MISC_FIFO_WRITE,ENABLED) |
		    PGRAPH_Def(MISC_FIFO,DISABLED) |
		    PGRAPH_Def(MISC_DMA_WRITE,ENABLED) |
		    PGRAPH_Def(MISC_DMA,DISABLED) |
		    PGRAPH_Def(MISC_CLASS_WRITE,ENABLED) |
		    PGRAPH_Val(MISC_CLASS, 0));

}


/* Will need to define here what all this lot actually does */

#define COLOR_CONTEXT_R5G5B5     0x0
#define COLOR_CONTEXT_R8G8B8     0x1
#define COLOR_CONTEXT_R10G10B10  0x2
#define COLOR_CONTEXT_Y8         0x3
#define COLOR_CONTEXT_Y16        0x4


#define GENERATE_CONTEXT(device,cfg,chroma,plane,clip,color,alpha) \
 DEVICE_BASE(device)|\
 SetBF(4:0,cfg)|SetBF(5:5,chroma)|SetBF(6:6,plane)|\
 SetBF(7:7,clip)|SetBF(8:8,0)|SetBF(12:9,color)|\
 SetBF(13:13,alpha)|SetBF(14:14,0)|SetBF(15:15,0)
  
/* This value does SRC & PATTERN */
/* The pattern is disabled/enabled by the ROP code we set up */
#define PATCH_CONTEXT 0x10

/* All contexts will be generated at run time as we need to set the color
 * format dynamically
 */
static NVObject ropObject;
static NVObject clipObject;
static NVObject patternObject;
static NVObject rectObject;
static NVObject blitObject;
static NVObject colourExpandObject;
static NVObject lineObject;
static NVObject linObject;
static NVObject chromaObject;

#define Info ErrorF

static void InitObject(NVObject *object)
{
  UINT32 hash;
  int i;

  /* Will put an entry into the hash table */
  hash=HASH_FIFO(object->id,object->channel);
  
  for(i=0;i<HASH_TABLE_DEPTH;i++) {
    if(localHash[hash][i].id==0) break; /* Found an empty slot!!! */
    /* is object already in cache? */
    if(localHash[hash][i].context==object->id) return;
  }
  if(i==HASH_TABLE_DEPTH) {
    /* There is no room at the inn. Since we can't cope with reloading
     * context we had better abort here!!
     */
    Info("**** NO ROOM FOR OBJECT %08lx IN HASH TABLE ****\n",object->id);
    graphicsEngineOk=0; /* Set flag so that we won't use accel */
    return;
  }
  /* Ok, bung entry in at appropriate place */
  localHash[hash][i].id=object->id;
  localHash[hash][i].context=object->context;
  (realHash+HASH_ENTRY(hash,i))->id=object->id;
  (realHash+HASH_ENTRY(hash,i))->context=object->context;
}

static void SetUpObjects(int bpp)
{
  int colorContext=(bpp==8) ? COLOR_CONTEXT_R8G8B8 : COLOR_CONTEXT_R5G5B5;
  
  ropObject.id=ROP_OBJECT_ID;
  ropObject.channel=0;
  ropObject.context=GENERATE_CONTEXT(UROP,0,0,0,0,0,0);
  InitObject(&ropObject);

  clipObject.id=CLIP_OBJECT_ID;
  clipObject.channel=0;
  clipObject.context=GENERATE_CONTEXT(UCLIP,0,0,0,0,0,0);
  InitObject(&clipObject);

  patternObject.id=PATTERN_OBJECT_ID;
  patternObject.channel=0; 
  patternObject.context=
     GENERATE_CONTEXT(UPATT,0,0,0,0,colorContext,0);
  InitObject(&patternObject);
  
  rectObject.id=RECT_OBJECT_ID;
  rectObject.channel=0; 
  rectObject.context=
     GENERATE_CONTEXT(URECT,PATCH_CONTEXT,0,0,1,colorContext,0);
  InitObject(&rectObject);


  blitObject.id=BLIT_OBJECT_ID;
  blitObject.channel=0; 
  blitObject.context=
     GENERATE_CONTEXT(UBLIT,PATCH_CONTEXT,0,0,1,colorContext,0);
  InitObject(&blitObject);

  colourExpandObject.id=COLOUR_EXPAND_OBJECT_ID;
  colourExpandObject.channel=0;
  colourExpandObject.context=
     GENERATE_CONTEXT(UBITMAP,PATCH_CONTEXT,0,0,1,colorContext,1);
  InitObject(&colourExpandObject);

  lineObject.id=LINE_OBJECT_ID;
  lineObject.channel=0;
  lineObject.context=
     GENERATE_CONTEXT(ULINE,PATCH_CONTEXT,0,0,1,colorContext,0);
  InitObject(&lineObject);

  linObject.id=LIN_OBJECT_ID;
  linObject.channel=0;
  linObject.context=
     GENERATE_CONTEXT(ULIN,PATCH_CONTEXT,0,0,1,colorContext,0);
  InitObject(&linObject);
}



static void ClearAndEnableInterrupts(void)
{
  PGRAPH_Write(INTR_0,PGRAPH_Def(INTR_0_RESERVED,RESET)|
                      PGRAPH_Def(INTR_0_CONTEXT_SWITCH,RESET)|
                      PGRAPH_Def(INTR_0_VBLANK,RESET)|
                      PGRAPH_Def(INTR_0_RANGE,RESET)|
                      PGRAPH_Def(INTR_0_METHOD_COUNT,RESET)|
                      PGRAPH_Def(INTR_0_SOFTWARE,RESET)|
                      PGRAPH_Def(INTR_0_COMPLEX_CLIP,RESET)|
                      PGRAPH_Def(INTR_0_NOTIFY,RESET));


  /* Enable all interrupts except VBLANK */
  PGRAPH_Write(INTR_EN_0,PGRAPH_Def(INTR_EN_0_RESERVED,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_CONTEXT_SWITCH,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_VBLANK,DISABLED)|
                         PGRAPH_Def(INTR_EN_0_RANGE,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_METHOD_COUNT,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_SOFTWARE,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_COMPLEX_CLIP,ENABLED)|
                         PGRAPH_Def(INTR_EN_0_NOTIFY,ENABLED));

  PGRAPH_Write(INTR_1,PGRAPH_Def(INTR_1_METHOD,RESET)|
                      PGRAPH_Def(INTR_1_DATA,RESET)|
                      PGRAPH_Def(INTR_1_NOTIFY_INST,RESET)|
                      PGRAPH_Def(INTR_1_DOUBLE_NOTIFY,RESET)|
                      PGRAPH_Def(INTR_1_CTXSW_NOTIFY,RESET));

  /* Don't care about any of this lot */
  PGRAPH_Write(INTR_EN_1,0);

  /* Reset the FIFO interrupt state */
  PFIFO_Write(INTR_0, PFIFO_Def(INTR_0_CACHE_ERROR,RESET)|
                      PFIFO_Def(INTR_0_RUNOUT,RESET)|
                      PFIFO_Def(INTR_0_RUNOUT_OVERFLOW,RESET));

  PFIFO_Write(INTR_EN_0, PFIFO_Def(INTR_EN_0_CACHE_ERROR,ENABLED)|
                         PFIFO_Def(INTR_EN_0_RUNOUT,ENABLED)|
                         PFIFO_Def(INTR_EN_0_RUNOUT_OVERFLOW,ENABLED));

  /* Switch on all the user devices in the master control */
  PMC_Write(ENABLE,PMC_Def(ENABLE_PAUDIO,ENABLED) |
                   PMC_Def(ENABLE_PDMA,ENABLED)  |
                   PMC_Def(ENABLE_PFIFO,ENABLED) |
                   PMC_Def(ENABLE_PGRAPH,ENABLED) |
                   PMC_Def(ENABLE_PRM,ENABLED) |
                   PMC_Def(ENABLE_PFB,ENABLED));
}

static void ResetEngine(void)
{
  /* Reset the graphics engine state machine */
  PGRAPH_Write(DEBUG_1,PGRAPH_Def(DEBUG_1_VOLATILE_RESET,LAST));
  PGRAPH_Write(DEBUG_0,PGRAPH_Def(DEBUG_0_STATE,RESET));
}  

static void EnableChannel(void)
{

  /* Set context control */
  PGRAPH_Write(CTX_CONTROL,PGRAPH_Def(CTX_CONTROL_MINIMUM_TIME,2MS) |
		           PGRAPH_Def(CTX_CONTROL_TIME,EXPIRED) |
		           PGRAPH_Def(CTX_CONTROL_CHID,VALID) |
		           PGRAPH_Def(CTX_CONTROL_SWITCHING,IDLE) |
		           PGRAPH_Def(CTX_CONTROL_DEVICE,ENABLED));


  PGRAPH_Write(MISC,PGRAPH_Def(MISC_FLOWTHRU_WRITE,ENABLED) |
		    PGRAPH_Def(MISC_FLOWTHRU,ENABLED) |
		    PGRAPH_Def(MISC_FIFO_WRITE,ENABLED) |
		    PGRAPH_Def(MISC_FIFO,ENABLED) |
		    PGRAPH_Def(MISC_DMA_WRITE,ENABLED) |
		    PGRAPH_Def(MISC_DMA,ENABLED) |
		    PGRAPH_Def(MISC_CLASS_WRITE,IGNORED) |
		    PGRAPH_Val(MISC_CLASS, 0));
}  

int SetupGraphicsEngine(int bpp)
{
  graphicsEngineOk=1;

  WaitForIdle();
  /* Possibly enable the hardware engines here. Should already be on though */
 
  DisableFifo();

  EnableFlowThru(); 

  ResetEngine();

  EnableOptimisations();

  InitDMAInstance();

  ClearAndEnableInterrupts();

  ClearOutContext();

  ClearOutHashTables();

  LoadChannelContext();

  SetUpObjects(bpp);

  EnableChannel();

  EnableFifo();

  return graphicsEngineOk;

}

static int channelOpen=0;

NvChannel *NvOpenChannel(void)
{
  if(channelOpen) return NULL;
   
  channelOpen=1;
  return (NvChannel*) nvCHAN0Port;
}

/* Bit of future-proofing here */
void NvCloseChannel(void)
{
  channelOpen=0;
}

void NvSync(void)
{
  WaitForIdle();
}

/* This function checks to see if an interrupt has been raised, then
 * prints out the appropriate registers so that you can attempt to 
 * figure out what is going on
 * if you start mucking around with this chip this will happen a lot 
 */



#define CheckBit(var,device,bitfield) \
if(var & device##_Mask(bitfield)) {\
  Info(#device"_"#bitfield" Set\n");\
}


void NvCheckForErrors(void) 
{
  UINT32 val=PMC_Read(INTR_0);
  /* Has an interrupt been raised ???? */  
  /*if(val==0) return;*/
  /*  Info("An Interrupt has been raised.\n");*/
  CheckBit(val,PMC,INTR_0_PAUDIO);
  CheckBit(val,PMC,INTR_0_PDMA);
  CheckBit(val,PMC,INTR_0_PFIFO);
  CheckBit(val,PMC,INTR_0_PGRAPH);
  CheckBit(val,PMC,INTR_0_PRM);
  CheckBit(val,PMC,INTR_0_PTIMER);
  CheckBit(val,PMC,INTR_0_PFB);
  CheckBit(val,PMC,INTR_0_SOFTWARE);

  val=PGRAPH_Read(INTR_0);
  CheckBit(val,PGRAPH,INTR_0_RESERVED);
  CheckBit(val,PGRAPH,INTR_0_CONTEXT_SWITCH);
  /* CheckBit(val,PGRAPH,INTR_0_VBLANK);*/
  CheckBit(val,PGRAPH,INTR_0_RANGE);
  CheckBit(val,PGRAPH,INTR_0_METHOD_COUNT);
  CheckBit(val,PGRAPH,INTR_0_SOFTWARE);
  CheckBit(val,PGRAPH,INTR_0_COMPLEX_CLIP);
  CheckBit(val,PGRAPH,INTR_0_NOTIFY);

  val=PFIFO_Read(INTR_0);
  CheckBit(val,PFIFO,INTR_0_CACHE_ERROR);
  CheckBit(val,PFIFO,INTR_0_RUNOUT);
  CheckBit(val,PFIFO,INTR_0_RUNOUT_OVERFLOW);
}
