/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_pipeline.h,v 1.3 2000/12/04 19:21:46 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef _R128_PIPELINE_H_
#define _R128_PIPELINE_H_

extern GLboolean r128DDBuildPrecalcPipeline( GLcontext *ctx );
extern GLuint r128DDRegisterPipelineStages( struct gl_pipeline_stage *out,
					    const struct gl_pipeline_stage *in,
					    GLuint nr );

extern void r128DDFastPathInit( void );
extern void r128DDFastPath( struct vertex_buffer *VB );

extern void r128DDEltPathInit( void );
extern void r128DDEltPath( struct vertex_buffer *VB );

#endif /* _R128_PIPELINE_H_ */
