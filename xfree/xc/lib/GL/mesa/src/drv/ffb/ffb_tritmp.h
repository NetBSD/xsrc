/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_tritmp.h,v 1.1 2000/06/20 05:08:40 dawes Exp $ */

static void TAG(ffb_triangle)(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2, GLuint pv)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	ffb_fbcPtr ffb = fmesa->regs;
	struct vertex_buffer *VB = ctx->VB;
	ffb_vertex *ffbVB = FFB_DRIVER_DATA(VB)->verts;
	const ffb_vertex *v0 = &ffbVB[e0];
	const ffb_vertex *v1 = &ffbVB[e1];
	const ffb_vertex *v2 = &ffbVB[e2];
#if (IND & FFB_TRI_OFFSET_BIT)
	GLuint ffb_zoffset = FFB_Z_FROM_FLOAT(ctx->PolygonZoffset);
#endif
#if (IND & FFB_TRI_FLAT_BIT)
	GLuint const_fg;
#endif
#if (IND & FFB_TRI_TWOSIDE_BIT)
	int which_color = (VB->ColorPtr == VB->Color[0]) ? 0 : 1;
#else
	const int which_color = 0;
#endif

#if (IND & FFB_TRI_CULL_BIT)
	{
		GLfloat (*win)[4] = VB->Win.data;
		GLfloat ex = win[e1][0] - win[e0][0];
		GLfloat ey = win[e1][1] - win[e0][1];
		GLfloat fx = win[e2][0] - win[e0][0];
		GLfloat fy = win[e2][1] - win[e0][1];
		GLfloat c = ex*fy-ey*fx;

		/* Culled... */
		if (c * ctx->backface_sign > 0)
			return;
	}
#endif

#if (IND & FFB_TRI_FLAT_BIT)
	const_fg = (((GLuint)VB->Color[which_color]->data[pv][0] << 0) |
		    ((GLuint)VB->Color[which_color]->data[pv][1] << 8) |
		    ((GLuint)VB->Color[which_color]->data[pv][2] << 16) |
		    ((GLuint)VB->Color[which_color]->data[pv][3] << 24));
#endif

	FFB_DUMP_PRIM(TRIANGLE);

#if (IND & FFB_TRI_FLAT_BIT)
	FFBFifo(fmesa, 1);
	ffb->fg = const_fg;
#endif

	/* First, check for a triangle strip/fan sequences.
	 * These checks rely on how MESA sends these primitives
	 * down to us in render_tmp.h
	 */
	if (v0 == fmesa->vtx_cache[1] &&
	    v1 == fmesa->vtx_cache[2] &&
	    v2 != fmesa->vtx_cache[0]) {
#if (IND & FFB_TRI_FLAT_BIT)
		FFBFifo(fmesa, 3);
#else
#if (IND & FFB_TRI_ALPHA_BIT)
		FFBFifo(fmesa, 7);
#else
		FFBFifo(fmesa, 6);
#endif
#endif
		FFB_DUMP_VERTEX(v2);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
		ffb->alpha = v2->color[which_color].alpha;
#endif
		ffb->red = v2->color[which_color].red;
		ffb->green = v2->color[which_color].green;
		ffb->blue = v2->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
		ffb->z = v2->z + ffb_zoffset;
#else
		ffb->z = v2->z;
#endif
		ffb->y = v2->y;
		ffb->x = v2->x;

		goto update_vcache;
	} else if (v0 == fmesa->vtx_cache[0] &&
		   v1 == fmesa->vtx_cache[2] &&
		   v2 != fmesa->vtx_cache[1]) {
#if (IND & FFB_TRI_FLAT_BIT)
		FFBFifo(fmesa, 3);
#else
#if (IND & FFB_TRI_ALPHA_BIT)
		FFBFifo(fmesa, 7);
#else
		FFBFifo(fmesa, 6);
#endif
#endif
		FFB_DUMP_VERTEX(v2);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
		ffb->alpha = v2->color[which_color].alpha;
#endif
		ffb->red = v2->color[which_color].red;
		ffb->green = v2->color[which_color].green;
		ffb->blue = v2->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
		ffb->z = v2->z + ffb_zoffset;
#else
		ffb->z = v2->z;
#endif
		ffb->dmyf = v2->y;
		ffb->dmxf = v2->x;

		goto update_vcache;
	}

#if (IND & FFB_TRI_FLAT_BIT)
	FFBFifo(fmesa, 9);
#else
#if (IND & FFB_TRI_ALPHA_BIT)
	FFBFifo(fmesa, 21);
#else
	FFBFifo(fmesa, 18);
#endif
#endif

	FFB_DUMP_VERTEX(v0);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v0->color[which_color].alpha;
#endif
	ffb->red = v0->color[which_color].red;
	ffb->green = v0->color[which_color].green;
	ffb->blue = v0->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v0->z + ffb_zoffset;
#else
	ffb->z = v0->z;
#endif
	ffb->ryf = v0->y;
	ffb->rxf = v0->x;

	FFB_DUMP_VERTEX(v1);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v1->color[which_color].alpha;
#endif
	ffb->red = v1->color[which_color].red;
	ffb->green = v1->color[which_color].green;
	ffb->blue = v1->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v1->z + ffb_zoffset;
#else
	ffb->z = v1->z;
#endif
	ffb->y = v1->y;
	ffb->x = v1->x;

	FFB_DUMP_VERTEX(v2);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v2->color[which_color].alpha;
#endif
	ffb->red = v2->color[which_color].red;
	ffb->green = v2->color[which_color].green;
	ffb->blue = v2->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v2->z + ffb_zoffset;
#else
	ffb->z = v2->z;
#endif
	ffb->y = v2->y;
	ffb->x = v2->x;

update_vcache:
	fmesa->vtx_cache[0] = (void *)v0;
	fmesa->vtx_cache[1] = (void *)v1;
	fmesa->vtx_cache[2] = (void *)v2;

	fmesa->ffbScreen->rp_active = 1;
}


static void TAG(ffb_quad)(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2, GLuint e3, GLuint pv)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	ffb_fbcPtr ffb = fmesa->regs;
	struct vertex_buffer *VB = ctx->VB;
	ffb_vertex *ffbVB = FFB_DRIVER_DATA(VB)->verts;
	const ffb_vertex *v0 = &ffbVB[e0];
	const ffb_vertex *v1 = &ffbVB[e1];
	const ffb_vertex *v2 = &ffbVB[e2];
	const ffb_vertex *v3 = &ffbVB[e3];
#if (IND & FFB_TRI_OFFSET_BIT)
	GLuint ffb_zoffset = FFB_Z_FROM_FLOAT(ctx->PolygonZoffset);
#endif
#if (IND & FFB_TRI_FLAT_BIT)
	GLuint const_fg;
#endif
#if (IND & FFB_TRI_TWOSIDE_BIT)
	int which_color = (VB->ColorPtr == VB->Color[0]) ? 0 : 1;
#else
	const int which_color = 0;
#endif

#if (IND & FFB_TRI_CULL_BIT)
	{
		GLfloat (*win)[4] = VB->Win.data;
		GLfloat ex = win[e2][0] - win[e0][0];
		GLfloat ey = win[e2][1] - win[e0][1];
		GLfloat fx = win[e3][0] - win[e1][0];
		GLfloat fy = win[e3][1] - win[e1][1];
		GLfloat c = ex*fy-ey*fx;

		/* Culled... */
		if (c * ctx->backface_sign > 0)
			return;
	}
#endif

#if (IND & FFB_TRI_FLAT_BIT)
	const_fg = (((GLuint)VB->Color[which_color]->data[pv][0] << 0) |
		    ((GLuint)VB->Color[which_color]->data[pv][1] << 8) |
		    ((GLuint)VB->Color[which_color]->data[pv][2] << 16) |
		    ((GLuint)VB->Color[which_color]->data[pv][3] << 24));
#endif

	FFB_DUMP_PRIM(QUAD);

#if (IND & FFB_TRI_FLAT_BIT)
	FFBFifo(fmesa, 13);
	ffb->fg = const_fg;
#else
#if (IND & FFB_TRI_ALPHA_BIT)
	FFBFifo(fmesa, 28);
#else
	FFBFifo(fmesa, 24);
#endif
#endif

	FFB_DUMP_VERTEX(v0);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v0->color[which_color].alpha;
#endif
	ffb->red = v0->color[which_color].red;
	ffb->green = v0->color[which_color].green;
	ffb->blue = v0->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v0->z + ffb_zoffset;
#else
	ffb->z = v0->z;
#endif
	ffb->ryf = v0->y;
	ffb->rxf = v0->x;

	FFB_DUMP_VERTEX(v1);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v1->color[which_color].alpha;
#endif
	ffb->red = v1->color[which_color].red;
	ffb->green = v1->color[which_color].green;
	ffb->blue = v1->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v1->z + ffb_zoffset;
#else
	ffb->z = v1->z;
#endif
	ffb->y = v1->y;
	ffb->x = v1->x;

	FFB_DUMP_VERTEX(v2);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v2->color[which_color].alpha;
#endif
	ffb->red = v2->color[which_color].red;
	ffb->green = v2->color[which_color].green;
	ffb->blue = v2->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v2->z + ffb_zoffset;
#else
	ffb->z = v2->z;
#endif
	ffb->y = v2->y;
	ffb->x = v2->x;

	FFB_DUMP_VERTEX(v3);
#if !(IND & FFB_TRI_FLAT_BIT)
#if (IND & FFB_TRI_ALPHA_BIT)
	ffb->alpha = v3->color[which_color].alpha;
#endif
	ffb->red = v3->color[which_color].red;
	ffb->green = v3->color[which_color].green;
	ffb->blue = v3->color[which_color].blue;
#endif
#if (IND & FFB_TRI_OFFSET_BIT)
	ffb->z = v3->z + ffb_zoffset;
#else
	ffb->z = v3->z;
#endif
	ffb->dmyf = v3->y;
	ffb->dmxf = v3->x;

	fmesa->ffbScreen->rp_active = 1;
}

static void TAG(init)(void)
{
	ffb_tri_tab[IND]	= TAG(ffb_triangle);
	ffb_quad_tab[IND]	= TAG(ffb_quad);
}

#undef IND
#undef TAG
