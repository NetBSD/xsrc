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

#ifndef _XGI_VIDEO_H_
#define _XGI_VIDEO_H_

#define PIXEL_FMT_YV12 FOURCC_YV12  /* 0x32315659 */
#define PIXEL_FMT_UYVY FOURCC_UYVY  /* 0x59565955 */
#define PIXEL_FMT_YUY2 FOURCC_YUY2  /* 0x32595559 */
#define PIXEL_FMT_RGB5 0x35315652
#define PIXEL_FMT_RGB6 0x36315652
#define PIXEL_FMT_YVYU 0x55595659   
#define PIXEL_FMT_NV12 0x3231564e   
#define PIXEL_FMT_NV21 0x3132564e   

#define IMAGE_MIN_WIDTH         32
#define IMAGE_MIN_HEIGHT        24
#define IMAGE_MAX_WIDTH         1920
#define IMAGE_MAX_HEIGHT        1080

#define DISPMODE_SINGLE1 0x1
#define DISPMODE_SINGLE2 0x2
#define DISPMODE_MIRROR  0x4

#define OFF_DELAY       200  /* milliseconds */
#define FREE_DELAY      60000

#define OFF_TIMER              0x01
#define FREE_TIMER             0x02
#define CLIENT_VIDEO_ON        0x04
#define CLIENT_CAPTURE_ON      0x08
#define TIMER_MASK      (OFF_TIMER | FREE_TIMER)

#define WEAVE    0
#define BOB_EVEN 1
#define BOB_ODD  2

typedef struct {
#ifdef XGI_USE_XAA
    FBLinearPtr     linear;
    FBAreaPtr       fbAreaPtr;
#endif

    int             fbSize;
    CARD32          bufAddr[2];

    unsigned char   currentBuf;

    short                   drw_x, drw_y, drw_w, drw_h;
    short                   src_x, src_y, src_w, src_h;
    int                             id;
    short                   srcPitch, height;

    INT32   brightness;
    INT32   contrast;
    INT32   saturation;
    INT32   hue;

    RegionRec       clip;
    CARD32          colorKey;

    CARD32          videoStatus;
    Time            offTime;
    Time            freeTime;
	
	short	 linebufMergeLimit;

    CARD32                  displayMode;

	Bool            grabbedByV4L;  /*V4L stuff*/
	int             pitch;
	int             offset;

	//:::: for capture
	INT32 mode;

        int  fd;
        int  usecount;
        char devname[16];
	int encoding;
	int videoflags;
        int update_flags;
	Bool double_buffer;
	int sec_offset;
	//~::::	
} XGIPortPrivRec, *XGIPortPrivPtr;

#define GET_PORT_PRIVATE(pScrn) \
   (XGIPortPrivPtr)((XGIPTR(pScrn))->adaptor->pPortPrivates[0].ptr)

//:::: for capture
struct xgi_framebuf
{
        // Hardware Information
        unsigned long   fbuf;
        unsigned long   size;

        // Video Information
        unsigned long   fboffset[4];
};

#define XGI_IOC_BASE    192
#define XGIIOC_G_FBUF           _IOWR('v', XGI_IOC_BASE+5, struct xgi_framebuf)
//~::::

#endif /* _XGI_VIDEO_H_ */

