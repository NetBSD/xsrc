#ifndef _MGA_STATE_H
#define _MGA_STATE_H


extern void mgaDDInitStateFuncs(GLcontext *ctx);
extern void mgaDDUpdateHwState( GLcontext *ctx );
extern void mgaDDUpdateState( GLcontext *ctx );
extern void mgaDDReducedPrimitiveChange( GLcontext *ctx, GLenum prim );

extern void mgaInitState( mgaContextPtr mmesa );

extern void mgaUpdateClipping(const GLcontext *ctx);



#endif
