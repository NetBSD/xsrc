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
