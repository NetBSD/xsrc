/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/ffb/ffb_clear.h,v 1.1.1.1 2004/04/08 09:16:51 alanh Exp $ */

#ifndef _FFB_CLEAR_H
#define _FFB_CLEAR_H

extern void ffbDDClear(GLcontext *ctx, GLbitfield mask, GLboolean all,
		       GLint cx, GLint cy, GLint cwidth, GLint cheight);

#endif /* !(_FFB_CLEAR_H) */
