/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_vb.c,v 1.2 2000/09/24 13:51:03 alanh Exp $
 *
 * GLX Hardware Device Driver for Sun Creator/Creator3D
 * Copyright (C) 2000 David S. Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID MILLER, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    David S. Miller <davem@redhat.com>
 */

#include "ffb_xmesa.h"
#include "ffb_context.h"
#include "ffb_vb.h"
#include "mem.h"
#include "stages.h"

#define COL {							\
	GLubyte *col = &(VB->Color[0]->data[i][0]);		\
	v->color[0].alpha = FFB_COLOR_FROM_UBYTE(col[3]);	\
	v->color[0].red   = FFB_COLOR_FROM_UBYTE(col[0]);	\
	v->color[0].green = FFB_COLOR_FROM_UBYTE(col[1]);	\
	v->color[0].blue  = FFB_COLOR_FROM_UBYTE(col[2]);	\
}

#define COL2 {							\
	GLubyte *col = &(VB->Color[0]->data[i][0]);		\
	v->color[0].alpha = FFB_COLOR_FROM_UBYTE(col[3]);	\
	v->color[0].red   = FFB_COLOR_FROM_UBYTE(col[0]);	\
	v->color[0].green = FFB_COLOR_FROM_UBYTE(col[1]);	\
	v->color[0].blue  = FFB_COLOR_FROM_UBYTE(col[2]);	\
								\
	col = &(VB->Color[1]->data[i][0]);			\
	v->color[1].alpha = FFB_COLOR_FROM_UBYTE(col[3]);	\
	v->color[1].red   = FFB_COLOR_FROM_UBYTE(col[0]);	\
	v->color[1].green = FFB_COLOR_FROM_UBYTE(col[1]);	\
	v->color[1].blue  = FFB_COLOR_FROM_UBYTE(col[2]);	\
}

#define COORD {					\
	GLfloat *win = VB->Win.data[i];		\
	GLfloat tmp;				\
	tmp = win[0] + ffbxoff;			\
	v->x = FFB_COORD_FROM_FLOAT(tmp);	\
	tmp  = - win[1] + ffbyoff;		\
	v->y = FFB_COORD_FROM_FLOAT(tmp);	\
	tmp = win[2] * (1.0f / 65536.0f);	\
	v->z = FFB_Z_FROM_FLOAT(tmp);		\
}	

#define NOP

#define SETUPFUNC(name,win,col)						\
static void name(struct vertex_buffer *VB, GLuint start, GLuint end)	\
{									\
	ffbContextPtr fmesa = FFB_CONTEXT(VB->ctx);			\
	__DRIdrawablePrivate *dPriv = fmesa->driDrawable;		\
	ffb_vertex *v;							\
	GLfloat ffbxoff = dPriv->x - 0.5;				\
	GLfloat ffbyoff = dPriv->h - 0.5 + dPriv->y;			\
	int i;								\
	(void) fmesa; (void) ffbxoff; (void) ffbyoff;			\
									\
	if (0)								\
		fprintf(stderr, #name ": VB(%p) start(%d) end(%d)\n",	\
			VB, start, end);				\
	/* Flush the vertex cache. */					\
	fmesa->vtx_cache[0] = fmesa->vtx_cache[1] =			\
		fmesa->vtx_cache[2] = fmesa->vtx_cache[3] = NULL;	\
	gl_import_client_data(VB, VB->ctx->RenderFlags,			\
			      (VB->ClipOrMask				\
			       ? VEC_WRITABLE | VEC_GOOD_STRIDE		\
			       : VEC_GOOD_STRIDE));			\
									\
	v = &(FFB_DRIVER_DATA(VB)->verts[start]);			\
									\
	if (VB->ClipOrMask == 0) {					\
		for (i = start; i < end; i++, v++) {			\
			win;						\
			col;						\
		}							\
	} else {							\
		for (i = start; i < end; i++, v++) {			\
			if (VB->ClipMask[i] == 0) {			\
				win;					\
			}						\
			col;						\
		}							\
	}								\
}

SETUPFUNC(rs_w,		COORD,NOP)
SETUPFUNC(rs_g,		NOP,COL)
SETUPFUNC(rs_g2,	NOP,COL2)
SETUPFUNC(rs_wg,	COORD,COL)
SETUPFUNC(rs_wg2,	COORD,COL2)

static void rs_invalid(struct vertex_buffer *VB, GLuint start, GLuint end)
{
	fprintf(stderr, "ffbRasterSetup(): invalid setup function\n");
}

typedef void (*setupFunc)(struct vertex_buffer *, GLuint, GLuint);

static setupFunc setup_func[8];

void ffbDDSetupInit(void)
{
	int i;

	for (i = 0; i < 4; i++)
		setup_func[i] = rs_invalid;

	setup_func[FFB_VB_WIN_BIT]					= rs_w;
	setup_func[FFB_VB_RGBA_BIT]					= rs_g;
	setup_func[FFB_VB_RGBA_BIT|FFB_VB_TWOSIDE_BIT]			= rs_g2;
	setup_func[FFB_VB_WIN_BIT|FFB_VB_RGBA_BIT]			= rs_wg;
	setup_func[FFB_VB_WIN_BIT|FFB_VB_RGBA_BIT|FFB_VB_TWOSIDE_BIT]	= rs_wg2;
}

static void ffbPrintSetupFlags(char *msg, GLuint flags)
{
	fprintf(stderr, "%s: %x %s%s%s\n",
		msg, flags,
		(flags & FFB_VB_WIN_BIT)	? " xyz,"  : "",
		(flags & FFB_VB_TWOSIDE_BIT)	? " twoside,"  : "",
		(flags & FFB_VB_RGBA_BIT)	? " rgba," : "");
}

void ffbChooseRasterSetupFunc(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	int funcindex;

	/* Currently just one full vertex setup type. */
	funcindex = FFB_VB_WIN_BIT | FFB_VB_RGBA_BIT;

	if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE)
		funcindex |= FFB_VB_TWOSIDE_BIT;

	if (MESA_VERBOSE)
		ffbPrintSetupFlags("ffb: full setup function", funcindex);

	fmesa->setupindex = funcindex;
	ctx->Driver.RasterSetup = setup_func[funcindex];
}

void ffbDDCheckPartialRasterSetup(GLcontext *ctx, struct gl_pipeline_stage *d)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	GLuint tmp = fmesa->setupdone;

	d->type = 0;
	fmesa->setupdone = 0;

	if ((ctx->Array.Summary & VERT_OBJ_ANY) == 0)
		return;

	if (ctx->IndirectTriangles)
		return;

	fmesa->setupdone = tmp;
}

void ffbDDPartialRasterSetup(struct vertex_buffer *VB)
{
	ffbContextPtr fmesa = FFB_CONTEXT(VB->ctx);
	GLuint new = VB->pipeline->new_outputs;
	GLuint available = VB->pipeline->outputs;
	GLuint ind = 0;

#if 1
	ind = FFB_VB_WIN_BIT | FFB_VB_RGBA_BIT;
	if (VB->ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE)
		ind |= FFB_VB_TWOSIDE_BIT;
#else
	if (new & VERT_WIN) {
		new = available;
		ind |= FFB_VB_WIN_BIT;
	}
	if (new & VERT_RGBA)
		ind |= FFB_VB_RGBA_BIT;
#endif

#if 0
	fmesa->setupdone &= ~ind;
	ind &= fmesa->setupindex;
	fmesa->setupdone |= ind;
#endif

	if (MESA_VERBOSE)
		ffbPrintSetupFlags("ffb: partial setup function", ind);

	if (ind)
		setup_func[ind](VB, VB->Start, VB->Count);
}

void ffbDDDoRasterSetup(struct vertex_buffer *VB)
{
	GLcontext *ctx = VB->ctx;

	if (VB->Type == VB_CVA_PRECALC)
		ffbDDPartialRasterSetup(VB);
	else if (ctx->Driver.RasterSetup)
		ctx->Driver.RasterSetup(VB, VB->CopyStart, VB->Count);
}

void ffbDDResizeVB(struct vertex_buffer *VB, GLuint size)
{
	FFBVertexBufferPtr mvb = FFB_DRIVER_DATA(VB);

	while (mvb->size < size)
		mvb->size *= 2;

	free(mvb->verts);
	mvb->verts = (ffb_vertex *)malloc(sizeof(ffb_vertex) * mvb->size);
	if (!mvb->verts) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	gl_vector1ui_free(&mvb->clipped_elements);
	gl_vector1ui_alloc(&mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32);
	if (!mvb->clipped_elements.start) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	ALIGN_FREE(VB->ClipMask);
	VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 4);
	if (!VB->ClipMask) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	if (VB->Type == VB_IMMEDIATE) {
		free(mvb->primitive);
		free(mvb->next_primitive);
		mvb->primitive = (GLuint *)malloc(sizeof(GLuint) * mvb->size);
		mvb->next_primitive = (GLuint *)malloc(sizeof(GLuint) * mvb->size);
		if (!mvb->primitive || !mvb->next_primitive) {
			fprintf(stderr, "ffb-glx: out of memory !\n");
			exit(1);
		}
	}
}

void ffbDDRegisterVB(struct vertex_buffer *VB)
{
	FFBVertexBufferPtr mvb;

	mvb = (FFBVertexBufferPtr) calloc(1, sizeof(*mvb));
	if (!mvb) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}
	mvb->size = VB->Size * 2;
	mvb->verts = (ffb_vertex *) malloc(sizeof(ffb_vertex) * mvb->size);
	if (!mvb->verts) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	gl_vector1ui_alloc(&mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32);
	if (!mvb->clipped_elements.start) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	ALIGN_FREE(VB->ClipMask);
	VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 4);
	if (!VB->ClipMask) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	mvb->primitive = (GLuint *)malloc(sizeof(GLuint) * mvb->size);
	mvb->next_primitive = (GLuint *)malloc(sizeof(GLuint) * mvb->size);
	if (!mvb->primitive || !mvb->next_primitive) {
		fprintf(stderr, "ffb-glx: out of memory !\n");
		exit(1);
	}

	VB->driver_data = mvb;
}

void ffbDDUnregisterVB(struct vertex_buffer *VB)
{
	FFBVertexBufferPtr mvb = FFB_DRIVER_DATA(VB);

	if (mvb) {
		if (mvb->verts)
			free(mvb->verts);
		if (mvb->primitive)
			free(mvb->primitive);
		if (mvb->next_primitive)
			free(mvb->next_primitive);
		gl_vector1ui_free(&mvb->clipped_elements);
		free(mvb);
		VB->driver_data = 0;
	}
}
