/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_vb.h,v 1.1 2000/06/20 05:08:40 dawes Exp $ */

#ifndef _FFB_VB_H
#define _FFB_VB_H

#include "vb.h"
#include "types.h"

/* Use this to disable ffb VB processing, presumable to try
 * and isolate bugs.
 */
#undef FFB_VB_DISABLE

/* These are all in 2:30 signed fixed point format. */
typedef struct {
	GLuint	alpha;
	GLuint	red;
	GLuint	green;
	GLuint	blue;
} ffb_color;

#define FFB_COLOR_FROM_UBYTE(VAL)	(((((GLuint)(VAL)) + 1) & (255 << 1)) << (30 - 8))

typedef struct {
	/* As for colors, this is in 2:30 signed fixed point. */
	GLint		z;

	/* These are in 16:16 fixed point. */
	GLuint		y, x;

	ffb_color	color[2];
} ffb_vertex;

#define FFB_Z_FROM_FLOAT(VAL)		((GLint)((VAL) * 1073741824.0f))
#define FFB_COORD_FROM_FLOAT(VAL)	((GLuint)((VAL) * 65536.0f))

struct ffb_vertex_buffer_t {
	GLvector1ui	clipped_elements;
	ffb_vertex	*verts;
	int		last_virt;
	GLuint		*primitive;
	GLuint		*next_primitive;
	GLuint		size;
};

typedef struct ffb_vertex_buffer_t *FFBVertexBufferPtr;

#define FFB_DRIVER_DATA(vb)	((FFBVertexBufferPtr)((vb)->driver_data))

#define FFB_VB_RGBA_BIT		0x01
#define FFB_VB_WIN_BIT		0x02
#define FFB_VB_TWOSIDE_BIT	0x04

extern void ffbDDSetupInit(void);
extern void ffbChooseRasterSetupFunc(GLcontext *);
extern void ffbDDCheckPartialRasterSetup(GLcontext *, struct gl_pipeline_stage *);
extern void ffbDDPartialRasterSetup(struct vertex_buffer *);
extern void ffbDDDoRasterSetup(struct vertex_buffer *);
extern void ffbDDResizeVB(struct vertex_buffer *, GLuint);
extern void ffbDDRegisterVB(struct vertex_buffer *);
extern void ffbDDUnregisterVB(struct vertex_buffer *);

#endif /* !(_FFB_VB_H) */
