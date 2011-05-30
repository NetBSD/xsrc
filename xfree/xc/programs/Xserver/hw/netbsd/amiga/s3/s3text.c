/* $XConsortium: s3text.c,v 1.2 94/10/12 20:07:37 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3text.c,v 3.9 1995/01/28 17:02:30 dawes Exp $ */
/*
 * Copyright 1992 by Kevin E. Martin, Chapel Hill, North Carolina.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kevin E. Martin not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Kevin E. Martin makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3text.c,v 2.3 1993/08/09 06:17:57 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#include	"amiga.h"
#include        "gcstruct.h"
#include        "fontstruct.h"
#include        "dixfontstr.h"
#include	"cfb.h"
#include	"amigaCV.h"

extern unsigned char s3SwapBits[256];
extern short s3alu[];

void s3SimpleStipple(
    int x, int y,
    int width, int height,
    unsigned char *pb,
    int pwidth,
    fbFd *inf)
{
    	    volatile caddr_t vgaBase = (inf->regs);  

	      __dolog ("Entering s3SimpleStipple\n");
 
	    WaitQueue(4);PCI_HACK();
    
	    S3_OUTW (CUR_X, (short) x);
	    S3_OUTW (CUR_Y, (short) y);
	    S3_OUTW (MAJ_AXIS_PCNT, (short) (width - 1));
	    S3_OUTW (MULTIFUNC_CNTL, MIN_AXIS_PCNT | (height-1));   
	    WaitIdle();
	    S3_OUTW (CMD, CMD_RECT | PCDATA | _16BIT | INC_Y | INC_X |
	     DRAW | PLANAR | WRTDATA | BYTSEQ);

	    
	    { /* The stipple code */
#define SWPBIT(s) (s3SwapBits[pb[(s)]])

	       int i, h, pix;
	       unsigned short getbuf;

	       for (h = 0; h < height; h++) {
		  pix = 0;
	       
		  for (i = 0; i < width; i += 16) {
#if 0
		     getbuf = (getbuf << 8) | SWPBIT (pix++);
		     getbuf = (getbuf << 8) | SWPBIT (pix++);
#else
		     getbuf = pb [pix + 1] << 8;
		     getbuf |= pb [pix]; 
		     pix += 2;
#endif
		     S3_OUTW (PIX_TRANS, getbuf);		  
		  }
		  pb += pwidth;
	       }
	    }

__dolog ("Leaving s3SimpleStipple\n");

}

/*
 * The guts of this should possibly be tidied up and put in s3im.c.
 * The generic Stipple functions in s3im.c have quite a large unnecessary
 * overhead for bitmap copies in the case that we are doing an exacy copy
 * with no tiling and starting from (0,0) in the source bitmap. - Jon.
 */
__inline__ static void
s3PolyGlyphBlt(
    DrawablePtr pDrawable,
    GC 		*pGC,
    int 	x, int y,
    unsigned int nglyph,
    CharInfoPtr *ppci,		/* array of character info */
    unsigned char *pglyphBase)	/* start of array of glyphs */
{
    int width, height;
    int nbyLine;			/* bytes per line of padded pixmap */
    FontPtr pfont;
    register int i;
    register int j;
    unsigned char *pbits;		/* buffer for PutImage */
    register unsigned char *pb;		/* temp pointer into buffer */
    register CharInfoPtr pci;		/* currect char info */
    register unsigned char *pglyph;	/* pointer bits in glyph */
    int gWidth, gHeight;		/* width and height of glyph */
    register int nbyGlyphWidth;		/* bytes per scanline of glyph */
    int nbyPadGlyph;			/* server padded line of glyph */
    fbFd *inf = amigaInfo(pGC->pScreen);
    

__dolog ("Entering s3PolyGlyphBlt\n");

    pfont = pGC->font;
    width = FONTMAXBOUNDS(pfont,rightSideBearing) - 
	    FONTMINBOUNDS(pfont,leftSideBearing);
    height = FONTMAXBOUNDS(pfont,ascent) +
	     FONTMAXBOUNDS(pfont,descent);

    nbyLine = PixmapBytePad(width, 1);
    pbits = (unsigned char *)ALLOCATE_LOCAL(height*nbyLine);
    if (!pbits)
    {
        return;
    }
    while(nglyph--)
    {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS(pglyphBase, pci);
	gWidth = GLYPHWIDTHPIXELS(pci);
	gHeight = GLYPHHEIGHTPIXELS(pci);
	if (gWidth && gHeight)
	{
	    nbyGlyphWidth = GLYPHWIDTHBYTESPADDED(pci);
	    nbyPadGlyph = PixmapBytePad(gWidth, 1);

	    if (nbyGlyphWidth == nbyPadGlyph
#if GLYPHPADBYTES != 4
	        && (((int) pglyph) & 3) == 0
#endif
		)
	    {
		pb = pglyph;
	    }
	    else
	    {
		for (i=0, pb = pbits; i<gHeight; i++, pb = pbits+(i*nbyPadGlyph))
		    for (j = 0; j < nbyGlyphWidth; j++)
			*pb++ = *pglyph++;
		pb = pbits;
	    }

	    s3SimpleStipple(x + pci->metrics.leftSideBearing,
			    y - pci->metrics.ascent,
			    gWidth, gHeight, pb, nbyPadGlyph, inf);
	}

	x += pci->metrics.characterWidth;
    }
    DEALLOCATE_LOCAL(pbits);
    __dolog ("Leaving s3PolyGlyphBlt\n");
}

void s3FontStipple(
    int x, int y,
    int width, int height,
    unsigned char *pb,
    int pwidth,
    Pixel id)
{
   fbFd *inf = amigaInfo(amigaCVsavepScreen);
   volatile caddr_t vgaBase =  (inf->regs);

   __dolog ("Entering s3Fontstipple\n");
   BLOCK_CURSOR;
   WaitQueue16_32(1, 2);PCI_HACK();
   S3_OUTW32 (WRT_MASK, id);
   WaitQueue16_32(5, 7);PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | s3alu[GXcopy]);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | s3alu[GXcopy]);
   S3_OUTW32 (FRGD_COLOR, ~0);
   S3_OUTW32 (BKGD_COLOR, 0);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPPC | COLCMPOP_F);

   s3SimpleStipple(x, y, width, height, pb, pwidth, inf);
   WaitQueue(3);PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   UNBLOCK_CURSOR;
   __dolog ("Leaving s3Fontstipple\n");

}

int
s3NoCPolyText(
     DrawablePtr pDraw,
     GCPtr pGC,
     int   x,
     int   y,
     int   count,
     char *chars,
     Bool is8bit)
{
   int   i;
   BoxPtr pBox;
   int   numRects;
   RegionPtr pRegion;
   int   yBand;
   int   maxAscent, maxDescent;
   int   minLeftBearing;
   FontPtr pfont = pGC->font;
   int ret_x;
   unsigned long n;
   CharInfoPtr *charinfo;
   fbFd *inf = amigaInfo(pGC->pScreen);
   volatile caddr_t vgaBase =  (inf->regs);

__dolog ("Entering s3NoCPolyText\n");

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;

    if (is8bit)
      GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    else
      GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
		(FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
		&n, charinfo);

   /*
    * If miPolyText8() is to be believed, the returned new X value is
    * completely independent of what happens during rendering.
    */
   ret_x = x;
   for (i = 0; i < n; i++) {
      ret_x += charinfo[i]->metrics.characterWidth;
   }

   if (n == 0) {
      DEALLOCATE_LOCAL(charinfo);
      return ret_x;
   }
   
   x += pDraw->x;
   y += pDraw->y;
   maxAscent = FONTMAXBOUNDS(pfont, ascent);
   maxDescent = FONTMAXBOUNDS(pfont, descent);
   minLeftBearing = FONTMINBOUNDS(pfont, leftSideBearing);
   pRegion = cfbGetCompositeClip(pGC);

   pBox = REGION_RECTS(pRegion);
   numRects = REGION_NUM_RECTS(pRegion);
   while (numRects && pBox->y2 <= y - maxAscent) {
      ++pBox;
      --numRects;
   }
   if (!numRects || pBox->y1 >= y + maxDescent) {
      DEALLOCATE_LOCAL(charinfo);
      return ret_x;
   }
   yBand = pBox->y1;
   while (numRects && pBox->y1 == yBand && pBox->x2 <= x + minLeftBearing) {
      ++pBox;
      --numRects;
   }
   if (!numRects) {
      DEALLOCATE_LOCAL(charinfo);   
      return ret_x;
   }
   BLOCK_CURSOR;
   WaitQueue16_32(5,7);PCI_HACK();
   S3_OUTW32 (WRT_MASK, pGC->planemask);   
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_DST);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPPC | COLCMPOP_F);
   S3_OUTW32 (FRGD_COLOR,  pGC->fgPixel);

   for (; --numRects >= 0; ++pBox) {
      WaitQueue(4);PCI_HACK();
      S3_OUTW (MULTIFUNC_CNTL, SCISSORS_L | (short)pBox->x1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | (short)pBox->y1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (short)(pBox->x2 - 1));
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | (short)(pBox->y2 - 1));
      s3PolyGlyphBlt(pDraw, pGC, x, y, (unsigned int)n, charinfo,
						FONTGLYPHS(pGC->font));

   }

   WaitQueue(7);PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (amigaVirtualWidth-1));
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | 0xfff);
   UNBLOCK_CURSOR;
   DEALLOCATE_LOCAL(charinfo);
   __dolog ("Leaving s3NoCPolyText\n");

   return ret_x;

}


/*
 * Imagetext is all in one function to avoid calling GetGlyphs() twice.
 * - Jon.
 */
 
int
s3NoCImageText(
     DrawablePtr pDraw,
     GCPtr pGC,
     int   x,
     int   y,
     int   count,
     char *chars,
     Bool is8bit)
{
   ExtentInfoRec info;		/* used by QueryGlyphExtents() */
   XID   gcvals[3];
   int   oldAlu, oldFS;
   unsigned long oldFG;
   xRectangle backrect;
   CharInfoPtr *charinfo;
   unsigned long n;
   fbFd *inf = amigaInfo(pGC->pScreen);
   volatile caddr_t vgaBase =  (inf->regs);
__dolog ("Entering s3NoCImageText\n");


   if (!(charinfo = (CharInfoPtr *) ALLOCATE_LOCAL(count * sizeof(CharInfoPtr))))
      return 0;


    if (is8bit)
      GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    else
      GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
		(FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
		&n, charinfo);


   QueryGlyphExtents(pGC->font, charinfo, n, &info);

   if (info.overallWidth >= 0) {
      backrect.x = x;
      backrect.width = info.overallWidth;
   } else {
      backrect.x = x + info.overallWidth;
      backrect.width = -info.overallWidth;
   }
   backrect.y = y - FONTASCENT(pGC->font);
   backrect.height = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

   oldAlu = pGC->alu;
   oldFG = pGC->fgPixel;
   oldFS = pGC->fillStyle;

 /* fill in the background */
   gcvals[0] = GXcopy;
   gcvals[1] = pGC->bgPixel;
   gcvals[2] = FillSolid;
   DoChangeGC(pGC, GCFunction | GCForeground | GCFillStyle, gcvals, 0);
   ValidateGC(pDraw, pGC);
   (*pGC->ops->PolyFillRect) (pDraw, pGC, 1, &backrect);

 /* put down the glyphs */
   gcvals[0] = oldFG;
   DoChangeGC(pGC, GCForeground, gcvals, 0);
   ValidateGC(pDraw, pGC);

   /* begin the font blitting */
   {
      BoxPtr pBox;
      int   numRects;
      RegionPtr pRegion;
      int   maxAscent, maxDescent;
      int   yBand;
      int   minLeftBearing;
      FontPtr pfont = pGC->font;
      
   x += pDraw->x;
   y += pDraw->y;
   maxAscent = FONTMAXBOUNDS(pfont, ascent);
   maxDescent = FONTMAXBOUNDS(pfont, descent);
   minLeftBearing = FONTMINBOUNDS(pfont, leftSideBearing);
   pRegion = cfbGetCompositeClip(pGC);

   pBox = REGION_RECTS(pRegion);
   numRects = REGION_NUM_RECTS(pRegion);
   while (numRects && pBox->y2 <= y - maxAscent) {
      ++pBox;
      --numRects;
   }
   if (!numRects || pBox->y1 >= y + maxDescent) {
      DEALLOCATE_LOCAL(charinfo);
      return 0;
   }
   yBand = pBox->y1;
   while (numRects && pBox->y1 == yBand && pBox->x2 <= x + minLeftBearing) {
      ++pBox;
      --numRects;
   }
   if (!numRects) {
      DEALLOCATE_LOCAL(charinfo);   
      return 0;
   }
   BLOCK_CURSOR;
   WaitQueue16_32(5,7);PCI_HACK();
   S3_OUTW32 (WRT_MASK, pGC->planemask);
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_DST);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPPC | COLCMPOP_F);
   S3_OUTW32 (FRGD_COLOR,  pGC->fgPixel);

   for (; --numRects >= 0; ++pBox) {
      WaitQueue(4);PCI_HACK();
      S3_OUTW (MULTIFUNC_CNTL, SCISSORS_L | (short)pBox->x1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | (short)pBox->y1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (short)(pBox->x2 - 1));
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | (short)(pBox->y2 - 1));
      s3PolyGlyphBlt(pDraw, pGC, x, y, (unsigned int)n, charinfo,
						FONTGLYPHS(pGC->font));

   }

   WaitQueue(7);PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (amigaVirtualWidth-1));
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | 0xfff);
   UNBLOCK_CURSOR;
   }
   
 /* put all the toys away when done playing */
   gcvals[0] = oldAlu;
   gcvals[1] = oldFG;
   gcvals[2] = oldFS;
   DoChangeGC(pGC, GCFunction | GCForeground | GCFillStyle, gcvals, 0);
   DEALLOCATE_LOCAL(charinfo);
   __dolog ("Entering s3NoCImageText\n");

   return 0;
}
