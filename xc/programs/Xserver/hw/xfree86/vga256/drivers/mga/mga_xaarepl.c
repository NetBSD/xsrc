/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_xaarepl.c,v 1.1.2.5 1999/07/26 06:54:49 hohndel Exp $ */


#define PSZ 8

#include "vga256.h"
#include "xf86.h"
#include "xf86xaa.h"

#define MAX_BLIT_PIXELS 	 0x40000

#ifndef GNUC
#define __inline__ /**/
#endif

extern void MGAStormSync();

static __inline__ CARD32* MoveDWORDS(dest, src, dwords)
   register CARD32* dest;
   register CARD32* src;
   register int dwords;
{
     while(dwords & ~0x03) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        src += 4;
        dest += 4;
        dwords -= 4;
     }  
     switch(dwords) {
        case 0: return(dest);
        case 1: dest[0] = src[0];
                return(dest + 1);
        case 2: dest[0] = src[0];
                dest[1] = src[1];
                return(dest + 2);
        case 3: dest[0] = src[0];
                dest[1] = src[1];
                dest[2] = src[2];
                return(dest + 3);
    }
    return dest;
}


void MGAWriteBitmap(x, y, w, h, src, srcwidth, srcx, srcy, 
			bg, fg, rop, planemask)
    int x, y, w, h;
    unsigned char *src;
    int srcwidth;
    int srcx, srcy;
    int bg, fg;
    int rop;
    unsigned int planemask;
{
    register unsigned char *srcp;
    int dwords, skipleft, maxlines;
    register CARD32 *destptr = 
			(CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;
    CARD32 *maxptr;

    xf86AccelInfoRec.SetupForCPUToScreenColorExpand(bg, fg, rop, planemask);
    
    srcp = (srcwidth * srcy) + (srcx >> 3) + src; 
    srcx &= 0x07;
    if(skipleft = (int)srcp & 0x03) {
        skipleft = (skipleft << 3) + srcx;
        maxlines = MAX_BLIT_PIXELS / ((w + skipleft + 31) & ~31);
        if(maxlines < h) {
            int newmax = MAX_BLIT_PIXELS / ((w + srcx +31) & ~31);
            if(newmax >= h) { /* do byte alignment to avoid the split */
                skipleft = srcx;
                maxlines = newmax;
            } else srcp = (unsigned char*)((long)srcp & ~0x03L);
        } else srcp = (unsigned char*)((long)srcp & ~0x03L);
    } else {
        skipleft = srcx; 
        maxlines = MAX_BLIT_PIXELS / ((w + skipleft + 31) & ~31);
    }
    
    w += skipleft; 
    x -= skipleft;
        
    dwords = (w + 31) >> 5;
    maxptr = destptr + (xf86AccelInfoRec.CPUToScreenColorExpandRange >> 2) 
    		- dwords;
    
    while(h > maxlines) {
	int numlines = maxlines;
       	xf86AccelInfoRec.SubsequentCPUToScreenColorExpand(
		x, y, w, maxlines, skipleft);  
    	while(numlines--) {
            destptr = MoveDWORDS(destptr, (CARD32*)srcp, dwords);
	    srcp += srcwidth;
	    if(destptr > maxptr)	
	    	destptr = (CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;
        }   
	h -= maxlines;
	y += maxlines; 
    }

    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand(x, y, w, h, skipleft);  

    while(h--) {
        destptr = MoveDWORDS(destptr, (CARD32*)srcp, dwords);
	srcp += srcwidth;
	if(destptr > maxptr)	
	    destptr = (CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;
    }    

    MGAStormSync();
}


static __inline__ CARD32* SetDWORDS(dest, value, dwords)
   register CARD32* dest;
   register CARD32 value;
   register int dwords;
{
    while(dwords & ~0x03) {
	dest[0] = dest[1] = dest[2] = dest[3] = value;
	dest += 4;
	dwords -= 4;
    }	
    switch(dwords) {
	case 0: return(dest);
	case 1: dest[0] = value; 
                return(dest + 1);
	case 2: dest[0] = dest[1] = value; 
                return(dest + 2);
	case 3: dest[0] = dest[1] = dest[2] = value; 
                return(dest + 3);
    }
    return dest;
}

static unsigned int ShiftMasks[32] = {
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
  0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
  0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 
  0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
  0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
  0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
  0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF
};

struct CARD32_unaligned {
	CARD32	unal32 
#if defined(__alpha__) && defined(__GNUC__)
	__attribute__((packed));
#endif
	;
};

typedef struct CARD32_unaligned CARD32u;

static void
MGAFillStippledCPUToScreenColorExpand(x, y, dwords, h, src, srcwidth,
stipplewidth, stippleheight, srcx, srcy)
    int x, y, dwords, h;
    unsigned char *src;
    int srcwidth;
    int stipplewidth, stippleheight;
    int srcx, srcy;
{
    unsigned char *srcp = (srcwidth * srcy) + src;
    register CARD32 *destptr = 
    			(CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;
    CARD32 *maxptr;

    maxptr = destptr + (xf86AccelInfoRec.CPUToScreenColorExpandRange >> 2)
    		- dwords;

    if(!((stipplewidth > 32) || (stipplewidth & (stipplewidth - 1)))) { 
    	CARD32 pattern;
    	register unsigned char* kludge = (unsigned char*)(&pattern);

	while(h--) {
	   switch(stipplewidth) {
		case 32:
	   	    pattern = ((CARD32u*)srcp)->unal32;
		    break;
	      	case 16:
		    kludge[0] = kludge[2] = srcp[0];
		    kludge[1] = kludge[3] = srcp[1];
		    break;
	      	case 8:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = srcp[0];
		    break;
	      	case 4:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x0F);
		    pattern |= (pattern << 4);
		    break;
		case 2:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x03);
		    pattern |= (pattern << 2);
		    pattern |= (pattern << 4);
		    break;
		default:	/* case 1: */
		    if(srcp[0] & 0x01) pattern = 0xffffffff;
		    else pattern = 0x00000000;
		    break;
	   }

	   if(srcx) 
		pattern = (pattern >> srcx) | (pattern << (32 - srcx));         
		 

   	   destptr = SetDWORDS(destptr, pattern, dwords); 
	   if(destptr > maxptr)	
	    	destptr = (CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else if(stipplewidth < 32) {
        int count, width, offset; 
	register CARD32 pattern;

	while(h--) {
	   width = stipplewidth;
	   pattern = ((CARD32u*)srcp)->unal32 & ShiftMasks[width];  
	   while(!(width & ~15)) {
		pattern |= (pattern << width);
		width <<= 1;	
	   }
	   pattern |= (pattern << width);
 
	   offset = srcx;

	   count = dwords;
	   while(count--) {
	   	*(destptr++) = (pattern >> offset) | 
				(pattern << (width - offset));
		offset += 32;
		while(offset >= width) 
		    offset -= width;
	   }

	   if(destptr > maxptr)	
	    	destptr = (CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else {
	register CARD32u* scratch;
	int shift, offset, scratch2, count;

	while(h--) {
	   count = dwords;
	   offset = srcx;
	   	   	
	   while(count--) {
	   	shift = stipplewidth - offset;
		scratch = (CARD32u*)(srcp + (offset >> 3));
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      *(destptr++) = (scratch->unal32 >> scratch2) |
			(scratch[1].unal32 << (32 - scratch2));
		   } else 
		       *(destptr++) = scratch->unal32;
		} else {
		    *(destptr++) = (((CARD32u*)srcp)->unal32 << shift) |
			((scratch->unal32 >> scratch2) & ShiftMasks[shift]);
		}
		offset += 32;
		while(offset >= stipplewidth) 
		    offset -= stipplewidth;
	   }	

	   if(destptr > maxptr)	
	    	destptr = (CARD32*)xf86AccelInfoRec.CPUToScreenColorExpandBase;

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    }

}


void
MGAFillRectStippled(pDrawable, pGC, nBoxInit, pBoxInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int nBoxInit;		/* number of rectangles to fill */
    BoxPtr pBoxInit;		/* Pointer to first rectangle to fill */
{
    PixmapPtr pPixmap;		/* Pixmap of the area to draw */
    int rectWidth;		/* Width of the rect to be drawn */
    int rectHeight;		/* Height of the rect to be drawn */
    BoxPtr pBox;		/* current rectangle to fill */
    int nBox;			/* Number of rectangles to fill */
    int xoffset, yoffset;
    Bool AlreadySetup = FALSE;
    int maxlines, y;

    pPixmap = pGC->stipple;

    for (nBox = nBoxInit, pBox = pBoxInit; nBox > 0; nBox--, pBox++) {

	rectWidth = pBox->x2 - pBox->x1;
	rectHeight = pBox->y2 - pBox->y1;

	if ((rectWidth > 0) && (rectHeight > 0)) {
	    if(!AlreadySetup) {
    		xf86AccelInfoRec.SetupForCPUToScreenColorExpand(
	    		(pGC->fillStyle == FillStippled) ? -1 : pGC->bgPixel, 
			pGC->fgPixel, pGC->alu, pGC->planemask);
		AlreadySetup = TRUE;
	    }
	    y = pBox->y1;

	    xoffset = (pBox->x1 - (pGC->patOrg.x + pDrawable->x))
	        % pPixmap->drawable.width;
	    if (xoffset < 0)
	        xoffset += pPixmap->drawable.width;
	    yoffset = (y - (pGC->patOrg.y + pDrawable->y))
	        % pPixmap->drawable.height;
	    if (yoffset < 0)
	        yoffset += pPixmap->drawable.height;

	    maxlines = MAX_BLIT_PIXELS / ((rectWidth + 31) & ~31);

	    while(rectHeight > maxlines) {
    		xf86AccelInfoRec.SubsequentCPUToScreenColorExpand(
			pBox->x1, y, rectWidth, maxlines, 0);  	
		MGAFillStippledCPUToScreenColorExpand(
	           pBox->x1, y, (rectWidth + 31) >> 5, maxlines,
	           pPixmap->devPrivate.ptr, pPixmap->devKind,
	           pPixmap->drawable.width, pPixmap->drawable.height,
	           xoffset, yoffset);
		rectHeight -= maxlines;
		y += maxlines;
	    }

    	    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand(
			pBox->x1, y, rectWidth, rectHeight, 0);  	
	    MGAFillStippledCPUToScreenColorExpand(
	           pBox->x1, y, (rectWidth + 31) >> 5, rectHeight,
	           pPixmap->devPrivate.ptr, pPixmap->devKind,
	           pPixmap->drawable.width, pPixmap->drawable.height,
	           xoffset, yoffset);
	}
    }	/* end for loop through each rectangle to draw */

    MGAStormSync();
}
