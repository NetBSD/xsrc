/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/ffb/ffb_lines.h,v 1.1.1.1 2004/04/08 09:16:51 alanh Exp $ */

#ifndef _FFB_LINES_H
#define _FFB_LINES_H

#include "ffb_context.h"

#define _FFB_NEW_LINE (_DD_NEW_FLATSHADE |	\
		       _DD_NEW_LINE_WIDTH |	\
		       _DD_NEW_LINE_STIPPLE |	\
		       _DD_NEW_LINE_SMOOTH |	\
		       _NEW_COLOR)

extern void ffbDDLinefuncInit(void);
extern void ffbChooseLineState(GLcontext *);
extern void ffb_fallback_line( GLcontext *ctx, ffb_vertex *v0, ffb_vertex *v1 );

#endif /* !(_FFB_LINES_H) */
