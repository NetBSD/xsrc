/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_state.h,v 1.1 2000/06/20 05:08:39 dawes Exp $ */

#ifndef _FFB_STATE_H
#define _FFB_STATE_H

extern void ffbDDInitStateFuncs(GLcontext *);
extern void ffbDDInitContextHwState(GLcontext *);

extern void ffbDDScissor(GLcontext *, GLint, GLint, GLint, GLint);
extern void ffbXformAreaPattern(ffbContextPtr, const GLubyte *);
extern void ffbSyncHardware(ffbContextPtr fmesa);

#endif /* !(_FFB_STATE_H) */
