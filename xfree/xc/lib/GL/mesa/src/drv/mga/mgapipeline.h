/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgapipeline.h,v 1.3 2001/04/10 16:07:50 dawes Exp $ */
/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */

#ifndef _MGA_PIPELINE_H
#define _MGA_PIPELINE_H


extern GLuint mgaDDRegisterPipelineStages( struct gl_pipeline_stage *out,
					   const struct gl_pipeline_stage *in,
					   GLuint nr );

extern GLboolean mgaDDBuildImmediatePipeline( GLcontext *ctx );
extern GLboolean mgaDDBuildPrecalcPipeline( GLcontext *ctx );

extern void mgaDDFastPath( struct vertex_buffer *VB );
extern void mgaDDFastPathInit( void );

extern void mgaDDEltPath( struct vertex_buffer *VB );
extern void mgaDDEltPathInit( void );

#endif
