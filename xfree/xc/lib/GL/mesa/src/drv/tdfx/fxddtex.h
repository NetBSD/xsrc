/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxddtex.h,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
#ifndef FXDDTEX_H
#define FXDDTEX_H


#include "texutil.h"


extern void fxPrintTextureData(tfxTexInfo * ti);

extern void fxTexGetFormat(GLenum, GrTextureFormat_t *, GLint *,
                           MesaIntTexFormat *, GLint *, GLboolean);

extern void fxTexGetInfo(const GLcontext *, int, int, GrLOD_t *,
                         GrAspectRatio_t *,
                         float *, float *, int *, int *, int *, int *);

extern GLboolean fxDDTexImage2D(GLcontext * ctx, GLenum target, GLint level,
                                GLenum format, GLenum type,
                                const GLvoid * pixels,
                                const struct gl_pixelstore_attrib *packing,
                                struct gl_texture_object *texObj,
                                struct gl_texture_image *texImage,
                                GLboolean * retainInternalCopy);

extern GLboolean fxDDTexSubImage2D(GLcontext * ctx, GLenum target,
                                   GLint level, GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height,
                                   GLenum format, GLenum type,
                                   const GLvoid * pixels,
                                   const struct gl_pixelstore_attrib *packing,
                                   struct gl_texture_object *texObj,
                                   struct gl_texture_image *texImage);

extern GLboolean fxDDTestProxyTexImage(GLcontext *ctx, GLenum target,
                                       GLint level, GLint internalFormat,
                                       GLenum format, GLenum type,
                                       GLint width, GLint height,
                                       GLint depth, GLint border );

extern GLvoid *fxDDGetTexImage(GLcontext * ctx, GLenum target, GLint level,
                               const struct gl_texture_object *texObj,
                               GLenum * formatOut, GLenum * typeOut,
                               GLboolean * freeImageOut);

extern GLboolean fxDDCompressedTexImage2D( GLcontext *ctx, GLenum target,
                                           GLint level, GLsizei imageSize,
                                           const GLvoid *data,
                                           struct gl_texture_object *texObj,
                                           struct gl_texture_image *texImage,
                                           GLboolean *retainInternalCopy);
extern GLboolean fxDDCompressedTexSubImage2D( GLcontext *ctx, GLenum target,
                                              GLint level, GLint xoffset,
                                              GLint yoffset, GLsizei width,
                                              GLint height, GLenum format,
                                              GLsizei imageSize, const GLvoid *data,
                                              struct gl_texture_object *texObj,
                                              struct gl_texture_image *texImage );
extern void fxDDGetCompressedTexImage( GLcontext *ctx, GLenum target,
                                       GLint lod, void *image,
                                       const struct gl_texture_object *texObj,
                                       struct gl_texture_image *texImage );
extern GLint fxDDSpecificCompressedTexFormat(GLcontext *ctx,
                                             GLint      internalFormat,
                                             GLint      numDimensions,
                                             GLint     *levelp,
                                             GLsizei   *widthp,
                                             GLsizei   *heightp,
                                             GLsizei   *depthp,
                                             GLint     *borderp,
                                             GLenum    *formatp,
                                             GLenum    *typep);
extern GLint fxDDBaseCompressedTexFormat(GLcontext *ctx,
                                         GLint      internalFormat);

#define fxDDIsCompressedFormatMacro(internalFormat) \
    (((internalFormat) == GL_COMPRESSED_RGB_FXT1_3DFX) || \
     ((internalFormat) == GL_COMPRESSED_RGBA_FXT1_3DFX))
#define fxDDIsCompressedGlideFormatMacro(internalFormat) \
    ((internalFormat) == GR_TEXFMT_ARGB_CMP_FXT1)
extern GLboolean fxDDIsCompressedFormat(GLcontext *ctx, GLint internalFormat);

extern void fxDDTexEnv(GLcontext *, GLenum, GLenum, const GLfloat *);

extern void fxDDTexParam(GLcontext *, GLenum, struct gl_texture_object *,
                         GLenum, const GLfloat *);

extern void fxDDTexBind(GLcontext *, GLenum, struct gl_texture_object *);

extern void fxDDTexDel(GLcontext *, struct gl_texture_object *);

extern GLboolean fxDDIsTextureResident(GLcontext *ctx,
                                       struct gl_texture_object *t);

extern void fxDDTexPalette(GLcontext *, struct gl_texture_object *);

extern void fxDDTexUseGlbPalette(GLcontext *, GLboolean);

/*
 * Calculate the image size of a compressed texture.
 *
 * We return 0 if we can't calculate the size.
 */

extern GLsizei fxDDCompressedImageSize(GLcontext *ctx,
                                       GLenum internalFormat,
                                       GLuint numDimensions,
                                       GLuint width,
                                       GLuint height,
                                       GLuint depth);
                                      
/*
 * This is not in glext.h, since this is not an EXT or ARB
 * extension.  It probably shouldn't be here exactly, but
 * there's not an obvious good place for it.
 */
#ifndef GL_S3_s3tc
#define GL_RGB_S3TC                       0x83A0
#define GL_RGB4_S3TC                      0x83A1
#define GL_RGBA_S3TC                      0x83A2
#define GL_RGBA4_S3TC                     0x83A3
#endif
#endif
