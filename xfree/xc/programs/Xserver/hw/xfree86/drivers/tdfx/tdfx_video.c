/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/tdfx/tdfx_video.c,v 1.5 2000/12/08 21:53:37 mvojkovi Exp $ */

/* Adapted from ../mga/mga_video.c */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86fbman.h"
#include "regionstr.h"

#include "tdfx.h"

#include "xf86xv.h"
#include "Xv.h"
#include "xaa.h"
#include "xaalocal.h"
#include "dixstruct.h"
#include "fourcc.h"

/* These should move into tdfxdefs.h with better names */
#define YUV_Y_BASE 0xC00000
#define YUV_U_BASE 0xD00000
#define YUV_V_BASE 0xE00000

#define SST_2D_FORMAT_YUYV  0x8
#define SST_2D_FORMAT_UYVY  0x9

#define YUVBASEADDR             0x80100
/* This should move to tdfx.h
 * Only one port for now due to need for better memory allocation. */
#define TDFX_MAX_PORTS 1

#define YUVSTRIDE               0x80104

#ifndef XvExtension
void TDFXInitVideo(ScreenPtr pScreen) {}
#else


void TDFXInitVideo(ScreenPtr pScreen);
void TDFXCloseVideo (ScreenPtr pScreen);

static XF86VideoAdaptorPtr TDFXSetupImageVideo(ScreenPtr);
static int  TDFXSetPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
static int  TDFXGetPortAttribute(ScrnInfoPtr, Atom ,INT32 *, pointer);

static void TDFXStopVideo(ScrnInfoPtr, pointer, Bool);
static void TDFXQueryBestSize(ScrnInfoPtr, Bool, short, short, short, short, 
			unsigned int *, unsigned int *, pointer);
static int  TDFXPutImage(ScrnInfoPtr, short, short, short, short, short, 
			short, short, short, int, unsigned char*, short, 
			short, Bool, RegionPtr, pointer);
#if 0
/* This isn't done yet, but eventually it will put images to the 
 * video overlay. */
static int TDFXPutImageOverlay(ScrnInfoPtr pScrn, short, short, 
			       short, short, short, short, 
			       short, short, int, unsigned char*, 
			       short, short, Bool, RegionPtr , pointer);
#endif

static int  TDFXQueryImageAttributes(ScrnInfoPtr, int, unsigned short *, 
			unsigned short *,  int *, int *);

/* These function is from tdfx_accel.c */
extern void TDFXFirstSync(ScrnInfoPtr pScrn);

static FBAreaPtr
TDFXAllocateOffscreenBuffer (ScrnInfoPtr pScrn, int id, int width, int height);

static void
TDFXDeallocateOffscreenBuffer (ScrnInfoPtr pScrn, int id);

void TDFXInitVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XF86VideoAdaptorPtr *adaptors, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    TDFXPtr pTDFX = TDFXPTR(pScrn);
    int num_adaptors;

    /* The hardware can't convert YUV->8 bit color */
    if(pTDFX->cpp == 1)
      return;
    
    newAdaptor = TDFXSetupImageVideo(pScreen);

    /* Initialize the offscreen buffer */
    pTDFX->offscreenYUVBuf = 0;

    num_adaptors = xf86XVListGenericAdaptors(pScrn, &adaptors);

    if(newAdaptor) {
	if(!num_adaptors) {
	    num_adaptors = 1;
	    adaptors = &newAdaptor;
	} else {
	    newAdaptors =  /* need to free this someplace */
		xalloc((num_adaptors + 1) * sizeof(XF86VideoAdaptorPtr*));
	    if(newAdaptors) {
		memcpy(newAdaptors, adaptors, num_adaptors * 
					sizeof(XF86VideoAdaptorPtr));
		newAdaptors[num_adaptors] = newAdaptor;
		adaptors = newAdaptors;
		num_adaptors++;
	    }
	}
    }

    if(num_adaptors)
    {
      xf86XVScreenInit(pScreen, adaptors, num_adaptors);
    }

    if(newAdaptors)
	xfree(newAdaptors);
}

void TDFXCloseVideo (ScreenPtr pScreen)
{

}

/* client libraries expect an encoding */
static XF86VideoEncodingRec DummyEncoding[1] =
{
  {   /* blit limit */
   0,
   "XV_IMAGE",
   1024, 0,   /* Height is a limitation of pixmap space, and filled in later. */
   {1, 1}
 }
};

#define NUM_FORMATS_OVERLAY 3

static XF86VideoFormatRec Formats[NUM_FORMATS_OVERLAY] = 
{
   {15, TrueColor}, {16, TrueColor}, {24, TrueColor},
};

/* #define NUM_IMAGES 4*/
#define NUM_IMAGES 2

static XF86ImageRec Images[NUM_IMAGES] =
{
  XVIMAGE_YV12,  /* YVU planar */
  XVIMAGE_I420   /* YUV planar */
#if 0
  /* These could be supported (without a temp bufer) using the 
   * host-to-screen-stretch */
  XVIMAGE_YUY2,  /* YUYV packed */
  XVIMAGE_UYVY   /* UYVY packed */
#endif
};


static XF86VideoAdaptorPtr
TDFXAllocAdaptor(ScrnInfoPtr pScrn)
{
    DevUnion *devUnions;
    int i;
    XF86VideoAdaptorPtr adapt;

    if(!(adapt = xf86XVAllocateVideoAdaptorRec(pScrn)))
      return NULL;
    /* Allocate a TDFX private structure */
    /* There is no need for a TDFX private structure at this time.  But a 
     * DevUnion has to be provided to the core Xv code. */
    if (!(devUnions = xcalloc (1, sizeof (DevUnion) * TDFX_MAX_PORTS)))
    {
      xfree (adapt);
      return NULL;
     }
    adapt->pPortPrivates = &devUnions[0];
    
    /* Fill in some of the DevUnion */
    for (i = 0; i < TDFX_MAX_PORTS; i++)
    {
      adapt->pPortPrivates[i].val = i;
      adapt->pPortPrivates[i].ptr = NULL;  /* No private data */
    }
    
    return adapt;
}



static XF86VideoAdaptorPtr 
TDFXSetupImageVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XF86VideoAdaptorPtr adapt;
    TDFXPtr pTDFX = TDFXPTR(pScrn);

    DummyEncoding[0].height = pTDFX->pixmapCacheLines;

    adapt = TDFXAllocAdaptor(pScrn);

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    /* Add VIDEO_OVERLAID_IMAGES if using the overlay */
    adapt->flags = 0;
    adapt->name = "3dfx Accelerated Video Engine";
    adapt->nEncodings = 1;
    adapt->pEncodings = &DummyEncoding[0];
    adapt->nFormats = NUM_FORMATS_OVERLAY;
    adapt->pFormats = Formats;
    adapt->nPorts = TDFX_MAX_PORTS;
    adapt->nAttributes = 0;
    adapt->pAttributes = NULL;
    adapt->nImages = NUM_IMAGES;
    adapt->pImages = Images;

    /* XXX For now all I'm implementing is PutImage so that programs like OMS
     * will work.  More will follow as I have time and need. */
    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = TDFXStopVideo;
    adapt->SetPortAttribute = TDFXSetPortAttribute;
    adapt->GetPortAttribute = TDFXGetPortAttribute;
    adapt->QueryBestSize = TDFXQueryBestSize;
    adapt->PutImage = TDFXPutImage;
    /*adapt->PutImage = TDFXPutImageOverlay;    */
    adapt->QueryImageAttributes = TDFXQueryImageAttributes;

    return adapt;
}


static void 
TDFXStopVideo(ScrnInfoPtr pScrn, pointer data, Bool cleanup)
{
  if (cleanup)
  {
    /* Deallocate the offscreen temporary buffer. */
    TDFXDeallocateOffscreenBuffer (pScrn, 0);
  }
}


static int 
TDFXSetPortAttribute (ScrnInfoPtr pScrn, 
		      Atom attribute,
		      INT32 value, 
		      pointer data) 
{
  return BadMatch;
}


static int 
TDFXGetPortAttribute(ScrnInfoPtr pScrn, 
		     Atom attribute,
		     INT32 *value, 
		     pointer data)
{
  return BadMatch;
}



static void 
TDFXQueryBestSize(ScrnInfoPtr pScrn, 
		  Bool motion,
		  short vid_w, short vid_h, 
		  short drw_w, short drw_h, 
		  unsigned int *p_w, unsigned int *p_h, 
		  pointer data)
{
  /* No alignment restrictions */
  *p_w = drw_w;
  *p_h = drw_h; 
}



/* This performs a screen to screen stretch blit.  All coordinates are
 * in screen memory.  This function assumes that the src and dst format 
 * registers have been setup already.  This function does not save 
 * the registers it trashes. */
static void
TDFXScreenToScreenYUVStretchBlit (ScrnInfoPtr pScrn,
				  short src_x1, short src_y1,
				  short src_x2, short src_y2,
				  short dst_x1, short dst_y1,
				  short dst_x2, short dst_y2)
{
   TDFXPtr pTDFX = TDFXPTR(pScrn);

  /* reformulate the paramaters the way the hardware wants them */
  INT32
    src_x = src_x1 & 0x1FFF,
    src_y = src_y1 & 0x1FFF,
    dst_x = dst_x1 & 0x1FFF,
    dst_y = dst_y1 & 0x1FFF,
    src_w = (src_x2 - src_x) & 0x1FFF,
    src_h = (src_y2 - src_y) & 0x1FFF,
    dst_w = (dst_x2 - dst_x) & 0x1FFF,
    dst_h = (dst_y2 - dst_y) & 0x1FFF;

  /* Setup for blit src and dest */
  TDFXMakeRoom(pTDFX, 5);
  DECLARE(SSTCP_DSTSIZE|SSTCP_SRCSIZE|SSTCP_DSTXY|
	  SSTCP_COMMAND|SSTCP_COMMANDEXTRA);
  /* We want the blit to wait for vsync. */
  TDFXWriteLong(pTDFX, SST_2D_COMMANDEXTRA, 4);
  TDFXWriteLong(pTDFX, SST_2D_SRCSIZE, src_w | (src_h<<16));
  TDFXWriteLong(pTDFX, SST_2D_DSTSIZE, dst_w | (dst_h<<16));
  TDFXWriteLong(pTDFX, SST_2D_DSTXY,   dst_x | (dst_y<<16));
  /* XXX find the ROP table and figure out why CC is the right choice. */
  TDFXWriteLong(pTDFX, SST_2D_COMMAND, 
		(0xCC<<24)|SST_2D_SCRNTOSCRNSTRETCH);
  /* Write to the launch area to start the blit */
  TDFXMakeRoom(pTDFX, 1);
  DECLARE_LAUNCH (1, 0);
  TDFXWriteLong(pTDFX, SST_2D_LAUNCH, src_x | (src_y<<16));
}

static void
YUVPlanarToPacked (ScrnInfoPtr pScrn,
		   short src_x, short src_y,
		   short src_h, short src_w,
		   int id, char *buf,
		   short width, short height,
		   FBAreaPtr fbarea)
{
  TDFXPtr pTDFX = TDFXPTR(pScrn);

   INT32 y;
   void *dst;
   char *psrc = buf,
     *pdst = 0;
   int count = 0;

  /* Register saves */
   INT32
     yuvBaseAddr,
     yuvStride;

  /* Save these registers so I can restore them when we are done. */
  yuvBaseAddr = TDFXReadLongMMIO(pTDFX, YUVBASEADDR);
  yuvStride =   TDFXReadLongMMIO(pTDFX, YUVSTRIDE);


  dst = 0;

  /* Set yuvBaseAddress register to point to the buffer. */
  TDFXWriteLongMMIO (pTDFX, YUVBASEADDR, pTDFX->fbOffset + 
		     pTDFX->stride * (fbarea->box.y1 + pScrn->virtualY) 
		     + fbarea->box.x1);
  /* Set yuvStride register to reflect stride of U and V planes */
  /* There is a subtle issue involved with copying the Y plane.  Y is
   * sampled at twice the rate of the U and V data, in both
   * directions.  But when packing their needs to be two Y values for
   * each U,V pair.  So if src_x is odd we will end up missing on
   * of the Y values.  To correct for this we will always adjust the src
   * x value.  This adjust is done both in computing the address of the
   * pixels to copy and when determining the amount of pixels to copy.
   * Note that care needs to be taken to insure that the offscreen
   * temporary buffer is allocated with enough space to hold the possible
   * extra column of data.
   */
  TDFXWriteLongMMIO (pTDFX, YUVSTRIDE, pTDFX->stride);
  psrc = (char*)buf;

  /* psrc points to the base of the Y plane, move out to src_x, src_y */
  psrc += (src_x & ~0x1) + src_y * width;
  pdst = (char *)pTDFX->MMIOBase[0] + YUV_Y_BASE;
  for (y = 0; y < src_h; y++)
  {   
    memcpy (pdst, psrc, src_w + (src_x & 0x1));
    psrc += width;
    /* YUV planar region is always 1024 bytes wide */
    pdst += 1024;
  }

  /* The difference between FOURCC_YV12 and FOURCC_I420 is the order
   * that the U and V planes appear in the buffer.  But at this point
   * I just send the second buffer as V and the third buffer as U.
   * Depending on the format, the packing will be different and we
   * handle it in the the way we pick the source format for the blit
   * later on. */

  pdst = (char *)pTDFX->MMIOBase[0] + YUV_V_BASE;
  psrc = (char*)buf + width * height;
  /* psrc now points to the base of the V plane, move out to src_x/2,
   * src_y/2 */
  psrc += (src_x >> 1) + (src_y >> 1) * (width >> 1);
  for (y = 0; y < src_h >> 1; y++)
  {
    /* YUV planar region is always 1024 bytes wide */
    memcpy (pdst, psrc, src_w >> 1);
    psrc += width >> 1;
    pdst += 1024;
  }
  pdst = (char *)pTDFX->MMIOBase[0] + YUV_U_BASE;
  psrc = (char*)buf + width * height + (width >> 1) * (height >> 1);
  /* psrc now points to the base of the U plane, move out to src_x/2,
   * src_y/2 */
  psrc += (src_x >> 1) + (src_y >> 1) * (width >> 1);
  for (y = 0; y < src_h >> 1; y++)
  {
    /* YUV planar region is always 1024 bytes wide */
    memcpy (pdst, psrc, src_w >> 1);
    psrc += width >> 1;
    pdst += 1024;
  }

  /* Before restoring trashed registers we have to wait for the conversion
   * to finish.  We aren't using the FIFO for this but the hardware can
   * take a little extra time even after we finish all the writes.  If we
   * restore the registers before it finishes it can store some of the YUV
   * data to the wrong place.  One particular wrong place is often on top
   * of the hardware cursor data.  So we wait for the status register
   * to go idle. */
  /* XXX Right now wait for the whole chip to go idle.  This is more than 
   * is required.  Find out which subsystem is handles the YUV packing and
   * wait only on that status bit. */
  count = 0;
  do
  {
    count++;
  } while ((TDFXReadLongMMIO(pTDFX, 0) & SST_BUSY) && count < 1000);

  /* Restore trashed registers */
  TDFXWriteLongMMIO(pTDFX, YUVBASEADDR, yuvBaseAddr);
  TDFXWriteLongMMIO(pTDFX, YUVSTRIDE, yuvStride);  
}

static int 
TDFXPutImage( 
	     ScrnInfoPtr pScrn, 
	     short src_x, short src_y, 
	     short drw_x, short drw_y,
	     short src_w, short src_h, 
	     short drw_w, short drw_h,
	     int id, unsigned char* buf, 
	     short width, short height, 
	     Bool sync,
	     RegionPtr clipBoxes, pointer data
	     )
{
   TDFXPtr pTDFX = TDFXPTR(pScrn);

   BoxPtr pbox;
   int nbox;

   FBAreaPtr fbarea;

   /* Make sure we are synced up (this really means, lock and find the 
    * fifo pointer. */
   TDFXFirstSync (pScrn);

   /* Do the right thing for the given format */
   switch (id)
   {
   case FOURCC_YV12:
   case FOURCC_I420:
     /* Get a buffer to use to store the packed YUV data */
     fbarea = TDFXAllocateOffscreenBuffer (pScrn, id, src_w, src_h);
    
     if (!fbarea)
     {
       return Success;
     }

     YUVPlanarToPacked (pScrn, src_x, src_y, src_h, src_w,
			id, (char *)buf, width, height,
			fbarea);
     /* Don't know what executed last so we need to send a NOP */
     TDFXSendNOP(pScrn);   

     /* Setup the dst and src format once, they don't change for all the
      * blits. */
     TDFXMakeRoom(pTDFX, 2);
     DECLARE(SSTCP_SRCFORMAT|SSTCP_DSTFORMAT);
     TDFXWriteLong(pTDFX, SST_2D_DSTFORMAT, 
		   pTDFX->stride|((pTDFX->cpp+1)<<16));
     if (id == FOURCC_YV12)
     {
       /* Packed format is YUYV */
       TDFXWriteLong(pTDFX, SST_2D_SRCFORMAT, 
		     pTDFX->stride|((SST_2D_FORMAT_YUYV)<<16));
     }
     else
     {
       /* Packed format is UYVY */
       TDFXWriteLong(pTDFX, SST_2D_SRCFORMAT, 
		     pTDFX->stride|((SST_2D_FORMAT_UYVY)<<16));
     }

     /* Traverse the clip boxes */
     nbox = REGION_NUM_RECTS(clipBoxes);
     pbox = REGION_RECTS(clipBoxes);
     
     while (nbox--)
     {
       /* The destination clip regions come with the clip boxes, but
	* the src coordinates have to be computed because we are doing
	* a stretch blit.  These macros perform that compuation, but
	* they could use some work.  When testing with still images these
	* computations caused some jitter in the resulting output, but
	* with actual video playback I haven't noticed any problems. */
#define SRC_X1 (fbarea->box.x1)
#define SRC_Y1 (fbarea->box.y1 + pScrn->virtualY)
#define SCALEX(dx) ((int)(((dx) * src_w + (drw_w>>1)) / drw_w))
#define SCALEY(dy) ((int)(((dy) * src_h + (drw_h>>1)) / drw_h))
       
       /* Do the screen-to-screen blit clipped to the clip boxes. */
       TDFXScreenToScreenYUVStretchBlit 
	 (pScrn,
	  SRC_X1 + SCALEX(pbox->x1 - drw_x), 
	  SRC_Y1 + SCALEY(pbox->y1 - drw_y),
	  SRC_X1 + SCALEX(pbox->x2 - drw_x), 
	  SRC_Y1 + SCALEY(pbox->y2 - drw_y),
	  /* these destination coordinates come
	   * right from the clip box. */
	  pbox->x1, pbox->y1,
	  pbox->x2, pbox->y2);
       pbox++;
     }

     /* Restore the WAX registers we trashed */
     TDFXMakeRoom(pTDFX, 2);
     DECLARE(SSTCP_SRCFORMAT|SSTCP_DSTFORMAT);
     TDFXWriteLong(pTDFX, SST_2D_DSTFORMAT, pTDFX->sst2DDstFmtShadow);
     TDFXWriteLong(pTDFX, SST_2D_SRCFORMAT, pTDFX->sst2DSrcFmtShadow);

     /* The rest of the driver won't know what I have done so I do a stall to 
      * make sure the next command sent will work. */
     TDFXSendNOP(pScrn);  
     break;
   /* XXX What am I supposed to do about the sync flag? */

   }

   return Success;
}

/* This code doesn't work yet.  Eventually this should use the video overlay
 * instead of the YUV-stretch-blit.  The overlay is better because it uses
 * bilinear filtering when scaling. */
#if 0
static int 
TDFXPutImageOverlay( 
	     ScrnInfoPtr pScrn, 
	     short src_x, short src_y, 
	     short drw_x, short drw_y,
	     short src_w, short src_h, 
	     short drw_w, short drw_h,
	     int id, unsigned char* buf, 
	     short width, short height, 
	     Bool sync,
	     RegionPtr clipBoxes, pointer data
	     )
{
   TDFXPtr pTDFX = TDFXPTR(pScrn);

   BoxPtr pbox;
   int nbox;

   /* Computed Register values */
   INT32 
     vidInFormat;

   static FBAreaPtr fbarea;

   /* Do the right thing for the given format */
   switch (id)
   {
   case FOURCC_YV12:
   case FOURCC_I420:

   
     /* Get a buffer to use to store the packed YUV data */
     fbarea = TDFXAllocateOffscreenBuffer (pScrn, id, src_w, src_h);
    
     if (!fbarea)
       return Success;

     YUVPlanarToPacked (pScrn,
			src_x, src_y, src_h, src_w,
			id, buf, width, height,
			fbarea);

     /* Setup the overlay */
     TDFXWriteLongMMIO(pTDFX, VIDOVERLAYSTARTCOORDS,
		       (drw_x&0x7FF) | ((drw_y & 0x7FF) << 12)
		       /* XXX Lower 2 bits of X and Y? */);
     TDFXWriteLongMMIO(pTDFX, VIDOVERLAYENDSCREENCOORDS,
		       ((drw_x+drw_w)&0x7FF) | (((drw_y+drw_h)&0x7FF) << 12));

     /* Set the Video in format */
     vidInFormat = 0;
     /* These magic numbers come from the spec on page 151 */
     if (id == FOURCC_YV12)
     {
       /* Packed format is YUYV */
       vidInFormat = 0x9 << 1;
     }
     else
     {
       /* Packed format is UYVY */
       vidInFormat = 0xA << 1;       
     }

     TDFXWriteLongMMIO (pTDFX, VIDINFORMAT, vidInFormat);
     TDFXWriteLongMMIO (pTDFX, VIDINSTRIDE, src_w);

     /* Use magenta as the chroma color */
     if (pTDFX->cpp == 2)
     {
       TDFXWriteLongMMIO(pTDFX, VIDCHROMAMIN,
			 0x0000F71F);
       TDFXWriteLongMMIO(pTDFX, VIDCHROMAMAX,
			 0x0000F71F);
     }
     else /* (pTDFX->cpp == 3) */
     {
       TDFXWriteLongMMIO(pTDFX, VIDCHROMAMIN,
			 0x00FF00FF);
       TDFXWriteLongMMIO(pTDFX, VIDCHROMAMAX,
			 0x00FF00FF);
     }
       
     /* Set the src address */
     TDFXWriteLongMMIO (pTDFX, VIDINADDR0, 
		    pTDFX->fbOffset 
		    + pTDFX->stride * (fbarea->box.y1 + pScrn->virtualY) 
		    + fbarea->box.x1);
     
     /* Traverse the clip boxes */
     nbox = REGION_NUM_RECTS(clipBoxes);
     pbox = REGION_RECTS(clipBoxes);
     
     while (nbox--)
     {
       pbox++;
     }
     break;
   }
   return Success;
}

#endif

static void
TDFXDeallocateOffscreenBuffer (ScrnInfoPtr pScrn, int id)
{
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  
  /* There is only one buffer so just deallocate it */
  if (pTDFX->offscreenYUVBuf)
    xf86FreeOffscreenArea (pTDFX->offscreenYUVBuf);
}

static FBAreaPtr
TDFXAllocateOffscreenBuffer (ScrnInfoPtr pScrn, int id, int width, int height)
{
  int myWidth;
  TDFXPtr pTDFX = TDFXPTR(pScrn);

  if (!pTDFX)
    return NULL;

  /* We tweak the width slightly */
  myWidth = width;
  /* We tweak the width slightly */
  myWidth = width;

  /* width is measured in pixels.  The pixels in the YUV image are alway
   * 8 bit YUV data.  But, the xf86 offscreen manager allocates in terms of
   * desktop pixels, so we adjust. */
  myWidth = myWidth / pTDFX->cpp;
  if (width % pTDFX->cpp)
    myWidth++;

  /* If we are putting up a subimage then we need an extra column of data
   * if the source width is odd, instead of checkin the width just always
   * allocate an extra column. */
  /* XXX is this really necessary? */
  myWidth++;

  if (pTDFX->offscreenYUVBuf != NULL &&
      myWidth == pTDFX->offscreenYUVBufWidth &&
      height == pTDFX->offscreenYUVBufHeight)
  {
    /* we already have a buffer, don't do anything. */
    return pTDFX->offscreenYUVBuf;
  }   
  
  /* We have a buffer, but its not the right size so resize it */
  if (pTDFX->offscreenYUVBuf != NULL)
  {
    if (!xf86ResizeOffscreenArea (pTDFX->offscreenYUVBuf, 
				  myWidth,
				  height))
    {
      return (NULL);
    }
  }
  else
  {
    /* Allocate a brand new buffer */
    pTDFX->offscreenYUVBuf = 
      xf86AllocateOffscreenArea (pScrn->pScreen, 
				 myWidth,
				 height, 
				 0,
				 NULL, NULL, NULL);    
  }

  /* Return the buffer */
  pTDFX->offscreenYUVBufWidth = myWidth;
  pTDFX->offscreenYUVBufHeight = height; 
  return (pTDFX->offscreenYUVBuf);
}


static int 
TDFXQueryImageAttributes(
    ScrnInfoPtr pScrn, 
    int id, 
    unsigned short *w, unsigned short *h, 
    int *pitches, int *offsets)
{

    int size;
    TDFXPtr pTDFX = TDFXPTR(pScrn);

    /* The Maximum size for 3dfx YUV planar space 
     * but our temporary buffer has to fit in the pixmap region which
     * is the same width as the desktop and pTDFX->pixmapCacheLines 
     * pixels high.
     */
    if(*w > 1024) *w = 1024;
    if (*w > pTDFX->stride) *w = pTDFX->stride;
    if(*h > pTDFX->pixmapCacheLines) *h = pTDFX->pixmapCacheLines;



    if (offsets) offsets[0] = 0;    
    switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	if (pitches) pitches[0] = *w;
	/* Size of Y plane plus the size of U and V planes */
	size = *w * *h;
	if (offsets) offsets[1] = size;
	size += ((*w >> 1) * (*h >> 1));
	if (offsets) offsets[2] = size;
	size += ((*w >> 1) * (*h >> 1));
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	size = *w << 1;
	if(pitches) pitches[0] = size;
	size *= *h;
	break;
    }
    return size;
}

#endif  /* !XvExtension */
