/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_gl.h,v 1.5 2001/02/07 13:26:16 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian Paul <brian@precisioninsight.com>
 */

#ifndef _GAMMA_GL_H_
#define _GAMMA_GL_H_

#include "GL/gl.h"
#include "glapi.h"

extern void gamma_error(GLenum error, const char *s);

extern void _gamma_Accum(GLenum op, GLfloat value);
extern void _gamma_AlphaFunc(GLenum func, GLclampf ref);
extern GLboolean _gamma_AreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);
extern void _gamma_ArrayElement(GLint i);
extern void _gamma_Begin(GLenum mode);
extern void _gamma_BindTexture(GLenum target, GLuint texture);
extern void _gamma_Bitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void _gamma_BlendFunc(GLenum sfactor, GLenum dfactor);
extern void _gamma_CallList(GLuint list);
extern void _gamma_CallLists(GLsizei n, GLenum type, const GLvoid *lists);
extern void _gamma_Clear(GLbitfield mask);
extern void _gamma_ClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void _gamma_ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void _gamma_ClearDepth(GLclampd depth);
extern void _gamma_ClearIndex(GLfloat c);
extern void _gamma_ClearStencil(GLint s);
extern void _gamma_ClipPlane(GLenum plane, const GLdouble *equation);
extern void _gamma_Color3b(GLbyte red, GLbyte green, GLbyte blue);
extern void _gamma_Color3bv(const GLbyte *v);
extern void _gamma_Color3d(GLdouble red, GLdouble green, GLdouble blue);
extern void _gamma_Color3dv(const GLdouble *v);
extern void _gamma_Color3f(GLfloat red, GLfloat green, GLfloat blue);
extern void _gamma_Color3fv(const GLfloat *v);
extern void _gamma_Color3i(GLint red, GLint green, GLint blue);
extern void _gamma_Color3iv(const GLint *v);
extern void _gamma_Color3s(GLshort red, GLshort green, GLshort blue);
extern void _gamma_Color3sv(const GLshort *v);
extern void _gamma_Color3ub(GLubyte red, GLubyte green, GLubyte blue);
extern void _gamma_Color3ubv(const GLubyte *v);
extern void _gamma_Color3ui(GLuint red, GLuint green, GLuint blue);
extern void _gamma_Color3uiv(const GLuint *v);
extern void _gamma_Color3us(GLushort red, GLushort green, GLushort blue);
extern void _gamma_Color3usv(const GLushort *v);
extern void _gamma_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void _gamma_Color4bv(const GLbyte *v);
extern void _gamma_Color4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void _gamma_Color4dv(const GLdouble *v);
extern void _gamma_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void _gamma_Color4fv(const GLfloat *v);
extern void _gamma_Color4i(GLint red, GLint green, GLint blue, GLint alpha);
extern void _gamma_Color4iv(const GLint *v);
extern void _gamma_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void _gamma_Color4sv(const GLshort *v);
extern void _gamma_Color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void _gamma_Color4ubv(const GLubyte *v);
extern void _gamma_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void _gamma_Color4uiv(const GLuint *v);
extern void _gamma_Color4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void _gamma_Color4usv(const GLushort *v);
extern void _gamma_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void _gamma_ColorMaterial(GLenum face, GLenum mode);
extern void _gamma_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void _gamma_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void _gamma_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void _gamma_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void _gamma_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void _gamma_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void _gamma_CullFace(GLenum mode);
extern void _gamma_DeleteLists(GLuint list, GLsizei range);
extern void _gamma_DeleteTextures(GLsizei n, const GLuint *textures);
extern void _gamma_DepthFunc(GLenum func);
extern void _gamma_DepthMask(GLboolean flag);
extern void _gamma_DepthRange(GLclampd zNear, GLclampd zFar);
extern void _gamma_Disable(GLenum cap);
extern void _gamma_DisableClientState(GLenum array);
extern void _gamma_DrawArrays(GLenum mode, GLint first, GLsizei count);
extern void _gamma_DrawBuffer(GLenum mode);
extern void _gamma_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern void _gamma_DrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
extern void _gamma_EdgeFlag(GLboolean flag);
extern void _gamma_EdgeFlagPointer(GLsizei stride, const GLvoid *pointer);
extern void _gamma_EdgeFlagv(const GLboolean *flag);
extern void _gamma_Enable(GLenum cap);
extern void _gamma_EnableClientState(GLenum array);
extern void _gamma_End(void);
extern void _gamma_EndList(void);
extern void _gamma_EvalCoord1d(GLdouble u);
extern void _gamma_EvalCoord1dv(const GLdouble *u);
extern void _gamma_EvalCoord1f(GLfloat u);
extern void _gamma_EvalCoord1fv(const GLfloat *u);
extern void _gamma_EvalCoord2d(GLdouble u, GLdouble v);
extern void _gamma_EvalCoord2dv(const GLdouble *u);
extern void _gamma_EvalCoord2f(GLfloat u, GLfloat v);
extern void _gamma_EvalCoord2fv(const GLfloat *u);
extern void _gamma_EvalMesh1(GLenum mode, GLint i1, GLint i2);
extern void _gamma_EvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void _gamma_EvalPoint1(GLint i);
extern void _gamma_EvalPoint2(GLint i, GLint j);
extern void _gamma_FeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
extern void _gamma_Finish(void);
extern void _gamma_Flush(void);
extern void _gamma_Fogf(GLenum pname, GLfloat param);
extern void _gamma_Fogfv(GLenum pname, const GLfloat *params);
extern void _gamma_Fogi(GLenum pname, GLint param);
extern void _gamma_Fogiv(GLenum pname, const GLint *params);
extern void _gamma_FrontFace(GLenum mode);
extern void _gamma_Frustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern GLuint _gamma_GenLists(GLsizei range);
extern void _gamma_GenTextures(GLsizei n, GLuint *textures);
extern void _gamma_GetBooleanv(GLenum val, GLboolean *b);
extern void _gamma_GetClipPlane(GLenum plane, GLdouble *equation);
extern void _gamma_GetDoublev(GLenum val, GLdouble *d);
extern GLenum _gamma_GetError(void);
extern void _gamma_GetFloatv(GLenum val, GLfloat *f);
extern void _gamma_GetIntegerv(GLenum val, GLint *i);
extern void _gamma_GetLightfv(GLenum light, GLenum pname, GLfloat *params);
extern void _gamma_GetLightiv(GLenum light, GLenum pname, GLint *params);
extern void _gamma_GetMapdv(GLenum target, GLenum query, GLdouble *v);
extern void _gamma_GetMapfv(GLenum target, GLenum query, GLfloat *v);
extern void _gamma_GetMapiv(GLenum target, GLenum query, GLint *v);
extern void _gamma_GetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
extern void _gamma_GetMaterialiv(GLenum face, GLenum pname, GLint *params);
extern void _gamma_GetPixelMapfv(GLenum map, GLfloat *values);
extern void _gamma_GetPixelMapuiv(GLenum map, GLuint *values);
extern void _gamma_GetPixelMapusv(GLenum map, GLushort *values);
extern void _gamma_GetPointerv(GLenum pname, void **params);
extern void _gamma_GetPolygonStipple(GLubyte *mask);
extern const GLubyte *_gamma_GetString(GLenum name);
extern void _gamma_GetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
extern void _gamma_GetTexEnviv(GLenum target, GLenum pname, GLint *params);
extern void _gamma_GetTexGendv(GLenum coord, GLenum pname, GLdouble *params);
extern void _gamma_GetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
extern void _gamma_GetTexGeniv(GLenum coord, GLenum pname, GLint *params);
extern void _gamma_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *texels);
extern void _gamma_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void _gamma_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
extern void _gamma_GetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
extern void _gamma_GetTexParameteriv(GLenum target, GLenum pname, GLint *params);
extern void _gamma_Hint(GLenum target, GLenum mode);
extern void _gamma_IndexMask(GLuint mask);
extern void _gamma_IndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
extern void _gamma_Indexd(GLdouble c);
extern void _gamma_Indexdv(const GLdouble *c);
extern void _gamma_Indexf(GLfloat c);
extern void _gamma_Indexfv(const GLfloat *c);
extern void _gamma_Indexi(GLint c);
extern void _gamma_Indexiv(const GLint *c);
extern void _gamma_Indexs(GLshort c);
extern void _gamma_Indexsv(const GLshort *c);
extern void _gamma_Indexub(GLubyte c);
extern void _gamma_Indexubv(const GLubyte *c);
extern void _gamma_InitNames(void);
extern void _gamma_InterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
extern GLboolean _gamma_IsEnabled(GLenum cap);
extern GLboolean _gamma_IsList(GLuint list);
extern GLboolean _gamma_IsTexture(GLuint texture);
extern void _gamma_LightModelf(GLenum pname, GLfloat param);
extern void _gamma_LightModelfv(GLenum pname, const GLfloat *params);
extern void _gamma_LightModeli(GLenum pname, GLint param);
extern void _gamma_LightModeliv(GLenum pname, const GLint *params);
extern void _gamma_Lightf(GLenum light, GLenum pname, GLfloat param);
extern void _gamma_Lightfv(GLenum light, GLenum pname, const GLfloat *params);
extern void _gamma_Lighti(GLenum light, GLenum pname, GLint param);
extern void _gamma_Lightiv(GLenum light, GLenum pname, const GLint *params);
extern void _gamma_LineStipple(GLint factor, GLushort pattern);
extern void _gamma_LineWidth(GLfloat width);
extern void _gamma_ListBase(GLuint base);
extern void _gamma_LoadIdentity(void);
extern void _gamma_LoadMatrixd(const GLdouble *m);
extern void _gamma_LoadMatrixf(const GLfloat *m);
extern void _gamma_LoadName(GLuint name);
extern void _gamma_LogicOp(GLenum opcode);
extern void _gamma_Map1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *pnts);
extern void _gamma_Map1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *pnts);
extern void _gamma_Map2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustr, GLint uord, GLdouble v1, GLdouble v2, GLint vstr, GLint vord, const GLdouble *pnts);
extern void _gamma_Map2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustr, GLint uord, GLfloat v1, GLfloat v2, GLint vstr, GLint vord, const GLfloat *pnts);
extern void _gamma_MapGrid1d(GLint un, GLdouble u1, GLdouble u2);
extern void _gamma_MapGrid1f(GLint un, GLfloat u1, GLfloat u2);
extern void _gamma_MapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void _gamma_MapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void _gamma_Materialf(GLenum face, GLenum pname, GLfloat param);
extern void _gamma_Materialfv(GLenum face, GLenum pname, const GLfloat *params);
extern void _gamma_Materiali(GLenum face, GLenum pname, GLint param);
extern void _gamma_Materialiv(GLenum face, GLenum pname, const GLint *params);
extern void _gamma_MatrixMode(GLenum mode);
extern void _gamma_MultMatrixd(const GLdouble *m);
extern void _gamma_MultMatrixf(const GLfloat *m);
extern void _gamma_NewList(GLuint list, GLenum mode);
extern void _gamma_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz);
extern void _gamma_Normal3bv(const GLbyte *v);
extern void _gamma_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz);
extern void _gamma_Normal3dv(const GLdouble *v);
extern void _gamma_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz);
extern void _gamma_Normal3fv(const GLfloat *v);
extern void _gamma_Normal3i(GLint nx, GLint ny, GLint nz);
extern void _gamma_Normal3iv(const GLint *v);
extern void _gamma_Normal3s(GLshort nx, GLshort ny, GLshort nz);
extern void _gamma_Normal3sv(const GLshort *v);
extern void _gamma_NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
extern void _gamma_Ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void _gamma_PassThrough(GLfloat token);
extern void _gamma_PixelMapfv(GLenum map, GLint mapsize, const GLfloat *values);
extern void _gamma_PixelMapuiv(GLenum map, GLint mapsize, const GLuint *values);
extern void _gamma_PixelMapusv(GLenum map, GLint mapsize, const GLushort *values);
extern void _gamma_PixelStoref(GLenum pname, GLfloat param);
extern void _gamma_PixelStorei(GLenum pname, GLint param);
extern void _gamma_PixelTransferf(GLenum pname, GLfloat param);
extern void _gamma_PixelTransferi(GLenum pname, GLint param);
extern void _gamma_PixelZoom(GLfloat xfactor, GLfloat yfactor);
extern void _gamma_PointSize(GLfloat size);
extern void _gamma_PolygonMode(GLenum face, GLenum mode);
extern void _gamma_PolygonOffset(GLfloat factor, GLfloat units);
extern void _gamma_PolygonStipple(const GLubyte *mask);
extern void _gamma_PopAttrib(void);
extern void _gamma_PopClientAttrib(void);
extern void _gamma_PopMatrix(void);
extern void _gamma_PopName(void);
extern void _gamma_PrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities);
extern void _gamma_PushAttrib(GLbitfield mask);
extern void _gamma_PushClientAttrib(GLuint mask);
extern void _gamma_PushMatrix(void);
extern void _gamma_PushName(GLuint name);
extern void _gamma_RasterPos2d(GLdouble x, GLdouble y);
extern void _gamma_RasterPos2dv(const GLdouble *v);
extern void _gamma_RasterPos2f(GLfloat x, GLfloat y);
extern void _gamma_RasterPos2fv(const GLfloat *v);
extern void _gamma_RasterPos2i(GLint x, GLint y);
extern void _gamma_RasterPos2iv(const GLint *v);
extern void _gamma_RasterPos2s(GLshort x, GLshort y);
extern void _gamma_RasterPos2sv(const GLshort *v);
extern void _gamma_RasterPos3d(GLdouble x, GLdouble y, GLdouble z);
extern void _gamma_RasterPos3dv(const GLdouble *v);
extern void _gamma_RasterPos3f(GLfloat x, GLfloat y, GLfloat z);
extern void _gamma_RasterPos3fv(const GLfloat *v);
extern void _gamma_RasterPos3i(GLint x, GLint y, GLint z);
extern void _gamma_RasterPos3iv(const GLint *v);
extern void _gamma_RasterPos3s(GLshort x, GLshort y, GLshort z);
extern void _gamma_RasterPos3sv(const GLshort *v);
extern void _gamma_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void _gamma_RasterPos4dv(const GLdouble *v);
extern void _gamma_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void _gamma_RasterPos4fv(const GLfloat *v);
extern void _gamma_RasterPos4i(GLint x, GLint y, GLint z, GLint w);
extern void _gamma_RasterPos4iv(const GLint *v);
extern void _gamma_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
extern void _gamma_RasterPos4sv(const GLshort *v);
extern void _gamma_ReadBuffer(GLenum mode);
extern void _gamma_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern void _gamma_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void _gamma_Rectdv(const GLdouble *v1, const GLdouble *v2);
extern void _gamma_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void _gamma_Rectfv(const GLfloat *v1, const GLfloat *v2);
extern void _gamma_Recti(GLint x1, GLint y1, GLint x2, GLint y2);
extern void _gamma_Rectiv(const GLint *v1, const GLint *v2);
extern void _gamma_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void _gamma_Rectsv(const GLshort *v1, const GLshort *v2);
extern GLint _gamma_RenderMode(GLenum mode);
extern void _gamma_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void _gamma_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void _gamma_Scaled(GLdouble x, GLdouble y, GLdouble z);
extern void _gamma_Scalef(GLfloat x, GLfloat y, GLfloat z);
extern void _gamma_Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
extern void _gamma_SelectBuffer(GLsizei numnames, GLuint *buffer);
extern void _gamma_ShadeModel(GLenum mode);
extern void _gamma_StencilFunc(GLenum func, GLint ref, GLuint mask);
extern void _gamma_StencilMask(GLuint mask);
extern void _gamma_StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
extern void _gamma_TexCoord1d(GLdouble s);
extern void _gamma_TexCoord1dv(const GLdouble *v);
extern void _gamma_TexCoord1f(GLfloat s);
extern void _gamma_TexCoord1fv(const GLfloat *v);
extern void _gamma_TexCoord1i(GLint s);
extern void _gamma_TexCoord1iv(const GLint *v);
extern void _gamma_TexCoord1s(GLshort s);
extern void _gamma_TexCoord1sv(const GLshort *v);
extern void _gamma_TexCoord2d(GLdouble s, GLdouble t);
extern void _gamma_TexCoord2dv(const GLdouble *v);
extern void _gamma_TexCoord2f(GLfloat s, GLfloat t);
extern void _gamma_TexCoord2fv(const GLfloat *v);
extern void _gamma_TexCoord2i(GLint s, GLint t);
extern void _gamma_TexCoord2iv(const GLint *v);
extern void _gamma_TexCoord2s(GLshort s, GLshort t);
extern void _gamma_TexCoord2sv(const GLshort *v);
extern void _gamma_TexCoord3d(GLdouble s, GLdouble t, GLdouble r);
extern void _gamma_TexCoord3dv(const GLdouble *v);
extern void _gamma_TexCoord3f(GLfloat s, GLfloat t, GLfloat r);
extern void _gamma_TexCoord3fv(const GLfloat *v);
extern void _gamma_TexCoord3i(GLint s, GLint t, GLint r);
extern void _gamma_TexCoord3iv(const GLint *v);
extern void _gamma_TexCoord3s(GLshort s, GLshort t, GLshort r);
extern void _gamma_TexCoord3sv(const GLshort *v);
extern void _gamma_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void _gamma_TexCoord4dv(const GLdouble *v);
extern void _gamma_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void _gamma_TexCoord4fv(const GLfloat *v);
extern void _gamma_TexCoord4i(GLint s, GLint t, GLint r, GLint q);
extern void _gamma_TexCoord4iv(const GLint *v);
extern void _gamma_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
extern void _gamma_TexCoord4sv(const GLshort *v);
extern void _gamma_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void _gamma_TexEnvf(GLenum target, GLenum pname, GLfloat param);
extern void _gamma_TexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
extern void _gamma_TexEnvi(GLenum target, GLenum pname, GLint param);
extern void _gamma_TexEnviv(GLenum target, GLenum pname, const GLint *params);
extern void _gamma_TexGend(GLenum coord, GLenum pname, GLdouble param);
extern void _gamma_TexGendv(GLenum coord, GLenum pname, const GLdouble *params);
extern void _gamma_TexGenf(GLenum coord, GLenum pname, GLfloat param);
extern void _gamma_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
extern void _gamma_TexGeni(GLenum coord, GLenum pname, GLint param);
extern void _gamma_TexGeniv(GLenum coord, GLenum pname, const GLint *params);
extern void _gamma_TexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *image);
extern void _gamma_TexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *image);
extern void _gamma_TexParameterf(GLenum target, GLenum pname, GLfloat param);
extern void _gamma_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
extern void _gamma_TexParameteri(GLenum target, GLenum pname, GLint param);
extern void _gamma_TexParameteriv(GLenum target, GLenum pname, const GLint *params);
extern void _gamma_TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
extern void _gamma_TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
extern void _gamma_Translated(GLdouble x, GLdouble y, GLdouble z);
extern void _gamma_Translatef(GLfloat x, GLfloat y, GLfloat z);
extern void _gamma_Vertex2d(GLdouble x, GLdouble y);
extern void _gamma_Vertex2dv(const GLdouble *v);
extern void _gamma_Vertex2f(GLfloat x, GLfloat y);
extern void _gamma_Vertex2fv(const GLfloat *v);
extern void _gamma_Vertex2i(GLint x, GLint y);
extern void _gamma_Vertex2iv(const GLint *v);
extern void _gamma_Vertex2s(GLshort x, GLshort y);
extern void _gamma_Vertex2sv(const GLshort *v);
extern void _gamma_Vertex3d(GLdouble x, GLdouble y, GLdouble z);
extern void _gamma_Vertex3dv(const GLdouble *v);
extern void _gamma_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
extern void _gamma_Vertex3fv(const GLfloat *v);
extern void _gamma_Vertex3i(GLint x, GLint y, GLint z);
extern void _gamma_Vertex3iv(const GLint *v);
extern void _gamma_Vertex3s(GLshort x, GLshort y, GLshort z);
extern void _gamma_Vertex3sv(const GLshort *v);
extern void _gamma_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void _gamma_Vertex4dv(const GLdouble *v);
extern void _gamma_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void _gamma_Vertex4fv(const GLfloat *v);
extern void _gamma_Vertex4i(GLint x, GLint y, GLint z, GLint w);
extern void _gamma_Vertex4iv(const GLint *v);
extern void _gamma_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
extern void _gamma_Vertex4sv(const GLshort *v);
extern void _gamma_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void _gamma_Viewport(GLint x, GLint y, GLsizei width, GLsizei height);


extern void _gamma_init_exec(struct _glapi_table *dispatch);
extern void _gamma_init_save(struct _glapi_table *dispatch);

#endif /* _GAMMA_GL_H_ */
