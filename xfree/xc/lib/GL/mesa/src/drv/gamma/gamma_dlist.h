/*
 * Mesa 3-D graphics library
 * Version:  3.0
 * Copyright (C) 1995-1998  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_dlist.h,v 1.3 2001/02/12 01:11:25 tsi Exp $ */


#ifndef DLIST_H
#define DLIST_H

#include "types.h"

extern void gamma_init_lists( void );

extern void gamma_destroy_list( GLuint list );



extern void gl_CallList( GLuint list );

extern void gl_CallLists( 
                          GLsizei n, GLenum type, const GLvoid *lists );

extern void gl_DeleteLists( GLuint list, GLsizei range );

extern void gl_EndList( void );

extern GLuint gl_GenLists( GLsizei range );

extern GLboolean gl_IsList( GLuint list );

extern void gl_ListBase( GLuint base );

extern void gl_NewList( GLuint list, GLenum mode );

extern void gl_save_Accum( GLenum op, GLfloat value );

extern void gl_save_AlphaFunc( GLenum func, GLclampf ref );

extern void gl_save_ArrayElement( GLint i );

extern void gl_save_BlendFunc(
                               GLenum sfactor, GLenum dfactor );

extern void gl_save_Begin( GLenum mode );

extern void gl_save_BindTexture( 
                                 GLenum target, GLuint texture );

extern void gl_save_Bitmap( GLsizei width, GLsizei height,
			    GLfloat xorig, GLfloat yorig,
			    GLfloat xmove, GLfloat ymove,
                            const GLubyte *bitmap );

extern void gl_save_CallList( GLuint list );

extern void gl_save_CallLists(
                               GLsizei n, GLenum type, const GLvoid *lists );

extern void gl_save_Clear( GLbitfield mask );

extern void gl_save_ClearAccum( GLfloat red, GLfloat green,
			        GLfloat blue, GLfloat alpha );

extern void gl_save_ClearColor( GLclampf red, GLclampf green,
			        GLclampf blue, GLclampf alpha );

extern void gl_save_ClearDepth( GLclampd depth );

extern void gl_save_ClearIndex( GLfloat c );

extern void gl_save_ClearStencil( GLint s );

extern void gl_save_ClipPlane( 
                               GLenum plane, const GLdouble *equ );

extern void gl_save_Color3f( GLfloat r, GLfloat g, GLfloat b );

extern void gl_save_Color3fv( const GLfloat *c );

extern void gl_save_Color4f( GLfloat r, GLfloat g,
                             GLfloat b, GLfloat a );

extern void gl_save_Color4fv( const GLfloat *c );

extern void gl_save_Color4ub( GLubyte r, GLubyte g,
                              GLubyte b, GLubyte a );

extern void gl_save_Color4ubv( const GLubyte *c );

extern void gl_save_ColorMask( GLboolean red, GLboolean green,
			       GLboolean blue, GLboolean alpha );

extern void gl_save_ColorMaterial( GLenum face, GLenum mode );

extern void gl_save_CopyPixels( GLint x, GLint y,
				GLsizei width, GLsizei height, GLenum type );

extern void gl_save_CopyTexImage1D( 
                                    GLenum target, GLint level,
                                    GLenum internalformat,
                                    GLint x, GLint y, GLsizei width,
                                    GLint border );

extern void gl_save_CopyTexImage2D(
                                    GLenum target, GLint level,
                                    GLenum internalformat,
                                    GLint x, GLint y, GLsizei width,
                                    GLsizei height, GLint border );

extern void gl_save_CopyTexSubImage1D(
                                       GLenum target, GLint level,
                                       GLint xoffset, GLint x, GLint y,
                                       GLsizei width );

extern void gl_save_CopyTexSubImage2D( 
                                       GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLint x, GLint y,
                                       GLsizei width, GLint height );

extern void gl_save_CullFace( GLenum mode );

extern void gl_save_DepthFunc( GLenum func );

extern void gl_save_DepthMask( GLboolean mask );

extern void gl_save_DepthRange( 
                                GLclampd nearval, GLclampd farval );

extern void gl_save_Disable( GLenum cap );

extern void gl_save_DrawArrays( 
                                GLenum mode, GLint first, GLsizei count );

extern void gl_save_DrawBuffer( GLenum mode );

extern void gl_save_DrawElements( 
                                  GLenum mode, GLsizei count,
                                  GLenum type, const GLvoid *indices );

extern void gl_save_DrawPixels( GLsizei width, GLsizei height,
                             GLenum format, GLenum type,
                             const GLvoid *pixels );

extern void gl_save_EdgeFlag( GLboolean flag );

extern void gl_save_Enable( GLenum cap );

extern void gl_save_End( void );

extern void gl_save_EvalCoord1f( GLfloat u );

extern void gl_save_EvalCoord2f( GLfloat u, GLfloat v );

extern void gl_save_EvalMesh1( 
                               GLenum mode, GLint i1, GLint i2 );

extern void gl_save_EvalMesh2( GLenum mode, GLint i1, GLint i2,
			       GLint j1, GLint j2 );

extern void gl_save_EvalPoint1( GLint i );

extern void gl_save_EvalPoint2( GLint i, GLint j );

extern void gl_save_Fogfv( 
                           GLenum pname, const GLfloat *params );

extern void gl_save_FrontFace( GLenum mode );

extern void gl_save_Frustum( GLdouble left, GLdouble right,
                             GLdouble bottom, GLdouble top,
                             GLdouble nearval, GLdouble farval );

extern void gl_save_Hint( GLenum target, GLenum mode );

extern void gl_save_Indexf( GLfloat index );

extern void gl_save_Indexi( GLint index );

extern void gl_save_IndexMask( GLuint mask );

extern void gl_save_InitNames( void );

extern void gl_save_Lightfv( GLenum light, GLenum pname,
                             const GLfloat *params );

extern void gl_save_LightModelfv( GLenum pname,
                                const GLfloat *params );

extern void gl_save_LineWidth( GLfloat width );

extern void gl_save_LineStipple( GLint factor,
                                 GLushort pattern );

extern void gl_save_ListBase( GLuint base );

extern void gl_save_LoadIdentity( void );

extern void gl_save_LoadMatrixf( const GLfloat *m );

extern void gl_save_LoadName( GLuint name );

extern void gl_save_LogicOp( GLenum opcode );

extern void gl_save_Map1f( GLenum target,
                           GLfloat u1, GLfloat u2, GLint stride,
			   GLint order, const GLfloat *points);

extern void gl_save_Map2f( GLenum target,
			   GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
			   GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
			   const GLfloat *points);

extern void gl_save_MapGrid1f( GLint un,
                               GLfloat u1, GLfloat u2 );

extern void gl_save_MapGrid2f( 
                               GLint un, GLfloat u1, GLfloat u2,
                               GLint vn, GLfloat v1, GLfloat v2 );

extern void gl_save_Materialfv( GLenum face, GLenum pname,
                                const GLfloat *params );

extern void gl_save_MatrixMode( GLenum mode );

extern void gl_save_MultMatrixf( const GLfloat *m );

extern void gl_save_NewList( GLuint list, GLenum mode );

extern void gl_save_Normal3fv( const GLfloat n[3] );

extern void gl_save_Normal3f( 
                              GLfloat nx, GLfloat ny, GLfloat nz );

extern void gl_save_Ortho( GLdouble left, GLdouble right,
                           GLdouble bottom, GLdouble top,
                           GLdouble nearval, GLdouble farval );

extern void gl_save_PassThrough( GLfloat token );

extern void gl_save_PixelMapfv( GLenum map, GLint mapsize,
                                const GLfloat *values );

extern void gl_save_PixelTransferf( 
                                    GLenum pname, GLfloat param );

extern void gl_save_PixelZoom(
                               GLfloat xfactor, GLfloat yfactor );

extern void gl_save_PointParameterfvEXT( GLenum pname,
                                         const GLfloat *params );

extern void gl_save_PointSize( GLfloat size );

extern void gl_save_PolygonMode( GLenum face, GLenum mode );

extern void gl_save_PolygonStipple( const GLubyte *pattern );

extern void gl_save_PolygonOffset( 
                                   GLfloat factor, GLfloat units );

extern void gl_save_PopAttrib( void );

extern void gl_save_PopMatrix( void );

extern void gl_save_PopName( void );

extern void gl_save_PrioritizeTextures( 
                                        GLsizei n, const GLuint *textures,
                                        const GLclampf *priorities );

extern void gl_save_PushAttrib( GLbitfield mask );

extern void gl_save_PushMatrix( void );

extern void gl_save_PushName( GLuint name );

extern void gl_save_RasterPos4f( 
                                 GLfloat x, GLfloat y, GLfloat z, GLfloat w );

extern void gl_save_ReadBuffer( GLenum mode );

extern void gl_save_Rectf(
                           GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );

extern void gl_save_Rotatef( GLfloat angle,
                             GLfloat x, GLfloat y, GLfloat z );

extern void gl_save_Scalef( GLfloat x, GLfloat y, GLfloat z );

extern void gl_save_Scissor( 
                             GLint x, GLint y, GLsizei width, GLsizei height );

extern void gl_save_ShadeModel( GLenum mode );

extern void gl_save_StencilFunc(
                                 GLenum func, GLint ref, GLuint mask );

extern void gl_save_StencilMask( GLuint mask );

extern void gl_save_StencilOp( 
                               GLenum fail, GLenum zfail, GLenum zpass );

extern void gl_save_TexCoord2f(  GLfloat s, GLfloat t );

extern void gl_save_TexCoord2fv( const GLfloat *v );

extern void gl_save_TexCoord3fv( const GLfloat *v );

extern void gl_save_TexCoord4f(  GLfloat s, GLfloat t,
                                GLfloat r, GLfloat q );

extern void gl_save_TexEnvfv( GLenum target, GLenum pname,
                              const GLfloat *params );

extern void gl_save_TexParameterfv( GLenum target,
                                    GLenum pname, const GLfloat *params );

extern void gl_save_TexGenfv( GLenum coord, GLenum pname,
                              const GLfloat *params );

extern void gl_save_TexImage1D( GLenum target,
                                GLint level, GLint components,
			        GLsizei width, GLint border,
                                GLenum format, GLenum type,
                                const GLvoid *pixels );

extern void gl_save_TexImage2D( GLenum target,
                                GLint level, GLint components,
			        GLsizei width, GLsizei height, GLint border,
                                GLenum format, GLenum type,
                                const GLvoid *pixels );

extern void gl_save_TexSubImage1D(
                                   GLenum target, GLint level,
                                   GLint xoffset, GLsizei width,
                                   GLenum format, GLenum type,
                                   const GLvoid *pixels );


extern void gl_save_TexSubImage2D(
                                   GLenum target, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLsizei width, GLsizei height,
                                   GLenum format, GLenum type,
                                   const GLvoid *pixels );

extern void gl_save_Translatef( 
                                GLfloat x, GLfloat y, GLfloat z );

extern void gl_save_Vertex2f( 
                              GLfloat x, GLfloat y );

extern void gl_save_Vertex3f(
                              GLfloat x, GLfloat y, GLfloat z );

extern void gl_save_Vertex4f( 
                              GLfloat x, GLfloat y, GLfloat z, GLfloat w );

extern void gl_save_Vertex3fv( const GLfloat *v );

extern void gl_save_Viewport( GLint x, GLint y,
                              GLsizei width, GLsizei height );

#endif
