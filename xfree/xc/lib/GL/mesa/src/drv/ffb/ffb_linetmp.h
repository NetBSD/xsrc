/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_linetmp.h,v 1.1 2000/06/20 05:08:38 dawes Exp $ */

static void TAG(ffb_line)(GLcontext *ctx, GLuint e0, GLuint e1, GLuint pv)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	ffb_fbcPtr ffb = fmesa->regs;
	struct vertex_buffer *VB = ctx->VB;
	ffb_vertex *ffbVB = FFB_DRIVER_DATA(VB)->verts;
	const ffb_vertex *v0 = &ffbVB[e0];
	const ffb_vertex *v1 = &ffbVB[e1];
#if (IND & FFB_LINE_OFFSET_BIT)
	GLuint ffb_zoffset = FFB_Z_FROM_FLOAT(ctx->LineZoffset);
#endif
#if (IND & FFB_LINE_TWOSIDE_BIT)
	const int which_color = (VB->ColorPtr == VB->Color[0]) ? 0 : 1;
#else
	const int which_color = 0;
#endif
#if (IND & FFB_LINE_FLAT_BIT)
	const GLuint const_fg = (((GLuint)VB->Color[which_color]->data[pv][0] << 0) |
				 ((GLuint)VB->Color[which_color]->data[pv][1] << 8) |
				 ((GLuint)VB->Color[which_color]->data[pv][2] << 16) |
				 ((GLuint)VB->Color[which_color]->data[pv][3] << 24));
#endif

#if (IND & FFB_LINE_FLAT_BIT)
	FFBFifo(fmesa, 1);
	ffb->fg = const_fg;
#endif

#if !(IND & FFB_LINE_WIDE_BIT)
	/* We actually need to do the LINE_LOOP/LINE_STRIP optimization
	 * in this case so that the line pattern works out correctly.
	 * Really, Mesa should be fixed so it sends us the real primitive.
	 */
	if (v0 == fmesa->vtx_cache[1] &&
	    v1 != fmesa->vtx_cache[0]) {
#if (IND & FFB_LINE_FLAT_BIT)
		FFBFifo(fmesa, 3);
#else
#if (IND & FFB_LINE_ALPHA_BIT)
		FFBFifo(fmesa, 7);
#else
		FFBFifo(fmesa, 6);
#endif
#endif
#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
		ffb->alpha	= v1->color[which_color].alpha;
#endif
		ffb->red	= v1->color[which_color].red;
		ffb->green	= v1->color[which_color].green;
		ffb->blue	= v1->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
		ffb->z		= v1->z + ffb_zoffset;
#else
		ffb->z		= v1->z;
#endif
		ffb->y		= v1->y;
		ffb->x		= v1->x;

		fmesa->vtx_cache[0] = (void *)v0;
		fmesa->vtx_cache[1] = (void *)v1;
		fmesa->ffbScreen->rp_active = 1;
		return;
	} else {
		fmesa->vtx_cache[0] = (void *)v0;
		fmesa->vtx_cache[1] = (void *)v1;
	}
#endif

#if (IND & FFB_LINE_FLAT_BIT)
#if !(IND & FFB_LINE_WIDE_BIT)
	/* (2 * 3) + 1 */
	FFBFifo(fmesa, 7);
#else
	/* (4 * 3) */
	FFBFifo(fmesa, 12);
#endif
#else
#if !(IND & FFB_LINE_WIDE_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
	/* (2 * 7) + 1 */
	FFBFifo(fmesa, 15);
#else
	/* (2 * 6) + 1 */
	FFBFifo(fmesa, 13);
#endif
#else
#if (IND & FFB_LINE_ALPHA_BIT)
	/* (4 * 7) */
	FFBFifo(fmesa, 28);
#else
	/* (4 * 6) */
	FFBFifo(fmesa, 24);
#endif
#endif
#endif

#if !(IND & FFB_LINE_WIDE_BIT)
	/* Using DDLINE or AALINE, init the line pattern state. */
	ffb->lpat = fmesa->lpat;

#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
	ffb->alpha	= v0->color[which_color].alpha;
#endif
	ffb->red	= v0->color[which_color].red;
	ffb->green	= v0->color[which_color].green;
	ffb->blue	= v0->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
	ffb->z		= v0->z + ffb_zoffset;
#else
	ffb->z		= v0->z;
#endif
	ffb->ryf	= v0->y;
	ffb->rxf	= v0->x;

#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
	ffb->alpha	= v1->color[which_color].alpha;
#endif
	ffb->red	= v1->color[which_color].red;
	ffb->green	= v1->color[which_color].green;
	ffb->blue	= v1->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
	ffb->z		= v1->z + ffb_zoffset;
#else
	ffb->z		= v1->z;
#endif
	ffb->y		= v1->y;
	ffb->x		= v1->x;

#else /* FFB_LINE_WIDE_BIT */
	/* Doing wide lines via triangles. */
	{
		float width = ctx->Line.Width;
		float dx, dy;
		GLuint ix, iy;

		/* It might be possible to precalculate this stuff in
		 * our vertex-buffer code.  That code works with
		 * screen coordinates so it might work.  One problem
		 * could be that due to potentially drawing polygons
		 * as lines, it would be very difficult to really
		 * determine if the calculations were necessary or not.
		 */
		dx = VB->Win.data[e0][0] - VB->Win.data[e1][0];
		dy = VB->Win.data[e0][1] - VB->Win.data[e1][1];
		ix = FFB_COORD_FROM_FLOAT(width * 0.5f);
		iy = 0;
		if ((dx * dx) > (dy * dy)) {
			iy = ix;
			ix = 0;
		}

		/* The vertex sequence using isolated triangles would
		 * be:
		 *
		 *	Vertex0: line vertex0 - I	RXF/RYF
		 *	Vertex1: line vertex1 + I	DOXF/DOYF
		 *	Vertex2: line vertex0 + I	DOXF/DOYF
		 *	Vertex3: line vertex0 - I	RXF/RYF
		 *	Vertex4: line vertex1 - I	DOXF/DOYF
		 *	Vertex5: line vertex1 + I	DOXF/DOYF
		 *
		 * Using star chaining we can optimize this into a
		 * four vertex sequence, as follows:
		 *
		 *	Vertex0: line vertex0 - I	RXF/RYF
		 *	Vertex1: line vertex0 + I	DOXF/DOYF
		 *	Vertex2: line vertex1 + I	DOXF/DOYF
		 *	Vertex3: line vertex1 - I	DMXF/DMYF
		 *
		 * The DMXF/DMYF vertex means "replace middle vertex
		 * with new vertex and draw".
		 */

		/* Vertex0: line vertex0 - I, RXF/RYF */
#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
		ffb->alpha	= v0->color[which_color].alpha;
#endif
		ffb->red	= v0->color[which_color].red;
		ffb->green	= v0->color[which_color].green;
		ffb->blue	= v0->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
		ffb->z		= v0->z + ffb_zoffset;
#else
		ffb->z		= v0->z;
#endif
		ffb->ryf	= v0->y - iy;
		ffb->rxf	= v0->x - ix;

		/* Vertex1: line vertex0 + I, DOXF/DOYF */
#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
		ffb->alpha	= v0->color[which_color].alpha;
#endif
		ffb->red	= v0->color[which_color].red;
		ffb->green	= v0->color[which_color].green;
		ffb->blue	= v0->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
		ffb->z		= v0->z + ffb_zoffset;
#else
		ffb->z		= v0->z;
#endif
		ffb->y		= v0->y + iy;
		ffb->x		= v0->x + ix;

		/* Vertex2: line vertex1 + I, DOXF/DOYF */
#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
		ffb->alpha	= v1->color[which_color].alpha;
#endif
		ffb->red	= v1->color[which_color].red;
		ffb->green	= v1->color[which_color].green;
		ffb->blue	= v1->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
		ffb->z		= v1->z + ffb_zoffset;
#else
		ffb->z		= v1->z;
#endif
		ffb->y		= v1->y + iy;
		ffb->x		= v1->x + ix;

		/* Vertex3: line vertex1 - I, DMXF/DMYF */
#if !(IND & FFB_LINE_FLAT_BIT)
#if (IND & FFB_LINE_ALPHA_BIT)
		ffb->alpha	= v1->color[which_color].alpha;
#endif
		ffb->red	= v1->color[which_color].red;
		ffb->green	= v1->color[which_color].green;
		ffb->blue	= v1->color[which_color].blue;
#endif
#if (IND & FFB_LINE_OFFSET_BIT)
		ffb->z		= v1->z + ffb_zoffset;
#else
		ffb->z		= v1->z;
#endif
		ffb->dmyf	= v1->y - iy;
		ffb->dmxf	= v1->x - ix;
	}
#endif /* !(FFB_LINE_WIDE_BIT) */

	fmesa->ffbScreen->rp_active = 1;
}

static void TAG(init)(void)
{
	ffb_line_tab[IND] = TAG(ffb_line);
}

#undef IND
#undef TAG
