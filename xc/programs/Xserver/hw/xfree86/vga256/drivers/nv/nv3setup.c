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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3setup.c,v 1.1.2.4 1998/01/24 11:55:09 dawes Exp $ */

#include <stdlib.h>


#include "nvuser.h"
#include "nv3ref.h"
#include "nvreg.h"


typedef struct {
  UINT32 id;
  UINT32 context;
}HashTableEntry;

#if 0
typedef struct {
  UINT32 context; /* Configuration for graphics pipe */
  UINT32 dmaNotifyInst; /* Pointers to DMA and notify instance data */
  UINT32 memFormatInst; /* Not sure what the hell this is */
  UINT32 unknown; /* Don't know what the 4th word does */
}ObjInstEntry;
  
/* Low level hardware representation of object */
typedef struct {
  HashTableEntry hash;
  ObjInstEntry inst;
}NVObject;
#endif

typedef struct {
  char patchConfig;  /* How H/W is configured */
  char zwrite;       /* Write to Z buffer */
  char chroma;       /* Chroma keying enabled */
  char plane;        /* Plane mask enabled */
  char clip;         /* User clip enabled */
  char colourFormat; /* 555RGB 888RGB etc */
  char alpha;        /* Alpha enabled */ 
}ObjectProperties;

typedef struct {
  int chid;                    /* Channel ID  (always 0 in this driver) */
  int id;                      /* Unique number for this object */
  int device;                  /* Which hardware device this object will use */
  int instance;                /* Instance number (if it has one) */
  ObjectProperties properties; /* Enabled for this object */
}NVObject;

extern int ErrorF(const char *fmt,...);

#define Info ErrorF

static int graphicsEngineOk;

#define WaitForIdle() while(PGRAPH_Read(STATUS)&1)

static void EnableOptimisations(void)
{
  /* Forget about this for the moment */
  /* Most of the opts are to do with 3D anyway */

 PGRAPH_Write(DEBUG_0,PGRAPH_Def(DEBUG_0_BULK_READS,ENABLED)|
                      PGRAPH_Def(DEBUG_0_WRITE_ONLY_ROPS_2D,ENABLED)|
                      PGRAPH_Def(DEBUG_0_DRAWDIR_AUTO,ENABLED));
            
 PGRAPH_Write(DEBUG_1,PGRAPH_Def(DEBUG_1_INSTANCE,ENABLED)|
                      PGRAPH_Def(DEBUG_1_CTX,ENABLED));

 PGRAPH_Write(DEBUG_2,PGRAPH_Def(DEBUG_2_DPWR_FIFO, ENABLED)|
                      PGRAPH_Def(DEBUG_2_VOLATILE_RESET,ENABLED)|
                      PGRAPH_Def(DEBUG_2_AVOID_RMW_BLEND,ENABLED)|
                      PGRAPH_Def(DEBUG_2_DPWR_FIFO,ENABLED));

 PGRAPH_Write(DEBUG_3,PGRAPH_Def(DEBUG_3_HONOR_ALPHA, ENABLED));
}

static void InitDMAInstance(void)
{
  PGRAPH_Write(DMA,0);
  PGRAPH_Write(NOTIFY,0);
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
  PFIFO_Write(CACHE0_PUSH0,PFIFO_Def(CACHE0_PUSH0_ACCESS,DISABLED));
  PFIFO_Write(CACHE0_PULL0,PFIFO_Def(CACHE0_PULL0_ACCESS,DISABLED));
  PFIFO_Write(CACHES,PFIFO_Def(CACHES_REASSIGN,ENABLED));
}

#define HASH_TABLE_ADDR (0/4)
#define HASH_TABLE_SIZE (4096/4)

#define HASH_TABLE_NUM_COLS 2
#define HASH_TABLE_NUM_ROWS 256

/* All sizes in words */
#define FIFO_CTX_ADDR (4608/4)
#define FIFO_CTX_SIZE (512/4)

#define RUN_OUT_ADDR (4096/4)
#define RUN_OUT_SIZE (512/4)

#define FREE_INSTANCE_ADDR (5120/4)

#define FREE_INST (FREE_INSTANCE_ADDR/16)

#define PRIVILEGED_RAM_SIZE 8192

#define PRAMINRead nvPRAMINPort(addr) nvPRAMINPort[addr]
#define PRAMINWrite(addr,val) nvPRAMINPort[addr]=(val)

/* Clear out channel 0 fifo context */
static void ClearOutFifoContext(void)
{
  int i;

  for(i=FIFO_CTX_ADDR;i<FIFO_CTX_SIZE;i++) {
   PRAMINWrite(i,0);
  }
}  

int NV3KbRamUsedByHW(void)
{
   return (PRIVILEGED_RAM_SIZE);
}



static void ClearOutContext(void)
{
  int i;

  /* Init context register */
  PGRAPH_Write(CTX_SWITCH,0);
  PGRAPH_Write(CTX_USER,0);

  ClearOutFifoContext();

  /* Set PUT and GET pointers to address 0*/
  PFIFO_Write(CACHE1_PUT,0);PFIFO_Write(CACHE1_GET,0);

  /* Make sure there is no runout data */
  PFIFO_Write(RUNOUT_PUT,0);
  PFIFO_Write(RUNOUT_GET,0);
  PFIFO_Write(RUNOUT_STATUS,0);

  /* Clear out CACHED CONTEXT registers */
  for(i=0;i<NV_PFIFO_CACHE1_CTX__SIZE_1;i++) {
    PFIFO_Write(CACHE1_CTX(i),0);
  }
  for(i=0;i<NV_PGRAPH_CTX_CACHE__SIZE_1;i++) {
    PGRAPH_Write(CTX_CACHE(i),0);
  }

}

static HashTableEntry   localHash[HASH_TABLE_NUM_ROWS][HASH_TABLE_NUM_COLS];
static HashTableEntry *realHash=NULL;


#define HASH_FIFO(h,c) ((((h)^((h)>>8)^((h)>>16)^((h)>>24))&0xFF)^((c)&0x7F))

#define HASH_ENTRY(row,col) ((((row)*HASH_TABLE_NUM_COLS)+(col))*2)

static void ClearOutHashTables(void)
{
  int i,j;


  /*  if(!realHash) realHash=(HashTableEntry *) nvPRAMINPort;*/

  /* Clear out host copy of hash table */
  for(i=0;i<HASH_TABLE_NUM_ROWS;i++) {
    for(j=0;j<HASH_TABLE_NUM_COLS;j++) {
      localHash[i][j].id=0;
      localHash[i][j].context=0;
      PRAMINWrite(HASH_ENTRY(i,j),0);
      PRAMINWrite(HASH_ENTRY(i,j)+1,0);
    }
  }
}

/* Defaults the context for the channel to be something sensible. 
 * This code is the basis of what needs to be context switched if this 
 * driver ever expands to cope with multiple channels at the same time.
 */

static void LoadChannelContext(int screenWidth,int screenHeight,int bpp)
{
  int i;
  UINT32 read;
  int pitch=((bpp+1)/8)*screenWidth;


  /* Force Cache 0 and Cache 1 to be set for channel 0 */

  PFIFO_Write(CACHE0_PUSH1,0);
  PFIFO_Write(CACHE1_PUSH1,0);

  /* Disable DMA FIFO pusher */
  PFIFO_Write(CACHE1_DMA0,0);
  PFIFO_Write(CONFIG_0,0);

  PFIFO_Write(CACHE1_PULL1, 0);
  read=PFIFO_Read(CACHE1_PULL1);
  PFIFO_Write(CACHE1_PULL1,read|PFIFO_Def(CACHE1_PULL1_CTX,DIRTY));
  read=PFIFO_Read(CACHE1_PULL1);
  /*  PFIFO_Write(CACHE1_PULL1,read|PFIFO_Def(CACHE1_PULL1_OBJECT,CHANGED));*/

  /* Set context control. Don't enable channel yet */
  PGRAPH_Write(CTX_CONTROL,PGRAPH_Def(CTX_CONTROL_MINIMUM_TIME,2MS) |
		           PGRAPH_Def(CTX_CONTROL_TIME,EXPIRED) |
		           PGRAPH_Def(CTX_CONTROL_CHID,INVALID) |
		           PGRAPH_Def(CTX_CONTROL_SWITCHING,IDLE) |
		           PGRAPH_Def(CTX_CONTROL_DEVICE,ENABLED));


  /* Make sure FIFO can access engines */
  PGRAPH_Write(FIFO,PGRAPH_Def(FIFO_ACCESS,ENABLED));
 
  /* NV3 has src and dest canvases */  
  PGRAPH_Write(SRC_CANVAS_MIN,0);
  PGRAPH_Write(SRC_CANVAS_MAX,PACK_UINT16(MAX_UINT16,MAX_UINT16));

  PGRAPH_Write(DST_CANVAS_MIN,0);
  PGRAPH_Write(DST_CANVAS_MAX,PACK_UINT16(MAX_UINT16,MAX_UINT16));

  /* May as well init this lot. Not sure though */
  PGRAPH_Write(DMA,0);
  PGRAPH_Write(NOTIFY,0);
  PGRAPH_Write(INSTANCE,0);
  PGRAPH_Write(MEMFMT,0);

  /* Set BOFFSET */
  /* I don't understand how these interact with the canvas registers. 
   * They do not seem to provide the same degree of functionality as 
   * pixel addressing is not possible. Shame really, as it looks like 
   * SGI style direct rendering is out. Perhaps I am missing something, 
   * as if you can't do this there seems to be no purpose served by 
   * having multiple channels!
   */
  PGRAPH_Write(BOFFSET0,0);PGRAPH_Write(BOFFSET1,0);
  PGRAPH_Write(BOFFSET2,0);PGRAPH_Write(BOFFSET3,0);

  /* Pitch is the length of a line in bytes */
  PGRAPH_Write(BPITCH0,pitch);PGRAPH_Write(BPITCH1,pitch);
  PGRAPH_Write(BPITCH2,pitch);PGRAPH_Write(BPITCH3,pitch);


  switch(bpp) { 
    case 8:
      PGRAPH_Write(BPIXEL,PGRAPH_Def(BPIXEL_DEPTH0_FMT,BITS_8)|
                          PGRAPH_Def(BPIXEL_DEPTH1_FMT,BITS_8)|
                          PGRAPH_Def(BPIXEL_DEPTH2_FMT,BITS_8)|
                          PGRAPH_Def(BPIXEL_DEPTH3_FMT,BITS_8));
      break;
    case 15:
    case 16:
      PGRAPH_Write(BPIXEL,PGRAPH_Def(BPIXEL_DEPTH0_FMT,BITS_16)|
                          PGRAPH_Def(BPIXEL_DEPTH1_FMT,BITS_16)|
                          PGRAPH_Def(BPIXEL_DEPTH2_FMT,BITS_16)|
                          PGRAPH_Def(BPIXEL_DEPTH3_FMT,BITS_16));
      break; 
    case 32:
      PGRAPH_Write(BPIXEL,PGRAPH_Def(BPIXEL_DEPTH0_FMT,BITS_32)|
                          PGRAPH_Def(BPIXEL_DEPTH1_FMT,BITS_32)|
                          PGRAPH_Def(BPIXEL_DEPTH2_FMT,BITS_32)|
                          PGRAPH_Def(BPIXEL_DEPTH3_FMT,BITS_32));
      break;  
    default:
      break;
  }


  PGRAPH_Write(CLIP_MISC,0);
  PGRAPH_Write(CLIP0_MIN,0);
  PGRAPH_Write(CLIP1_MIN,0);
  PGRAPH_Write(CLIP0_MAX,0);
  PGRAPH_Write(CLIP1_MAX,0);

  PGRAPH_Write(ABS_UCLIP_XMIN,0);
  PGRAPH_Write(ABS_UCLIP_YMIN,0);
  PGRAPH_Write(ABS_UCLIP_XMAX,0x7fff);
  PGRAPH_Write(ABS_UCLIP_YMAX,0x7fff);

  /* Used for the win95 text class */
  PGRAPH_Write(ABS_UCLIPA_XMIN,0);
  PGRAPH_Write(ABS_UCLIPA_YMIN,0);
  PGRAPH_Write(ABS_UCLIPA_XMAX,0);
  PGRAPH_Write(ABS_UCLIPA_YMAX,0);
 
  PGRAPH_Write(CLIPX_0,0);
  PGRAPH_Write(CLIPX_1,0);
  PGRAPH_Write(CLIPY_0,0);
  PGRAPH_Write(CLIPY_1,0);

  PGRAPH_Write(SOURCE_COLOR,0);
  /* These are dubious. Probable doc error
     
  PGRAPH_Write(MONO_COLOR0,0);
  PGRAPH_Write(MONO_COLOR1,0);
  */
  PGRAPH_Write(CHROMA,0);
    
  PGRAPH_Write(CONTROL_OUT,0);
 
  /* Beta and Plane Mask */
  PGRAPH_Write(PLANE_MASK,0xffffffff);
  PGRAPH_Write(BETA,0);

  for(i=0;i<NV_PGRAPH_ABS_X_RAM__SIZE_1;i++) {
    PGRAPH_Write(ABS_X_RAM(i),0);

  }
  for(i=0;i<NV_PGRAPH_ABS_Y_RAM__SIZE_1;i++) {
    PGRAPH_Write(ABS_Y_RAM(i),0);
  }

  PGRAPH_Write(ABS_ICLIP_XMAX,0);PGRAPH_Write(ABS_ICLIP_YMAX,0);

  PGRAPH_Write(XY_LOGIC_MISC0,0);PGRAPH_Write(XY_LOGIC_MISC1,0);
  PGRAPH_Write(XY_LOGIC_MISC2,0);PGRAPH_Write(XY_LOGIC_MISC3,0);
  PGRAPH_Write(X_MISC,0);PGRAPH_Write(Y_MISC,0);

  /* Pattern registers . Initialise to something sensible */
  PGRAPH_Write(PATT_COLOR0_0,0);PGRAPH_Write(PATT_COLOR0_1,0xff);
  PGRAPH_Write(PATT_COLOR1_0,1);PGRAPH_Write(PATT_COLOR1_1,0xff);
  PGRAPH_Write(PATTERN(0),0xffffffff);PGRAPH_Write(PATTERN(1),0xffffffff);
  PGRAPH_Write(PATTERN_SHAPE,0);

  /* Set the ROP to be COPY (Uses Microshaft raster op codes) */
  PGRAPH_Write(ROP3,0xcc);

  PGRAPH_Write(EXCEPTIONS,0);

}

/* Will need to define here what all this lot actually does */

#define COLOR_CONTEXT_R5G5B5     0x0
#define COLOR_CONTEXT_R8G8B8     0x1
#define COLOR_CONTEXT_R10G10B10  0x2
#define COLOR_CONTEXT_Y8         0x3
#define COLOR_CONTEXT_Y16        0x4

  
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


static void PlaceObjectInHashTable(NVObject *object)
{
  UINT32 hash;
  UINT32 context;
  int i;

  /* Will put an entry into the hash table */
  /* Always use channel0 for now !! */
  hash=HASH_FIFO(object->id,object->chid);
  
  for(i=0;i<HASH_TABLE_NUM_COLS;i++) {
    if(localHash[hash][i].id==0) break; /* Found an empty slot!!! */
    /* is object already in cache? */
    if(localHash[hash][i].context==object->id) return;
  }
  if(i==HASH_TABLE_NUM_COLS) {
    /* There is no room at the inn. Since we can't cope with reloading
     * context we had better abort here!!
     */
    Info("**** NO ROOM FOR OBJECT %08lx IN HASH TABLE ****\n",object->id);
    graphicsEngineOk=0; /* Set flag so that we won't use accel */
    return;
  }

  context=((object->device)<<16)|SetBF(23:23,1)|SetBF(15:0,object->instance)|
          SetBF(30:24,object->chid);
#ifdef DEBUG
  ErrorF("Placing object %x instance %x in hash\n",object->id,context);
#endif
  /* Ok, bung entry in at appropriate place */
  localHash[hash][i].id=object->id;
  localHash[hash][i].context=context;
  PRAMINWrite(HASH_ENTRY(hash,i),object->id);
  PRAMINWrite(HASH_ENTRY(hash,i)+1,context);
}




static void PlaceObjectInInstTable(NVObject *object)
{
  ObjectProperties *p=&(object->properties);
  UINT32 context;

  /* This DEFINATELY needs to be symbolic !!!! */
  context=SetBF(28:24,p->patchConfig)|SetBF(13:13,p->chroma)|
          SetBF(14:14,p->plane)|SetBF(15:15,p->clip)|
          SetBF(2:0,p->colourFormat)|SetBF(3:3,p->alpha)|(1<<20);

#ifdef DEBUG
  ErrorF("Object %x instance %x context %x\n",object->id,object->instance,
         context);
#endif
  PRAMINWrite((object->instance<<2)+0,context);
  PRAMINWrite((object->instance<<2)+1,0);
  PRAMINWrite((object->instance<<2)+2,0);
  PRAMINWrite((object->instance<<2)+3,0);
  
}


/* Not exactly terribly complex at the moment, but if we ever get 
 * round to destroying objects ....
 */
static int AllocateFreeInstance(void)
{
  static int freeInstance=FREE_INST;

  return freeInstance++;
}

static int defaultColourFormat=COLOR_CONTEXT_R8G8B8;

static void InitObject(NVObject *o,int id,int device)
{
  ObjectProperties *p=&(o->properties);

  o->id=id;
  o->chid=0;
  o->instance=AllocateFreeInstance();
  o->device=device;

  p->patchConfig=PATCH_CONTEXT;
  p->zwrite=0; 
  p->chroma=0; 
  p->plane=0;
  p->clip=1;
  p->colourFormat=defaultColourFormat;
  p->alpha=0;
}


#define OBJECT_CLASS(dev) ((DEVICE_BASE(dev)&0x007f0000)>>16)

static void SetUpObjects(int bpp)
{
  defaultColourFormat=(bpp==16) ? COLOR_CONTEXT_R5G5B5 : COLOR_CONTEXT_R8G8B8;

  InitObject(&ropObject,ROP_OBJECT_ID,OBJECT_CLASS(UROP));
  PlaceObjectInHashTable(&ropObject);
  PlaceObjectInInstTable(&ropObject);

  InitObject(&clipObject,CLIP_OBJECT_ID,OBJECT_CLASS(UCLIP));
  PlaceObjectInHashTable(&clipObject);
  PlaceObjectInInstTable(&clipObject);

  InitObject(&rectObject,RECT_OBJECT_ID,OBJECT_CLASS(URECT));
  PlaceObjectInHashTable(&rectObject);
  PlaceObjectInInstTable(&rectObject);

  InitObject(&blitObject,BLIT_OBJECT_ID,OBJECT_CLASS(UBLIT));
  PlaceObjectInHashTable(&blitObject);
  PlaceObjectInInstTable(&blitObject);

  InitObject(&colourExpandObject,COLOUR_EXPAND_OBJECT_ID,
             OBJECT_CLASS(UBITMAP));
  colourExpandObject.properties.alpha=1; /* Alpha on for transparency */
  PlaceObjectInHashTable(&colourExpandObject);
  PlaceObjectInInstTable(&colourExpandObject);

  InitObject(&lineObject,LINE_OBJECT_ID,OBJECT_CLASS(ULINE));
  PlaceObjectInHashTable(&lineObject);
  PlaceObjectInInstTable(&lineObject);

  InitObject(&linObject,LIN_OBJECT_ID,OBJECT_CLASS(ULIN));
  PlaceObjectInHashTable(&linObject);
  PlaceObjectInInstTable(&linObject);

}



static void ClearAndEnableInterrupts(void)
{

  PGRAPH_Write(INTR_0,PGRAPH_Def(INTR_0_RESERVED,RESET)|
                      PGRAPH_Def(INTR_0_CONTEXT_SWITCH,RESET)|
                      PGRAPH_Def(INTR_0_VBLANK,RESET)|
                      PGRAPH_Def(INTR_0_RANGE,RESET)|
                      PGRAPH_Def(INTR_0_METHOD_COUNT,RESET)|
                      PGRAPH_Def(INTR_0_FORMAT,RESET)|
                      PGRAPH_Def(INTR_0_COMPLEX_CLIP,RESET)|
                      PGRAPH_Def(INTR_0_NOTIFY,RESET));
    
  PGRAPH_Write(INTR_EN_0,0xffffffff);

  PGRAPH_Write(INTR_1,PGRAPH_Def(INTR_1_METHOD,RESET)|
                      PGRAPH_Def(INTR_1_DATA,RESET)|
                      PGRAPH_Def(INTR_1_DOUBLE_NOTIFY,RESET)|
                      PGRAPH_Def(INTR_1_CTXSW_NOTIFY,RESET));

  /* Don't care about any of this lot */
  PGRAPH_Write(INTR_EN_1,0xffffffff);

  PGRAPH_Write(DMA_INTR_0,0xffffffff);
  PGRAPH_Write(DMA_INTR_EN_0,0xffffffff);

  
  /* Reset the FIFO interrupt state */
  PFIFO_Write(INTR_0, PFIFO_Def(INTR_0_CACHE_ERROR,RESET)|
                      PFIFO_Def(INTR_0_RUNOUT,RESET)|
                      PFIFO_Def(INTR_0_RUNOUT_OVERFLOW,RESET)|
                      PFIFO_Def(INTR_0_DMA_PUSHER,RESET)|
                      PFIFO_Def(INTR_0_DMA_PTE,RESET));

  PFIFO_Write(INTR_EN_0, PFIFO_Def(INTR_EN_0_CACHE_ERROR,ENABLED)|
                         PFIFO_Def(INTR_EN_0_RUNOUT,ENABLED)|
                         PFIFO_Def(INTR_EN_0_RUNOUT_OVERFLOW,ENABLED));

  /* Switch on all the user devices in the master control */
  PMC_Write(ENABLE,0xffffffff);

  PMC_Write(INTR_EN_0,PMC_Def(INTR_EN_0_INTA,HARDWARE));
}

static void ResetEngine(void)
{
  PMC_Write(ENABLE,0xffff00ff);
  PMC_Write(ENABLE,0xffffffff);
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
}  

/* This function sets up instance memory to be layed out as follows 
 *
 *          4K hash Table
 */
static void InitInstanceMemory(void)
{
  int i;
  /* This will set the hash table to be 4K, located at address 0 
   * in instance memory
   */
  PFIFO_Write(RAMHT,PFIFO_Val(RAMHT_BASE_ADDRESS,0)| 
                    PFIFO_Def(RAMHT_SIZE,4K));
  
  /* NB, these values must be aligned on a 512 byte boundary !! */
  PFIFO_Write(RAMRO,PFIFO_Val(RAMRO_BASE_ADDRESS,16)|
                    PFIFO_Def(RAMRO_SIZE,512));
                      
  /* Set FIFO context. Need 32 bytes per channel . Smallest size is 
   * 512 bytes which is more than enough for this driver
   */
  PFIFO_Write(RAMFC,PFIFO_Val(RAMFC_BASE_ADDRESS,17));
  
  /* This means that the first FREE instance memory starts at address 
   * 320 (in 16 byte lumps !!)
   */   
  for(i=0;i<(1024*1024)/4;i++) {
     PRAMINWrite(i,0x0);
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
  WaitForIdle();
}

/* This function checks to see if an interrupt has been raised, then
 * prints out the appropriate registers so that you can attempt to 
 * figure out what is going on
 * if you start mucking around with this chip this will happen a lot 
 */



#define CheckBit(var,device,bitfield) \
if(var & device##_Mask(bitfield)) {\
  Info("<%s %d> "#device"_"#bitfield" Set\n",fileName,lineNo);\
}


void NV3CheckForErrors(char *fileName,int lineNo) 
{
  UINT32 val=PMC_Read(INTR_0);
  /* Has an interrupt been raised ???? */  
  /*if(val==0) return;*/
  /*  Info("An Interrupt has been raised.\n");*/
  CheckBit(val,PMC,INTR_0_PAUDIO);
  CheckBit(val,PMC,INTR_0_PFIFO);
  CheckBit(val,PMC,INTR_0_PGRAPH0);
  CheckBit(val,PMC,INTR_0_PGRAPH1);
  CheckBit(val,PMC,INTR_0_PVIDEO);
  CheckBit(val,PMC,INTR_0_PTIMER);
  CheckBit(val,PMC,INTR_0_PFB);
  CheckBit(val,PMC,INTR_0_PBUS);
  CheckBit(val,PMC,INTR_0_SOFTWARE);

  val=PGRAPH_Read(INTR_0);
  CheckBit(val,PGRAPH,INTR_0_RESERVED);
  CheckBit(val,PGRAPH,INTR_0_CONTEXT_SWITCH);
  CheckBit(val,PGRAPH,INTR_0_VBLANK);
  CheckBit(val,PGRAPH,INTR_0_RANGE);
  CheckBit(val,PGRAPH,INTR_0_METHOD_COUNT);
  CheckBit(val,PGRAPH,INTR_0_COMPLEX_CLIP);
  CheckBit(val,PGRAPH,INTR_0_NOTIFY);

  val=PGRAPH_Read(INTR_1);
  CheckBit(val,PGRAPH,INTR_1_METHOD);
  CheckBit(val,PGRAPH,INTR_1_DATA);
  CheckBit(val,PGRAPH,INTR_1_DOUBLE_NOTIFY);
  CheckBit(val,PGRAPH,INTR_1_CTXSW_NOTIFY);

  val=PFIFO_Read(INTR_0);
  CheckBit(val,PFIFO,INTR_0_CACHE_ERROR);
  CheckBit(val,PFIFO,INTR_0_RUNOUT);
  CheckBit(val,PFIFO,INTR_0_RUNOUT_OVERFLOW);
  CheckBit(val,PFIFO,INTR_0_DMA_PUSHER);
  CheckBit(val,PFIFO,INTR_0_DMA_PTE);


  val=PGRAPH_Read(DMA_INTR_0);
  CheckBit(val,PGRAPH,DMA_INTR_0_INSTANCE);
  CheckBit(val,PGRAPH,DMA_INTR_0_PRESENT);
  CheckBit(val,PGRAPH,DMA_INTR_0_PROTECTION);
  CheckBit(val,PGRAPH,DMA_INTR_0_LINEAR);
  CheckBit(val,PGRAPH,DMA_INTR_0_NOTIFY);


}
