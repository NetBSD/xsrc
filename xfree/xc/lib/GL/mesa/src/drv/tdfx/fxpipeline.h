/* $XFree86: $ */
#ifndef FXPIPELINE_H
#define FXPIPELINE_H


extern GLuint fxDDRegisterPipelineStages(struct gl_pipeline_stage *out,
                                         const struct gl_pipeline_stage *in,
                                         GLuint nr);

extern GLboolean fxDDBuildPrecalcPipeline(GLcontext * ctx);

extern void fxDDOptimizePrecalcPipeline(GLcontext * ctx,
                                        struct gl_pipeline *pipe);


#endif
