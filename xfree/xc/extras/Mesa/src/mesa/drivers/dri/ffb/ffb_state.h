/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/ffb/ffb_state.h,v 1.1.1.1 2004/04/08 09:16:52 alanh Exp $ */

#ifndef _FFB_STATE_H
#define _FFB_STATE_H

extern void ffbDDInitStateFuncs(GLcontext *);
extern void ffbDDInitContextHwState(GLcontext *);

extern void ffbCalcViewport(GLcontext *);
extern void ffbXformAreaPattern(ffbContextPtr, const GLubyte *);
extern void ffbSyncHardware(ffbContextPtr fmesa);

#endif /* !(_FFB_STATE_H) */
