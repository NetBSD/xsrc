/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxsetup.h,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
#ifndef FXSETUP_H
#define FXSETUP_H

extern void fxTexValidate(GLcontext * ctx, struct gl_texture_object *tObj);

extern void fxDDEnable(GLcontext *, GLenum, GLboolean);

extern void fxDDAlphaFunc(GLcontext *, GLenum, GLclampf);

extern void fxDDBlendFunc(GLcontext *, GLenum, GLenum);

extern void fxDDBlendFuncSeparate(GLcontext *ctx,
                                  GLenum sfactorRGB, GLenum sfactorA,
                                  GLenum dfactorRGB, GLenum dfactorA);

extern GrStencil_t fxConvertGLStencilOp(GLenum op);

extern void fxDDScissor(GLcontext *ctx,
                        GLint x, GLint y, GLsizei w, GLsizei h);

extern void fxDDFogfv(GLcontext *ctx, GLenum pname, const GLfloat * params);

extern GLboolean fxDDColorMask(GLcontext *ctx,
                               GLboolean r, GLboolean g,
                               GLboolean b, GLboolean a);

extern void fxDDShadeModel(GLcontext * ctx, GLenum mode);

extern void fxDDCullFace(GLcontext * ctx, GLenum mode);

extern void fxDDFrontFace(GLcontext * ctx, GLenum mode);

extern void fxSetupFXUnits(GLcontext *);

#endif
