/*
 * $XConsortium: s3Cursor.c,v 1.5 95/01/23 15:33:57 kaleb Exp $
 * $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3Cursor.c,v 3.19 1995/06/29 13:30:49 dawes Exp $
 * 
 * Copyright 1991 MIPS Computer Systems, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* Header: /home/src/xfree86/server/ddx/xf86/accel/s3/RCS/s3Cursor.c,v 2.4 1993/07/06 10:23:47 jon Exp jon */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3Cursor.c,v 2.5 1993/08/09 06:17:57 jon Exp jon
 */

/*
 * Modified for the Cybervision 64 by Michael Teske 
 */

#include <signal.h>

#define NEED_EVENTS
#include	"amiga.h"

#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"mifillarc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaCV.h"
#include	"migc.h"


static Bool amigaCVRealizeCursor();
static Bool amigaCVUnrealizeCursor();
static void amigaCVSetCursor();
static void amigaCVMoveCursor();
static void amigaCVRecolorCursor();
static void amigaQueryBestSize();
void amigaCVRestoreCursor();

extern miPointerScreenFuncRec   amigaPointerScreenFuncs;

static miPointerSpriteFuncRec amigaCVPointerSpriteFuncs =
{
   amigaCVRealizeCursor,
   amigaCVUnrealizeCursor,
   amigaCVSetCursor,
   amigaCVMoveCursor,
};


 Bool amigaCVBlockCursor;
 Bool amigaCVReloadCursor = FALSE; 



#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)      amigaCursorPtr pCurPriv = GetCursorPrivate(s)

#define MAX_CURS 64

static int amigaCVCursGeneration = -1;


static Bool useSWCursor = FALSE;
static unsigned short xpan; /* for panning */
static unsigned short ypan;
static CursorPtr s3SaveCursors[MAXSCREENS];
int s3hotX, s3hotY;
static int CursX, CursY; /* cursor save position */


/* fuck the optimizer! */
static int xvgar(volatile caddr_t ba, int idx)
{
register int erg;
asm volatile ("movel %1, %%a0;\
                movel %2,%%d1;\
                movel %%a0@(%%d1), %0;" :\
                "=r" (erg):\
                "g" (ba), "d" (idx) :\
                "a0", "d0", "d1");
return erg;
}         





#define VerticalRetraceWait(ba) \
{ \
        while ((xvgar(ba, GREG_INPUT_STATUS1_R) & 0x08) == 0x00) ; \
        while ((xvgar(ba, GREG_INPUT_STATUS1_R) & 0x08) == 0x08) ; \
        while ((xvgar(ba, GREG_INPUT_STATUS1_R) & 0x08) == 0x00) ; \
}



__inline short swap16(unsigned short x)
{
	unsigned short r;

	r = ((x & 0xff) << 8) | ((x & 0xff00) >> 8);
	return r;
}




Bool
amigaCVCursorInit(ScreenPtr pScr)
{
   SetupCursor(pScr);

   s3hotX = 0;
   s3hotY = 0;
   amigaCVBlockCursor = FALSE;
   amigaCVReloadCursor = FALSE;
   

    if (!(miPointerInitialize(pScr, &amigaCVPointerSpriteFuncs,
				   &amigaPointerScreenFuncs, FALSE)))
            return FALSE;

    pScr->QueryBestSize = amigaQueryBestSize;

    amigaCVCursGeneration = serverGeneration;
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    pCurPriv->width = MAX_CURS;
    pCurPriv->height= MAX_CURS;

   return TRUE;
}

static void
amigaQueryBestSize(int class, unsigned short *pwidth, unsigned short *pheight,
    ScreenPtr pScreen)
{
    SetupCursor(pScreen);

    switch (class)
    {
    case CursorShape:
        if (*pwidth > pCurPriv->width)
            *pwidth = pCurPriv->width;
        if (*pheight > pCurPriv->height)
            *pheight = pCurPriv->height;
        if (*pwidth > pScreen->width)
            *pwidth = pScreen->width;
        if (*pheight > pScreen->height)
            *pheight = pScreen->height;
        break;
    default:
        mfbQueryBestSize(class, pwidth, pheight, pScreen);
        break;
    }
}

static Bool
amigaCVRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
   register int i, j;
   unsigned short *pServMsk;
   unsigned short *pServSrc;
   int   index = pScr->myNum;
   pointer *pPriv = &pCurs->bits->devPriv[index];
   int   wsrc, h;
   unsigned short *ram;
   CursorBitsPtr bits = pCurs->bits;

   if (pCurs->bits->refcnt > 1)
      return TRUE;

   ram = (unsigned short *)xalloc(1024);
   *pPriv = (pointer) ram;

   if (!ram)
      return FALSE;

   pServSrc = (unsigned short *)bits->source;
   pServMsk = (unsigned short *)bits->mask;


   h = bits->height;
   if (h > MAX_CURS)
      h = MAX_CURS;

   wsrc = PixmapBytePad(bits->width, 1);	/* words per line */

   for (i = 0; i < MAX_CURS; i++) {
      for (j = 0; j < MAX_CURS / 16; j++) {
	 unsigned short mask, source;

	 if (i < h && j < wsrc / 2) {
	    mask = *pServMsk++;
	    source = *pServSrc++;

#if 0
	    ((char *)&mask)[0] = s3SwapBits[((unsigned char *)&mask)[0]];
	    ((char *)&mask)[1] = s3SwapBits[((unsigned char *)&mask)[1]];

	    ((char *)&source)[0] = s3SwapBits[((unsigned char *)&source)[0]];
	    ((char *)&source)[1] = s3SwapBits[((unsigned char *)&source)[1]];
#endif

	    if (j < MAX_CURS / 8) { /* j < MAX_CURS / 16 implies this */
	       *ram++ = ~mask;
	       *ram++ = source & mask;
	    }


	 } else {
	    *ram++ = 0xffff;
	    *ram++ = 0x0;
	 }
      }
      if (j < wsrc / 2) {
	 pServMsk += (wsrc/2 - j);
	 pServSrc += (wsrc/2 - j);
      }
   }
   return TRUE;
}

static Bool
amigaCVUnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
   pointer priv;

   if (pCurs->bits->refcnt <= 1 &&
       (priv = pCurs->bits->devPriv[pScr->myNum]))
      xfree(priv);
   return TRUE;
}

static void 
amigaCVLoadCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{
   int   index = pScr->myNum;
   int   i, j;
   int   n, bytes_remaining, xpos, ypos, ram_loc;
   unsigned short *ram, *cptr;
   unsigned char tmp;
   int cpos;

   fbFd *inf = amigaInfo(pScr);
   volatile caddr_t vgaBase = (inf->regs);
   volatile caddr_t fb = (inf->fb);
   int depth = inf->info.gd_planes;


   if (!pCurs)
      return;

   /* Wait for vertical retrace */
  VerticalRetraceWait(vgaBase);

   /* turn cursor off */

   WCrt(vgaBase, CRT_ID_HWGC_MODE, 0x00);

   /* move cursor off-screen */

   WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_X_HI, 0x7);
   WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_X_LO,  0xff);
   WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_Y_LO, 0xff);
   WCrt(vgaBase, CRT_ID_HWGC_DSTART_X, 0x3f);
   WCrt(vgaBase, CRT_ID_HWGC_DSTART_Y, 0x3f);
   WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_Y_HI, 0x7);


   /* Load storage location.  */
   cpos = (inf->info.gd_fbsize - 2*1024)/1024;


   WCrt(vgaBase, CRT_ID_HWGC_START_AD_LO, (cpos & 0xff));
   WCrt(vgaBase, CRT_ID_HWGC_START_AD_HI, (cpos >> 8));

   ram = (unsigned short *)pCurs->bits->devPriv[index];

   BLOCK_CURSOR;
   /* s3 stuff */
   WaitIdle();

   WaitQueue(4);



   
   /* write data into framebuffer */
   cptr = (unsigned short *) (fb + cpos *1024);

   VerticalRetraceWait(vgaBase);


   switch (depth)
	{
	case 8:
	   for (i = 0; i < 512; i++)
     		*cptr++ = *ram++;
	   break;
	case 15: case 16:
	   for (i = 0; i < 512; i++)
     		*cptr++ = swap16(*ram++);
	   break;
	case 24: case 32:
	   for (i = 0; i < 512; i++)
		{
     		*cptr++ = swap16(*(ram+1));
		*cptr++ = swap16(*ram);
		ram+=2;
		}
	   break;
        }





   UNBLOCK_CURSOR;

   /* Wait for vertical retrace */
/*   VerticalRetraceWait(vgaBase);*/

   /* position cursor */
   amigaCVMoveCursor(pScr, x, y);

   amigaCVRecolorCursor(pScr, pCurs); 

   /* turn cursor on */
   WCrt(vgaBase, CRT_ID_HWGC_MODE, 0x01);

}

static void
amigaCVSetCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y,
    Bool generateEvent)
{
   int index = pScr->myNum;

   if (!pCurs)
      return;


   s3hotX = pCurs->bits->xhot;
   s3hotY = pCurs->bits->yhot;
   s3SaveCursors[index] = pCurs;

   if (!amigaCVBlockCursor) {
         amigaCVLoadCursor(pScr, pCurs, x, y);
   } else
      amigaCVReloadCursor = TRUE;
}

void
amigaCVRestoreCursor(ScreenPtr pScr)
{
   int index = pScr->myNum;
   

   if (useSWCursor) 
      return;

   amigaCVReloadCursor = FALSE;
   amigaCVLoadCursor(pScr, s3SaveCursors[index], CursX, CursY);
}

#if 0
void
amigaCVRepositionCursor(ScreenPtr pScr)
{
   int x, y;

   miPointerPosition(&x, &y);
   /* Wait for vertical retrace */
   VerticalRetraceWait(vgaBase);
   amigaCVMoveCursor(pScr, x, y);
}
#endif




void
amigaCVSetPanning2(fbFd *inf, unsigned short xoff, unsigned short yoff)
{
        volatile caddr_t ba = inf->regs;
        int depth = inf->info.gd_planes;

        unsigned long off;

        xpan = xoff;
        ypan = yoff;
        if (depth > 8 && depth <= 16) xoff *= 2;
        else if (depth > 16) xoff *= 4;

        xvgar(ba, ACT_ADDRESS_RESET);
#if 0
        WAttr(ba, ACT_ID_HOR_PEL_PANNING, (unsigned char)((xoff << 1) & 0x07));

        /* have the color lookup function normally again */
        vgaw(ba,  ACT_ADDRESS_W, 0x20);
#endif
        if (depth == 8)
                off = ((yoff * amigaVirtualWidth)/ 4) + (xoff >> 2);
        else if (depth == 16)
                off = ((yoff * amigaVirtualWidth * 2)/ 4) + (xoff >> 2);
        else
                off = ((yoff * amigaVirtualWidth * 4)/ 4) + (xoff >> 2);

        WCrt(ba, CRT_ID_START_ADDR_LOW, ((unsigned char)off));
        off >>= 8;
        WCrt(ba, CRT_ID_START_ADDR_HIGH, ((unsigned char)off));
        off >>= 8;
        WCrt(ba, CRT_ID_EXT_SYS_CNTL_3, (off & 0x0f));

}        





static void
amigaCVMoveCursor(ScreenPtr pScr, int x, int y)
{
   unsigned char xoff, yoff;
   fbFd *inf = amigaInfo(pScr);
   volatile caddr_t vgaBase = (inf->regs);


   CursX = x;
   CursY = y;
   if (amigaCVBlockCursor)
      return;



  if (x < xpan)
       amigaCVSetPanning2(inf, x, ypan);
  if (x >= (xpan + amigaRealWidth))
           amigaCVSetPanning2(inf, (1 + x - amigaRealWidth), ypan);
  if (y < ypan)
           amigaCVSetPanning2(inf,  xpan, y);
  if (y >= (ypan + amigaRealHeight))
           amigaCVSetPanning2(inf,  xpan, (1 + y - amigaRealHeight));
  x -= xpan;
  y -= ypan; 


   x -= s3hotX;
   y -= s3hotY;



#if 0
   if (s3Bpp > 2)
      x &= ~1;
#endif
   

   /*
    * Make these even when used.  There is a bug/feature on at least
    * some chipsets that causes a "shadow" of the cursor in interlaced
    * mode.  Making this even seems to have no visible effect, so just
    * do it for the generic case.
    */
   if (x < 0) {
     xoff = ((-x) & 0xFE);
     x = 0;
   } else {
     xoff = 0;
   }

   if (y < 0) {
      yoff = ((-y) & 0xFE);
      y = 0;
   } else {
      yoff = 0;
   }

   WaitIdle();

   /* This is the recomended order to move the cursor */

        WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_X_HI, (x >> 8));
        WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_X_LO, (x & 0xff));

                
        WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_Y_LO, (y & 0xff));
        WCrt(vgaBase, CRT_ID_HWGC_DSTART_X, xoff);
        WCrt(vgaBase, CRT_ID_HWGC_DSTART_Y, yoff);
        WCrt(vgaBase, CRT_ID_HWGC_ORIGIN_Y_HI, (y >> 8));

}

void
amigaCVRenewCursorColor(ScreenPtr pScr)
{

   if (s3SaveCursors[pScr->myNum])
      amigaCVRecolorCursor(pScr, s3SaveCursors[pScr->myNum], TRUE);
}

static void
amigaCVRecolorCursor(ScreenPtr pScr, CursorPtr pCurs, Bool displayed)
{
   ColormapPtr pmap;
   unsigned short packedcolfg, packedcolbg;
   xColorItem sourceColor, maskColor;

   unsigned char test;
   fbFd *inf = amigaInfo(pScr);
   volatile caddr_t vgaBase = (inf->regs);
   int depth = inf->info.gd_planes;
   volatile caddr_t hwc;
#if 1
   /* Cheat a bit here XXX */

                /* reset colour stack */
                /*test = RCrt(vgaBase, CRT_ID_HWGC_MODE);*/
		vgaw(vgaBase, CRT_ADDRESS, CRT_ID_HWGC_MODE);
		test = xvgar(vgaBase, CRT_ADDRESS_R);

                /*asm volatile("nop");*/
                switch (depth) {

                        case 8: 
				if (amigaFlipPixels) {
	                                /* info->cmap.green[1] */
        	                        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, 0);
                	                hwc = vgaBase + CRT_ADDRESS_W;
					*hwc = 0;
				} else {
	                                /* info->cmap.green[1] */
        	                        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, 1);
                	                hwc = vgaBase + CRT_ADDRESS_W;
					*hwc = 1;
				}
				break; 
			case 15:			
			case 16:
                                /* info->cmap.green[1] */
                                WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, 0);
                                hwc = vgaBase + CRT_ADDRESS_W;
				*hwc = 0;
				break; 
			case 32: case 24:
				WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, 0);
                                hwc = vgaBase + CRT_ADDRESS_W;
				*hwc = 0;
				*hwc = 0;
                }


                /*test = RCrt(vgaBase, CRT_ID_HWGC_MODE);*/
                /*asm volatile("nop");*/
		vgaw(vgaBase, CRT_ADDRESS, CRT_ID_HWGC_MODE);
		test = xvgar(vgaBase, CRT_ADDRESS_R);


                switch (depth) {
                        case 8:
				if (amigaFlipPixels) {
	                                WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, 1);
					hwc = vgaBase + CRT_ADDRESS_W; 
					*hwc = 1;
				} else {
	                                WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, 0);
					hwc = vgaBase + CRT_ADDRESS_W; 
					*hwc = 0;
				}
                                break;
                        case 15: case 16:
                                WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, 0xff);
                                hwc = vgaBase + CRT_ADDRESS_W;
				*hwc = 0xff;
				break;
                        case 32: case 24:
				WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, 0xff);
                                hwc = vgaBase + CRT_ADDRESS_W;
				*hwc = 0xff;
				*hwc = 0xff;
                }

#else

   switch (depth) {
     case 8:
	(*pScr->ListInstalledColormaps)(pScr, &pmap);
	sourceColor.red = pCurs->foreRed;
	sourceColor.green = pCurs->foreGreen;
	sourceColor.blue = pCurs->foreBlue;
	FakeAllocColor(pmap, &sourceColor);
	maskColor.red = pCurs->backRed;
	maskColor.green = pCurs->backGreen;
	maskColor.blue = pCurs->backBlue;
	FakeAllocColor(pmap, &maskColor);
	FakeFreeColor(pmap, sourceColor.pixel);
	FakeFreeColor(pmap, maskColor.pixel);


        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
        asm volatile("nop");
	WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, sourceColor.pixel);
	WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, sourceColor.pixel);
        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
	asm volatile("nop");
	WCrt(vgaBase, CRT_ID_HWGC_BG_STACK,maskColor.pixel);
	WCrt(vgaBase, CRT_ID_HWGC_BG_STACK,maskColor.pixel);

	break;
     case 16:
#if 0
        if (s3InfoRec.depth == 15) {
	   packedcolfg = ((pCurs->foreRed   & 0xf800) >>  1) 
	               | ((pCurs->foreGreen & 0xf800) >>  6)
		       | ((pCurs->foreBlue  & 0xf800) >> 11);
	   packedcolbg = ((pCurs->backRed   & 0xf800) >>  1) 
	               | ((pCurs->backGreen & 0xf800) >>  6)
		       | ((pCurs->backBlue  & 0xf800) >> 11);
	} else 
#endif
	{
	   packedcolfg = ((pCurs->foreRed   & 0xf800) >>  0) 
	               | ((pCurs->foreGreen & 0xfc00) >>  5)
		       | ((pCurs->foreBlue  & 0xf800) >> 11);
	   packedcolbg = ((pCurs->backRed   & 0xf800) >>  0) 
	               | ((pCurs->backGreen & 0xfc00) >>  5)
		       | ((pCurs->backBlue  & 0xf800) >> 11);
	}


        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
        asm volatile("nop");
        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, packedcolfg);
        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, packedcolfg>>8);
        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
        asm volatile("nop");
        WCrt(vgaBase, CRT_ID_HWGC_BG_STACK,packedcolbg);
        WCrt(vgaBase, CRT_ID_HWGC_BG_STACK,packedcolbg>>8);

     break;
     case 32:

        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
        asm volatile("nop");
        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, pCurs->foreBlue >>8);
        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, pCurs->foreGreen>>8);
        WCrt(vgaBase, CRT_ID_HWGC_FG_STACK, pCurs->foreRed  >>8);

        test = RCrt(vgaBase, CRT_ID_HWGC_MODE);
        asm volatile("nop");
	WCrt(vgaBase, CRT_ID_HWGC_BG_STACK,  pCurs->backBlue >>8);
	WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, pCurs->backGreen>>8);
	WCrt(vgaBase, CRT_ID_HWGC_BG_STACK, pCurs->backRed>>8);

     break;
   }

#endif

}

#if 0
void
s3WarpCursor(ScreenPtr pScr, int x, int y)
{
   if (xf86VTSema) {
      /* Wait for vertical retrace */
      VerticalRetraceWait();
   }
   miPointerWarpCursor(pScr, x, y);
   xf86Info.currentScreen = pScr;
}

#endif
