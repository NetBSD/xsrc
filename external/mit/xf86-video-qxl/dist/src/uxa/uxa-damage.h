void
uxa_damage_composite (RegionPtr  region,
		      CARD8      op,
		      PicturePtr pSrc,
		      PicturePtr pMask,
		      PicturePtr pDst,
		      INT16      xSrc,
		      INT16      ySrc,
		      INT16      xMask,
		      INT16      yMask,
		      INT16      xDst,
		      INT16      yDst,
		      CARD16     width,
		      CARD16     height);
void
uxa_damage_glyphs (RegionPtr		region,
		   CARD8		op,
		   PicturePtr	pSrc,
		   PicturePtr	pDst,
		   PictFormatPtr	maskFormat,
		   INT16		xSrc,
		   INT16		ySrc,
		   int		nlist,
		   GlyphListPtr	list,
		   GlyphPtr	       *glyphs);

void
uxa_damage_add_traps (RegionPtr   region,
		     PicturePtr  pPicture,
		     INT16	    x_off,
		     INT16	    y_off,
		     int	    ntrap,
		      xTrap	    *traps);

void
uxa_damage_fill_spans (RegionPtr   region,
		       DrawablePtr pDrawable,
		       GC	    *pGC,
		       int	     npt,
		       DDXPointPtr ppt,
		       int	    *pwidth,
		       int	     fSorted);

void
uxa_damage_set_spans (RegionPtr    region,
		      DrawablePtr  pDrawable,
		      GCPtr	     pGC,
		      char	    *pcharsrc,
		      DDXPointPtr  ppt,
		      int	    *pwidth,
		      int	     npt,
		      int	     fSorted);

void
uxa_damage_put_image (RegionPtr    region,
		DrawablePtr  pDrawable,
		GCPtr	     pGC,
		int	     depth,
		int	     x,
		int	     y,
		int	     w,
		int	     h,
		int	     leftPad,
		int	     format,
		      char	    *pImage);

void
uxa_damage_copy_area(RegionPtr    region,
		     DrawablePtr  pSrc,
		     DrawablePtr  pDst,
		     GC	         *pGC,
		     int	  srcx,
		     int	  srcy,
		     int	    width,
		     int	    height,
		     int	    dstx,
		     int	    dsty);

void
uxa_damage_copy_plane (RegionPtr	region,
		       DrawablePtr	pSrc,
		       DrawablePtr	pDst,
		       GCPtr		pGC,
		       int		srcx,
		       int		srcy,
		       int		width,
		       int		height,
		       int		dstx,
		       int		dsty,
		       unsigned long	bitPlane);

void
uxa_damage_poly_point (RegionPtr   region,
		 DrawablePtr pDrawable,
		 GCPtr	    pGC,
		 int	    mode,
		 int	    npt,
		       xPoint	    *ppt);

void
uxa_damage_poly_lines (RegionPtr  region,
		 DrawablePtr pDrawable,
		 GCPtr	    pGC,
		 int	    mode,
		 int	    npt,
		       DDXPointPtr ppt);
void
uxa_damage_poly_segment (RegionPtr    region,
			 DrawablePtr	pDrawable,
			 GCPtr	pGC,
			 int		nSeg,
			 xSegment	*pSeg);
void
uxa_damage_poly_rectangle (RegionPtr    region,
			   DrawablePtr  pDrawable,
			   GCPtr        pGC,
			   int	  nRects,
			   xRectangle  *pRects);
void
uxa_damage_poly_arc (RegionPtr    region,
		     DrawablePtr  pDrawable,
		     GCPtr	    pGC,
		     int	    nArcs,
		     xArc	    *pArcs);

void
uxa_damage_fill_polygon (RegionPtr     region,
			 DrawablePtr	pDrawable,
			 GCPtr		pGC,
			 int		shape,
			 int		mode,
			 int		npt,
			 DDXPointPtr	ppt);
void
uxa_damage_poly_fill_rect (RegionPtr   region,
			   DrawablePtr	pDrawable,
			   GCPtr	pGC,
			   int		nRects,
			   xRectangle	*pRects);
void
uxa_damage_poly_fill_arc (RegionPtr    region,
			  DrawablePtr	pDrawable,
			  GCPtr		pGC,
			  int		nArcs,
			  xArc		*pArcs);

void
uxa_damage_chars (RegionPtr	region,
		   DrawablePtr	pDrawable,
		   FontPtr	font,
		   int		x,
		   int		y,
		   unsigned int	n,
		   CharInfoPtr	*charinfo,
		   Bool		imageblt,
		  int		subWindowMode);

int
uxa_damage_text (RegionPtr	region,
	    DrawablePtr	    pDrawable,
	    GCPtr	    pGC,
	    int		    x,
	    int		    y,
	    unsigned long   count,
	    char	    *chars,
	    FontEncoding    fontEncoding,
		 Bool	    textType);

int
uxa_damage_poly_text_8(RegionPtr	region,
		       DrawablePtr pDrawable,
		       GCPtr	    pGC,
		       int	    x,
		       int	    y,
		       int	    count,
		       char	    *chars);

int
uxa_damage_poly_text_16 (RegionPtr	region,
			 DrawablePtr	pDrawable,
			 GCPtr		pGC,
			 int		x,
			 int		y,
			 int		count,
			 unsigned short	*chars);

void
uxa_damage_image_text_8(RegionPtr	region,
		 DrawablePtr	pDrawable,
		 GCPtr		pGC,
		 int		x,
		 int		y,
		 int		count,
			char		*chars);

void
uxa_damage_image_text_16 (RegionPtr	region,
		  DrawablePtr	pDrawable,
		  GCPtr		pGC,
		  int		x,
		  int		y,
		  int		count,
			  unsigned short *chars);

void
uxa_damage_image_glyph_blt(RegionPtr	region,
		    DrawablePtr	    pDrawable,
		    GCPtr	    pGC,
		    int		    x,
		    int		    y,
		    unsigned int    nglyph,
		    CharInfoPtr	    *ppci,
			   pointer	    pglyphBase);

void
uxa_damage_poly_glyph_blt(RegionPtr	region,
		   DrawablePtr	pDrawable,
		   GCPtr	pGC,
		   int		x,
		   int		y,
		   unsigned int	nglyph,
		   CharInfoPtr	*ppci,
			  pointer	pglyphBase);

void
uxa_damage_push_pixels (RegionPtr	region,
		 GCPtr		pGC,
		 PixmapPtr	pBitMap,
		 DrawablePtr	pDrawable,
		 int		dx,
		 int		dy,
		 int		xOrg,
			int		yOrg);

void
uxa_damage_copy_window (RegionPtr	region,
			WindowPtr	pWindow,
			DDXPointRec	ptOldOrg,
			RegionPtr	prgnSrc);
