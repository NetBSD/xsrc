/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/ffb/ffb_points.h,v 1.1.1.1 2004/04/08 09:16:51 alanh Exp $ */

#ifndef _FFB_POINTS_H
#define _FFB_POINTS_H

extern void ffbDDPointfuncInit(void);

#define _FFB_NEW_POINT (_DD_NEW_POINT_SIZE | 	\
			_DD_NEW_POINT_SMOOTH |	\
			_NEW_COLOR)

extern void ffbChoosePointState(GLcontext *);
extern void ffb_fallback_point( GLcontext *ctx, ffb_vertex *v0 );

#endif /* !(_FFB_POINTS_H) */
