/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxtexman.h,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
#ifndef FXTEXMAN_H
#define FXTEXMAN_H


#define fxTMGetTexInfo(o) ((tfxTexInfo*)((o)->DriverData))

extern void fxTMReloadMipMapLevel(GLcontext *, struct gl_texture_object *,
                                  GLint);
extern void fxTMReloadSubMipMapLevel(GLcontext *,
                                     struct gl_texture_object *, GLint, GLint,
                                     GLint);

extern void fxTMInit(fxMesaContext ctx);

extern void fxTMClose(fxMesaContext ctx);

extern void fxTMRestoreTextures_NoLock(fxMesaContext ctx);

extern void fxTMMoveInTM(fxMesaContext, struct gl_texture_object *, FxU32);

extern void fxTMMoveOutTM(fxMesaContext, struct gl_texture_object *);

extern void fxTMMoveOutTM_NoLock(fxMesaContext, struct gl_texture_object *);

extern void fxTMFreeTexture(fxMesaContext, struct gl_texture_object *);


#endif
