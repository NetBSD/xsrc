#ifndef _I810_PIPELINE_H
#define _I810_PIPELINE_H

extern GLuint i810DDRegisterPipelineStages( struct gl_pipeline_stage *out,
					   const struct gl_pipeline_stage *in,
					   GLuint nr );

extern GLboolean i810DDBuildPrecalcPipeline( GLcontext *ctx ); 

extern void i810DDFastPath( struct vertex_buffer *VB );
extern void i810DDFastPathInit( void );


#endif
