/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgabuffers.h,v 1.2 2000/06/22 16:59:24 tsi Exp $ */

#ifndef MGA_BUFFERS_H
#define MGA_BUFFERS_H

void mgaDDSetReadBuffer(GLcontext *ctx, GLframebuffer *buffer,
			GLenum mode );

GLboolean mgaDDSetDrawBuffer(GLcontext *ctx, GLenum mode );

void mgaUpdateRects( mgaContextPtr mmesa, GLuint buffers );

#endif
