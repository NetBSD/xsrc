/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgabuffers.h,v 1.3 2000/08/25 13:42:23 dawes Exp $ */

#ifndef MGA_BUFFERS_H
#define MGA_BUFFERS_H

void mgaDDSetReadBuffer(GLcontext *ctx, GLframebuffer *buffer,
			GLenum mode );

GLboolean mgaDDSetDrawBuffer(GLcontext *ctx, GLenum mode );

void mgaUpdateRects( mgaContextPtr mmesa, GLuint buffers );

#endif
