/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxpipeline.h,v 1.1 2000/09/24 13:51:16 alanh Exp $ */
#ifndef FXPIPELINE_H
#define FXPIPELINE_H


extern GLuint fxDDRegisterPipelineStages(struct gl_pipeline_stage *out,
                                         const struct gl_pipeline_stage *in,
                                         GLuint nr);

extern GLboolean fxDDBuildPrecalcPipeline(GLcontext * ctx);

extern void fxDDOptimizePrecalcPipeline(GLcontext * ctx,
                                        struct gl_pipeline *pipe);


#endif
