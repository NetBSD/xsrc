#ifndef _FFB_CONTEXT_H
#define _FFB_CONTEXT_H

#include <X11/Xlibint.h>
#include "dri_mesaint.h"
#include "dri_mesa.h"

#include "types.h"

#include "ffb_xmesa.h"

typedef struct ffb_context_t {
	GLcontext		*glCtx;
	GLuint			MonoColor;
	GLframebuffer		*glBuffer;

	ffb_fbcPtr		regs;
	volatile char		*sfb32;

	int			hw_locked;
	int			SWrender;

	int			back_buffer;	/* 0 = bufferA, 1 = bufferB */

	/* Because MESA does not send us the raw primitives,
	 * we send everything to the chip as independant lines,
	 * points, tris, and quads.  If we could get the real
	 * primitive being used by the user, we can optimize
	 * things a lot.  This is particularly useful for
	 * tri strips/fans, and quad strips/fans as FFB
	 * specifically can optimize these cases.
	 *
	 * I suspect MESA does not preserve things to make it's
	 * transformation/clip/cull optimizations simpler.
	 *
	 * Anyways, to try and get around this, we record the
	 * vertices used in the most recent primitive and we
	 * detect tri strips/fans and quad strips/fans this
	 * way.  Actually, we only need to record the ffb_vertex
	 * pointers, and this makes the tests cheaper and the
	 * flushing faster (at VB updates and reduced primitive
	 * changes).
	 */
	void			*vtx_cache[4];

	/* This records state bits when a per-fragment attribute has
	 * been set which prevents us from rendering in hardware.
	 *
	 * As attributes change, some of these bits may clear as
	 * we move back within the chips capabilities.  If they
	 * all clear, we return to full hw rendering.
	 */
	unsigned int		bad_fragment_attrs;
#define FFB_BADATTR_FOG		0x00000001	/* Bad fog possible only when < FFB2 */
#define FFB_BADATTR_BLENDFUNC	0x00000002	/* Any non-const func based upon dst alpha */
#define FFB_BADATTR_BLENDROP	0x00000004	/* Blend enabled and LogicOP != GL_COPY */
#define FFB_BADATTR_BLENDEQN	0x00000008	/* Blend equation other than ADD */
#define FFB_BADATTR_STENCIL	0x00000010	/* Stencil enabled when < FFB2+ */

	unsigned int		state_dirty;
	unsigned int		state_fifo_ents;
#define FFB_STATE_FBC		0x00000001
#define FFB_STATE_PPC		0x00000002
#define FFB_STATE_DRAWOP	0x00000004
#define FFB_STATE_ROP		0x00000008
#define FFB_STATE_LPAT		0x00000010
#define FFB_STATE_PMASK		0x00000020
#define FFB_STATE_XPMASK	0x00000040
#define FFB_STATE_YPMASK	0x00000080
#define FFB_STATE_ZPMASK	0x00000100
#define FFB_STATE_XCLIP		0x00000200
#define FFB_STATE_CMP		0x00000400
#define FFB_STATE_MATCHAB	0x00000800
#define FFB_STATE_MAGNAB	0x00001000
#define FFB_STATE_MATCHC	0x00002000
#define FFB_STATE_MAGNC		0x00004000
#define FFB_STATE_DCUE		0x00008000
#define FFB_STATE_BLEND		0x00010000
#define FFB_STATE_CLIP		0x00020000
#define FFB_STATE_STENCIL	0x00040000
#define FFB_STATE_APAT		0x00080000
#define FFB_STATE_WID		0x00100000
#define FFB_STATE_ALL		0x001fffff

	unsigned int		state_all_fifo_ents;

	/* General hw reg state. */
	unsigned int		fbc;
	unsigned int		ppc;
	unsigned int		drawop;
	unsigned int		rop;

	unsigned int		lpat;
#define FFB_LPAT_BAD		0xffffffff

	unsigned int		wid;
	unsigned int		pmask;
	unsigned int		xpmask;
	unsigned int		ypmask;
	unsigned int		zpmask;
	unsigned int		xclip;
	unsigned int		cmp;
	unsigned int		matchab;
	unsigned int		magnab;
	unsigned int		matchc;
	unsigned int		magnc;

	/* Depth cue unit hw reg state. */
	unsigned int		dcss;	/* All FFB		*/
	unsigned int		dcsf;	/* All FFB		*/
	unsigned int		dcsb;	/* All FFB		*/
	unsigned int		dczf;	/* All FFB		*/
	unsigned int		dczb;	/* All FFB		*/
	unsigned int		dcss1;	/* >=FFB2 only		*/
	unsigned int		dcss2;	/* >=FFB2 only		*/
	unsigned int		dcss3;	/* >=FFB2 only		*/
	unsigned int		dcs2;	/* >=FFB2 only		*/
	unsigned int		dcs3;	/* >=FFB2 only		*/
	unsigned int		dcs4;	/* >=FFB2 only		*/
	unsigned int		dcd2;	/* >=FFB2 only		*/
	unsigned int		dcd3;	/* >=FFB2 only		*/
	unsigned int		dcd4;	/* >=FFB2 only		*/

	/* Blend unit hw reg state. */
	unsigned int		blendc;
	unsigned int		blendc1;
	unsigned int		blendc2;

	/* ViewPort clipping hw reg state. */
	unsigned int		vclipmin;
	unsigned int		vclipmax;
	unsigned int		vclipzmin;
	unsigned int		vclipzmax;
	struct {
		unsigned int	min;
		unsigned int	max;
	} aux_clips[4];

	/* Stencil control hw reg state.  >=FFB2+ only. */
	unsigned int		stencil;
	unsigned int		stencilctl;
	unsigned int		consty;		/* Stencil Ref */

	/* Area pattern (used for polygon stipples). */
	unsigned int		pattern[32];

	/* Fog state. */
	float			Znear, Zfar;

	drmContext		hHWContext;
	drmLock			*driHwLock;
	int			driFd;
	Display			*display;

	unsigned int		clear_pixel;
	unsigned int		clear_depth;
	unsigned int		clear_stencil;

	unsigned int		setupindex;
	unsigned int		setupdone;

	/* Rendering functions. */
	points_func   PointsFunc;
	line_func     LineFunc;
	triangle_func TriangleFunc;
	quad_func     QuadFunc;

	__DRIdrawablePrivate	*driDrawable;
	__DRIscreenPrivate	*driScreen;
	ffbScreenPrivate	*ffbScreen;
	ffb_dri_state_t		*ffb_sarea;
} ffbContextRec, *ffbContextPtr;

#define FFB_CONTEXT(ctx)	((ffbContextPtr)((ctx)->DriverCtx))

/* We want the depth values written during software rendering
 * to match what the hardware is going to put there when we
 * hw render.
 *
 * The Z buffer is 28 bits deep.  Smooth shaded primitives
 * specify a 2:30 signed fixed point Z value in the range 0.0
 * to 1.0 inclusive.
 *
 * So for example, when hw rendering, the largest Z value of
 * 1.0 would produce a value of 0x0fffffff in the actual Z
 * buffer, which is the maximum value.
 *
 * Mesa's depth type is a 32-bit int, so we use the following macro
 * to convert to/from FFB hw Z values.  Note we also have to clear
 * out the top bits as that is where the Y (stencil) buffer is stored
 * and during hw Z buffer reads it is always there. (During writes
 * we tell the hw to discard those top 4 bits).
 */
#define Z_TO_MESA(VAL)		((GLdepth)(((VAL) & 0x0fffffff) << (32 - 28)))
#define Z_FROM_MESA(VAL)	(((GLuint)(VAL)) >> (32 - 28))

#endif /* !(_FFB_CONTEXT_H) */
