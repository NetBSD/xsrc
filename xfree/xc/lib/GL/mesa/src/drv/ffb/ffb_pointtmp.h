/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_pointtmp.h,v 1.2 2000/12/05 21:18:33 dawes Exp $ */

static void TAG(ffb_points)(GLcontext *ctx, GLuint first, GLuint last)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	ffb_fbcPtr ffb = fmesa->regs;
	struct vertex_buffer *VB = ctx->VB;
	ffb_vertex *ffbVB = FFB_DRIVER_DATA(VB)->verts;
#if (IND & FFB_POINT_OFFSET_BIT)
#if (IND & FFB_POINT_AA_BIT)
	GLuint ffb_zoffset = FFB_Z_FROM_FLOAT(ctx->PointZoffset);
#else
	GLuint ffb_zoffset = Z_FROM_MESA(ctx->PointZoffset);
#endif
#endif
#if (IND & FFB_POINT_BIG_BIT)
	GLuint sz = FFB_COORD_FROM_FLOAT(ctx->Point.Size * 0.5f);
#endif
	int i;

	for (i = first; i < last; i++) {
		if (VB->ClipMask[i] == 0) {
			ffb_vertex *tmp = &ffbVB[i];
#if !(IND & FFB_POINT_BIG_BIT)
#if (IND & FFB_POINT_AA_BIT)
			FFBFifo(fmesa, 4);
			ffb->fg = (((GLuint)VB->Color[0]->data[i][0] << 0) |
				   ((GLuint)VB->Color[0]->data[i][1] << 8) |
				   ((GLuint)VB->Color[0]->data[i][2] << 16) |
				   ((GLuint)VB->Color[0]->data[i][3] << 24));
#if (IND & FFB_POINT_OFFSET_BIT)
			ffb->z = tmp->z + ffb_zoffset;
#else
			ffb->z = tmp->z;
#endif
			ffb->y = tmp->y + 0x8000;
			ffb->x = tmp->x + 0x8000;
#else
			FFBFifo(fmesa, 4);
			ffb->fg = (((GLuint)VB->Color[0]->data[i][0] << 0) |
				   ((GLuint)VB->Color[0]->data[i][1] << 8) |
				   ((GLuint)VB->Color[0]->data[i][2] << 16) |
				   ((GLuint)VB->Color[0]->data[i][3] << 24));
#if (IND & FFB_POINT_OFFSET_BIT)
			ffb->constz =
				(Z_FROM_MESA(VB->Win.data[i][2]) + ffb_zoffset);
#else
			ffb->constz = Z_FROM_MESA(VB->Win.data[i][2]);
#endif
			ffb->bh = tmp->y >> 16;
			ffb->bw = tmp->x >> 16;
#endif
#else /* FFB_POINT_BIG_BIT */
			/* Doing big points via triangles. */
			FFBFifo(fmesa, 10);

			ffb->fg = (((GLuint)VB->Color[0]->data[i][0] << 0) |
				   ((GLuint)VB->Color[0]->data[i][1] << 8) |
				   ((GLuint)VB->Color[0]->data[i][2] << 16) |
				   ((GLuint)VB->Color[0]->data[i][3] << 24));
#if (IND & FFB_POINT_OFFSET_BIT)
			ffb->constz =
				(Z_FROM_MESA(VB->Win.data[i][2]) + ffb_zoffset);
#else
			ffb->constz = Z_FROM_MESA(VB->Win.data[i][2]);
#endif

			/* The vertex sequence using isolated triangles would
			 * be:
			 *	Vertex0: VX - sz, VY - sz	RXF/RYF
			 *	Vertex1: VX + sz, VY - sz	DOXF/DOYF
			 *	Vertex2: VX + sz, VY + sz	DOXF/DOYF
			 *	Vertex3: VX + sz, VY + sz	RXF/RYF
			 *	Vertex4: VX - sz, VY + sz	DOXF/DOYF
			 *	Vertex5: VX - sz, VY - sz	DOXF/DOYF
			 *
			 * Using star chaining we can optimize this into a
			 * four vertex sequence, as follows:
			 *
			 *	Vertex0: VX - sz, VY - sz	RXF/RYF
			 *	Vertex1: VX + sz, VY - sz	DOXF/DOYF
			 *	Vertex2: VX + sz, VY + sz	DOXF/DOYF
			 *	Vertex3: VX - sz, VY + sz	DMXF/DMYF
			 */

			/* Vertex0: VX - sz, VY - sz, RXF/RYF */
			ffb->ryf = tmp->y - sz;
			ffb->rxf = tmp->x - sz;

			/* Vertex1: VX + sz, VY - sz, DOXF/DOYF */
			ffb->y = tmp->y - sz;
			ffb->x = tmp->x + sz;

			/* Vertex2: VX + sz, VY + sz, DOXF/DOYF */
			ffb->y = tmp->y + sz;
			ffb->x = tmp->x + sz;

			/* Vertex3: VX - sz, VY + sz, DMXF/DMYF */
			ffb->dmyf = tmp->y + sz;
			ffb->dmxf = tmp->x - sz;
#endif /* !(FFB_POINT_BIG_BIT) */
		}
	}

	fmesa->ffbScreen->rp_active = 1;
}

static void TAG(init)(void)
{
	ffb_points_tab[IND] = TAG(ffb_points);
}

#undef IND
#undef TAG
